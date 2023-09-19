// Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.

package main

import (
	"fmt"
	"log"
	"net"
	"os"
	"path"
	"strconv"
	"strings"
	"time"

	"github.com/google/uuid"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	pluginapi "k8s.io/kubernetes/pkg/kubelet/apis/deviceplugin/v1alpha1"
)

const (
	resourceName = "nvidia.com/gpu"
	serverSock   = pluginapi.DevicePluginPath + "nvidia.sock"
)

// Constants to represent the various device list strategies
const (
	DeviceListStrategyEnvvar       = "envvar"
	DeviceListStrategyVolumeMounts = "volume-mounts"
)

// Constants to represent the various device id strategies
const (
	DeviceIDStrategyUUID  = "uuid"
	DeviceIDStrategyIndex = "index"
)

// Constants for use by the 'volume-mounts' device list strategy
const (
	deviceListAsVolumeMountsHostPath          = "/dev/null"
	deviceListAsVolumeMountsContainerPathRoot = "/var/run/nvidia-container-devices"
)

// NvidiaDevicePlugin implements the Kubernetes device plugin API
type NvidiaDevicePlugin struct {
	devs   []*pluginapi.Device
	vdevs  []*VDevice
	socket string

	stop   chan interface{}
	health chan *pluginapi.Device

	server *grpc.Server
}

// NewNvidiaDevicePlugin returns an initialized NvidiaDevicePlugin
func NewNvidiaDevicePlugin() *NvidiaDevicePlugin {
	return &NvidiaDevicePlugin{
		devs:   getDevices(),
		socket: serverSock,

		stop:   make(chan interface{}),
		health: make(chan *pluginapi.Device),
	}
}

// dial establishes the gRPC communication with the registered device plugin.
func dial(unixSocketPath string, timeout time.Duration) (*grpc.ClientConn, error) {
	c, err := grpc.Dial(unixSocketPath, grpc.WithInsecure(), grpc.WithBlock(),
		grpc.WithTimeout(timeout),
		grpc.WithDialer(func(addr string, timeout time.Duration) (net.Conn, error) {
			return net.DialTimeout("unix", addr, timeout)
		}),
	)

	if err != nil {
		return nil, err
	}

	return c, nil
}

// Start starts the gRPC server of the device plugin
func (m *NvidiaDevicePlugin) Start() error {
	err := m.cleanup()
	if err != nil {
		return err
	}

	fmt.Println("m.devs=", m.devs)
	m.vdevs = Device2VDevice(m.devs)
	fmt.Println("m.vdevs=", m.vdevs)

	sock, err := net.Listen("unix", m.socket)
	if err != nil {
		return err
	}

	m.server = grpc.NewServer([]grpc.ServerOption{}...)
	pluginapi.RegisterDevicePluginServer(m.server, m)

	go m.server.Serve(sock)

	// Wait for server to start by launching a blocking connexion
	conn, err := dial(m.socket, 5*time.Second)
	if err != nil {
		return err
	}
	conn.Close()

	go m.healthcheck()

	return nil
}

// Stop stops the gRPC server
func (m *NvidiaDevicePlugin) Stop() error {
	if m.server == nil {
		return nil
	}

	m.server.Stop()
	m.server = nil
	close(m.stop)

	return m.cleanup()
}

// Register registers the device plugin for the given resourceName with Kubelet.
func (m *NvidiaDevicePlugin) Register(kubeletEndpoint, resourceName string) error {
	conn, err := dial(kubeletEndpoint, 5*time.Second)
	if err != nil {
		return err
	}
	defer conn.Close()

	client := pluginapi.NewRegistrationClient(conn)
	reqt := &pluginapi.RegisterRequest{
		Version:      pluginapi.Version,
		Endpoint:     path.Base(m.socket),
		ResourceName: resourceName,
	}

	_, err = client.Register(context.Background(), reqt)
	if err != nil {
		return err
	}
	return nil
}

func (m *NvidiaDevicePlugin) apiDevices() []*pluginapi.Device {
	var pdevs []*pluginapi.Device
	for _, d := range m.vdevs {
		pdevs = append(pdevs, &d.Device)
	}
	return pdevs
}

// ListAndWatch lists devices and update that list according to the health status
func (m *NvidiaDevicePlugin) ListAndWatch(e *pluginapi.Empty, s pluginapi.DevicePlugin_ListAndWatchServer) error {
	fmt.Println("ListAndWatch", m.apiDevices())
	//s.Send(&pluginapi.ListAndWatchResponse{Devices: m.devs})
	s.Send(&pluginapi.ListAndWatchResponse{Devices: m.apiDevices()})

	for {
		select {
		case <-m.stop:
			return nil
		case d := <-m.health:
			// FIXME: there is no way to recover from the Unhealthy state.
			d.Health = pluginapi.Unhealthy
			s.Send(&pluginapi.ListAndWatchResponse{Devices: m.devs})
		}
	}
}

func (m *NvidiaDevicePlugin) unhealthy(dev *pluginapi.Device) {
	m.health <- dev
}

// Allocate which return list of devices.
func (m *NvidiaDevicePlugin) Allocate(ctx context.Context, r *pluginapi.AllocateRequest) (*pluginapi.AllocateResponse, error) {
	fmt.Println("Allocate", m.devs)
	devs := m.devs
	var response pluginapi.AllocateResponse

	vdevices, err := VDevicesByIDs(m.vdevs, r.DevicesIDs)
	if err != nil {
		return nil, err
	}
	uuids := UniqueDeviceIDs(vdevices)
	deviceIDs := uuids

	for i, id := range deviceIDs {
		if !deviceExists(devs, id) {
			return nil, fmt.Errorf("invalid allocation request: unknown device: %s", id)
		}

		devRuntime := new(pluginapi.DeviceRuntimeSpec)
		devRuntime.ID = id
		// Only add env vars to the first device.
		// Will be fixed by: https://github.com/kubernetes/kubernetes/pull/53031
		if i == 0 {
			var mapEnvs []string
			devRuntime.Envs = make(map[string]string)
			devRuntime.Mounts = []*pluginapi.Mount{}
			for i, vd := range vdevices {
				limitKey := fmt.Sprintf("CUDA_DEVICE_MEMORY_LIMIT_%v", i)
				devRuntime.Envs[limitKey] = fmt.Sprintf("%vm", vd.memory)
				mapEnvs = append(mapEnvs, fmt.Sprintf("%v:%v", i, vd.dev.UUID))
			}
			devRuntime.Envs["CUDA_DEVICE_SM_LIMIT"] = strconv.Itoa(int(100 * deviceCoresScalingFlag / float64(deviceSplitCountFlag)))
			devRuntime.Envs["NVIDIA_DEVICE_MAP"] = strings.Join(mapEnvs, " ")
			devRuntime.Envs["CUDA_DEVICE_MEMORY_SHARED_CACHE"] = fmt.Sprintf("/tmp/%v.cache", uuid.NewString())
			if deviceMemoryScalingFlag > 1 {
				devRuntime.Envs["CUDA_OVERSUBSCRIBE"] = "true"
			}
			devRuntime.Envs["NVIDIA_VISIBLE_DEVICES"] = strings.Join(deviceIDs, ",")
			devRuntime.Mounts = append(devRuntime.Mounts,
				&pluginapi.Mount{ContainerPath: "/usr/local/vgpu/libvgpu.so",
					HostPath: "/usr/local/vgpu/libvgpu.so", ReadOnly: true},
				&pluginapi.Mount{ContainerPath: "/etc/ld.so.preload",
					HostPath: "/usr/local/vgpu/ld.so.preload", ReadOnly: true},
				&pluginapi.Mount{ContainerPath: "/usr/local/vgpu/pciinfo.vgpu",
					HostPath: os.Getenv("PCIBUSFILE"), ReadOnly: true},
				&pluginapi.Mount{ContainerPath: "/usr/bin/vgpuvalidator",
					HostPath: "/usr/local/vgpu/vgpuvalidator", ReadOnly: true},
				&pluginapi.Mount{ContainerPath: "/vgpu",
					HostPath: "/usr/local/vgpu/license", ReadOnly: true},
				)
			if verboseFlag > 5 {
				log.Printf("Debug: allocate request %v, response %v\n",
					r.DevicesIDs, deviceIDs)
			}
		}
		response.Spec = append(response.Spec, devRuntime)
	}
	return &response, nil
}

func (m *NvidiaDevicePlugin) cleanup() error {
	if err := os.Remove(m.socket); err != nil && !os.IsNotExist(err) {
		return err
	}

	return nil
}

func (m *NvidiaDevicePlugin) healthcheck() {
	ctx, cancel := context.WithCancel(context.Background())

	xids := make(chan *pluginapi.Device)
	go watchXIDs(ctx, m.devs, xids)

	for {
		select {
		case <-m.stop:
			cancel()
			return
		case dev := <-xids:
			m.unhealthy(dev)
		}
	}
}

// Serve starts the gRPC server and register the device plugin to Kubelet
func (m *NvidiaDevicePlugin) Serve() error {
	err := m.Start()
	if err != nil {
		log.Printf("Could not start device plugin: %s", err)
		return err
	}
	log.Println("Starting to serve on", m.socket)

	err = m.Register(pluginapi.KubeletSocket, resourceName)
	if err != nil {
		log.Printf("Could not register device plugin: %s", err)
		m.Stop()
		return err
	}
	log.Println("Registered device plugin with Kubelet")

	return nil
}

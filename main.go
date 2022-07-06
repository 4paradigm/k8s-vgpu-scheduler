// Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.

package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"strings"
	"syscall"

	"github.com/NVIDIA/gpu-monitoring-tools/bindings/go/nvml"
	//"github.com/NVIDIA/nvidia-docker/src/nvml"
	"github.com/fsnotify/fsnotify"
	cli "github.com/urfave/cli/v2"
	pluginapi "k8s.io/kubernetes/pkg/kubelet/apis/deviceplugin/v1alpha1"
)

var failOnInitErrorFlag bool
var passDeviceSpecsFlag bool
var deviceListStrategyFlag string
var deviceIDStrategyFlag string
var nvidiaDriverRootFlag string
var deviceSplitCountFlag uint
var deviceMemoryScalingFlag float64
var deviceCoresScalingFlag float64
var enableLegacyPreferredFlag bool
var verboseFlag int
var version string // This should be set at build time to indicate the actual version

func main() {
	c := cli.NewApp()
	c.Version = version
	c.Before = validateFlags
	c.Action = start

	c.Flags = []cli.Flag{
		&cli.BoolFlag{
			Name:        "fail-on-init-error",
			Value:       true,
			Usage:       "fail the plugin if an error is encountered during initialization, otherwise block indefinitely",
			Destination: &failOnInitErrorFlag,
			EnvVars:     []string{"FAIL_ON_INIT_ERROR"},
		},
		&cli.BoolFlag{
			Name:        "pass-device-specs",
			Value:       false,
			Usage:       "pass the list of DeviceSpecs to the kubelet on Allocate()",
			Destination: &passDeviceSpecsFlag,
			EnvVars:     []string{"PASS_DEVICE_SPECS"},
		},
		&cli.StringFlag{
			Name:        "device-list-strategy",
			Value:       "envvar",
			Usage:       "the desired strategy for passing the device list to the underlying runtime:\n\t\t[envvar | volume-mounts]",
			Destination: &deviceListStrategyFlag,
			EnvVars:     []string{"DEVICE_LIST_STRATEGY"},
		},
		&cli.StringFlag{
			Name:        "device-id-strategy",
			Value:       "uuid",
			Usage:       "the desired strategy for passing device IDs to the underlying runtime:\n\t\t[uuid | index]",
			Destination: &deviceIDStrategyFlag,
			EnvVars:     []string{"DEVICE_ID_STRATEGY"},
		},
		&cli.StringFlag{
			Name:        "nvidia-driver-root",
			Value:       "/",
			Usage:       "the root path for the NVIDIA driver installation (typical values are '/' or '/run/nvidia/driver')",
			Destination: &nvidiaDriverRootFlag,
			EnvVars:     []string{"NVIDIA_DRIVER_ROOT"},
		},
		&cli.UintFlag{
			Name:        "device-split-count",
			Value:       2,
			Usage:       "the number for NVIDIA device split)",
			Destination: &deviceSplitCountFlag,
			EnvVars:     []string{"DEVICE_SPLIT_COUNT"},
		},
		&cli.Float64Flag{
			Name:        "device-memory-scaling",
			Value:       1.0,
			Usage:       "the ratio for NVIDIA device memory scaling)",
			Destination: &deviceMemoryScalingFlag,
			EnvVars:     []string{"DEVICE_MEMORY_SCALING"},
		},
		&cli.Float64Flag{
			Name:        "device-cores-scaling",
			Value:       1.0,
			Usage:       "the ratio for NVIDIA device cores scaling)",
			Destination: &deviceCoresScalingFlag,
			EnvVars:     []string{"DEVICE_CORES_SCALING"},
		},
		&cli.BoolFlag{
			Name:        "enable-legacy-preferred",
			Value:       false,
			Usage:       "enable when kubelet does not support preferred allocation",
			Destination: &enableLegacyPreferredFlag,
			EnvVars:     []string{"ENABLE_LEGACY_PREFERRED"},
		},
		&cli.IntFlag{
			Name:        "verbose",
			Value:       0,
			Usage:       "log verbose level",
			Destination: &verboseFlag,
			EnvVars:     []string{"VERBOSE"},
		},
	}

	err := c.Run(os.Args)
	if err != nil {
		log.SetOutput(os.Stderr)
		log.Printf("Error: %v", err)
		os.Exit(1)
	}
}

func validateFlags(c *cli.Context) error {
	if deviceListStrategyFlag != DeviceListStrategyEnvvar && deviceListStrategyFlag != DeviceListStrategyVolumeMounts {
		return fmt.Errorf("invalid --device-list-strategy option: %v", deviceListStrategyFlag)
	}

	if deviceIDStrategyFlag != DeviceIDStrategyUUID && deviceIDStrategyFlag != DeviceIDStrategyIndex {
		return fmt.Errorf("invalid --device-id-strategy option: %v", deviceIDStrategyFlag)
	}
	if deviceSplitCountFlag < 1 {
		return fmt.Errorf("invalid --device-split-count option: %v", deviceSplitCountFlag)
	}
	if deviceMemoryScalingFlag <= 0 {
		return fmt.Errorf("invalid --device-memory-scaling option: %v", deviceMemoryScalingFlag)
	}
	if deviceCoresScalingFlag <= 0 {
		return fmt.Errorf("invalid --device-core-scaling option: %v", deviceCoresScalingFlag)
	}
	return nil
}

func start(c *cli.Context) error {

	log.Println("Loading PciInfo")
	cmd := exec.Command("lspci")
	out, err := cmd.Output()
	if err != nil {
		return err
	}
	pcibusfile := os.Getenv("PCIBUSFILE")
	pcibusstr := ""
	for idx, val := range strings.Split(string(out), "\n") {
		fmt.Println(idx, "=", val)
		if len(val) > 1 {
			pcibusid := strings.Split(val, " ")[0]
			if strings.Contains(val, "NVIDIA") {
				fmt.Println("found", pcibusid)
				pcibusstr = pcibusstr + pcibusid + "\n"
			}
		}
	}
	fmt.Println("pcibusstr=", pcibusstr)
	if len(pcibusfile) > 0 {
		ioutil.WriteFile(pcibusfile, []byte(pcibusstr), 0644)
	}

	log.Println("Loading NVML")
	if err := nvml.Init(); err != nil {
		log.Printf("Failed to start nvml with error: %s.", err)
		os.Exit(1)
	}
	defer func() { log.Println("Shutdown of NVML returned:", nvml.Shutdown()) }()

	log.Println("Fetching devices.")
	if len(getDevices()) == 0 {
		log.Println("No devices found. Waiting indefinitely.")
		select {}
	}

	log.Println("Starting FS watcher.")
	watcher, err := newFSWatcher(pluginapi.DevicePluginPath)
	if err != nil {
		log.Println("Failed to created FS watcher.")
		os.Exit(1)
	}
	defer watcher.Close()

	log.Println("Starting OS watcher.")
	sigs := newOSWatcher(syscall.SIGHUP, syscall.SIGINT, syscall.SIGTERM, syscall.SIGQUIT)

	restart := true
	var devicePlugin *NvidiaDevicePlugin

L:
	for {
		if restart {
			if devicePlugin != nil {
				devicePlugin.Stop()
			}

			devicePlugin = NewNvidiaDevicePlugin()
			if err := devicePlugin.Serve(); err != nil {
				log.Println("Could not contact Kubelet, retrying. Did you enable the device plugin feature gate?")
			} else {
				restart = false
			}
		}

		select {
		case event := <-watcher.Events:
			if event.Name == pluginapi.KubeletSocket && event.Op&fsnotify.Create == fsnotify.Create {
				log.Printf("inotify: %s created, restarting.", pluginapi.KubeletSocket)
				restart = true
			}

		case err := <-watcher.Errors:
			log.Printf("inotify: %s", err)

		case s := <-sigs:
			switch s {
			case syscall.SIGHUP:
				log.Println("Received SIGHUP, restarting.")
				restart = true
			default:
				log.Printf("Received signal \"%v\", shutting down.", s)
				devicePlugin.Stop()
				break L
			}
		}
	}
	return nil
}

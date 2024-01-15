English version|[中文版](README_cn.md)

<img src="https://github.com/4paradigm/k8s-vgpu-scheduler/blob/libopensource/HAMi.jpg" width="400px">

# Heterogeneous AI Computing Virtualization Middleware

[![build status](https://github.com/4paradigm/k8s-vgpu-scheduler/actions/workflows/main.yml/badge.svg)](https://github.com/4paradigm/k8s-vgpu-scheduler/actions/workflows/main.yml)
[![docker pulls](https://img.shields.io/docker/pulls/4pdosc/k8s-vgpu.svg)](https://hub.docker.com/r/4pdosc/k8s-vgpu)
[![slack](https://img.shields.io/badge/Slack-Join%20Slack-blue)](https://join.slack.com/t/k8s-device-plugin/shared_invite/zt-oi9zkr5c-LsMzNmNs7UYg6usc0OiWKw)
[![discuss](https://img.shields.io/badge/Discuss-Ask%20Questions-blue)](https://github.com/4paradigm/k8s-device-plugin/discussions)
[![Contact Me](https://img.shields.io/badge/Contact%20Me-blue)](https://github.com/4paradigm/k8s-vgpu-scheduler#contact)

## Supperted devices

[![nvidia GPU](https://img.shields.io/badge/Nvidia-GPU-blue)](https://github.com/4paradigm/k8s-vgpu-scheduler#preparing-your-gpu-nodes)
[![cambricon MLU](https://img.shields.io/badge/Cambricon-Mlu-blue)](docs/cambricon-mlu-support.md)
[![hygon DCU](https://img.shields.io/badge/Hygon-DCU-blue)](docs/hygon-dcu-support.md)

## Introduction

![img](./imgs/example.jpg)

**Heterogeneous AI Computing Virtualization Middleware(HAMI), once called(k8s-vGPU-scheduler) is an "all in one" chart to manage your Heterogeneous AI Computing Devices in k8s cluster**, it has everything you expect for, including:

***Device sharing***: Each task can allocate a portion of device instead of a whole device, thus a device can be shared among multiple tasks.

***Device Memory Control***: Devicess can be allocated with certain device memory size (i.e 3000M) or device memory percentage of whole GPU(i.e 50%) and have made it that it does not exceed the boundary.

***Device Type Specification***: You can specify which type of Device to use or to avoid for a certain task, by setting annotations such as "nvidia.com/use-gputype" or "nvidia.com/nouse-gputype". 

***Easy to use***: You don't need to modify your task yaml to use our scheduler. All your jobs will be automatically supported after installation. In addition, you can specify your resource name other than "nvidia.com/gpu" if you wish

## Features

- Hard Limit on Device Memory.

A simple demostration for Hard Limit:
A task with the following resources.
```
      resources:
        limits:
          nvidia.com/gpu: 1 # requesting 1 vGPU
          nvidia.com/gpumem: 3000 # Each vGPU contains 3000m device memory
```
will see 3G device memory inside container

![img](./imgs/hard_limit.jpg)

- Allows partial device allocation by specifying device memory 
- Hard Limit on streaming multiprocessor.
- Allows partial device allocation by specifying device core usage
- Zero changes to existing programs

## When to use

1. Device sharing(or device virtualization) on kubernetes
2. Scenarios when pods need to be allocated with certain device memory usage or device cores.
3. Needs to balance GPU usage in cluster with mutiple GPU node
4. Low utilization of device memory and computing units, such as running 10 tf-servings on one GPU.
5. Situations that require a large number of small GPUs, such as teaching scenarios where one GPU is provided for multiple students to use, and the cloud platform that provides small GPU instance.

## Quick Start

### Prerequisites

The list of prerequisites for running the NVIDIA device plugin is described below:
* NVIDIA drivers >= 440
* CUDA Version > 10.2
* nvidia-docker version > 2.0
* Kubernetes version >= 1.16
* glibc >= 2.17 & glibc < 2.3.0
* kernel version >= 3.10
* helm > 3.0


###  Preparing your GPU Nodes

<details> <summary> Configure nvidia-container-toolkit </summary>

The following steps need to be executed on all your GPU nodes.
This README assumes that the NVIDIA drivers and the `nvidia-container-toolkit` have been pre-installed.
It also assumes that you have configured the `nvidia-container-runtime` as the default low-level runtime to use.

Please see: https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html

#### Example for debian-based systems with `docker` and `containerd`

##### Install the `nvidia-container-toolkit`
```bash
distribution=$(. /etc/os-release;echo $ID$VERSION_ID)
curl -s -L https://nvidia.github.io/libnvidia-container/gpgkey | sudo apt-key add -
curl -s -L https://nvidia.github.io/libnvidia-container/$distribution/libnvidia-container.list | sudo tee /etc/apt/sources.list.d/libnvidia-container.list

sudo apt-get update && sudo apt-get install -y nvidia-container-toolkit
```

##### Configure `docker`
When running `kubernetes` with `docker`, edit the config file which is usually
present at `/etc/docker/daemon.json` to set up `nvidia-container-runtime` as
the default low-level runtime:
```json
{
    "default-runtime": "nvidia",
    "runtimes": {
        "nvidia": {
            "path": "/usr/bin/nvidia-container-runtime",
            "runtimeArgs": []
        }
    }
}
```
And then restart `docker`:
```
$ sudo systemctl daemon-reload && systemctl restart docker
```

##### Configure `containerd`
When running `kubernetes` with `containerd`, edit the config file which is
usually present at `/etc/containerd/config.toml` to set up
`nvidia-container-runtime` as the default low-level runtime:
```
version = 2
[plugins]
  [plugins."io.containerd.grpc.v1.cri"]
    [plugins."io.containerd.grpc.v1.cri".containerd]
      default_runtime_name = "nvidia"

      [plugins."io.containerd.grpc.v1.cri".containerd.runtimes]
        [plugins."io.containerd.grpc.v1.cri".containerd.runtimes.nvidia]
          privileged_without_host_devices = false
          runtime_engine = ""
          runtime_root = ""
          runtime_type = "io.containerd.runc.v2"
          [plugins."io.containerd.grpc.v1.cri".containerd.runtimes.nvidia.options]
            BinaryName = "/usr/bin/nvidia-container-runtime"
```
And then restart `containerd`:
```
$ sudo systemctl daemon-reload && systemctl restart containerd
```

</details>

<details> <summary> Label your nodes </summary>

You need to label your GPU nodes which can be scheduled by HAMI by adding "gpu=on", otherwise, it cannot be managed by our scheduler.

```
kubectl label nodes {nodeid} gpu=on
```

</details>

###  Install and Uninstall

<details> <summary> Installation </summary>

First, you need to heck your Kubernetes version by the using the following command

```
kubectl version
```

Then, add our repo in helm

```
helm repo add vgpu-charts https://4paradigm.github.io/k8s-vgpu-scheduler
```

You need to set the Kubernetes scheduler image version according to your Kubernetes server version during installation. For example, if your cluster server version is 1.16.8, then you should use the following command for deployment

```
helm install vgpu vgpu-charts/vgpu --set scheduler.kubeScheduler.imageTag=v1.16.8 -n kube-system
```

You can customize your installation by adjusting [configs](docs/config.md).

You can verify your installation by the following command:

```
$ kubectl get pods -n kube-system
```

If the following two pods `vgpu-device-plugin` and `vgpu-scheduler` are in *Running* state, then your installation is successful.

</details>

<details> <summary> Upgrade </summary>

To Upgrade the k8s-vGPU to the latest version, all you need to do is update the repo and restart the chart.

```
$ helm uninstall vgpu -n kube-system
$ helm repo update
$ helm install vgpu vgpu -n kube-system
```
> **WARNING:** *If you upgrade HAMi without clearing your submitted tasks, it may result in segmentation fault.*

</details>

<details> <summary> Uninstall </summary>

```
helm uninstall vgpu -n kube-system
```

> **NOTICE:** *Uninstallation won't kill running tasks.*

</details>

### Submit Task

<details> <summary> Task example </summary>

NVIDIA vGPUs can now be requested by a container
using the `nvidia.com/gpu` resource type:

```
apiVersion: v1
kind: Pod
metadata:
  name: gpu-pod
spec:
  containers:
    - name: ubuntu-container
      image: ubuntu:18.04
      command: ["bash", "-c", "sleep 86400"]
      resources:
        limits:
          nvidia.com/gpu: 2 # requesting 2 vGPUs
          nvidia.com/gpumem: 3000 # Each vGPU contains 3000m device memory （Optional,Integer）
          nvidia.com/gpucores: 30 # Each vGPU uses 30% of the entire GPU （Optional,Integer)
```

You should be cautious that if the task can't fit in any GPU node(ie. the number of `nvidia.com/gpu` you request exceeds the number of GPU in any node). The task will get stuck in `pending` state.

You can now execute `nvidia-smi` command in the container and see the difference of GPU memory between vGPU and real GPU.

> **WARNING:** *1. if you don't request vGPUs when using the device plugin with NVIDIA images all
> the vGPUs on the machine will be exposed inside your container.*
> 
> *2. Do not set "nodeName" field, use "nodeSelector" instead.* 

#### More examples

Click [here](docs/examples/nvidia/)

</details>

### Monitor

<details> <summary> Get cluster overview </summary>

Monitoring is automatically enabled after installation. You can get the overview of cluster information by visiting the following url:

```
http://{scheduler ip}:{monitorPort}/metrics
```

Default monitorPort is 31993, other values can be set using `--set deivcePlugin.service.httpPort` during installation.

grafana dashboard [example](docs/dashboard.md)

> **Note** The status of a node won't be collected before you submit a task

</details>

## [Benchmarks](docs/benchmark.md)

## Known Issues

- Currently, A100 MIG can only support "none" and "mixed" mode 
- Currently, task with filed "nodeName" can't be scheduled, please use "nodeSelector" instead
- Currently, only computing tasks are supported, and video codec processing is not supported.

## Roadmap

Heterogeneous AI Computing device to support

| Production  | manufactor | MemoryIsolation | CoreIsolation | MultiCard support |
|-------------|------------|-----------------|---------------|-------------------|
| GPU         | NVIDIA     | ✅             | ✅            | ✅                 |
| MLU         | Cambricon  | ✅              | ❌        | ❌                    |

- Support video codec processing
- Support Multi-Instance GPUs (MIG)

## Issues and Contributing

* You can report a bug, a doubt or modify by [filing a new issue](https://github.com/4paradigm/k8s-vgpu-scheduler/issues/new)
* If you want to know more or have ideas, you can participate in the [Discussions](https://github.com/4paradigm/k8s-device-plugin/discussions) and the [slack](https://join.slack.com/t/k8s-device-plugin/shared_invite/zt-oi9zkr5c-LsMzNmNs7UYg6usc0OiWKw) exchanges

## Contact

Owner & Maintainer: Limengxuan

Feel free to reach me by 

```
email: <limengxuan@4paradigm.com> 
phone: +86 18810644493
WeChat: xuanzong4493
```
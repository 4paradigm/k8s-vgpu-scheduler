# Protocol

## Device Register

HAMi needs to know the spec of each AI devices in the cluster in order to schedule properly. During device registration, device-plugin needs to patch the spec of each device into node annotations, in the format of the following:

```
HAMi.sh/node-handshake: Requesting_2024.01.22 07:11:32
HAMi.sh/node-handshake-mlu: Requesting_2024.01.10 04:06:57
HAMi.sh/node-mlu-register: MLU-45013011-2257-0000-0000-000000000000,10,23308,0,MLU-MLU370-X4,0,false:MLU-54043011-2257-0000-0000-000000000000,10,23308,0,
HAMi.sh/node-nvidia-register: GPU-00552014-5c87-89ac-b1a6-7b53aa24b0ec,10,32768,100,NVIDIA-Tesla V100-PCIE-32GB,0,true:GPU-0fc3eda5-e98b-a25b-5b0d-cf5c855d1448,10,32768,100,NVIDIA-Tesla V100-PCIE-32GB,0,true:
```

In the example above, this node has two different AI devices, 2 Nvidia-V100 GPUs, and 2 Cambircon 370-X4 MLUs

## Schedule Decision

HAMi scheduler needs to patch schedule decisions into pod annotations, in the format of the following:

```
HAMi.sh/devices-to-allocate:{ctr1 request}:{ctr2 request}:...{Last ctr request}:
HAMi.sh/device-node: {schedule decision node}
HAMi.sh/device-schedule-time: {timestamp}
```

each container request is in the following format:

```
{device UUID},{device type keywork},{device memory request}:{device core request}
```

for example:

A pod with 2 containers, first container requests 1 GPU with 3G device Memory, second container requests 1 GPU with 5G device Memory, then the patched annotations will be like the

```
HAMi.sh/devices-to-allocate: GPU-0fc3eda5-e98b-a25b-5b0d-cf5c855d1448,NVIDIA,3000,0:GPU-0fc3eda5-e98b-a25b-5b0d-cf5c855d1448,NVIDIA,5000,0: 
HAMi.sh/vgpu-node: node67-4v100
HAMi.sh/vgpu-time: 1705054796
```


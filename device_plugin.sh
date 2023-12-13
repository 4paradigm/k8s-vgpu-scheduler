/nvidia-device-plugin --resource-name=nvidia.com/gpu --mig-strategy=none --scheduler-endpoint=vgpu-scheduler:1080 --device-memory-scaling=1 --device-split-count=10 -v=4

/mlu-device-plugin --mode=env-share --virtualization-num=10 --mlulink-policy=best-effort --cnmon-path=/usr/bin/cnmon --scheduler-endpoint=vgpu-scheduler:1080

scheduler --resource-name=nvidia.com/gpu --resource-mem=nvidia.com/gpumem --resource-cores=nvidia.com/gpucores --http_bind=0.0.0.0:443 --grpc_bind=0.0.0.0:1080 --cert_file=/tls/tls.crt --key_file=/tls/tls.key --scheduler-name=4pd-scheduler --default-mem=5000 --default-cores=5 --debug -v=4
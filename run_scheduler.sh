/scheduler --resource-name=nvidia.com/gpu --resource-mem=nvidia.com/gpumem --resource-cores=nvidia.com/gpucores --resource-mem-percentage=nvidia.com/gpumem-percentage \
--resource-priority=nvidia.com/priority --http_bind=0.0.0.0:443 --cert_file=/tls/tls.crt --key_file=/tls/tls.key --scheduler-name=4pd-scheduler --debug -v=4

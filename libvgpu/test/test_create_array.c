#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <dlfcn.h>

#include "test_utils.h"



int main() {
    

    CHECK_DRV_API(cuInit(0));

    CUdevice device;
    CHECK_DRV_API(cuDeviceGet(&device, 0));

    CUcontext ctx;
    CHECK_DRV_API(cuCtxCreate(&ctx, 0, device));

    size_t usage = 0;
    size_t t_size=0;
    CHECK_NVML_API(get_current_memory_usage(&usage));

    CUarray handle;
    CUDA_ARRAY_DESCRIPTOR allocateArrayAttr = \
        {2048, 2048, CU_AD_FORMAT_UNSIGNED_INT32, 2};
	CHECK_DRV_API(cuArrayCreate(&handle, &allocateArrayAttr));
    CHECK_NVML_API(get_current_memory_usage(&usage));


    CHECK_DRV_API(cuArrayDestroy(handle));
    CHECK_NVML_API(get_current_memory_usage(&usage));

    CHECK_NVML_API(nvmlShutdown());
    CHECK_DRV_API(cuCtxDestroy(ctx));
    return 0;
}

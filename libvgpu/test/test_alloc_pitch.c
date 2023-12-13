#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime.h>

#include "test_utils.h"


int main() {
    CHECK_DRV_API(cuInit(0));

    CUdevice device;
    CHECK_DRV_API(cuDeviceGet(&device, 0));

    CUcontext ctx;
    CHECK_DRV_API(cuCtxCreate(&ctx, 0, device));

    size_t usage = 0;
    CHECK_NVML_API(get_current_memory_usage(&usage));

    CUdeviceptr dptr;
    size_t pitch;
    CHECK_DRV_API(cuMemAllocPitch(&dptr, &pitch, 4096 - 8, 10, 16));
    CHECK_NVML_API(get_current_memory_usage(&usage));
	CHECK_DRV_API(cuMemFree(dptr));
    CHECK_NVML_API(get_current_memory_usage(&usage));

    CHECK_DRV_API(cuMemAllocPitch(&dptr, &pitch, 4096 - 4, 10, 8));
    CHECK_NVML_API(get_current_memory_usage(&usage));
    CHECK_DRV_API(cuMemFree(dptr));
    CHECK_NVML_API(get_current_memory_usage(&usage));

    CHECK_DRV_API(cuMemAllocPitch(&dptr, &pitch, 4096 - 1, 10, 4));
    CHECK_NVML_API(get_current_memory_usage(&usage));
    CHECK_DRV_API(cuMemFree(dptr));
    CHECK_NVML_API(get_current_memory_usage(&usage));

    CHECK_NVML_API(nvmlShutdown());
    CHECK_DRV_API(cuCtxDestroy(ctx));
    return 0;
}

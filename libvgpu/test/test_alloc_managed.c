#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <dlfcn.h>

#include "test_utils.h"


size_t usage = 0;

int test(CUdeviceptr *dptr,size_t bytes) {
    CHECK_DRV_API(cuMemAllocManaged(dptr, bytes, CU_MEM_ATTACH_GLOBAL));
//    CHECK_NVML_API(get_current_memory_usage(&usage));
//    CHECK_DRV_API(cuMemFree(dptr));
//    CHECK_NVML_API(get_current_memory_usage(&usage));
    
//    printf("\n");
    return 0;
}


int main() {
    
    CHECK_DRV_API(cuInit(0));

    CUdevice device;
    CHECK_DRV_API(cuDeviceGet(&device, TEST_DEVICE_ID));

    CUcontext ctx;
    CHECK_DRV_API(cuCtxCreate(&ctx, 0, device));
    CHECK_NVML_API(get_current_memory_usage(&usage));

    size_t arr[84] = {};
    int k = 0;
    for (k = 0; k < 28; ++k) {
        // power of 2
        arr[3 * k] = 2 << k;

        // power of 2 plus 1
        arr[3 * k + 1] = (2 << k) + 1;

        // power of 2 minus 1
        arr[3 * k + 2] = (2 << k) - 1;
    }

    CUdeviceptr dptr[84];
    for (k = 0; k < 84; ++k) {
        if (0 != test(&dptr[k],arr[k])) {
            fprintf(stderr, "Test alloc %lu bytes failed\n", arr[k]);
            return -1;
        }
    }

    for (k=0;k<84;++k) {
        cuMemFree(dptr[k]);
    }

    CHECK_NVML_API(nvmlShutdown());
    CHECK_DRV_API(cuCtxDestroy(ctx));
    return 0;
}

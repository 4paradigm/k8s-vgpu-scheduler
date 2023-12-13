#include <stdio.h>
#include <cuda.h>
#include <dlfcn.h>
#include <cuda_runtime.h>
#include <assert.h>

#include "test_utils.h"


size_t usage = 0;

int test(size_t bytes) {
    CUdeviceptr dptr;
    uint64_t t_size;
    CHECK_DRV_API(cuMemAlloc(&dptr, bytes));
    CHECK_NVML_API(get_current_memory_usage(&usage));
    CHECK_DRV_API(cuMemFree(dptr));
    CHECK_NVML_API(get_current_memory_usage(&usage));
    return 0;
}

int alloc_trim_test(){
    size_t arr[93] = {};
    int k = 0;
    for (k = 0; k < 30; ++k) {
        // power of 2
        arr[3 * k] = 2 << k;

        // power of 2 plus 1
        arr[3 * k + 1] = (2 << k) + 1;

        // power of 2 minus 1
        arr[3 * k + 2] = (2 << k) - 1;
    }

    for (k = 0; k < 90; ++k) {
        printf("k=%d,arr[k]=%lu\n",k,arr[k]);
        assert(test(arr[k])==0);
        /*if (0 != test(arr[k])) {
            fprintf(stderr, "Test alloc %lu bytes failed\n", arr[k]);
            return -1;
        }*/
    }
}

int alloc_oversize_test(){
    size_t tmp_size = (2 << 29) + 2;
    int k=0;
    printf("size=%lu\n",tmp_size);
    CUdeviceptr dp[10];
    for (k=0;k<10;k++){
        cuMemAlloc(&dp[k],tmp_size);
    }
    CHECK_NVML_API(get_current_memory_usage(&usage));

    for (k=0;k<10;k+=2)
        cuMemFree(dp[k]);

    CHECK_NVML_API(get_current_memory_usage(&usage));

    for (k=1;k<10;k+=2)
        cuMemFree(dp[k]);

    CHECK_NVML_API(get_current_memory_usage(&usage));
}


int main() {
    CHECK_DRV_API(cuInit(0));

    CUdevice device;
    CHECK_DRV_API(cuDeviceGet(&device, TEST_DEVICE_ID));

    CUcontext ctx;
    CHECK_DRV_API(cuCtxCreate(&ctx, 0, device));
    CHECK_NVML_API(get_current_memory_usage(&usage));

    CHECK_ALLOC_TEST(alloc_trim_test());
    
    CHECK_ALLOC_TEST(alloc_oversize_test());

    CHECK_NVML_API(nvmlShutdown());
    CHECK_DRV_API(cuCtxDestroy(ctx));
    return 0;
}

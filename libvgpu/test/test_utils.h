#ifndef __TEST_UTILS_H__
#define __TEST_UTILS_H__

#include "include/nvml_prefix.h"
#include <nvml.h>
#include <pthread.h>
#include <stdio.h>


#ifndef TEST_DEVICE_ID
#define TEST_DEVICE_ID 0
#endif


#define CHECK_RUNTIME_API(f) {                \
    cudaError_t status = (f);                 \
    if (status != cudaSuccess) {              \
        fprintf(stderr,                       \
            "CUDA runtime error at line "     \
            "%s:%d: %d\n",                    \
            __FILE__, __LINE__, status);      \
        return status;                        \
    } }                                       \


#define CHECK_DRV_API(f)  {                   \
    CUresult status = (f);                    \
    if (status != CUDA_SUCCESS) {             \
    	fprintf(stderr,                       \
    		"CUDA driver api error at line "  \
    		"%s:%d: %d\n",                    \
    		__FILE__, __LINE__, status);      \
    	return status;                        \
    } }                                       \

#define CHECK_NVML_API(f)  {                  \
    nvmlReturn_t status = (f);                \
    if (status != NVML_SUCCESS) {             \
    	fprintf(stderr,                       \
    		"NVML api error at line "         \
    		"%d: %d\n",                       \
    		__LINE__, status);                \
    	return status;                        \
    } }                                       \

#define CHECK_ALLOC_TEST(f) {				  \
	fprintf(stderr,"Testing %s\n",#f);	 	  \
	(f);									 \
	fprintf(stderr,"Test %s succeed\n",#f);	 \
}

pthread_once_t __status_initialize_nvml = PTHREAD_ONCE_INIT;                            
nvmlDevice_t __current_nvml_device;


int nvml_init() {
	CHECK_NVML_API(nvmlInit());
	CHECK_NVML_API(nvmlDeviceGetHandleByIndex(TEST_DEVICE_ID, &__current_nvml_device));
	return 0;
}

nvmlDevice_t get_nvml_device() {
	(void) pthread_once(&__status_initialize_nvml, (void (*)(void)) nvml_init);
	return __current_nvml_device;
}

nvmlReturn_t get_current_memory_usage(size_t* usage) {
	nvmlDevice_t dev = get_nvml_device();
	nvmlMemory_t memory;
	CHECK_NVML_API(nvmlDeviceGetMemoryInfo(dev, &memory));
	int64_t diff = memory.used > (*usage) ? \
        memory.used - (*usage) :
        -((int64_t)((*usage) - memory.used));
	printf("Current usage: %llu bytes, diff: %ld bytes, total: %llu bytes\n",
		memory.used, diff, memory.total);
	*usage = memory.used;
	return NVML_SUCCESS;
}


#endif

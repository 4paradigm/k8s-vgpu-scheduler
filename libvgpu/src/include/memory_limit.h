#ifndef __MEMORY_LIMIT_H__
#define __MEMORY_LIMIT_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "static_config.h"


#define CUDA_DEVICE_MEMORY_LIMIT "CUDA_DEVICE_MEMORY_LIMIT"
#define CUDA_DEVICE_MEMORY_LIMIT_KEY_LENGTH 32
#define CUDA_DEVICE_SM_LIMIT "CUDA_DEVICE_SM_LIMIT"
#define CUDA_DEVICE_SM_LIMIT_KEY_LENGTH 32

#define ENSURE_INITIALIZED() ensure_initialized();        \

extern int wait_status_self(int status);

#define ENSURE_RUNNING() {                                \
   /* LOG_DEBUG("Memory op at %d",__LINE__); */              \
    ensure_initialized();                                 \
    while(!wait_status_self(1)) { LOG_DEBUG("E1"); sleep(1); }             \
}                                                         \

#define INC_MEMORY_OR_RETURN_ERROR(bytes) {               \
    CUdevice dev;                                         \
    CHECK_DRV_API(cuCtxGetDevice(&dev));                  \
    if (inc_current_device_memory_usage(dev, bytes) !=    \
        CUDA_DEVICE_MEMORY_UPDATE_SUCCESS) {              \
        return CUDA_ERROR_OUT_OF_MEMORY;                  \
    } }                                                   \

#define DECL_MEMORY_ON_ERROR(res, bytes) {                \
    CUdevice dev;                                         \
    CHECK_DRV_API(cuCtxGetDevice(&dev));                  \
    if (res != CUDA_SUCCESS) {                            \
        decl_current_device_memory_usage(dev, bytes);     \
    } }                                                   \

#define DECL_MEMORY_ON_SUCCESS(res, bytes) {              \
    CUdevice dev;                                         \
    CHECK_DRV_API(cuCtxGetDevice(&dev));                  \
    if (res == CUDA_SUCCESS) {                            \
        decl_current_device_memory_usage(dev, bytes);     \
    } }                                                   \

#define INC_MEMORY_OR_RETURN_ERROR_WITH_DEV(d, bytes) {   \
    if (inc_current_device_memory_usage(d, bytes) !=      \
        CUDA_DEVICE_MEMORY_UPDATE_SUCCESS) {              \
        return CUDA_ERROR_OUT_OF_MEMORY;                  \
    }                                                     \

#define DECL_MEMORY_ON_ERROR_WITH_DEV(dev, res, bytes)    \
    if (res != CUDA_SUCCESS) {                            \
        decl_current_device_memory_usage(dev, bytes);     \
    }                                                     \

#define DECL_MEMORY_ON_SUCCESS_WITH_DEV(dev, res, bytes)  \
    if (res == CUDA_SUCCESS) {                            \
        decl_current_device_memory_usage(dev, bytes);     \
    }                                                     \
/*
#define OOM_CHECK()                                       \
    CUdevice dev;                                         \
    CHECK_DRV_API(cuCtxGetDevice(&dev));                  \
    oom_check(dev);
*/

#include "multiprocess/multiprocess_memory_limit.h"

#endif  // __MEMORY_LIMIT_H__

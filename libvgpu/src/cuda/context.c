#include "include/libcuda_hook.h"
#include "multiprocess/multiprocess_memory_limit.h"

extern int context_size;
extern int cuda_to_nvml_map[16];
extern int ctx_activate[16];


CUresult cuDevicePrimaryCtxGetState( CUdevice dev, unsigned int* flags, int* active ){
    LOG_DEBUG("into cuDevicePrimaryCtxGetState dev=%d",dev);
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuDevicePrimaryCtxGetState,dev,flags,&active);
    return res;
}

CUresult cuDevicePrimaryCtxRetain(CUcontext *pctx, CUdevice dev){
    LOG_INFO("dev=%d context_size=%d",dev,context_size);
    //for Initialization only
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuDevicePrimaryCtxRetain,pctx,dev);
    if (ctx_activate[dev] == 0) {
        add_gpu_device_memory_usage(getpid(),dev,context_size,0); 
    }
    ctx_activate[dev] = 1;
    return res;
}


CUresult cuDevicePrimaryCtxSetFlags_v2( CUdevice dev, unsigned int  flags ){
    LOG_WARN("into cuDevicePrimaryCtxSetFlags dev=%d flags=%d",dev,flags);
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuDevicePrimaryCtxSetFlags_v2,dev,flags);
}

CUresult cuDevicePrimaryCtxRelease_v2( CUdevice dev ){
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuDevicePrimaryCtxRelease_v2,dev);
    ctx_activate[dev] = 0;
    return res;
}

CUresult cuCtxGetDevice(CUdevice* device) {
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxGetDevice,device);
    return res;
}

CUresult cuCtxCreate_v2 ( CUcontext* pctx, unsigned int  flags, CUdevice dev ){
    LOG_WARN("into cuCtxCreate pctx=%p flags=%d dev=%d",pctx,flags,dev);
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxCreate_v2,pctx,flags,dev);
    return res;
}

CUresult cuCtxDestroy_v2 ( CUcontext ctx ){
    LOG_WARN("into cuCtxDestroy_v2 ctx=%p",ctx);
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxDestroy_v2,ctx);
}

CUresult cuCtxGetApiVersion ( CUcontext ctx, unsigned int* version ){
    LOG_INFO("into cuCtxGetApiVersion ctx=%p",ctx);
    CUresult res =  CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxGetApiVersion,ctx,version);
    if (res!=CUDA_SUCCESS){
        LOG_ERROR("cuCtxGetApiVersion res=%d",res);
    }
    return res;
}

CUresult cuCtxGetCacheConfig ( CUfunc_cache* pconfig ){
    LOG_WARN("into cuCtxGetCacheConfig");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxGetCacheConfig,pconfig);
}

CUresult cuCtxGetCurrent ( CUcontext* pctx ){
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxGetCurrent,pctx);
    return res;
}

CUresult cuCtxGetFlags ( unsigned int* flags ){
    LOG_WARN("into cuCtxGetFlags flags=%p",flags);
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxGetFlags,flags);
}

CUresult cuCtxGetLimit ( size_t* pvalue, CUlimit limit ){
    LOG_WARN("into cuCtxGetLimit pvalue=%p",pvalue);
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxGetLimit,pvalue,limit);
}

CUresult cuCtxGetSharedMemConfig ( CUsharedconfig* pConfig ){
    LOG_WARN("cuCtxGetSharedMemConfig pConfig=%p",pConfig);
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxGetSharedMemConfig,pConfig);
}

CUresult cuCtxGetStreamPriorityRange ( int* leastPriority, int* greatestPriority ){
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxGetStreamPriorityRange,leastPriority,greatestPriority);
    if (res!=CUDA_SUCCESS){
        LOG_WARN("cuCtxGetStreamPriorityRange err=%d",res);
    }
    return res;
}

CUresult cuCtxPopCurrent_v2 ( CUcontext* pctx ){
    LOG_INFO("cuCtxPopCurrent pctx=%p",pctx);
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxPopCurrent_v2,pctx);
}

CUresult cuCtxPushCurrent_v2 ( CUcontext ctx ){
    LOG_INFO("cuCtxPushCurrent ctx=%p",ctx);
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxPushCurrent_v2,ctx);
}

CUresult cuCtxSetCacheConfig ( CUfunc_cache config ){
    LOG_WARN("cuCtxSetCacheConfig config=%d",config);
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxSetCacheConfig,config);
}

CUresult cuCtxSetCurrent ( CUcontext ctx ){
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxSetCurrent,ctx);
    if (res!=CUDA_SUCCESS){
        LOG_WARN("cuCtxSetCurrent failed res=%d",res);
    }
    return res;
}

CUresult cuCtxSetLimit ( CUlimit limit, size_t value ){
    LOG_WARN("cuCtxSetLimit");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxSetLimit,limit,value);
}

CUresult cuCtxSetSharedMemConfig ( CUsharedconfig config ){
    LOG_WARN("cuCtxSetSharedMemConfig");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxSetSharedMemConfig,config);
}

CUresult cuCtxSynchronize ( void ){
    LOG_DEBUG("INTO CtxSync");
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxSynchronize);
    return res;
}


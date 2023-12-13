#include "include/libcuda_hook.h"

CUresult cuStreamCreate(CUstream *phstream, unsigned int flags){
    LOG_INFO("cuStreamCreate %p",phstream);
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuStreamCreate,phstream,flags);
    return res;
}

CUresult cuStreamDestroy_v2 ( CUstream hStream ){
    LOG_DEBUG("cuStreamDestory_v2 %p",hStream);
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuStreamDestroy_v2,hStream);
}

CUresult cuStreamSynchronize(CUstream hstream){
    LOG_DEBUG("cuStreamSync %p",hstream);
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuStreamSynchronize,hstream);
    return res;
}

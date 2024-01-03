#include "include/libcuda_hook.h"
#include "include/cuda_addition_func.h"
#include "multiprocess/multiprocess_memory_limit.h"
#include "include/nvml_prefix.h"
#include "include/libnvml_hook.h"

#include "allocator/allocator.h"
#include "include/memory_limit.h"

extern int cuda_to_nvml_map[16];

CUresult cuDeviceGetAttribute ( int* pi, CUdevice_attribute attrib, CUdevice dev ) {
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuDeviceGetAttribute,pi,attrib,dev);
    LOG_DEBUG("[%d]cuDeviceGetAttribute dev=%d attrib=%d %d",res,dev,(int)attrib,*pi);
    return res;
}

CUresult cuDeviceGet(CUdevice *device,int ordinal){
    LOG_DEBUG("into cuDeviceGet ordinal=%d\n",ordinal);
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuDeviceGet,device,ordinal);
    return res;
}

CUresult cuDeviceGetCount( int* count ) {
    LOG_DEBUG("into cuDeviceGetCount");
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuDeviceGetCount,count);
    LOG_DEBUG("cuDeviceGetCount res=%d count=%d",res,*count);
    return res;
}

CUresult cuDeviceGetName(char *name, int len, CUdevice dev) {
    LOG_DEBUG("into cuDeviceGetName");
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry, cuDeviceGetName, name, len, dev);
    return res;
}

CUresult cuDeviceCanAccessPeer( int* canAccessPeer, CUdevice dev, CUdevice peerDev ) {
    LOG_INFO("into cuDeviceCanAccessPeer %d %d",dev,peerDev);
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuDeviceCanAccessPeer,canAccessPeer,dev,peerDev);
}

CUresult cuDeviceGetP2PAttribute(int *value, CUdevice_P2PAttribute attrib,
                                 CUdevice srcDevice, CUdevice dstDevice) {
    LOG_DEBUG("into cuDeviceGetP2PAttribute\n");
    return CUDA_OVERRIDE_CALL(cuda_library_entry, cuDeviceGetP2PAttribute, value,
                         attrib, srcDevice, dstDevice);
}

CUresult cuDeviceGetByPCIBusId(CUdevice *dev, const char *pciBusId) {
    return CUDA_OVERRIDE_CALL(cuda_library_entry, cuDeviceGetByPCIBusId, dev,
                         pciBusId);
}

CUresult cuDeviceGetPCIBusId(char *pciBusId, int len, CUdevice dev) {
    LOG_INFO("into cuDeviceGetPCIBusId dev=%d len=%d",dev,len);
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry, cuDeviceGetPCIBusId, pciBusId, len,
                        dev);
    return res;
}

CUresult cuDeviceGetUuid(CUuuid* uuid,CUdevice dev) {
    LOG_DEBUG("into cuDeviceGetUuid dev=%d",dev);
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuDeviceGetUuid,uuid,dev);
    return res;
}

CUresult cuDeviceGetDefaultMemPool(CUmemoryPool *pool_out, CUdevice dev) {
    LOG_WARN("cuDeviceGetDefaultMemPool");
    return CUDA_OVERRIDE_CALL(cuda_library_entry, cuDeviceGetDefaultMemPool,
                         pool_out, dev);
}

CUresult cuDeviceGetMemPool(CUmemoryPool *pool, CUdevice dev){
    LOG_WARN("cuDeviceGetMemPool");
    return CUDA_OVERRIDE_CALL(cuda_library_entry, cuDeviceGetMemPool, pool, dev);
}

CUresult cuDeviceGetLuid(char *luid, unsigned int *deviceNodeMask,
                         CUdevice dev) {
  LOG_DEBUG("cuDeviceGetLuid");
  return CUDA_OVERRIDE_CALL(cuda_library_entry, cuDeviceGetLuid, luid,
                         deviceNodeMask, dev);
}

CUresult cuDeviceTotalMem_v2 ( size_t* bytes, CUdevice dev ) {
    LOG_DEBUG("into cuDeviceTotalMem");
    ENSURE_INITIALIZED();
    size_t limit = get_current_device_memory_limit(dev);
    *bytes = limit;
    return CUDA_SUCCESS;
}

CUresult cuDriverGetVersion(int *driverVersion) {
    LOG_DEBUG("into cuDriverGetVersion__");
    
    //stub dlsym to prelaod cuda functions
    dlsym(RTLD_DEFAULT,"cuDriverGetVersion");

    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuDriverGetVersion,driverVersion);
    //*driverVersion=11030;
    if ((res==CUDA_SUCCESS) && (driverVersion!=NULL)) {
        LOG_INFO("driver version=%d",*driverVersion);
    }
    return res;
}

CUresult cuDeviceGetTexture1DLinearMaxWidth(size_t *maxWidthInElements, CUarray_format format, unsigned numChannels, CUdevice dev){
    LOG_WARN("cuDeviceGetTexture1DLinearMaxWidth");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuDeviceGetTexture1DLinearMaxWidth,maxWidthInElements,format,numChannels,dev);
}

CUresult cuDeviceSetMemPool(CUdevice dev, CUmemoryPool pool) {
    LOG_WARN("cuDeviceSetMemPool");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuDeviceSetMemPool,dev,pool);
}

CUresult cuFlushGPUDirectRDMAWrites(CUflushGPUDirectRDMAWritesTarget target, CUflushGPUDirectRDMAWritesScope scope) {
   LOG_WARN("cuFlushGPUDirectRDMAWrites");
   return CUDA_OVERRIDE_CALL(cuda_library_entry,cuFlushGPUDirectRDMAWrites,target,scope);
}

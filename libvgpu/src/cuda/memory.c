#include <dirent.h>
#include <time.h>

#include "allocator/allocator.h"
#include "include/libcuda_hook.h"
#include "include/memory_limit.h"

extern int pidfound;

const size_t cuarray_format_bytes[33] = {
    0,  // 0x00
    1,  // CU_AD_FORMAT_UNSIGNED_INT8 = 0x01
    2,  // CU_AD_FORMAT_UNSIGNED_INT16 = 0x02
    4,  // CU_AD_FORMAT_UNSIGNED_INT32 = 0x03
    0,  // 0x04
    0,  // 0x05
    0,  // 0x06
    0,  // 0x07
    1,  // CU_AD_FORMAT_SIGNED_INT8 = 0x08
    2,  // CU_AD_FORMAT_SIGNED_INT16 = 0x09
    4,  // CU_AD_FORMAT_SIGNED_INT32 = 0x0a
    0,  // 0x0b
    0,  // 0x0c
    0,  // 0x0d
    0,  // 0x0e
    0,  // 0x0f
    2,  // CU_AD_FORMAT_HALF = 0x10
    0,  // 0x11
    0,  // 0x12
    0,  // 0x13
    0,  // 0x14
    0,  // 0x15
    0,  // 0x16
    0,  // 0x17
    0,  // 0x18
    0,  // 0x19
    0,  // 0x1a
    0,  // 0x1b
    0,  // 0x1c
    0,  // 0x1d
    0,  // 0x1e
    0,  // 0x1f       
    4   // CU_AD_FORMAT_FLOAT = 0x20
};

extern size_t round_up(size_t size,size_t align);
extern void rate_limiter(int grids, int blocks);

int check_oom() {
//    return 0;
    CUdevice dev;
    CHECK_DRV_API(cuCtxGetDevice(&dev));
    return oom_check(dev,0);
}

uint64_t compute_3d_array_alloc_bytes(const CUDA_ARRAY3D_DESCRIPTOR* desc) {
    if (desc==NULL) {
        LOG_WARN("compute_3d_array_alloc_bytes desc is null");
    }else{
        LOG_DEBUG("compute_3d_array_alloc_bytes height=%ld width=%ld",desc->Height,desc->Width);
    }
    uint64_t bytes = desc->Width * desc->NumChannels;
    if (desc->Height != 0) {
        bytes *= desc->Height;
    }
    if (desc->Depth != 0) {
        bytes *= desc->Depth;
    }
    bytes *= cuarray_format_bytes[desc->Format];

    // TODO: take acount of alignment and etc
    // bytes ++ ???
    return bytes;
}


uint64_t compute_array_alloc_bytes(const CUDA_ARRAY_DESCRIPTOR* desc) {
    if (desc==NULL) {
        LOG_WARN("compute_array_alloc_bytes desc is null");
    }else{
        LOG_DEBUG("compute_array_alloc_bytes height=%ld width=%ld",desc->Height,desc->Width);
    }

    uint64_t bytes = desc->Width * desc->NumChannels;
    if (desc->Height != 0) {
        bytes *= desc->Height;
    }
    bytes *= cuarray_format_bytes[desc->Format];

    // TODO: take acount of alignment and etc
    // bytes ++ ???
    return bytes;
}

CUresult cuArray3DCreate_v2(CUarray* arr, const CUDA_ARRAY3D_DESCRIPTOR* desc) {
    LOG_WARN("cuArray3DCreate_v2");
    /*uint64_t bytes*/
    compute_3d_array_alloc_bytes(desc);
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuArray3DCreate_v2, arr, desc);
    if (res != CUDA_SUCCESS) {
        return res;
    }
    return res;
}


CUresult cuArrayCreate_v2(CUarray* arr, const CUDA_ARRAY_DESCRIPTOR* desc) {
    LOG_WARN("cuArrayCreate_v2");
    /*uint64_t bytes*/
    compute_array_alloc_bytes(desc);
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuArrayCreate_v2, arr, desc);
    if (res != CUDA_SUCCESS) {
        return res;
    }
    return res;
}


CUresult cuArrayDestroy(CUarray arr) {
    CUDA_ARRAY3D_DESCRIPTOR desc;
    LOG_WARN("cuArrayDestory");
    CHECK_DRV_API(cuArray3DGetDescriptor(&desc, arr));
    /*uint64_t bytes*/
    compute_3d_array_alloc_bytes(&desc);
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuArrayDestroy, arr);
    return res;
}

CUresult cuMemoryAllocate(CUdeviceptr* dptr, size_t bytesize, size_t* bytesallocated,void* data){
    CUresult res;
    if (bytesallocated!=NULL)
        *bytesallocated = bytesize;
    res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemAlloc_v2,dptr,bytesize);
    return res;
}

CUresult cuMemAlloc_v2(CUdeviceptr* dptr, size_t bytesize) {
    LOG_INFO("into cuMemAllocing_v2 dptr=%p bytesize=%ld",dptr,bytesize);
    ENSURE_RUNNING();
    CUresult res = allocate_raw(dptr,bytesize);
    if (res!=CUDA_SUCCESS)
        return res;
    LOG_INFO("res=%d, cuMemAlloc_v2 success dptr=%p bytesize=%lu",0,(void *)*dptr,bytesize);
    return CUDA_SUCCESS;
}

CUresult cuMemAllocHost_v2(void** hptr, size_t bytesize) {
    LOG_DEBUG("cuMemAllocHost_v2 hptr=%p bytesize=%ld",hptr,bytesize);
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemAllocHost_v2, hptr, bytesize);
    if (res != CUDA_SUCCESS) {
        return res;
    }
    if (check_oom()) {
        CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemFreeHost, *hptr);
        return CUDA_ERROR_OUT_OF_MEMORY;
    }
    return res;
}

CUresult cuMemAllocManaged(CUdeviceptr* dptr, size_t bytesize, unsigned int flags) {
    LOG_DEBUG("cuMemAllocManaged dptr=%p bytesize=%ld",dptr,bytesize);
    ENSURE_RUNNING();
    CUdevice dev;
    CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxGetDevice,&dev);
    if (oom_check(dev,bytesize)){
        return CUDA_ERROR_OUT_OF_MEMORY;
    }
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemAllocManaged, dptr, bytesize, flags);
    if (res == CUDA_SUCCESS) {
        add_chunk_only(*dptr,bytesize);
    }
    return res;
}

CUresult cuMemAllocPitch_v2(CUdeviceptr* dptr, size_t* pPitch, size_t WidthInBytes, 
                                      size_t Height, unsigned int ElementSizeBytes) {
    LOG_WARN("cuMemAllocPitch_v2 dptr=%p (%ld,%ld)",dptr,WidthInBytes,Height);
    size_t guess_pitch = (((WidthInBytes - 1) / ElementSizeBytes) + 1) * ElementSizeBytes;
    size_t bytesize = guess_pitch * Height;
    ENSURE_RUNNING();
    CUdevice dev;
    CUDA_OVERRIDE_CALL(cuda_library_entry,cuCtxGetDevice,&dev);
    if (oom_check(dev,bytesize)){
        return CUDA_ERROR_OUT_OF_MEMORY;
    }
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemAllocPitch_v2, dptr, pPitch, WidthInBytes, Height, ElementSizeBytes);
    if (res == CUDA_SUCCESS) {
        add_chunk_only(*dptr,bytesize);
    }
    return res;
}

CUresult cuMemFree_v2(CUdeviceptr dptr) {
    LOG_DEBUG("cuMemFree_v2 dptr=%llx",dptr);
    if (dptr == 0) {  // NULL
        return CUDA_SUCCESS;
    }
    CUresult res = free_raw(dptr);
    LOG_INFO("after free_raw dptr=%p res=%d",(void *)dptr,res);
    return res;
}


CUresult cuMemFreeHost(void* hptr) {
    /*CUdeviceptr dptr;*/
    /*CHECK_DRV_API(cuMemHostGetDevicePointer(&dptr, hptr, 0));*/
    /*size_t bytesize;*/
    /*CHECK_DRV_API(cuMemGetAddressRange(NULL, &bytesize, dptr));*/
    LOG_DEBUG("cuMemFreeHost_v2 hptr=%p",hptr);
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemFreeHost, hptr);
    return res;
}

CUresult cuMemHostAlloc(void** hptr, size_t bytesize, unsigned int flags) {
    LOG_DEBUG("cuMemHostAlloc hptr=%p bytesize=%lu",hptr,bytesize);
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemHostAlloc, hptr, bytesize, flags);
    if (res != CUDA_SUCCESS) {
        return res;
    }
    if (check_oom()) {
        CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemFreeHost, *hptr);
        *hptr = NULL;
        return CUDA_ERROR_OUT_OF_MEMORY;
    }
    return res;
}


CUresult cuMemHostRegister_v2(void* hptr, size_t bytesize, unsigned int flags) {
    /*int trackable = 1;*/
    /*if (flags != CU_MEMHOSTREGISTER_DEVICEMAP) {*/
    /*    fprintf(stderr, "only CU_MEMHOSTREGISTER_DEVICEMAP can be freed, current=%u\n", flags);*/
    /*    trackable = 0;*/
    /*}*/
    // TODO: process flags properly
    LOG_DEBUG("cuMemHostRegister_v2 hptr=%p bytesize=%ld",hptr,bytesize);
    CUdevice dev;
    cuCtxGetDevice(&dev);
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemHostRegister_v2, hptr, bytesize, flags);
    LOG_DEBUG("cuMemHostRegister_v2 returned :%d(%p:%ld)",res,hptr,bytesize);
    if (res != CUDA_SUCCESS) {
        return res;
    }
    if (check_oom()) {
        CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemHostUnregister, hptr);
        return CUDA_ERROR_OUT_OF_MEMORY;
    }
    //LOG_WARN("222:%d(%p:%ld)",res,hptr,bytesize);
    return res;
    //return CUDA_SUCCESS;
}


CUresult cuMemHostUnregister(void* hptr) {
    /*CUdeviceptr dptr;*/
    /*CUresult flag = cuMemHostGetDevicePointer(&dptr, hptr, 0);*/
    /*size_t bytesize = 0;*/
    /*if (flag == CUDA_SUCCESS) {*/
    /*    // only device map registry is trackable*/
    /*    CHECK_DRV_API(cuMemGetAddressRange(NULL, &bytesize, dptr));*/
    /*}*/
    LOG_DEBUG("cuMemHostUnregister hptr=%p",hptr);
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemHostUnregister, hptr);
    
    /*if (flag == CUDA_SUCCESS && bytesize > 0) {*/
    /*    // only device map registry is trackable*/
    /*    DECL_MEMORY_ON_SUCCESS(res, bytesize);*/
    /*}*/
    //return CUDA_SUCCESS;    
    return res;
}


CUresult cuMemcpy(CUdeviceptr dst, CUdeviceptr src, size_t ByteCount ){
    LOG_DEBUG("cuMemcpy dst=%llx src=%llx count=%lu",dst,src,ByteCount);
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpy,dst,src,ByteCount);
    return res;
}
CUresult cuPointerGetAttribute ( void* data, CUpointer_attribute attribute, CUdeviceptr ptr ){
    LOG_DEBUG("cuPointGetAttribue data=%p attribute=%d ptr=%llx",data,(int)attribute,ptr);
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuPointerGetAttribute,data,attribute,ptr);
    return res;
}

CUresult cuPointerGetAttributes ( unsigned int  numAttributes, CUpointer_attribute* attributes, void** data, CUdeviceptr ptr ) {
    LOG_DEBUG("cuPointGetAttribue data=%p ptr=%llx",data,ptr);
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuPointerGetAttributes,numAttributes,attributes,data,ptr);
    int cur=0;
    for (cur=0;cur<numAttributes;cur++){
        if (attributes[cur]==CU_POINTER_ATTRIBUTE_MEMORY_TYPE){
            int j = check_memory_type(ptr);
            //*(int *)(data[cur])=1;
            LOG_DEBUG("check result = %d %d",j,*(int *)(data[cur]));
        }else{
            if (attributes[cur]==CU_POINTER_ATTRIBUTE_IS_MANAGED){
                *(int *)(data[cur])=0;    
            }
        }
    }
    return res;
}

CUresult cuPointerSetAttribute ( const void* value, CUpointer_attribute attribute, CUdeviceptr ptr ){
    LOG_DEBUG("cuPointSetAttribue value=%p attribute=%d ptr=%llx",value,(int)attribute,ptr);
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuPointerSetAttribute,value,attribute,ptr);
    return res;
}


CUresult cuIpcCloseMemHandle(CUdeviceptr dptr){
    LOG_DEBUG("cuIpcCloseMemHandle dptr=%llx",dptr);
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuIpcCloseMemHandle,dptr);
}

CUresult cuIpcGetMemHandle ( CUipcMemHandle* pHandle, CUdeviceptr dptr ){
    LOG_DEBUG("cuIpcGetMemHandle dptr=%llx",dptr);
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuIpcGetMemHandle,pHandle,dptr);
}

CUresult cuIpcOpenMemHandle_v2 ( CUdeviceptr* pdptr, CUipcMemHandle handle, unsigned int  Flags ){
    LOG_DEBUG("cuIpcGetMemHandle dptr=%p",pdptr);
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuIpcOpenMemHandle_v2,pdptr,handle,Flags);
}


CUresult cuMemGetAddressRange_v2( CUdeviceptr* pbase, size_t* psize, CUdeviceptr dptr ){
    //TODO: Translate back
    LOG_DEBUG("cuMemGetAddressRange_v2,dptr=%llx",dptr);
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemGetAddressRange_v2,pbase,psize,dptr);
    return res;
}

CUresult cuMemcpyAsync ( CUdeviceptr dst, CUdeviceptr src, size_t ByteCount, CUstream hStream ){
    LOG_DEBUG("cuMemcpyAsync,dst=%llx src=%llx count=%lu",dst,src,ByteCount);
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpyAsync,dst,src,ByteCount,hStream);
    return res; 
}

CUresult cuMemcpyAtoD_v2( CUdeviceptr dstDevice, CUarray srcArray, size_t srcOffset, size_t ByteCount ){
    LOG_DEBUG("cuMemcpyAtoD_v2,dst=%llx count=%lu",dstDevice,ByteCount);
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpyAtoD_v2,dstDevice,srcArray,srcOffset,ByteCount);
}

CUresult cuMemcpyDtoA_v2 ( CUarray dstArray, size_t dstOffset, CUdeviceptr srcDevice, size_t ByteCount ){
    LOG_DEBUG("cuMemcpyDtoA_v2,src=%llx count=%lu",srcDevice,ByteCount);
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpyDtoA_v2,dstArray,dstOffset,srcDevice,ByteCount);
}

CUresult cuMemcpyDtoD_v2 ( CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount ){
    LOG_DEBUG("cuMemcpyDtoD_v2,dst=%llx src=%llx count=%lu",dstDevice,srcDevice,ByteCount);
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpyDtoD_v2,dstDevice,srcDevice,ByteCount);
}

CUresult cuMemcpyDtoDAsync_v2( CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream ){
    LOG_DEBUG("cuMemcpyDtoDAsync_v2,dst=%llx src=%llx count=%lu",dstDevice,srcDevice,ByteCount);
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpyDtoDAsync_v2,dstDevice,srcDevice,ByteCount,hStream);
}

CUresult cuMemcpyDtoH_v2(void* dstHost, CUdeviceptr srcDevice, size_t ByteCount) {
    // TODO: compute bytesize
    LOG_DEBUG("cuMemcpyDtoH_v2,dst=%p src=%llx count=%lu",dstHost,srcDevice,ByteCount);
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpyDtoH_v2, dstHost, srcDevice, ByteCount);
    return res;
}

CUresult cuMemcpyDtoHAsync_v2 ( void* dstHost, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream ){
    LOG_DEBUG("cuMemcpyDtoHAsync_v2,dst=%p src=%llx count=%lu",dstHost,srcDevice,ByteCount);
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpyDtoHAsync_v2,dstHost,srcDevice,ByteCount,hStream); 
}


CUresult cuMemcpyHtoD_v2(CUdeviceptr srcDevice, const void* dstHost, size_t ByteCount) {
    // TODO: compute bytesize
    LOG_DEBUG("cuMemcpyHtoD_v2,srcDevice=%llx dstHost=%p count=%lu",srcDevice,dstHost,ByteCount);
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpyHtoD_v2, srcDevice, dstHost, ByteCount);
    return res;
}

CUresult cuMemcpyHtoDAsync_v2( CUdeviceptr dstDevice, const void* srcHost, size_t ByteCount, CUstream hStream ){
    LOG_DEBUG("cuMemcpyHtoDAsync_v2,dst=%llx src=%p count=%lu",dstDevice,srcHost,ByteCount);
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpyHtoDAsync_v2,dstDevice,srcHost,ByteCount,hStream);
    return res;
}


CUresult cuMemcpyPeer(CUdeviceptr dstDevice, CUcontext dstContext, CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount) {
    LOG_DEBUG("cuMemcpyPeer dstDevice=%llx srcDevice=%llx",dstDevice,srcDevice);
    ENSURE_RUNNING();
    CUresult res=CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpyPeer,dstDevice,dstContext,srcDevice,srcContext,ByteCount);
    return res;
}

CUresult cuMemcpyPeerAsync ( CUdeviceptr dstDevice, CUcontext dstContext, CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount, CUstream hStream){
    LOG_DEBUG("into cuMemcpyPeerAsync dstDevice=%llx srcDevice=%llx",dstDevice,srcDevice);
    ENSURE_RUNNING();
    CUresult res=CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpyPeerAsync,dstDevice,dstContext,srcDevice,srcContext,ByteCount,hStream);
    return res;
}

CUresult cuMemsetD16_v2 ( CUdeviceptr dstDevice, unsigned short us, size_t N ){
    ENSURE_RUNNING();
    CUresult res=CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemsetD16_v2,dstDevice,us,N);
    return res;
}

CUresult cuMemsetD16Async ( CUdeviceptr dstDevice, unsigned short us, size_t N, CUstream hStream ){
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemsetD16Async,dstDevice,us,N,hStream);
}

CUresult cuMemsetD2D16_v2 ( CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height ){
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemsetD2D16_v2,dstDevice,dstPitch,us,Width,Height);
}

CUresult cuMemsetD2D16Async (CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height, CUstream hStream ){
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemsetD2D16Async,dstDevice,dstPitch,us,Width,Height,hStream);
}

CUresult cuMemsetD2D32_v2 ( CUdeviceptr dstDevice, size_t dstPitch, unsigned int  ui, size_t Width, size_t Height ){
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemsetD2D32_v2,dstDevice,dstPitch,ui,Width,Height);
}


CUresult cuMemsetD2D32Async ( CUdeviceptr dstDevice, size_t dstPitch, unsigned int  ui, size_t Width, size_t Height, CUstream hStream ){
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemsetD2D32Async,dstDevice,dstPitch,ui,Width,Height,hStream);
}

CUresult cuMemsetD2D8_v2 ( CUdeviceptr dstDevice, size_t dstPitch, unsigned char  uc, size_t Width, size_t Height ){
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemsetD2D8_v2,dstDevice,dstPitch,uc,Width,Height);
}

CUresult cuMemsetD2D8Async ( CUdeviceptr dstDevice, size_t dstPitch, unsigned char  uc, size_t Width, size_t Height, CUstream hStream ){
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemsetD2D8Async,dstDevice,dstPitch,uc,Width,Height,hStream);
}

CUresult cuMemsetD32_v2 ( CUdeviceptr dstDevice, unsigned int  ui, size_t N ){
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemsetD32_v2,dstDevice,ui,N);
    return res;
}

CUresult cuMemsetD32Async ( CUdeviceptr dstDevice, unsigned int  ui, size_t N, CUstream hStream ){
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemsetD32Async,dstDevice,ui,N,hStream);
}   


CUresult cuMemsetD8_v2 ( CUdeviceptr dstDevice, unsigned char  uc, size_t N ){
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemsetD8_v2,dstDevice,uc,N);
}

CUresult cuMemsetD8Async ( CUdeviceptr dstDevice, unsigned char  uc, size_t N, CUstream hStream ){
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemsetD8Async,dstDevice,uc,N,hStream);
}

CUresult cuMemAdvise( CUdeviceptr devPtr, size_t count, CUmem_advise advice, CUdevice device ){
    LOG_DEBUG("cuMemAdvise devPtr=%llx count=%lx",devPtr,count);
    ENSURE_RUNNING();
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemAdvise,devPtr,count,advice,device);
}

#ifdef HOOK_MEMINFO_ENABLE
CUresult cuMemGetInfo_v2(size_t* free, size_t* total) {
    CUdevice dev;
    LOG_DEBUG("cuMemGetInfo_v2");
    ENSURE_INITIALIZED();
    CHECK_DRV_API(cuCtxGetDevice(&dev));
    size_t usage = get_current_device_memory_usage(dev);
    size_t limit = get_current_device_memory_limit(dev);
    if (limit == 0) {
        CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemGetInfo_v2, free, total);
        //LOG_INFO("orig free=%ld total=%ld\n",*free,*total);
        LOG_INFO("orig free=%ld total=%ld",*free,*total);
        *free= *total - usage;
        LOG_INFO("after free=%ld total=%ld",*free,*total);
        return CUDA_SUCCESS;
    } else if (limit < usage) {
        LOG_WARN("limit < usage; usage=%ld, limit=%ld",usage,limit);
        return CUDA_ERROR_INVALID_VALUE;
    } else {
        CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemGetInfo_v2, free, total);
        LOG_MSG("orig free=%ld total=%ld limit=%ld usage=%ld",*free,*total,limit,usage);
        *free = limit - usage;
        *total = limit;
        return CUDA_SUCCESS;
    }
}
#endif

CUresult cuMipmappedArrayCreate(CUmipmappedArray* pHandle, 
                                          const CUDA_ARRAY3D_DESCRIPTOR* pMipmappedArrayDesc, 
                                          unsigned int numMipmapLevels) {
    // TODO: compute bytesize
    LOG_DEBUG("cuMipmappedArrayCreate\n");
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMipmappedArrayCreate, pHandle, pMipmappedArrayDesc, numMipmapLevels);
    if (res != CUDA_SUCCESS) {
        return res;
    }
    if (check_oom()) {
        CUDA_OVERRIDE_CALL(cuda_library_entry,cuMipmappedArrayDestroy, *pHandle);
        return CUDA_ERROR_OUT_OF_MEMORY;
    }
    return res;
}

CUresult cuMipmappedArrayDestroy(CUmipmappedArray hMipmappedArray) {
    // TODO: compute bytesize
    LOG_DEBUG("cuMipmappedArrayDestory\n");
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMipmappedArrayDestroy, hMipmappedArray);
    return res;
}

CUresult cuLaunchKernel ( CUfunction f, unsigned int  gridDimX, unsigned int  gridDimY, unsigned int  gridDimZ, unsigned int  blockDimX, unsigned int  blockDimY, unsigned int  blockDimZ, unsigned int  sharedMemBytes, CUstream hStream, void** kernelParams, void** extra ){
    ENSURE_RUNNING();

    if (pidfound==1){ 
        rate_limiter(gridDimX * gridDimY * gridDimZ,
                   blockDimX * blockDimY * blockDimZ);
    }
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuLaunchKernel,f,gridDimX,gridDimY,gridDimZ,blockDimX,blockDimY,blockDimZ,sharedMemBytes,hStream,kernelParams,extra);
    return res;
}


CUresult cuLaunchCooperativeKernel ( CUfunction f, unsigned int  gridDimX, unsigned int  gridDimY, unsigned int  gridDimZ, unsigned int  blockDimX, unsigned int  blockDimY, unsigned int  blockDimZ, unsigned int  sharedMemBytes, CUstream hStream, void** kernelParams ){
    ENSURE_RUNNING();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuLaunchCooperativeKernel,f,gridDimX,gridDimY,gridDimZ,blockDimX,blockDimY,blockDimZ,sharedMemBytes,hStream,kernelParams);
    return res;
}

CUresult cuMemoryFree(CUdeviceptr dptr) {
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemFree_v2,dptr);
    return res;
}

CUresult cuMemAddressReserve ( CUdeviceptr* ptr, size_t size, size_t alignment, CUdeviceptr addr, unsigned long long flags ) {
    LOG_INFO("cuMemAddressReserve:%lx %lld",size,addr);
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemAddressReserve,ptr,size,alignment,addr,flags);
    return res;
}

CUresult cuMemCreate ( CUmemGenericAllocationHandle* handle, size_t size, const CUmemAllocationProp* prop, unsigned long long flags ) {
    LOG_INFO("cuMemCreate:");
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemCreate,handle,size,prop,flags);
    return res;
}

CUresult cuMemMap( CUdeviceptr ptr, size_t size, size_t offset, CUmemGenericAllocationHandle handle, unsigned long long flags ) {
    LOG_INFO("cuMemMap");
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemMap,ptr,size,offset,handle,flags);
    return res;
}

CUresult  cuMemAllocAsync(CUdeviceptr *dptr, size_t bytesize, CUstream hStream) {
    LOG_WARN("cuMemAllocAsync:%ld",bytesize);
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemAllocAsync,dptr,bytesize,hStream);
}

CUresult cuMemHostGetDevicePointer_v2(CUdeviceptr *pdptr, void *p, unsigned int Flags){
    LOG_INFO("cuMemHostGetDevicePointer");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemHostGetDevicePointer_v2,pdptr,p,Flags);
}

CUresult cuMemHostGetFlags(unsigned int *pFlags, void *p){
    LOG_INFO("cuMemHostGetFlags");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemHostGetFlags,pFlags,p);
}

CUresult cuMemPoolTrimTo(CUmemoryPool pool, size_t minBytesToKeep){
    LOG_WARN("cuMemPoolTrimTo");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemPoolTrimTo,pool,minBytesToKeep);
}

CUresult cuMemPoolSetAttribute(CUmemoryPool pool, CUmemPool_attribute attr, void *value) {
    LOG_WARN("cuMemPoolSetAttribute");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemPoolSetAttribute,pool,attr,value);
}

CUresult cuMemPoolGetAttribute(CUmemoryPool pool, CUmemPool_attribute attr, void *value) {
    LOG_WARN("cuMemPoolGetAttribute");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemPoolGetAttribute,pool,attr,value);
}

CUresult cuMemPoolSetAccess(CUmemoryPool pool, const CUmemAccessDesc *map, size_t count) {
    LOG_WARN("cuMemPoolSetAccess");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemPoolSetAccess,pool,map,count);
}

CUresult cuMemPoolGetAccess(CUmemAccess_flags *flags, CUmemoryPool memPool, CUmemLocation *location) {
    LOG_WARN("cuMemPoolGetAccess");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemPoolGetAccess,flags,memPool,location);
}

CUresult cuMemPoolCreate(CUmemoryPool *pool, const CUmemPoolProps *poolProps) {
    LOG_WARN("cuMemPoolCreate");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemPoolCreate,pool,poolProps);
}

CUresult cuMemPoolDestroy(CUmemoryPool pool) {
    LOG_WARN("cuMemPoolDestroy");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemPoolDestroy,pool);
}

CUresult cuMemAllocFromPoolAsync(CUdeviceptr *dptr, size_t bytesize, CUmemoryPool pool, CUstream hStream) {
    LOG_WARN("cuMemAllocFromPoolAsync");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemAllocFromPoolAsync,dptr,bytesize,pool,hStream);
}

CUresult cuMemPoolExportToShareableHandle(void *handle_out, CUmemoryPool pool, CUmemAllocationHandleType handleType, unsigned long long flags) {
    LOG_WARN("cuMemPoolExportToShareableHandle");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemPoolExportToShareableHandle,handle_out,pool,handleType,flags);
}

CUresult cuMemPoolImportFromShareableHandle(
        CUmemoryPool *pool_out,
        void *handle,
        CUmemAllocationHandleType handleType,
        unsigned long long flags) {
            LOG_WARN("cuMemPoolImportFromShareableHandle");
            return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemPoolImportFromShareableHandle,pool_out,handle,handleType,flags);
        }

CUresult cuMemPoolExportPointer(CUmemPoolPtrExportData *shareData_out, CUdeviceptr ptr) {
    LOG_WARN("cuMemPoolExportPointer");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemPoolExportPointer,shareData_out,ptr);
}

CUresult cuMemPoolImportPointer(CUdeviceptr *ptr_out, CUmemoryPool pool, CUmemPoolPtrExportData *shareData) {
    LOG_WARN("cuMemPoolImportPointer");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemPoolImportPointer,ptr_out,pool,shareData);
}
/*
CUresult cuMemcpy2DUnaligned(const CUDA_MEMCPY2D *pCopy) {
    LOG_WARN("cuMemcpy2DUnaligned");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpy2DUnaligned,pCopy);
}*/

CUresult cuMemcpy2DUnaligned_v2(const CUDA_MEMCPY2D *pCopy) {
    LOG_WARN("cuMemcpy2DUnaligned_v2");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpy2DUnaligned_v2,pCopy);
}
/*
CUresult cuMemcpy2DAsync(const CUDA_MEMCPY2D *pCopy, CUstream hStream) {
    LOG_WARN("cuMemcpy2DAsync");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpy2DAsync,pCopy,hStream);
}*/

CUresult cuMemcpy2DAsync_v2(const CUDA_MEMCPY2D *pCopy, CUstream hStream) {
    LOG_WARN("cuMemcpy2DAsync_v2");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpy2DAsync,pCopy,hStream);
}

/*
CUresult cuMemcpy3D(const CUDA_MEMCPY3D *pCopy) {
    LOG_WARN("cuMemcpy3D");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpy3D,pCopy);
}*/

CUresult cuMemcpy3D_v2(const CUDA_MEMCPY3D *pCopy) {
    LOG_WARN("cuMemcpy3D_v2");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpy3D_v2,pCopy);
}
/*
CUresult cuMemcpy3DAsync(const CUDA_MEMCPY3D *pCopy, CUstream hStream) {
    LOG_WARN("cuMemcpy3DAsync");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpy3DAsync,pCopy,hStream);
}*/

CUresult cuMemcpy3DAsync_v2(const CUDA_MEMCPY3D *pCopy, CUstream hStream) {
    LOG_WARN("cuMemcpy3DAsync_v2");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpy3DAsync_v2,pCopy,hStream);
}

CUresult cuMemcpy3DPeer(const CUDA_MEMCPY3D_PEER *pCopy) {
    LOG_WARN("cuMemcpy3DPeer");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpy3DPeer,pCopy);
}

CUresult cuMemcpy3DPeerAsync(const CUDA_MEMCPY3D_PEER *pCopy, CUstream hStream) {
    LOG_WARN("cuMemcpy3DPeerAsync");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemcpy3DPeerAsync,pCopy,hStream);
}

CUresult cuMemPrefetchAsync(CUdeviceptr devPtr, size_t count, CUdevice dstDevice, CUstream hStream) {
    LOG_WARN("cuMemPrefetchAsync");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemPrefetchAsync,devPtr,count,dstDevice,hStream);
}

CUresult cuMemRangeGetAttribute(void *data, size_t dataSize, CUmem_range_attribute attribute, CUdeviceptr devPtr, size_t count) {
    LOG_WARN("cuMemRangeGetAttribute");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemRangeGetAttribute,data,dataSize,attribute,devPtr,count);
}

CUresult cuMemRangeGetAttributes(void **data, size_t *dataSizes, CUmem_range_attribute *attributes, size_t numAttributes, CUdeviceptr devPtr, size_t count) {
    LOG_WARN("cuMemRangeGetAttributes");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemRangeGetAttributes,data,dataSizes,attributes,numAttributes,devPtr,count);
}

/* External Resource Management */
CUresult cuImportExternalMemory(CUexternalMemory *extMem_out, const CUDA_EXTERNAL_MEMORY_HANDLE_DESC *memHandleDesc) {
    LOG_WARN("cuImportExternalMemory");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuImportExternalMemory,extMem_out,memHandleDesc);
}

CUresult cuExternalMemoryGetMappedBuffer(CUdeviceptr *devPtr, CUexternalMemory extMem, const CUDA_EXTERNAL_MEMORY_BUFFER_DESC *bufferDesc) {
    LOG_WARN("cuExternalMemoryGetMappedBuffer");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuExternalMemoryGetMappedBuffer,devPtr,extMem,bufferDesc);
}

CUresult cuExternalMemoryGetMappedMipmappedArray(CUmipmappedArray *mipmap, CUexternalMemory extMem, const CUDA_EXTERNAL_MEMORY_MIPMAPPED_ARRAY_DESC *mipmapDesc) {
    LOG_WARN("cuExternalMemoryGetMappedMipmappedArray");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuExternalMemoryGetMappedMipmappedArray,mipmap,extMem,mipmapDesc);
}

CUresult cuDestroyExternalMemory(CUexternalMemory extMem) {
    LOG_WARN("cuDestroyExternalMemory");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuDestroyExternalMemory,extMem);
}

CUresult cuImportExternalSemaphore(CUexternalSemaphore *extSem_out, const CUDA_EXTERNAL_SEMAPHORE_HANDLE_DESC *semHandleDesc) {
    LOG_WARN("cuImportExternalSemaphore");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuImportExternalSemaphore,extSem_out,semHandleDesc);
}

CUresult cuSignalExternalSemaphoresAsync(const CUexternalSemaphore *extSemArray, const CUDA_EXTERNAL_SEMAPHORE_SIGNAL_PARAMS *paramsArray, unsigned int numExtSems, CUstream stream) {
    LOG_WARN("cuSignalExternalSemaphoresAsync");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuSignalExternalSemaphoresAsync,extSemArray,paramsArray,numExtSems,stream);
}

CUresult cuWaitExternalSemaphoresAsync(const CUexternalSemaphore *extSemArray, const CUDA_EXTERNAL_SEMAPHORE_WAIT_PARAMS *paramsArray, unsigned int numExtSems, CUstream stream) {
    LOG_WARN("cuWaitExternalSemaphoresAsync");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuWaitExternalSemaphoresAsync,extSemArray,paramsArray,numExtSems,stream);
}

CUresult cuDestroyExternalSemaphore(CUexternalSemaphore extSem) {
    LOG_WARN("cuDestroyExternalSemaphore");
    return CUDA_OVERRIDE_CALL(cuda_library_entry,cuDestroyExternalSemaphore,extSem);
}
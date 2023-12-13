#include "include/libcuda_hook.h"
#include <string.h>
#include "include/libvgpu.h"

typedef void* (*fp_dlsym)(void*, const char*);
extern fp_dlsym real_dlsym;

cuda_entry_t cuda_library_entry[] = {
    /* Init Part    */ 
    {.name = "cuInit"},
    /* Deivce Part */
    {.name = "cuDeviceGetAttribute"},
    {.name = "cuDeviceGet"},
    {.name = "cuDeviceGetCount"},
    {.name = "cuDeviceGetName"},
    {.name = "cuDeviceCanAccessPeer"},
    {.name = "cuDeviceGetP2PAttribute"},
    {.name = "cuDeviceGetByPCIBusId"},
    {.name = "cuDeviceGetPCIBusId"},
    {.name = "cuDeviceGetUuid"},
    {.name = "cuDeviceGetDefaultMemPool"},
    {.name = "cuDeviceGetLuid"},
    {.name = "cuDeviceGetMemPool"},
    {.name = "cuDeviceTotalMem_v2"},
    {.name = "cuDriverGetVersion"},
    {.name = "cuDeviceGetTexture1DLinearMaxWidth"},
    {.name = "cuDeviceSetMemPool"},
    {.name = "cuFlushGPUDirectRDMAWrites"},

    /* Context Part */
    {.name = "cuDevicePrimaryCtxGetState"},
    {.name = "cuDevicePrimaryCtxRetain"},
    {.name = "cuDevicePrimaryCtxSetFlags_v2"},
    {.name = "cuDevicePrimaryCtxRelease_v2"},
    {.name = "cuCtxGetDevice"},
    {.name = "cuCtxCreate_v2"},
    {.name = "cuCtxDestroy_v2"},
    {.name = "cuCtxGetApiVersion"},
    {.name = "cuCtxGetCacheConfig"},
    {.name = "cuCtxGetCurrent"},
    {.name = "cuCtxGetFlags"},
    {.name = "cuCtxGetLimit"},
    {.name = "cuCtxGetSharedMemConfig"},
    {.name = "cuCtxGetStreamPriorityRange"},
    {.name = "cuCtxPopCurrent_v2"},
    {.name = "cuCtxPushCurrent_v2"},
    {.name = "cuCtxSetCacheConfig"},
    {.name = "cuCtxSetCurrent"},
    {.name = "cuCtxSetLimit"},
    {.name = "cuCtxSetSharedMemConfig"},
    {.name = "cuCtxSynchronize"},
    //{.name = "cuCtxEnablePeerAccess"},
    {.name = "cuGetExportTable"},
    /* Stream Part */
    {.name = "cuStreamCreate"},
    {.name = "cuStreamDestroy_v2"},
    {.name = "cuStreamSynchronize"},
    /* Memory Part */
    {.name = "cuArray3DCreate_v2"},
    {.name = "cuArrayCreate_v2"},
    {.name = "cuArrayDestroy"},
    {.name = "cuMemAlloc_v2"},
    {.name = "cuMemAllocHost_v2"},
    {.name = "cuMemAllocManaged"},
    {.name = "cuMemAllocPitch_v2"},
    {.name = "cuMemFree_v2"},
    {.name = "cuMemFreeHost"},
    {.name = "cuMemHostAlloc"},
    {.name = "cuMemHostRegister_v2"},
    {.name = "cuMemHostUnregister"},
    {.name = "cuMemcpyDtoH_v2"},
    {.name = "cuMemcpyHtoD_v2"},
    {.name = "cuMipmappedArrayCreate"},
    {.name = "cuMipmappedArrayDestroy"},
    {.name = "cuMemGetInfo_v2"},
    {.name = "cuMemcpy"},
    {.name = "cuPointerGetAttribute"},
    {.name = "cuPointerGetAttributes"},
    {.name = "cuPointerSetAttribute"},
    {.name = "cuIpcCloseMemHandle"},
    {.name = "cuIpcGetMemHandle"},
    {.name = "cuIpcOpenMemHandle_v2"},
    {.name = "cuMemGetAddressRange_v2"},
    {.name = "cuMemcpyAsync"},
    {.name = "cuMemcpyAtoD_v2"},
    {.name = "cuMemcpyDtoA_v2"},
    {.name = "cuMemcpyDtoD_v2"},
    {.name = "cuMemcpyDtoDAsync_v2"},
    {.name = "cuMemcpyDtoHAsync_v2"},
    {.name = "cuMemcpyHtoDAsync_v2"},
    {.name = "cuMemcpyPeer"},
    {.name = "cuMemcpyPeerAsync"},
    {.name = "cuMemsetD16_v2"},
    {.name = "cuMemsetD16Async"},
    {.name = "cuMemsetD2D16_v2"},
    {.name = "cuMemsetD2D16Async"},
    {.name = "cuMemsetD2D32_v2"},
    {.name = "cuMemsetD2D32Async"},
    {.name = "cuMemsetD2D8_v2"},
    {.name = "cuMemsetD2D8Async"},
    {.name = "cuMemsetD32_v2"},
    {.name = "cuMemsetD32Async"},
    {.name = "cuMemsetD8_v2"},
    {.name = "cuMemsetD8Async"},
    {.name = "cuMemAdvise"},
    {.name = "cuFuncSetCacheConfig"},
    {.name = "cuFuncSetSharedMemConfig"},
    {.name = "cuFuncGetAttribute"},
    {.name = "cuFuncSetAttribute"},
    {.name = "cuLaunchKernel"},
    {.name = "cuLaunchCooperativeKernel"},
    /* cuEvent Part */
    {.name = "cuEventCreate"},
    {.name = "cuEventDestroy_v2"},
    {.name = "cuModuleLoad"},
    {.name = "cuModuleLoadData"},
    {.name = "cuModuleLoadDataEx"},
    {.name = "cuModuleLoadFatBinary"},
    {.name = "cuModuleGetFunction"},
    {.name = "cuModuleUnload"},
    {.name = "cuModuleGetGlobal_v2"},
    {.name = "cuModuleGetTexRef"},
    {.name = "cuModuleGetSurfRef"},
    {.name = "cuLinkAddData_v2"},
    {.name = "cuLinkCreate_v2"},
    {.name = "cuLinkAddFile_v2"},
    {.name = "cuLinkComplete"},
    {.name = "cuLinkDestroy"},
    /* Virtual Memory Part */
    {.name = "cuMemAddressReserve"},
    {.name = "cuMemCreate"},
    {.name = "cuMemMap"},
    {.name = "cuMemAllocAsync"},
    /* cuda11.7 new api memory part */
    {.name = "cuMemHostGetDevicePointer_v2"},
    {.name = "cuMemHostGetFlags"},
    {.name = "cuMemPoolTrimTo"},
    {.name = "cuMemPoolSetAttribute"},
    {.name = "cuMemPoolGetAttribute"},
    {.name = "cuMemPoolSetAccess"},
    {.name = "cuMemPoolGetAccess"},
    {.name = "cuMemPoolCreate"},
    {.name = "cuMemPoolDestroy"},
    {.name = "cuMemAllocFromPoolAsync"},
    {.name = "cuMemPoolExportToShareableHandle"},
    {.name = "cuMemPoolImportFromShareableHandle"},
    {.name = "cuMemPoolExportPointer"},
    {.name = "cuMemPoolImportPointer"},
    {.name = "cuMemcpy2DUnaligned_v2"},
    {.name = "cuMemcpy2DAsync_v2"},
    {.name = "cuMemcpy3D_v2"},
    {.name = "cuMemcpy3DAsync_v2"},
    {.name = "cuMemcpy3DPeer"},
    {.name = "cuMemcpy3DPeerAsync"},
    {.name = "cuMemPrefetchAsync"},
    {.name = "cuMemRangeGetAttribute"},
    {.name = "cuMemRangeGetAttributes"},
    /* cuda 11.7 external resource interoperability */
    {.name = "cuImportExternalMemory"},
    {.name = "cuExternalMemoryGetMappedBuffer"},
    {.name = "cuExternalMemoryGetMappedMipmappedArray"},
    {.name = "cuDestroyExternalMemory"},
    {.name = "cuImportExternalSemaphore"},
    {.name = "cuSignalExternalSemaphoresAsync"},
    {.name = "cuWaitExternalSemaphoresAsync"},
    {.name = "cuDestroyExternalSemaphore"},
    /* Graph part */
    {.name = "cuGraphCreate"},
    {.name = "cuGraphAddKernelNode_v2"},
    {.name = "cuGraphKernelNodeGetParams_v2"},
    {.name = "cuGraphKernelNodeSetParams_v2"},
    {.name = "cuGraphAddMemcpyNode"},
    {.name = "cuGraphMemcpyNodeGetParams"},
    {.name = "cuGraphMemcpyNodeSetParams"},
    {.name = "cuGraphAddMemsetNode"},
    {.name = "cuGraphMemsetNodeGetParams"},
    {.name = "cuGraphMemsetNodeSetParams"},
    {.name = "cuGraphAddHostNode"},
    {.name = "cuGraphHostNodeGetParams"},
    {.name = "cuGraphHostNodeSetParams"},
    {.name = "cuGraphAddChildGraphNode"},
    {.name = "cuGraphChildGraphNodeGetGraph"},
    {.name = "cuGraphAddEmptyNode"},
    {.name = "cuGraphAddEventRecordNode"},
    {.name = "cuGraphEventRecordNodeGetEvent"},
    {.name = "cuGraphEventRecordNodeSetEvent"},
    {.name = "cuGraphAddEventWaitNode"},
    {.name = "cuGraphEventWaitNodeGetEvent"},
    {.name = "cuGraphEventWaitNodeSetEvent"},
    {.name = "cuGraphAddExternalSemaphoresSignalNode"},
    {.name = "cuGraphExternalSemaphoresSignalNodeGetParams"},
    {.name = "cuGraphExternalSemaphoresSignalNodeSetParams"},
    {.name = "cuGraphAddExternalSemaphoresWaitNode"},
    {.name = "cuGraphExternalSemaphoresWaitNodeGetParams"},
    {.name = "cuGraphExternalSemaphoresWaitNodeSetParams"},
    {.name = "cuGraphExecExternalSemaphoresSignalNodeSetParams"},
    {.name = "cuGraphExecExternalSemaphoresWaitNodeSetParams"},
    {.name = "cuGraphClone"},
    {.name = "cuGraphNodeFindInClone"},
    {.name = "cuGraphNodeGetType"},
    {.name = "cuGraphGetNodes"},
    {.name = "cuGraphGetRootNodes"},
    {.name = "cuGraphGetEdges"},
    {.name = "cuGraphNodeGetDependencies"},
    {.name = "cuGraphNodeGetDependentNodes"},
    {.name = "cuGraphAddDependencies"},
    {.name = "cuGraphRemoveDependencies"},
    {.name = "cuGraphDestroyNode"},
    {.name = "cuGraphInstantiate"},
    {.name = "cuGraphInstantiateWithFlags"},
    {.name = "cuGraphUpload"},
    {.name = "cuGraphLaunch"},
    {.name = "cuGraphExecDestroy"},
    {.name = "cuGraphDestroy"},

    {.name = "cuGetProcAddress"},
    {.name = "cuGetProcAddress_v2"},
};

int prior_function(char tmp[500]) {
    char *pos = tmp + strlen(tmp) - 3;
    if (pos[0]=='_' && pos[1]=='v') {
        if (pos[2]=='2')
            pos[0]='\0';
        else
            pos[2]--;
        return 1;
    }
    return 0;
}

void load_cuda_libraries() {
    void *table = NULL;
    int i = 0;
    char cuda_filename[FILENAME_MAX];
    char tmpfunc[500];

    LOG_INFO("Start hijacking");

    snprintf(cuda_filename, FILENAME_MAX - 1, "%s","libcuda.so.1");
    cuda_filename[FILENAME_MAX - 1] = '\0';

    table = dlopen(cuda_filename, RTLD_NOW | RTLD_NODELETE);
    if (!table) {
        LOG_WARN("can't find library %s", cuda_filename);
    }

    for (i = 0; i < CUDA_ENTRY_END; i++) {
        LOG_DEBUG("LOADING %s %d",cuda_library_entry[i].name,i);
        cuda_library_entry[i].fn_ptr = real_dlsym(table, cuda_library_entry[i].name);
        if (!cuda_library_entry[i].fn_ptr) {
            cuda_library_entry[i].fn_ptr=real_dlsym(RTLD_NEXT,cuda_library_entry[i].name);
            if (!cuda_library_entry[i].fn_ptr){
                LOG_INFO("can't find function %s in %s", cuda_library_entry[i].name,cuda_filename);
                memset(tmpfunc,0,500);
                strcpy(tmpfunc,cuda_library_entry[i].name);
                while (prior_function(tmpfunc)) {
                    cuda_library_entry[i].fn_ptr=real_dlsym(RTLD_NEXT,tmpfunc);
                    if (cuda_library_entry[i].fn_ptr) {
                        LOG_INFO("found prior function %s",tmpfunc);
                        break;
                    } 
                }
            }
        }
    }
    LOG_INFO("loaded_cuda_libraries");
    if (cuda_library_entry[0].fn_ptr==NULL){
        LOG_WARN("is NULL");
    }
    dlclose(table);
}


void *find_symbols_in_table(const char *symbol) {
    char symbol_v[500];
    void *pfn;
    strcpy(symbol_v,symbol);
    strcat(symbol_v,"_v3");
    pfn = __dlsym_hook_section(NULL,symbol_v);
    if (pfn!=NULL) {
        return pfn;
    }
    symbol_v[strlen(symbol_v)-1]='2';
    pfn = __dlsym_hook_section(NULL,symbol_v);
    if (pfn!=NULL) {
        return pfn;
    }
    pfn = __dlsym_hook_section(NULL,symbol);
    if (pfn!=NULL) {
        return pfn;
    }
    return NULL;
}

CUresult (*cuGetProcAddress_real) ( const char* symbol, void** pfn, int  cudaVersion, cuuint64_t flags ); 

CUresult _cuGetProcAddress ( const char* symbol, void** pfn, int  cudaVersion, cuuint64_t flags ) {
    LOG_INFO("into _cuGetProcAddress symbol=%s:%d",symbol,cudaVersion);
    *pfn = find_symbols_in_table(symbol);
    if (*pfn==NULL){
        CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuGetProcAddress,symbol,pfn,cudaVersion,flags);
        return res;
    }else{
        LOG_DEBUG("found symbol %s",symbol);
        return CUDA_SUCCESS;
    }
}

CUresult cuGetProcAddress ( const char* symbol, void** pfn, int  cudaVersion, cuuint64_t flags ) {
    LOG_INFO("into cuGetProcAddress symbol=%s:%d",symbol,cudaVersion);
    *pfn = find_symbols_in_table(symbol);
    if (strcmp(symbol,"cuGetProcAddress")==0) {
        CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuGetProcAddress,symbol,pfn,cudaVersion,flags); 
        if (res==CUDA_SUCCESS) {
            cuGetProcAddress_real=*pfn;
            *pfn=_cuGetProcAddress;
        }
        return res;
    }
    if (*pfn==NULL){
        CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuGetProcAddress,symbol,pfn,cudaVersion,flags);
        return res;
    }else{
        LOG_DEBUG("found symbol %s",symbol);
        return CUDA_SUCCESS;
    }
}

CUresult _cuGetProcAddress_v2(const char *symbol, void **pfn, int cudaVersion, cuuint64_t flags, CUdriverProcAddressQueryResult *symbolStatus){
    LOG_INFO("into _cuGetProcAddress_v2 symbol=%s:%d",symbol,cudaVersion);
    *pfn = find_symbols_in_table(symbol);
    if (*pfn==NULL){
        CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuGetProcAddress_v2,symbol,pfn,cudaVersion,flags,symbolStatus);
        return res;
    }else{
        LOG_DEBUG("found symbol %s",symbol);
        return CUDA_SUCCESS;
    } 
}

CUresult cuGetProcAddress_v2(const char *symbol, void **pfn, int cudaVersion, cuuint64_t flags, CUdriverProcAddressQueryResult *symbolStatus){
    LOG_INFO("into cuGetProcAddress_v2 symbol=%s:%d",symbol,cudaVersion);
    *pfn = find_symbols_in_table(symbol);
    if (strcmp(symbol,"cuGetProcAddress_v2")==0) {
        CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuGetProcAddress_v2,symbol,pfn,cudaVersion,flags,symbolStatus); 
        if (res==CUDA_SUCCESS) {
            cuGetProcAddress_real=*pfn;
            *pfn=_cuGetProcAddress_v2;
        }
        return res;
    }
    if (*pfn==NULL){
        CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuGetProcAddress_v2,symbol,pfn,cudaVersion,flags,symbolStatus);
        return res;
    }else{
        LOG_DEBUG("found symbol %s",symbol);
        return CUDA_SUCCESS;
    } 
}
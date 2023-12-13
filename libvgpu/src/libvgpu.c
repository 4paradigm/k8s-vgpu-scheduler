//#include "memory_limit.h"
#include <fcntl.h>
#include <dlfcn.h>
#include "include/nvml_prefix.h"
#include <nvml.h>
#include "include/nvml_prefix.h"
#include "include/cuda_addition_func.h"
#include "include/log_utils.h"
#include "include/libcuda_hook.h"
#include "include/libvgpu.h"
#include "include/utils.h"
#include "include/nvml_override.h"
#include "allocator/allocator.h"
#include "multiprocess/multiprocess_memory_limit.h"

extern void init_utilization_watcher(void);
extern void utilization_watcher(void);
extern void initial_virtual_map(void); 
extern int set_host_pid(int hostpid);
extern void allocator_init(void);
void preInit();
char *(*real_realpath)(const char *path, char *resolved_path);

pthread_once_t pre_cuinit_flag = PTHREAD_ONCE_INIT;
pthread_once_t post_cuinit_flag = PTHREAD_ONCE_INIT;
pthread_once_t dlsym_init_flag = PTHREAD_ONCE_INIT;

/* pidfound is to enable core utilization, if we don't find hostpid in container, then we have no
 where to find its core utilization */
extern int pidfound;

/* cuda_to_nvml_map indicates cuda_visible_devices, we need to map it into nvml_visible_devices, 
to let device-memory be counted successfully*/
extern int cuda_to_nvml_map[16];

/* used to switch on/off the core utilization limitation*/
extern int env_utilization_switch;

/* context size for a certain task, we need to add it into device-memory usage*/
extern int context_size;

/* This is the symbol search function */
fp_dlsym real_dlsym = NULL;

pthread_mutex_t dlsym_lock;
typedef struct {
    int tid;
    void *pointer;
}tid_dl_map;

#define DLMAP_SIZE 100
tid_dl_map dlmap[DLMAP_SIZE];
int dlmap_count=0;

void init_dlsym(){
    LOG_DEBUG("init_dlsym\n");
    pthread_mutex_init(&dlsym_lock,NULL);
    dlmap_count=0;
    memset(dlmap, 0, sizeof(tid_dl_map)*DLMAP_SIZE);
}

int check_dlmap(int tid,void *pointer){
    int i;
    int cursor = (dlmap_count < DLMAP_SIZE) ? dlmap_count : DLMAP_SIZE;
    for (i=cursor-1; i>=0; i--) {
        if ((dlmap[i].tid==tid) && (dlmap[i].pointer==pointer))
            return 1;
    }
    cursor = dlmap_count % DLMAP_SIZE;
    dlmap[cursor].tid = tid;
    dlmap[cursor].pointer = pointer;
    dlmap_count++;
    return 0;
}


FUNC_ATTR_VISIBLE void* dlsym(void* handle, const char* symbol) {
    pthread_once(&dlsym_init_flag,init_dlsym);
    LOG_DEBUG("into dlsym %s",symbol);
    if (real_dlsym == NULL) {
        real_dlsym = dlvsym(RTLD_NEXT,"dlsym","GLIBC_2.2.5");
        if (real_dlsym == NULL) {
            real_dlsym = _dl_sym(RTLD_NEXT, "dlsym", dlsym);
            if (real_dlsym == NULL)
                LOG_ERROR("real dlsym not found");
        }
    }
    if (handle == RTLD_NEXT) {
        void *h = real_dlsym(RTLD_NEXT,symbol);
        int tid;
        pthread_mutex_lock(&dlsym_lock);
        tid = pthread_self();
        if (check_dlmap(tid,h)){
            LOG_WARN("recursive dlsym : %s\n",symbol);
            h = NULL;
        }
        pthread_mutex_unlock(&dlsym_lock);
        return h;
    }
    if (symbol[0] == 'c' && symbol[1] == 'u') {
        pthread_once(&pre_cuinit_flag,(void(*)(void))preInit);
        void* f = __dlsym_hook_section(handle, symbol);
        if (f != NULL)
            return f;
    }
#ifdef HOOK_NVML_ENABLE
    if (symbol[0] == 'n' && symbol[1] == 'v' &&
          symbol[2] == 'm' && symbol[3] == 'l' ) {
        void* f = __dlsym_hook_section_nvml(handle, symbol);
        if (f != NULL) {
            return f;
        }
    }
#endif
    return real_dlsym(handle, symbol);
}

void* __dlsym_hook_section(void* handle, const char* symbol) {
    int it;
    for (it=0;it<CUDA_ENTRY_END;it++){
        if (strcmp(cuda_library_entry[it].name,symbol) == 0){
            if (cuda_library_entry[it].fn_ptr == NULL) {
                LOG_WARN("NEED TO RETURN NULL");
                return NULL;
            }else{
                break;
            }
        }
    }
    //Context
    DLSYM_HOOK_FUNC(cuCtxGetDevice);
    DLSYM_HOOK_FUNC(cuCtxCreate);
    DLSYM_HOOK_FUNC(cuDevicePrimaryCtxGetState);
    DLSYM_HOOK_FUNC(cuDevicePrimaryCtxRetain);
    DLSYM_HOOK_FUNC(cuDevicePrimaryCtxSetFlags_v2);
    DLSYM_HOOK_FUNC(cuDevicePrimaryCtxRelease_v2);
    DLSYM_HOOK_FUNC(cuDriverGetVersion);
    DLSYM_HOOK_FUNC(cuDeviceGetTexture1DLinearMaxWidth);
    DLSYM_HOOK_FUNC(cuDeviceSetMemPool);
    DLSYM_HOOK_FUNC(cuFlushGPUDirectRDMAWrites);

    DLSYM_HOOK_FUNC(cuCtxDestroy);
    DLSYM_HOOK_FUNC(cuCtxGetApiVersion);
    DLSYM_HOOK_FUNC(cuCtxGetCacheConfig);
    DLSYM_HOOK_FUNC(cuCtxGetCurrent);
    DLSYM_HOOK_FUNC(cuCtxGetDevice);
    DLSYM_HOOK_FUNC(cuCtxGetFlags);
    DLSYM_HOOK_FUNC(cuCtxGetLimit);
    DLSYM_HOOK_FUNC(cuCtxGetSharedMemConfig);
    DLSYM_HOOK_FUNC(cuCtxGetStreamPriorityRange);
    DLSYM_HOOK_FUNC(cuCtxPopCurrent);
    DLSYM_HOOK_FUNC(cuCtxPushCurrent);
    DLSYM_HOOK_FUNC(cuCtxSetCacheConfig);
    DLSYM_HOOK_FUNC(cuCtxSetCurrent);
    DLSYM_HOOK_FUNC(cuCtxSetLimit);
    DLSYM_HOOK_FUNC(cuCtxSetSharedMemConfig);
    DLSYM_HOOK_FUNC(cuCtxSynchronize);
    //DLSYM_HOOK_FUNC(cuCtxEnablePeerAccess);
    DLSYM_HOOK_FUNC(cuGetExportTable);


    DLSYM_HOOK_FUNC(cuInit);
    DLSYM_HOOK_FUNC(cuArray3DCreate_v2);
    DLSYM_HOOK_FUNC(cuArrayCreate_v2);
    DLSYM_HOOK_FUNC(cuArrayDestroy);
    DLSYM_HOOK_FUNC(cuMemAlloc_v2);
    DLSYM_HOOK_FUNC(cuMemAllocHost_v2);
    DLSYM_HOOK_FUNC(cuMemAllocManaged);
    DLSYM_HOOK_FUNC(cuMemAllocPitch_v2);
    DLSYM_HOOK_FUNC(cuMemFree_v2);
    DLSYM_HOOK_FUNC(cuMemFreeHost);
    DLSYM_HOOK_FUNC(cuMemHostAlloc);
    DLSYM_HOOK_FUNC(cuMemHostRegister_v2);
    DLSYM_HOOK_FUNC(cuMemHostUnregister);
    DLSYM_HOOK_FUNC(cuMipmappedArrayCreate);
    DLSYM_HOOK_FUNC(cuMipmappedArrayDestroy);
    DLSYM_HOOK_FUNC(cuStreamCreate);
    DLSYM_HOOK_FUNC(cuStreamDestroy_v2);
    DLSYM_HOOK_FUNC(cuStreamSynchronize);
    DLSYM_HOOK_FUNC(cuDeviceGetAttribute);
    DLSYM_HOOK_FUNC(cuDeviceGetCount);
    DLSYM_HOOK_FUNC(cuDeviceGet);
    DLSYM_HOOK_FUNC(cuDeviceGetName);
    DLSYM_HOOK_FUNC(cuDeviceCanAccessPeer);
    DLSYM_HOOK_FUNC(cuDeviceGetP2PAttribute);
    DLSYM_HOOK_FUNC(cuDeviceGetByPCIBusId);
    DLSYM_HOOK_FUNC(cuDeviceGetPCIBusId);
    DLSYM_HOOK_FUNC(cuDeviceGetDefaultMemPool);
    DLSYM_HOOK_FUNC(cuDeviceGetLuid);
    DLSYM_HOOK_FUNC(cuDeviceGetUuid);
    DLSYM_HOOK_FUNC(cuDeviceGetMemPool);
    DLSYM_HOOK_FUNC(cuDeviceTotalMem_v2);

    DLSYM_HOOK_FUNC(cuPointerGetAttributes);
    DLSYM_HOOK_FUNC(cuPointerGetAttribute);
    DLSYM_HOOK_FUNC(cuPointerSetAttribute);
    DLSYM_HOOK_FUNC(cuFuncSetCacheConfig);
    DLSYM_HOOK_FUNC(cuFuncSetSharedMemConfig);
    DLSYM_HOOK_FUNC(cuFuncGetAttribute);
    DLSYM_HOOK_FUNC(cuFuncSetAttribute);
    DLSYM_HOOK_FUNC(cuLaunchKernel);
    DLSYM_HOOK_FUNC(cuLaunchCooperativeKernel);
    DLSYM_HOOK_FUNC(cuIpcOpenMemHandle_v2);
    DLSYM_HOOK_FUNC(cuIpcGetMemHandle);
    DLSYM_HOOK_FUNC(cuIpcCloseMemHandle);
    DLSYM_HOOK_FUNC(cuMemGetAddressRange_v2);
    DLSYM_HOOK_FUNC(cuMemcpy);
    DLSYM_HOOK_FUNC(cuMemcpyPeer);
    DLSYM_HOOK_FUNC(cuMemcpyPeerAsync);
    DLSYM_HOOK_FUNC(cuMemcpyAsync);
    DLSYM_HOOK_FUNC(cuMemcpyAtoD_v2);
    DLSYM_HOOK_FUNC(cuMemcpyDtoA_v2);
    DLSYM_HOOK_FUNC(cuMemcpyDtoDAsync_v2);
    DLSYM_HOOK_FUNC(cuMemcpyDtoD_v2);
    DLSYM_HOOK_FUNC(cuMemcpyDtoDAsync_v2);
    DLSYM_HOOK_FUNC(cuMemcpyDtoH_v2);
    DLSYM_HOOK_FUNC(cuMemcpyDtoHAsync_v2);
    DLSYM_HOOK_FUNC(cuMemcpyHtoD_v2);
    DLSYM_HOOK_FUNC(cuMemcpyHtoDAsync_v2);
    DLSYM_HOOK_FUNC(cuMemsetD16_v2);
    DLSYM_HOOK_FUNC(cuMemsetD16Async);
    DLSYM_HOOK_FUNC(cuMemsetD2D16_v2);
    DLSYM_HOOK_FUNC(cuMemsetD2D16Async);
    DLSYM_HOOK_FUNC(cuMemsetD2D32_v2);
    DLSYM_HOOK_FUNC(cuMemsetD2D32Async);
    DLSYM_HOOK_FUNC(cuMemsetD2D8_v2);
    DLSYM_HOOK_FUNC(cuMemsetD2D8Async);
    DLSYM_HOOK_FUNC(cuMemsetD32_v2);
    DLSYM_HOOK_FUNC(cuMemsetD32Async);
    DLSYM_HOOK_FUNC(cuMemsetD8_v2);
    DLSYM_HOOK_FUNC(cuMemsetD8Async);
    DLSYM_HOOK_FUNC(cuMemAdvise);

    DLSYM_HOOK_FUNC(cuEventCreate);
    DLSYM_HOOK_FUNC(cuEventDestroy_v2);
    DLSYM_HOOK_FUNC(cuModuleLoad);
    DLSYM_HOOK_FUNC(cuModuleLoadData);
    DLSYM_HOOK_FUNC(cuModuleLoadDataEx);
    DLSYM_HOOK_FUNC(cuModuleLoadFatBinary);
    DLSYM_HOOK_FUNC(cuModuleGetFunction);
    DLSYM_HOOK_FUNC(cuModuleUnload);
    DLSYM_HOOK_FUNC(cuModuleGetGlobal_v2);
    DLSYM_HOOK_FUNC(cuModuleGetTexRef);
    DLSYM_HOOK_FUNC(cuModuleGetSurfRef);
    DLSYM_HOOK_FUNC(cuLinkAddData_v2);
    DLSYM_HOOK_FUNC(cuLinkCreate_v2);
    DLSYM_HOOK_FUNC(cuLinkAddFile_v2);
    DLSYM_HOOK_FUNC(cuLinkComplete);
    DLSYM_HOOK_FUNC(cuLinkDestroy);

    DLSYM_HOOK_FUNC(cuMemAddressReserve);
    DLSYM_HOOK_FUNC(cuMemCreate);
    DLSYM_HOOK_FUNC(cuMemMap);
    DLSYM_HOOK_FUNC(cuMemAllocAsync);
    DLSYM_HOOK_FUNC(cuGetProcAddress);
    DLSYM_HOOK_FUNC(cuGetProcAddress_v2);
    /* cuda 11.7 new memory ops */
    DLSYM_HOOK_FUNC(cuMemHostGetDevicePointer_v2);
    DLSYM_HOOK_FUNC(cuMemHostGetFlags);
    DLSYM_HOOK_FUNC(cuMemPoolTrimTo);
    DLSYM_HOOK_FUNC(cuMemPoolSetAttribute);
    DLSYM_HOOK_FUNC(cuMemPoolGetAttribute);
    DLSYM_HOOK_FUNC(cuMemPoolSetAccess);
    DLSYM_HOOK_FUNC(cuMemPoolGetAccess);
    DLSYM_HOOK_FUNC(cuMemPoolCreate);
    DLSYM_HOOK_FUNC(cuMemPoolDestroy);
    DLSYM_HOOK_FUNC(cuMemAllocFromPoolAsync);
    DLSYM_HOOK_FUNC(cuMemPoolExportToShareableHandle);
    DLSYM_HOOK_FUNC(cuMemPoolImportFromShareableHandle);
    DLSYM_HOOK_FUNC(cuMemPoolExportPointer);
    DLSYM_HOOK_FUNC(cuMemPoolImportPointer);
    DLSYM_HOOK_FUNC(cuMemcpy2DUnaligned_v2);
    DLSYM_HOOK_FUNC(cuMemcpy2DAsync_v2);
    DLSYM_HOOK_FUNC(cuMemcpy3D_v2);
    DLSYM_HOOK_FUNC(cuMemcpy3DAsync_v2);
    DLSYM_HOOK_FUNC(cuMemcpy3DPeer);
    DLSYM_HOOK_FUNC(cuMemcpy3DPeerAsync);
    DLSYM_HOOK_FUNC(cuMemPrefetchAsync);
    DLSYM_HOOK_FUNC(cuMemRangeGetAttribute);
    DLSYM_HOOK_FUNC(cuMemRangeGetAttributes);
    /* cuda 11.7 external resource interoperability */
    DLSYM_HOOK_FUNC(cuImportExternalMemory);
    DLSYM_HOOK_FUNC(cuExternalMemoryGetMappedBuffer);
    DLSYM_HOOK_FUNC(cuExternalMemoryGetMappedMipmappedArray);
    DLSYM_HOOK_FUNC(cuDestroyExternalMemory);
    DLSYM_HOOK_FUNC(cuImportExternalSemaphore);
    DLSYM_HOOK_FUNC(cuSignalExternalSemaphoresAsync);
    DLSYM_HOOK_FUNC(cuWaitExternalSemaphoresAsync);
    DLSYM_HOOK_FUNC(cuDestroyExternalSemaphore);
    /* cuda Graph */
    DLSYM_HOOK_FUNC(cuGraphCreate);
    DLSYM_HOOK_FUNC(cuGraphAddKernelNode_v2);
    DLSYM_HOOK_FUNC(cuGraphKernelNodeGetParams_v2);
    DLSYM_HOOK_FUNC(cuGraphKernelNodeSetParams_v2);
    DLSYM_HOOK_FUNC(cuGraphAddMemcpyNode);
    DLSYM_HOOK_FUNC(cuGraphMemcpyNodeGetParams);
    DLSYM_HOOK_FUNC(cuGraphMemcpyNodeSetParams);
    DLSYM_HOOK_FUNC(cuGraphAddMemsetNode);
    DLSYM_HOOK_FUNC(cuGraphMemsetNodeGetParams);
    DLSYM_HOOK_FUNC(cuGraphMemsetNodeSetParams);
    DLSYM_HOOK_FUNC(cuGraphAddHostNode);
    DLSYM_HOOK_FUNC(cuGraphHostNodeGetParams);
    DLSYM_HOOK_FUNC(cuGraphHostNodeSetParams);
    DLSYM_HOOK_FUNC(cuGraphAddChildGraphNode);
    DLSYM_HOOK_FUNC(cuGraphChildGraphNodeGetGraph);
    DLSYM_HOOK_FUNC(cuGraphAddEmptyNode);
    DLSYM_HOOK_FUNC(cuGraphAddEventRecordNode);
    DLSYM_HOOK_FUNC(cuGraphEventRecordNodeGetEvent);
    DLSYM_HOOK_FUNC(cuGraphEventRecordNodeSetEvent);
    DLSYM_HOOK_FUNC(cuGraphAddEventWaitNode);
    DLSYM_HOOK_FUNC(cuGraphEventWaitNodeGetEvent);
    DLSYM_HOOK_FUNC(cuGraphEventWaitNodeSetEvent);
    DLSYM_HOOK_FUNC(cuGraphAddExternalSemaphoresSignalNode);
    DLSYM_HOOK_FUNC(cuGraphExternalSemaphoresSignalNodeGetParams);
    DLSYM_HOOK_FUNC(cuGraphExternalSemaphoresSignalNodeSetParams);
    DLSYM_HOOK_FUNC(cuGraphAddExternalSemaphoresWaitNode);
    DLSYM_HOOK_FUNC(cuGraphExternalSemaphoresWaitNodeGetParams);
    DLSYM_HOOK_FUNC(cuGraphExternalSemaphoresWaitNodeSetParams);
    DLSYM_HOOK_FUNC(cuGraphExecExternalSemaphoresSignalNodeSetParams);
    DLSYM_HOOK_FUNC(cuGraphExecExternalSemaphoresWaitNodeSetParams);
    DLSYM_HOOK_FUNC(cuGraphClone);
    DLSYM_HOOK_FUNC(cuGraphNodeFindInClone);
    DLSYM_HOOK_FUNC(cuGraphNodeGetType);
    DLSYM_HOOK_FUNC(cuGraphGetNodes);
    DLSYM_HOOK_FUNC(cuGraphGetRootNodes);
    DLSYM_HOOK_FUNC(cuGraphGetEdges);
    DLSYM_HOOK_FUNC(cuGraphNodeGetDependencies);
    DLSYM_HOOK_FUNC(cuGraphNodeGetDependentNodes);
    DLSYM_HOOK_FUNC(cuGraphAddDependencies);
    DLSYM_HOOK_FUNC(cuGraphRemoveDependencies);
    DLSYM_HOOK_FUNC(cuGraphDestroyNode);
    DLSYM_HOOK_FUNC(cuGraphInstantiate);
    DLSYM_HOOK_FUNC(cuGraphInstantiateWithFlags);
    DLSYM_HOOK_FUNC(cuGraphUpload);
    DLSYM_HOOK_FUNC(cuGraphLaunch);
    DLSYM_HOOK_FUNC(cuGraphExecDestroy);
    DLSYM_HOOK_FUNC(cuGraphDestroy);
#ifdef HOOK_MEMINFO_ENABLE
    DLSYM_HOOK_FUNC(cuMemGetInfo_v2);
#endif
    return NULL;
}

void* __dlsym_hook_section_nvml(void* handle, const char* symbol) {
    DLSYM_HOOK_FUNC(nvmlInit);
    /** nvmlShutdown */
    DLSYM_HOOK_FUNC(nvmlShutdown);
    /** nvmlErrorString */
    DLSYM_HOOK_FUNC(nvmlErrorString);
    /** nvmlDeviceGetHandleByIndex */
    DLSYM_HOOK_FUNC(nvmlDeviceGetHandleByIndex);
    /** nvmlDeviceGetComputeRunningProcesses */
    DLSYM_HOOK_FUNC(nvmlDeviceGetComputeRunningProcesses);
    /** nvmlDeviceGetPciInfo */
    DLSYM_HOOK_FUNC(nvmlDeviceGetPciInfo);
    /** nvmlDeviceGetProcessUtilization */
    DLSYM_HOOK_FUNC(nvmlDeviceGetProcessUtilization);
    /** nvmlDeviceGetCount */
    DLSYM_HOOK_FUNC(nvmlDeviceGetCount);
    /** nvmlDeviceClearAccountingPids */
    DLSYM_HOOK_FUNC(nvmlDeviceClearAccountingPids);
    /** nvmlDeviceClearCpuAffinity */
    DLSYM_HOOK_FUNC(nvmlDeviceClearCpuAffinity);
    /** nvmlDeviceClearEccErrorCounts */
    DLSYM_HOOK_FUNC(nvmlDeviceClearEccErrorCounts);
    /** nvmlDeviceDiscoverGpus */
    DLSYM_HOOK_FUNC(nvmlDeviceDiscoverGpus);
    /** nvmlDeviceFreezeNvLinkUtilizationCounter */
    DLSYM_HOOK_FUNC(nvmlDeviceFreezeNvLinkUtilizationCounter);
    /** nvmlDeviceGetAccountingBufferSize */
    DLSYM_HOOK_FUNC(nvmlDeviceGetAccountingBufferSize);
    /** nvmlDeviceGetAccountingMode */
    DLSYM_HOOK_FUNC(nvmlDeviceGetAccountingMode);
    /** nvmlDeviceGetAccountingPids */
    DLSYM_HOOK_FUNC(nvmlDeviceGetAccountingPids);
    /** nvmlDeviceGetAccountingStats */
    DLSYM_HOOK_FUNC(nvmlDeviceGetAccountingStats);
    /** nvmlDeviceGetActiveVgpus */
    DLSYM_HOOK_FUNC(nvmlDeviceGetActiveVgpus);
    /** nvmlDeviceGetAPIRestriction */
    DLSYM_HOOK_FUNC(nvmlDeviceGetAPIRestriction);
    /** nvmlDeviceGetApplicationsClock */
    DLSYM_HOOK_FUNC(nvmlDeviceGetApplicationsClock);
    /** nvmlDeviceGetAutoBoostedClocksEnabled */
    DLSYM_HOOK_FUNC(nvmlDeviceGetAutoBoostedClocksEnabled);
    /** nvmlDeviceGetBAR1MemoryInfo */
    DLSYM_HOOK_FUNC(nvmlDeviceGetBAR1MemoryInfo);
    /** nvmlDeviceGetBoardId */
    DLSYM_HOOK_FUNC(nvmlDeviceGetBoardId);
    /** nvmlDeviceGetBoardPartNumber */
    DLSYM_HOOK_FUNC(nvmlDeviceGetBoardPartNumber);
    /** nvmlDeviceGetBrand */
    DLSYM_HOOK_FUNC(nvmlDeviceGetBrand);
    /** nvmlDeviceGetBridgeChipInfo */
    DLSYM_HOOK_FUNC(nvmlDeviceGetBridgeChipInfo);
    /** nvmlDeviceGetClock */
    DLSYM_HOOK_FUNC(nvmlDeviceGetClock);
    /** nvmlDeviceGetClockInfo */
    DLSYM_HOOK_FUNC(nvmlDeviceGetClockInfo);
    /** nvmlDeviceGetComputeMode */
    DLSYM_HOOK_FUNC(nvmlDeviceGetComputeMode);
    /** nvmlDeviceGetCount_v2 */
    DLSYM_HOOK_FUNC(nvmlDeviceGetCount_v2);
    /** nvmlDeviceGetCpuAffinity */
    DLSYM_HOOK_FUNC(nvmlDeviceGetCpuAffinity);
    /** nvmlDeviceGetCreatableVgpus */
    DLSYM_HOOK_FUNC(nvmlDeviceGetCreatableVgpus);
    /** nvmlDeviceGetCudaComputeCapability */
    DLSYM_HOOK_FUNC(nvmlDeviceGetCudaComputeCapability);
    /** nvmlDeviceGetCurrentClocksThrottleReasons */
    DLSYM_HOOK_FUNC(nvmlDeviceGetCurrentClocksThrottleReasons);
    /** nvmlDeviceGetCurrPcieLinkGeneration */
    DLSYM_HOOK_FUNC(nvmlDeviceGetCurrPcieLinkGeneration);
    /** nvmlDeviceGetCurrPcieLinkWidth */
    DLSYM_HOOK_FUNC(nvmlDeviceGetCurrPcieLinkWidth);
    /** nvmlDeviceGetDecoderUtilization */
    DLSYM_HOOK_FUNC(nvmlDeviceGetDecoderUtilization);
    /** nvmlDeviceGetDefaultApplicationsClock */
    DLSYM_HOOK_FUNC(nvmlDeviceGetDefaultApplicationsClock);
    /** nvmlDeviceGetDetailedEccErrors */
    DLSYM_HOOK_FUNC(nvmlDeviceGetDetailedEccErrors);
    /** nvmlDeviceGetDisplayActive */
    DLSYM_HOOK_FUNC(nvmlDeviceGetDisplayActive);
    /** nvmlDeviceGetDisplayMode */
    DLSYM_HOOK_FUNC(nvmlDeviceGetDisplayMode);
    /** nvmlDeviceGetDriverModel */
    DLSYM_HOOK_FUNC(nvmlDeviceGetDriverModel);
    /** nvmlDeviceGetEccMode */
    DLSYM_HOOK_FUNC(nvmlDeviceGetEccMode);
    /** nvmlDeviceGetEncoderCapacity */
    DLSYM_HOOK_FUNC(nvmlDeviceGetEncoderCapacity);
    /** nvmlDeviceGetEncoderSessions */
    DLSYM_HOOK_FUNC(nvmlDeviceGetEncoderSessions);
    /** nvmlDeviceGetEncoderStats */
    DLSYM_HOOK_FUNC(nvmlDeviceGetEncoderStats);
    /** nvmlDeviceGetEncoderUtilization */
    DLSYM_HOOK_FUNC(nvmlDeviceGetEncoderUtilization);
    /** nvmlDeviceGetEnforcedPowerLimit */
    DLSYM_HOOK_FUNC(nvmlDeviceGetEnforcedPowerLimit);
    /** nvmlDeviceGetFanSpeed */
    DLSYM_HOOK_FUNC(nvmlDeviceGetFanSpeed);
    /** nvmlDeviceGetFanSpeed_v2 */
    DLSYM_HOOK_FUNC(nvmlDeviceGetFanSpeed_v2);
    /** nvmlDeviceGetFieldValues */
    DLSYM_HOOK_FUNC(nvmlDeviceGetFieldValues);
    /** nvmlDeviceGetGpuOperationMode */
    DLSYM_HOOK_FUNC(nvmlDeviceGetGpuOperationMode);
    /** nvmlDeviceGetGraphicsRunningProcesses */
    DLSYM_HOOK_FUNC(nvmlDeviceGetGraphicsRunningProcesses);
    /** nvmlDeviceGetGridLicensableFeatures */
    DLSYM_HOOK_FUNC(nvmlDeviceGetGridLicensableFeatures);
    /** nvmlDeviceGetHandleByIndex_v2 */
    DLSYM_HOOK_FUNC(nvmlDeviceGetHandleByIndex_v2);
    /** nvmlDeviceGetHandleByPciBusId */
    DLSYM_HOOK_FUNC(nvmlDeviceGetHandleByPciBusId);
    /** nvmlDeviceGetHandleByPciBusId_v2 */
    DLSYM_HOOK_FUNC(nvmlDeviceGetHandleByPciBusId_v2);
    /** nvmlDeviceGetHandleBySerial */
    DLSYM_HOOK_FUNC(nvmlDeviceGetHandleBySerial);
    /** nvmlDeviceGetHandleByUUID */
    DLSYM_HOOK_FUNC(nvmlDeviceGetHandleByUUID);
    /** nvmlDeviceGetIndex */
    DLSYM_HOOK_FUNC(nvmlDeviceGetIndex);
    /** nvmlDeviceGetInforomConfigurationChecksum */
    DLSYM_HOOK_FUNC(nvmlDeviceGetInforomConfigurationChecksum);
    /** nvmlDeviceGetInforomImageVersion */
    DLSYM_HOOK_FUNC(nvmlDeviceGetInforomImageVersion);
    /** nvmlDeviceGetInforomVersion */
    DLSYM_HOOK_FUNC(nvmlDeviceGetInforomVersion);
    /** nvmlDeviceGetMaxClockInfo */
    DLSYM_HOOK_FUNC(nvmlDeviceGetMaxClockInfo);
    /** nvmlDeviceGetMaxCustomerBoostClock */
    DLSYM_HOOK_FUNC(nvmlDeviceGetMaxCustomerBoostClock);
    /** nvmlDeviceGetMaxPcieLinkGeneration */
    DLSYM_HOOK_FUNC(nvmlDeviceGetMaxPcieLinkGeneration);
    /** nvmlDeviceGetMaxPcieLinkWidth */
    DLSYM_HOOK_FUNC(nvmlDeviceGetMaxPcieLinkWidth);
    /** nvmlDeviceGetMemoryErrorCounter */
    DLSYM_HOOK_FUNC(nvmlDeviceGetMemoryErrorCounter);
    /** nvmlDeviceGetMemoryInfo */
    DLSYM_HOOK_FUNC(nvmlDeviceGetMemoryInfo);
    /** nvmlDeviceGetMemoryInfo_v2*/
    DLSYM_HOOK_FUNC(nvmlDeviceGetMemoryInfo_v2);
    /** nvmlDeviceGetMinorNumber */
    DLSYM_HOOK_FUNC(nvmlDeviceGetMinorNumber);
    /** nvmlDeviceGetMPSComputeRunningProcesses */
    DLSYM_HOOK_FUNC(nvmlDeviceGetMPSComputeRunningProcesses);
    /** nvmlDeviceGetMultiGpuBoard */
    DLSYM_HOOK_FUNC(nvmlDeviceGetMultiGpuBoard);
    /** nvmlDeviceGetName */
    DLSYM_HOOK_FUNC(nvmlDeviceGetName);
    /** nvmlDeviceGetNvLinkCapability */
    DLSYM_HOOK_FUNC(nvmlDeviceGetNvLinkCapability);
    /** nvmlDeviceGetNvLinkErrorCounter */
    DLSYM_HOOK_FUNC(nvmlDeviceGetNvLinkErrorCounter);
    /** nvmlDeviceGetNvLinkRemotePciInfo */
    DLSYM_HOOK_FUNC(nvmlDeviceGetNvLinkRemotePciInfo);
    /** nvmlDeviceGetNvLinkRemotePciInfo_v2 */
    DLSYM_HOOK_FUNC(nvmlDeviceGetNvLinkRemotePciInfo_v2);
    /** nvmlDeviceGetNvLinkState */
    DLSYM_HOOK_FUNC(nvmlDeviceGetNvLinkState);
    /** nvmlDeviceGetNvLinkUtilizationControl */
    DLSYM_HOOK_FUNC(nvmlDeviceGetNvLinkUtilizationControl);
    /** nvmlDeviceGetNvLinkUtilizationCounter */
    DLSYM_HOOK_FUNC(nvmlDeviceGetNvLinkUtilizationCounter);
    /** nvmlDeviceGetNvLinkVersion */
    DLSYM_HOOK_FUNC(nvmlDeviceGetNvLinkVersion);
    /** nvmlDeviceGetP2PStatus */
    DLSYM_HOOK_FUNC(nvmlDeviceGetP2PStatus);
    /** nvmlDeviceGetPcieReplayCounter */
    DLSYM_HOOK_FUNC(nvmlDeviceGetPcieReplayCounter);
    /** nvmlDeviceGetPcieThroughput */
    DLSYM_HOOK_FUNC(nvmlDeviceGetPcieThroughput);
    /** nvmlDeviceGetPciInfo_v2 */
    DLSYM_HOOK_FUNC(nvmlDeviceGetPciInfo_v2);
    /** nvmlDeviceGetPciInfo_v3 */
    DLSYM_HOOK_FUNC(nvmlDeviceGetPciInfo_v3);
    /** nvmlDeviceGetPerformanceState */
    DLSYM_HOOK_FUNC(nvmlDeviceGetPerformanceState);
    /** nvmlDeviceGetPersistenceMode */
    DLSYM_HOOK_FUNC(nvmlDeviceGetPersistenceMode);
    /** nvmlDeviceGetPowerManagementDefaultLimit */
    DLSYM_HOOK_FUNC(nvmlDeviceGetPowerManagementDefaultLimit);
    /** nvmlDeviceGetPowerManagementLimit */
    DLSYM_HOOK_FUNC(nvmlDeviceGetPowerManagementLimit);
    /** nvmlDeviceGetPowerManagementLimitConstraints */
    DLSYM_HOOK_FUNC(nvmlDeviceGetPowerManagementLimitConstraints);
    /** nvmlDeviceGetPowerManagementMode */
    DLSYM_HOOK_FUNC(nvmlDeviceGetPowerManagementMode);
    /** nvmlDeviceGetPowerState */
    DLSYM_HOOK_FUNC(nvmlDeviceGetPowerState);
    /** nvmlDeviceGetPowerUsage */
    DLSYM_HOOK_FUNC(nvmlDeviceGetPowerUsage);
    /** nvmlDeviceGetRetiredPages */
    DLSYM_HOOK_FUNC(nvmlDeviceGetRetiredPages);
    /** nvmlDeviceGetRetiredPagesPendingStatus */
    DLSYM_HOOK_FUNC(nvmlDeviceGetRetiredPagesPendingStatus);
    /** nvmlDeviceGetSamples */
    DLSYM_HOOK_FUNC(nvmlDeviceGetSamples);
    /** nvmlDeviceGetSerial */
    DLSYM_HOOK_FUNC(nvmlDeviceGetSerial);
    /** nvmlDeviceGetSupportedClocksThrottleReasons */
    DLSYM_HOOK_FUNC(nvmlDeviceGetSupportedClocksThrottleReasons);
    /** nvmlDeviceGetSupportedEventTypes */
    DLSYM_HOOK_FUNC(nvmlDeviceGetSupportedEventTypes);
    /** nvmlDeviceGetSupportedGraphicsClocks */
    DLSYM_HOOK_FUNC(nvmlDeviceGetSupportedGraphicsClocks);
    /** nvmlDeviceGetSupportedMemoryClocks */
    DLSYM_HOOK_FUNC(nvmlDeviceGetSupportedMemoryClocks);
    /** nvmlDeviceGetSupportedVgpus */
    DLSYM_HOOK_FUNC(nvmlDeviceGetSupportedVgpus);
    /** nvmlDeviceGetTemperature */
    DLSYM_HOOK_FUNC(nvmlDeviceGetTemperature);
    /** nvmlDeviceGetTemperatureThreshold */
    DLSYM_HOOK_FUNC(nvmlDeviceGetTemperatureThreshold);
    /** nvmlDeviceGetTopologyCommonAncestor */
    DLSYM_HOOK_FUNC(nvmlDeviceGetTopologyCommonAncestor);
    /** nvmlDeviceGetTopologyNearestGpus */
    DLSYM_HOOK_FUNC(nvmlDeviceGetTopologyNearestGpus);
    /** nvmlDeviceGetTotalEccErrors */
    DLSYM_HOOK_FUNC(nvmlDeviceGetTotalEccErrors);
    /** nvmlDeviceGetTotalEnergyConsumption */
    DLSYM_HOOK_FUNC(nvmlDeviceGetTotalEnergyConsumption);
    /** nvmlDeviceGetUtilizationRates */
    DLSYM_HOOK_FUNC(nvmlDeviceGetUtilizationRates);
    /** nvmlDeviceGetUUID */
    DLSYM_HOOK_FUNC(nvmlDeviceGetUUID);
    /** nvmlDeviceGetVbiosVersion */
    DLSYM_HOOK_FUNC(nvmlDeviceGetVbiosVersion);
    /** nvmlDeviceGetVgpuMetadata */
    DLSYM_HOOK_FUNC(nvmlDeviceGetVgpuMetadata);
    /** nvmlDeviceGetVgpuProcessUtilization */
    DLSYM_HOOK_FUNC(nvmlDeviceGetVgpuProcessUtilization);
    /** nvmlDeviceGetVgpuUtilization */
    DLSYM_HOOK_FUNC(nvmlDeviceGetVgpuUtilization);
    /** nvmlDeviceGetViolationStatus */
    DLSYM_HOOK_FUNC(nvmlDeviceGetViolationStatus);
    /** nvmlDeviceGetVirtualizationMode */
    DLSYM_HOOK_FUNC(nvmlDeviceGetVirtualizationMode);
    /** nvmlDeviceModifyDrainState */
    DLSYM_HOOK_FUNC(nvmlDeviceModifyDrainState);
    /** nvmlDeviceOnSameBoard */
    DLSYM_HOOK_FUNC(nvmlDeviceOnSameBoard);
    /** nvmlDeviceQueryDrainState */
    DLSYM_HOOK_FUNC(nvmlDeviceQueryDrainState);
    /** nvmlDeviceRegisterEvents */
    DLSYM_HOOK_FUNC(nvmlDeviceRegisterEvents);
    /** nvmlDeviceRemoveGpu */
    DLSYM_HOOK_FUNC(nvmlDeviceRemoveGpu);
    /** nvmlDeviceRemoveGpu_v2 */
    DLSYM_HOOK_FUNC(nvmlDeviceRemoveGpu_v2);
    /** nvmlDeviceResetApplicationsClocks */
    DLSYM_HOOK_FUNC(nvmlDeviceResetApplicationsClocks);
    /** nvmlDeviceResetNvLinkErrorCounters */
    DLSYM_HOOK_FUNC(nvmlDeviceResetNvLinkErrorCounters);
    /** nvmlDeviceResetNvLinkUtilizationCounter */
    DLSYM_HOOK_FUNC(nvmlDeviceResetNvLinkUtilizationCounter);
    /** nvmlDeviceSetAccountingMode */
    DLSYM_HOOK_FUNC(nvmlDeviceSetAccountingMode);
    /** nvmlDeviceSetAPIRestriction */
    DLSYM_HOOK_FUNC(nvmlDeviceSetAPIRestriction);
    /** nvmlDeviceSetApplicationsClocks */
    DLSYM_HOOK_FUNC(nvmlDeviceSetApplicationsClocks);
    /** nvmlDeviceSetAutoBoostedClocksEnabled */
    DLSYM_HOOK_FUNC(nvmlDeviceSetAutoBoostedClocksEnabled);
    /** nvmlDeviceSetComputeMode */
    DLSYM_HOOK_FUNC(nvmlDeviceSetComputeMode);
    /** nvmlDeviceSetCpuAffinity */
    DLSYM_HOOK_FUNC(nvmlDeviceSetCpuAffinity);
    /** nvmlDeviceSetDefaultAutoBoostedClocksEnabled */
    DLSYM_HOOK_FUNC(nvmlDeviceSetDefaultAutoBoostedClocksEnabled);
    /** nvmlDeviceSetDriverModel */
    DLSYM_HOOK_FUNC(nvmlDeviceSetDriverModel);
    /** nvmlDeviceSetEccMode */
    DLSYM_HOOK_FUNC(nvmlDeviceSetEccMode);
    /** nvmlDeviceSetGpuOperationMode */
    DLSYM_HOOK_FUNC(nvmlDeviceSetGpuOperationMode);
    /** nvmlDeviceSetNvLinkUtilizationControl */
    DLSYM_HOOK_FUNC(nvmlDeviceSetNvLinkUtilizationControl);
    /** nvmlDeviceSetPersistenceMode */
    DLSYM_HOOK_FUNC(nvmlDeviceSetPersistenceMode);
    /** nvmlDeviceSetPowerManagementLimit */
    DLSYM_HOOK_FUNC(nvmlDeviceSetPowerManagementLimit);
    /** nvmlDeviceSetVirtualizationMode */
    DLSYM_HOOK_FUNC(nvmlDeviceSetVirtualizationMode);
    /** nvmlDeviceValidateInforom */
    DLSYM_HOOK_FUNC(nvmlDeviceValidateInforom);
    /** nvmlEventSetCreate */
    DLSYM_HOOK_FUNC(nvmlEventSetCreate);
    /** nvmlEventSetFree */
    DLSYM_HOOK_FUNC(nvmlEventSetFree);
    /** nvmlEventSetWait */
    DLSYM_HOOK_FUNC(nvmlEventSetWait);
    /** nvmlGetVgpuCompatibility */
    DLSYM_HOOK_FUNC(nvmlGetVgpuCompatibility);
    /** nvmlInit_v2 */
    DLSYM_HOOK_FUNC(nvmlInit_v2);
    /** nvmlInitWithFlags */
    DLSYM_HOOK_FUNC(nvmlInitWithFlags);
    /** nvmlInternalGetExportTable */
    DLSYM_HOOK_FUNC(nvmlInternalGetExportTable);
    /** nvmlSystemGetCudaDriverVersion */
    DLSYM_HOOK_FUNC(nvmlSystemGetCudaDriverVersion);
    /** nvmlSystemGetCudaDriverVersion_v2 */
    DLSYM_HOOK_FUNC(nvmlSystemGetCudaDriverVersion_v2);
    /** nvmlSystemGetDriverVersion */
    DLSYM_HOOK_FUNC(nvmlSystemGetDriverVersion);
    /** nvmlSystemGetHicVersion */
    DLSYM_HOOK_FUNC(nvmlSystemGetHicVersion);
    /** nvmlSystemGetNVMLVersion */
    DLSYM_HOOK_FUNC(nvmlSystemGetNVMLVersion);
    /** nvmlSystemGetProcessName */
    DLSYM_HOOK_FUNC(nvmlSystemGetProcessName);
    /** nvmlSystemGetTopologyGpuSet */
    DLSYM_HOOK_FUNC(nvmlSystemGetTopologyGpuSet);
    /** nvmlUnitGetCount */
    DLSYM_HOOK_FUNC(nvmlUnitGetCount);
    /** nvmlUnitGetDevices */
    DLSYM_HOOK_FUNC(nvmlUnitGetDevices);
    /** nvmlUnitGetFanSpeedInfo */
    DLSYM_HOOK_FUNC(nvmlUnitGetFanSpeedInfo);
    /** nvmlUnitGetHandleByIndex */
    DLSYM_HOOK_FUNC(nvmlUnitGetHandleByIndex);
    /** nvmlUnitGetLedState */
    DLSYM_HOOK_FUNC(nvmlUnitGetLedState);
    /** nvmlUnitGetPsuInfo */
    DLSYM_HOOK_FUNC(nvmlUnitGetPsuInfo);
    /** nvmlUnitGetTemperature */
    DLSYM_HOOK_FUNC(nvmlUnitGetTemperature);
    /** nvmlUnitGetUnitInfo */
    DLSYM_HOOK_FUNC(nvmlUnitGetUnitInfo);
    /** nvmlUnitSetLedState */
    DLSYM_HOOK_FUNC(nvmlUnitSetLedState);
    /** nvmlVgpuInstanceGetEncoderCapacity */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetEncoderCapacity);
    /** nvmlVgpuInstanceGetEncoderSessions */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetEncoderSessions);
    /** nvmlVgpuInstanceGetEncoderStats */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetEncoderStats);
    /** nvmlVgpuInstanceGetFbUsage */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetFbUsage);
    /** nvmlVgpuInstanceGetFrameRateLimit */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetFrameRateLimit);
    /** nvmlVgpuInstanceGetLicenseStatus */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetLicenseStatus);
    /** nvmlVgpuInstanceGetMetadata */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetMetadata);
    /** nvmlVgpuInstanceGetType */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetType);
    /** nvmlVgpuInstanceGetUUID */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetUUID);
    /** nvmlVgpuInstanceGetVmDriverVersion */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetVmDriverVersion);
    /** nvmlVgpuInstanceGetVmID */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetVmID);
    /** nvmlVgpuInstanceSetEncoderCapacity */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceSetEncoderCapacity);
    /** nvmlVgpuTypeGetClass */
    DLSYM_HOOK_FUNC(nvmlVgpuTypeGetClass);
    /** nvmlVgpuTypeGetDeviceID */
    DLSYM_HOOK_FUNC(nvmlVgpuTypeGetDeviceID);
    /** nvmlVgpuTypeGetFramebufferSize */
    DLSYM_HOOK_FUNC(nvmlVgpuTypeGetFramebufferSize);
    /** nvmlVgpuTypeGetFrameRateLimit */
    DLSYM_HOOK_FUNC(nvmlVgpuTypeGetFrameRateLimit);
    /** nvmlVgpuTypeGetLicense */
    DLSYM_HOOK_FUNC(nvmlVgpuTypeGetLicense);
    /** nvmlVgpuTypeGetMaxInstances */
    DLSYM_HOOK_FUNC(nvmlVgpuTypeGetMaxInstances);
    /** nvmlVgpuTypeGetName */
    DLSYM_HOOK_FUNC(nvmlVgpuTypeGetName);
    /** nvmlVgpuTypeGetNumDisplayHeads */
    DLSYM_HOOK_FUNC(nvmlVgpuTypeGetNumDisplayHeads);
    /** nvmlVgpuTypeGetResolution */
    DLSYM_HOOK_FUNC(nvmlVgpuTypeGetResolution);
    /** nvmlDeviceGetFBCSessions */
    DLSYM_HOOK_FUNC(nvmlDeviceGetFBCSessions);
    /** nvmlDeviceGetFBCStats */
    DLSYM_HOOK_FUNC(nvmlDeviceGetFBCStats);
    /** nvmlDeviceGetGridLicensableFeatures_v2 */
    DLSYM_HOOK_FUNC(nvmlDeviceGetGridLicensableFeatures_v2);
    /** nvmlDeviceGetRetiredPages_v2 */
    DLSYM_HOOK_FUNC(nvmlDeviceGetRetiredPages_v2);
    /** nvmlDeviceResetGpuLockedClocks */
    DLSYM_HOOK_FUNC(nvmlDeviceResetGpuLockedClocks);
    /** nvmlDeviceSetGpuLockedClocks */
    DLSYM_HOOK_FUNC(nvmlDeviceSetGpuLockedClocks);
    /** nvmlVgpuInstanceGetAccountingMode */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetAccountingMode);
    /** nvmlVgpuInstanceGetAccountingPids */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetAccountingPids);
    /** nvmlVgpuInstanceGetAccountingStats */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetAccountingStats);
    /** nvmlVgpuInstanceGetFBCSessions */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetFBCSessions);
    /** nvmlVgpuInstanceGetFBCStats */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetFBCStats);
    /** nvmlVgpuTypeGetMaxInstancesPerVm */
    DLSYM_HOOK_FUNC(nvmlVgpuTypeGetMaxInstancesPerVm);
    /** nvmlGetVgpuVersion */
    DLSYM_HOOK_FUNC(nvmlGetVgpuVersion);
    /** nvmlSetVgpuVersion */
    DLSYM_HOOK_FUNC(nvmlSetVgpuVersion);
    /** nvmlDeviceGetGridLicensableFeatures_v3 */
    DLSYM_HOOK_FUNC(nvmlDeviceGetGridLicensableFeatures_v3);
    /** nvmlDeviceGetHostVgpuMode */
    DLSYM_HOOK_FUNC(nvmlDeviceGetHostVgpuMode);
    /** nvmlDeviceGetPgpuMetadataString */
    DLSYM_HOOK_FUNC(nvmlDeviceGetPgpuMetadataString);
    /** nvmlVgpuInstanceGetEccMode */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetEccMode);
    /** nvmlComputeInstanceDestroy */
    DLSYM_HOOK_FUNC(nvmlComputeInstanceDestroy);
    /** nvmlComputeInstanceGetInfo */
    DLSYM_HOOK_FUNC(nvmlComputeInstanceGetInfo);
    /** nvmlDeviceCreateGpuInstance */
    DLSYM_HOOK_FUNC(nvmlDeviceCreateGpuInstance);
    /** nvmlDeviceGetArchitecture */
    DLSYM_HOOK_FUNC(nvmlDeviceGetArchitecture);
    /** nvmlDeviceGetAttributes */
    DLSYM_HOOK_FUNC(nvmlDeviceGetAttributes);
    /** nvmlDeviceGetAttributes_v2 */
    DLSYM_HOOK_FUNC(nvmlDeviceGetAttributes_v2);
    /** nvmlDeviceGetComputeInstanceId */
    DLSYM_HOOK_FUNC(nvmlDeviceGetComputeInstanceId);
    /** nvmlDeviceGetCpuAffinityWithinScope */
    DLSYM_HOOK_FUNC(nvmlDeviceGetCpuAffinityWithinScope);
    /** nvmlDeviceGetDeviceHandleFromMigDeviceHandle */
    DLSYM_HOOK_FUNC(nvmlDeviceGetDeviceHandleFromMigDeviceHandle);
    /** nvmlDeviceGetGpuInstanceById */
    DLSYM_HOOK_FUNC(nvmlDeviceGetGpuInstanceById);
    /** nvmlDeviceGetGpuInstanceId */
    DLSYM_HOOK_FUNC(nvmlDeviceGetGpuInstanceId);
    /** nvmlDeviceGetGpuInstancePossiblePlacements */
    DLSYM_HOOK_FUNC(nvmlDeviceGetGpuInstancePossiblePlacements);
    /** nvmlDeviceGetGpuInstanceProfileInfo */
    DLSYM_HOOK_FUNC(nvmlDeviceGetGpuInstanceProfileInfo);
    /** nvmlDeviceGetGpuInstanceRemainingCapacity */
    DLSYM_HOOK_FUNC(nvmlDeviceGetGpuInstanceRemainingCapacity);
    /** nvmlDeviceGetGpuInstances */
    DLSYM_HOOK_FUNC(nvmlDeviceGetGpuInstances);
    /** nvmlDeviceGetMaxMigDeviceCount */
    DLSYM_HOOK_FUNC(nvmlDeviceGetMaxMigDeviceCount);
    /** nvmlDeviceGetMemoryAffinity */
    DLSYM_HOOK_FUNC(nvmlDeviceGetMemoryAffinity);
    /** nvmlDeviceGetMigDeviceHandleByIndex */
    DLSYM_HOOK_FUNC(nvmlDeviceGetMigDeviceHandleByIndex);
    /** nvmlDeviceGetMigMode */
    DLSYM_HOOK_FUNC(nvmlDeviceGetMigMode);
    /** nvmlDeviceGetRemappedRows */
    DLSYM_HOOK_FUNC(nvmlDeviceGetRemappedRows);
    /** nvmlDeviceGetRowRemapperHistogram */
    DLSYM_HOOK_FUNC(nvmlDeviceGetRowRemapperHistogram);
    /** nvmlDeviceIsMigDeviceHandle */
    DLSYM_HOOK_FUNC(nvmlDeviceIsMigDeviceHandle);
    /** nvmlDeviceSetMigMode */
    DLSYM_HOOK_FUNC(nvmlDeviceSetMigMode);
    /** nvmlEventSetWait_v2 */
    DLSYM_HOOK_FUNC(nvmlEventSetWait_v2);
    /** nvmlGpuInstanceCreateComputeInstance */
    DLSYM_HOOK_FUNC(nvmlGpuInstanceCreateComputeInstance);
    /** nvmlGpuInstanceDestroy */
    DLSYM_HOOK_FUNC(nvmlGpuInstanceDestroy);
    /** nvmlGpuInstanceGetComputeInstanceById */
    DLSYM_HOOK_FUNC(nvmlGpuInstanceGetComputeInstanceById);
    /** nvmlGpuInstanceGetComputeInstanceProfileInfo */
    DLSYM_HOOK_FUNC(nvmlGpuInstanceGetComputeInstanceProfileInfo);
    /** nvmlGpuInstanceGetComputeInstanceRemainingCapacity */
    DLSYM_HOOK_FUNC(nvmlGpuInstanceGetComputeInstanceRemainingCapacity);
    /** nvmlGpuInstanceGetComputeInstances */
    DLSYM_HOOK_FUNC(nvmlGpuInstanceGetComputeInstances);
    /** nvmlGpuInstanceGetInfo */
    DLSYM_HOOK_FUNC(nvmlGpuInstanceGetInfo);
    /** nvmlVgpuInstanceClearAccountingPids */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceClearAccountingPids);
    /** nvmlVgpuInstanceGetMdevUUID */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetMdevUUID);
    /** nvmlComputeInstanceGetInfo_v2 */
    DLSYM_HOOK_FUNC(nvmlComputeInstanceGetInfo_v2);
    /** nvmlDeviceGetComputeRunningProcesses_v2 */
    DLSYM_HOOK_FUNC(nvmlDeviceGetComputeRunningProcesses_v2);
    /** nvmlDeviceGetGraphicsRunningProcesses_v2 */
    DLSYM_HOOK_FUNC(nvmlDeviceGetGraphicsRunningProcesses_v2);
    /** nvmlDeviceSetTemperatureThreshold */
    DLSYM_HOOK_FUNC(nvmlDeviceSetTemperatureThreshold);
    /** nvmlRetry_NvRmControl */
    //DLSYM_HOOK_FUNC(nvmlRetry_NvRmControl);
    /** nvmlVgpuInstanceGetGpuInstanceId */
    DLSYM_HOOK_FUNC(nvmlVgpuInstanceGetGpuInstanceId);
    /** nvmlVgpuTypeGetGpuInstanceProfileId */
    DLSYM_HOOK_FUNC(nvmlVgpuTypeGetGpuInstanceProfileId);
    return NULL;
}

void preInit(){
    LOG_MSG("Initializing.....");
    if (real_dlsym == NULL) {
        real_dlsym = _dl_sym(RTLD_NEXT, "dlsym", dlsym);
    }
    real_realpath = NULL;
    load_cuda_libraries();
    nvmlInit();
    ENSURE_INITIALIZED(); 
}

void postInit(){
    map_cuda_visible_devices();
    allocator_init();

    try_lock_unified_lock();
    nvmlReturn_t res = set_task_pid();
    try_unlock_unified_lock();


    LOG_MSG("Initialized");
    if (res!=NVML_SUCCESS){
        LOG_WARN("SET_TASK_PID FAILED.");
        pidfound=0;
    }else{
        pidfound=1;
    }
    //add_gpu_device_memory_usage(getpid(),0,context_size,0);
    env_utilization_switch = set_env_utilization_switch();
    init_utilization_watcher();
}

CUresult cuInit(unsigned int Flags){
    LOG_INFO("Into cuInit");
    pthread_once(&pre_cuinit_flag,(void(*)(void))preInit);
    ENSURE_INITIALIZED();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuInit,Flags);
    if (res != CUDA_SUCCESS){
        LOG_ERROR("cuInit failed:%d",res);
        return res;
    }
    pthread_once(&post_cuinit_flag, (void(*) (void))postInit);
    return CUDA_SUCCESS;
}


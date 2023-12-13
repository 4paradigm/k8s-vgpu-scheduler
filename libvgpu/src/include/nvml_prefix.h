#ifndef __NVML_PREFIX__
#define __NVML_PREFIX__

#define NVML_NO_UNVERSIONED_FUNC_DEFS

#ifndef NVML_NO_UNVERSIONED_FUNC_DEFS
#define nvmlInit                                nvmlInit_v2
#define nvmlDeviceGetPciInfo                    nvmlDeviceGetPciInfo_v3
#define nvmlDeviceGetCount                      nvmlDeviceGetCount_v2
#define nvmlDeviceGetHandleByIndex              nvmlDeviceGetHandleByIndex_v2
#define nvmlDeviceGetHandleByPciBusId           nvmlDeviceGetHandleByPciBusId_v2
#define nvmlDeviceGetNvLinkRemotePciInfo        nvmlDeviceGetNvLinkRemotePciInfo_v2
#define nvmlDeviceRemoveGpu                     nvmlDeviceRemoveGpu_v2
#define nvmlDeviceGetGridLicensableFeatures     nvmlDeviceGetGridLicensableFeatures_v3
#define nvmlEventSetWait                        nvmlEventSetWait_v2
#define nvmlDeviceGetAttributes                 nvmlDeviceGetAttributes_v2
#define nvmlDeviceGetComputeRunningProcesses    nvmlDeviceGetComputeRunningProcesses_v2
#define nvmlDeviceGetGraphicsRunningProcesses   nvmlDeviceGetGraphicsRunningProcesses_v2
#endif

typedef struct {
  void *fn_ptr;
  char *name;
} entry_t;

extern entry_t nvml_library_entry[];

#endif
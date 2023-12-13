#ifndef __LIBNVML_HOOK_H__
#define __LIBNVML_HOOK_H__

#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <cuda.h>
#include <pthread.h>
#include "include/nvml-subset.h"
#include "include/log_utils.h"
#include "include/nvml_prefix.h"

#define FILENAME_MAX 4096

typedef nvmlReturn_t (*driver_sym_t)();

#define NVML_OVERRIDE_ENUM(x) OVERRIDE_##x

#define NVML_FIND_ENTRY(table, sym) ({ (table)[NVML_OVERRIDE_ENUM(sym)].fn_ptr; })

#define NVML_OVERRIDE_CALL(table, sym, ...)                                    \
  ({                                                                           \
    LOG_DEBUG("Hijacking %s", #sym);                                           \
    driver_sym_t _entry = NVML_FIND_ENTRY(table, sym);                         \
    _entry(__VA_ARGS__);                                                       \
  })

#define NVML_OVERRIDE_CALL_NO_LOG(table, sym, ...)                             \
  ({                                                                           \
    driver_sym_t _entry = NVML_FIND_ENTRY(table, sym);                         \
    _entry(__VA_ARGS__);                                                       \
  })

/**
 * NVML management library enumerator entry
 */
typedef enum {
  /** nvmlInit */
  NVML_OVERRIDE_ENUM(nvmlInit),
  /** nvmlShutdown */
  NVML_OVERRIDE_ENUM(nvmlShutdown),
  /** nvmlErrorString */
  NVML_OVERRIDE_ENUM(nvmlErrorString),
  /** nvmlDeviceGetHandleByIndex */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetHandleByIndex),
  /** nvmlDeviceGetComputeRunningProcesses */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetComputeRunningProcesses),
  /** nvmlDeviceGetPciInfo */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetPciInfo),
  /** nvmlDeviceGetProcessUtilization */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetProcessUtilization),
  /** nvmlDeviceGetCount */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetCount),
  /** nvmlDeviceClearAccountingPids */
  NVML_OVERRIDE_ENUM(nvmlDeviceClearAccountingPids),
  /** nvmlDeviceClearCpuAffinity */
  NVML_OVERRIDE_ENUM(nvmlDeviceClearCpuAffinity),
  /** nvmlDeviceClearEccErrorCounts */
  NVML_OVERRIDE_ENUM(nvmlDeviceClearEccErrorCounts),
  /** nvmlDeviceDiscoverGpus */
  NVML_OVERRIDE_ENUM(nvmlDeviceDiscoverGpus),
  /** nvmlDeviceFreezeNvLinkUtilizationCounter */
  NVML_OVERRIDE_ENUM(nvmlDeviceFreezeNvLinkUtilizationCounter),
  /** nvmlDeviceGetAccountingBufferSize */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetAccountingBufferSize),
  /** nvmlDeviceGetAccountingMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetAccountingMode),
  /** nvmlDeviceGetAccountingPids */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetAccountingPids),
  /** nvmlDeviceGetAccountingStats */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetAccountingStats),
  /** nvmlDeviceGetActiveVgpus */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetActiveVgpus),
  /** nvmlDeviceGetAPIRestriction */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetAPIRestriction),
  /** nvmlDeviceGetApplicationsClock */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetApplicationsClock),
  /** nvmlDeviceGetAutoBoostedClocksEnabled */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetAutoBoostedClocksEnabled),
  /** nvmlDeviceGetBAR1MemoryInfo */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetBAR1MemoryInfo),
  /** nvmlDeviceGetBoardId */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetBoardId),
  /** nvmlDeviceGetBoardPartNumber */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetBoardPartNumber),
  /** nvmlDeviceGetBrand */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetBrand),
  /** nvmlDeviceGetBridgeChipInfo */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetBridgeChipInfo),
  /** nvmlDeviceGetClock */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetClock),
  /** nvmlDeviceGetClockInfo */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetClockInfo),
  /** nvmlDeviceGetComputeMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetComputeMode),
  /** nvmlDeviceGetCount_v2 */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetCount_v2),
  /** nvmlDeviceGetCpuAffinity */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetCpuAffinity),
  /** nvmlDeviceGetCreatableVgpus */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetCreatableVgpus),
  /** nvmlDeviceGetCudaComputeCapability */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetCudaComputeCapability),
  /** nvmlDeviceGetCurrentClocksThrottleReasons */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetCurrentClocksThrottleReasons),
  /** nvmlDeviceGetCurrPcieLinkGeneration */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetCurrPcieLinkGeneration),
  /** nvmlDeviceGetCurrPcieLinkWidth */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetCurrPcieLinkWidth),
  /** nvmlDeviceGetDecoderUtilization */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetDecoderUtilization),
  /** nvmlDeviceGetDefaultApplicationsClock */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetDefaultApplicationsClock),
  /** nvmlDeviceGetDetailedEccErrors */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetDetailedEccErrors),
  /** nvmlDeviceGetDisplayActive */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetDisplayActive),
  /** nvmlDeviceGetDisplayMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetDisplayMode),
  /** nvmlDeviceGetDriverModel */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetDriverModel),
  /** nvmlDeviceGetEccMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetEccMode),
  /** nvmlDeviceGetEncoderCapacity */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetEncoderCapacity),
  /** nvmlDeviceGetEncoderSessions */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetEncoderSessions),
  /** nvmlDeviceGetEncoderStats */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetEncoderStats),
  /** nvmlDeviceGetEncoderUtilization */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetEncoderUtilization),
  /** nvmlDeviceGetEnforcedPowerLimit */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetEnforcedPowerLimit),
  /** nvmlDeviceGetFanSpeed */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetFanSpeed),
  /** nvmlDeviceGetFanSpeed_v2 */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetFanSpeed_v2),
  /** nvmlDeviceGetFieldValues */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetFieldValues),
  /** nvmlDeviceGetGpuOperationMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetGpuOperationMode),
  /** nvmlDeviceGetGraphicsRunningProcesses */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetGraphicsRunningProcesses),
  /** nvmlDeviceGetGridLicensableFeatures */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetGridLicensableFeatures),
  /** nvmlDeviceGetHandleByIndex_v2 */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetHandleByIndex_v2),
  /** nvmlDeviceGetHandleByPciBusId */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetHandleByPciBusId),
  /** nvmlDeviceGetHandleByPciBusId_v2 */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetHandleByPciBusId_v2),
  /** nvmlDeviceGetHandleBySerial */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetHandleBySerial),
  /** nvmlDeviceGetHandleByUUID */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetHandleByUUID),
  /** nvmlDeviceGetIndex */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetIndex),
  /** nvmlDeviceGetInforomConfigurationChecksum */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetInforomConfigurationChecksum),
  /** nvmlDeviceGetInforomImageVersion */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetInforomImageVersion),
  /** nvmlDeviceGetInforomVersion */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetInforomVersion),
  /** nvmlDeviceGetMaxClockInfo */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetMaxClockInfo),
  /** nvmlDeviceGetMaxCustomerBoostClock */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetMaxCustomerBoostClock),
  /** nvmlDeviceGetMaxPcieLinkGeneration */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetMaxPcieLinkGeneration),
  /** nvmlDeviceGetMaxPcieLinkWidth */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetMaxPcieLinkWidth),
  /** nvmlDeviceGetMemoryErrorCounter */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetMemoryErrorCounter),
  /** nvmlDeviceGetMemoryInfo */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetMemoryInfo),
  /** nvmlDeviceGetMemoryInfo_v2 */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetMemoryInfo_v2),
  /** nvmlDeviceGetMinorNumber */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetMinorNumber),
  /** nvmlDeviceGetMPSComputeRunningProcesses */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetMPSComputeRunningProcesses),
  /** nvmlDeviceGetMultiGpuBoard */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetMultiGpuBoard),
  /** nvmlDeviceGetName */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetName),
  /** nvmlDeviceGetNvLinkCapability */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetNvLinkCapability),
  /** nvmlDeviceGetNvLinkErrorCounter */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetNvLinkErrorCounter),
  /** nvmlDeviceGetNvLinkRemotePciInfo */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetNvLinkRemotePciInfo),
  /** nvmlDeviceGetNvLinkRemotePciInfo_v2 */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetNvLinkRemotePciInfo_v2),
  /** nvmlDeviceGetNvLinkState */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetNvLinkState),
  /** nvmlDeviceGetNvLinkUtilizationControl */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetNvLinkUtilizationControl),
  /** nvmlDeviceGetNvLinkUtilizationCounter */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetNvLinkUtilizationCounter),
  /** nvmlDeviceGetNvLinkVersion */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetNvLinkVersion),
  /** nvmlDeviceGetP2PStatus */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetP2PStatus),
  /** nvmlDeviceGetPcieReplayCounter */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetPcieReplayCounter),
  /** nvmlDeviceGetPcieThroughput */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetPcieThroughput),
  /** nvmlDeviceGetPciInfo_v2 */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetPciInfo_v2),
  /** nvmlDeviceGetPciInfo_v3 */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetPciInfo_v3),
  /** nvmlDeviceGetPerformanceState */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetPerformanceState),
  /** nvmlDeviceGetPersistenceMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetPersistenceMode),
  /** nvmlDeviceGetPowerManagementDefaultLimit */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetPowerManagementDefaultLimit),
  /** nvmlDeviceGetPowerManagementLimit */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetPowerManagementLimit),
  /** nvmlDeviceGetPowerManagementLimitConstraints */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetPowerManagementLimitConstraints),
  /** nvmlDeviceGetPowerManagementMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetPowerManagementMode),
  /** nvmlDeviceGetPowerState */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetPowerState),
  /** nvmlDeviceGetPowerUsage */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetPowerUsage),
  /** nvmlDeviceGetRetiredPages */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetRetiredPages),
  /** nvmlDeviceGetRetiredPagesPendingStatus */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetRetiredPagesPendingStatus),
  /** nvmlDeviceGetSamples */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetSamples),
  /** nvmlDeviceGetSerial */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetSerial),
  /** nvmlDeviceGetSupportedClocksThrottleReasons */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetSupportedClocksThrottleReasons),
  /** nvmlDeviceGetSupportedEventTypes */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetSupportedEventTypes),
  /** nvmlDeviceGetSupportedGraphicsClocks */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetSupportedGraphicsClocks),
  /** nvmlDeviceGetSupportedMemoryClocks */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetSupportedMemoryClocks),
  /** nvmlDeviceGetSupportedVgpus */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetSupportedVgpus),
  /** nvmlDeviceGetTemperature */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetTemperature),
  /** nvmlDeviceGetTemperatureThreshold */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetTemperatureThreshold),
  /** nvmlDeviceGetTopologyCommonAncestor */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetTopologyCommonAncestor),
  /** nvmlDeviceGetTopologyNearestGpus */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetTopologyNearestGpus),
  /** nvmlDeviceGetTotalEccErrors */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetTotalEccErrors),
  /** nvmlDeviceGetTotalEnergyConsumption */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetTotalEnergyConsumption),
  /** nvmlDeviceGetUtilizationRates */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetUtilizationRates),
  /** nvmlDeviceGetUUID */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetUUID),
  /** nvmlDeviceGetVbiosVersion */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetVbiosVersion),
  /** nvmlDeviceGetVgpuMetadata */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetVgpuMetadata),
  /** nvmlDeviceGetVgpuProcessUtilization */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetVgpuProcessUtilization),
  /** nvmlDeviceGetVgpuUtilization */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetVgpuUtilization),
  /** nvmlDeviceGetViolationStatus */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetViolationStatus),
  /** nvmlDeviceGetVirtualizationMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetVirtualizationMode),
  /** nvmlDeviceModifyDrainState */
  NVML_OVERRIDE_ENUM(nvmlDeviceModifyDrainState),
  /** nvmlDeviceOnSameBoard */
  NVML_OVERRIDE_ENUM(nvmlDeviceOnSameBoard),
  /** nvmlDeviceQueryDrainState */
  NVML_OVERRIDE_ENUM(nvmlDeviceQueryDrainState),
  /** nvmlDeviceRegisterEvents */
  NVML_OVERRIDE_ENUM(nvmlDeviceRegisterEvents),
  /** nvmlDeviceRemoveGpu */
  NVML_OVERRIDE_ENUM(nvmlDeviceRemoveGpu),
  /** nvmlDeviceRemoveGpu_v2 */
  NVML_OVERRIDE_ENUM(nvmlDeviceRemoveGpu_v2),
  /** nvmlDeviceResetApplicationsClocks */
  NVML_OVERRIDE_ENUM(nvmlDeviceResetApplicationsClocks),
  /** nvmlDeviceResetNvLinkErrorCounters */
  NVML_OVERRIDE_ENUM(nvmlDeviceResetNvLinkErrorCounters),
  /** nvmlDeviceResetNvLinkUtilizationCounter */
  NVML_OVERRIDE_ENUM(nvmlDeviceResetNvLinkUtilizationCounter),
  /** nvmlDeviceSetAccountingMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetAccountingMode),
  /** nvmlDeviceSetAPIRestriction */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetAPIRestriction),
  /** nvmlDeviceSetApplicationsClocks */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetApplicationsClocks),
  /** nvmlDeviceSetAutoBoostedClocksEnabled */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetAutoBoostedClocksEnabled),
  /** nvmlDeviceSetComputeMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetComputeMode),
  /** nvmlDeviceSetCpuAffinity */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetCpuAffinity),
  /** nvmlDeviceSetDefaultAutoBoostedClocksEnabled */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetDefaultAutoBoostedClocksEnabled),
  /** nvmlDeviceSetDriverModel */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetDriverModel),
  /** nvmlDeviceSetEccMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetEccMode),
  /** nvmlDeviceSetGpuOperationMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetGpuOperationMode),
  /** nvmlDeviceSetNvLinkUtilizationControl */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetNvLinkUtilizationControl),
  /** nvmlDeviceSetPersistenceMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetPersistenceMode),
  /** nvmlDeviceSetPowerManagementLimit */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetPowerManagementLimit),
  /** nvmlDeviceSetVirtualizationMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetVirtualizationMode),
  /** nvmlDeviceValidateInforom */
  NVML_OVERRIDE_ENUM(nvmlDeviceValidateInforom),
  /** nvmlEventSetCreate */
  NVML_OVERRIDE_ENUM(nvmlEventSetCreate),
  /** nvmlEventSetFree */
  NVML_OVERRIDE_ENUM(nvmlEventSetFree),
  /** nvmlEventSetWait */
  NVML_OVERRIDE_ENUM(nvmlEventSetWait),
  /** nvmlGetVgpuCompatibility */
  NVML_OVERRIDE_ENUM(nvmlGetVgpuCompatibility),
  /** nvmlInit_v2 */
  NVML_OVERRIDE_ENUM(nvmlInit_v2),
  /** nvmlInitWithFlags */
  NVML_OVERRIDE_ENUM(nvmlInitWithFlags),
  /** nvmlInternalGetExportTable */
  NVML_OVERRIDE_ENUM(nvmlInternalGetExportTable),
  /** nvmlSystemGetCudaDriverVersion */
  NVML_OVERRIDE_ENUM(nvmlSystemGetCudaDriverVersion),
  /** nvmlSystemGetCudaDriverVersion_v2 */
  NVML_OVERRIDE_ENUM(nvmlSystemGetCudaDriverVersion_v2),
  /** nvmlSystemGetDriverVersion */
  NVML_OVERRIDE_ENUM(nvmlSystemGetDriverVersion),
  /** nvmlSystemGetHicVersion */
  NVML_OVERRIDE_ENUM(nvmlSystemGetHicVersion),
  /** nvmlSystemGetNVMLVersion */
  NVML_OVERRIDE_ENUM(nvmlSystemGetNVMLVersion),
  /** nvmlSystemGetProcessName */
  NVML_OVERRIDE_ENUM(nvmlSystemGetProcessName),
  /** nvmlSystemGetTopologyGpuSet */
  NVML_OVERRIDE_ENUM(nvmlSystemGetTopologyGpuSet),
  /** nvmlUnitGetCount */
  NVML_OVERRIDE_ENUM(nvmlUnitGetCount),
  /** nvmlUnitGetDevices */
  NVML_OVERRIDE_ENUM(nvmlUnitGetDevices),
  /** nvmlUnitGetFanSpeedInfo */
  NVML_OVERRIDE_ENUM(nvmlUnitGetFanSpeedInfo),
  /** nvmlUnitGetHandleByIndex */
  NVML_OVERRIDE_ENUM(nvmlUnitGetHandleByIndex),
  /** nvmlUnitGetLedState */
  NVML_OVERRIDE_ENUM(nvmlUnitGetLedState),
  /** nvmlUnitGetPsuInfo */
  NVML_OVERRIDE_ENUM(nvmlUnitGetPsuInfo),
  /** nvmlUnitGetTemperature */
  NVML_OVERRIDE_ENUM(nvmlUnitGetTemperature),
  /** nvmlUnitGetUnitInfo */
  NVML_OVERRIDE_ENUM(nvmlUnitGetUnitInfo),
  /** nvmlUnitSetLedState */
  NVML_OVERRIDE_ENUM(nvmlUnitSetLedState),
  /** nvmlVgpuInstanceGetEncoderCapacity */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetEncoderCapacity),
  /** nvmlVgpuInstanceGetEncoderSessions */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetEncoderSessions),
  /** nvmlVgpuInstanceGetEncoderStats */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetEncoderStats),
  /** nvmlVgpuInstanceGetFbUsage */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetFbUsage),
  /** nvmlVgpuInstanceGetFrameRateLimit */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetFrameRateLimit),
  /** nvmlVgpuInstanceGetLicenseStatus */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetLicenseStatus),
  /** nvmlVgpuInstanceGetMetadata */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetMetadata),
  /** nvmlVgpuInstanceGetType */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetType),
  /** nvmlVgpuInstanceGetUUID */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetUUID),
  /** nvmlVgpuInstanceGetVmDriverVersion */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetVmDriverVersion),
  /** nvmlVgpuInstanceGetVmID */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetVmID),
  /** nvmlVgpuInstanceSetEncoderCapacity */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceSetEncoderCapacity),
  /** nvmlVgpuTypeGetClass */
  NVML_OVERRIDE_ENUM(nvmlVgpuTypeGetClass),
  /** nvmlVgpuTypeGetDeviceID */
  NVML_OVERRIDE_ENUM(nvmlVgpuTypeGetDeviceID),
  /** nvmlVgpuTypeGetFramebufferSize */
  NVML_OVERRIDE_ENUM(nvmlVgpuTypeGetFramebufferSize),
  /** nvmlVgpuTypeGetFrameRateLimit */
  NVML_OVERRIDE_ENUM(nvmlVgpuTypeGetFrameRateLimit),
  /** nvmlVgpuTypeGetLicense */
  NVML_OVERRIDE_ENUM(nvmlVgpuTypeGetLicense),
  /** nvmlVgpuTypeGetMaxInstances */
  NVML_OVERRIDE_ENUM(nvmlVgpuTypeGetMaxInstances),
  /** nvmlVgpuTypeGetName */
  NVML_OVERRIDE_ENUM(nvmlVgpuTypeGetName),
  /** nvmlVgpuTypeGetNumDisplayHeads */
  NVML_OVERRIDE_ENUM(nvmlVgpuTypeGetNumDisplayHeads),
  /** nvmlVgpuTypeGetResolution */
  NVML_OVERRIDE_ENUM(nvmlVgpuTypeGetResolution),
  /** nvmlDeviceGetFBCSessions */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetFBCSessions),
  /** nvmlDeviceGetFBCStats */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetFBCStats),
  /** nvmlDeviceGetGridLicensableFeatures_v2 */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetGridLicensableFeatures_v2),
  /** nvmlDeviceGetRetiredPages_v2 */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetRetiredPages_v2),
  /** nvmlDeviceResetGpuLockedClocks */
  NVML_OVERRIDE_ENUM(nvmlDeviceResetGpuLockedClocks),
  /** nvmlDeviceSetGpuLockedClocks */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetGpuLockedClocks),
  /** nvmlGetBlacklistDeviceCount */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetAccountingMode),
  /** nvmlVgpuInstanceGetAccountingPids */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetAccountingPids),
  /** nvmlVgpuInstanceGetAccountingStats */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetAccountingStats),
  /** nvmlVgpuInstanceGetFBCSessions */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetFBCSessions),
  /** nvmlVgpuInstanceGetFBCStats */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetFBCStats),
  /** nvmlVgpuTypeGetMaxInstancesPerVm */
  NVML_OVERRIDE_ENUM(nvmlVgpuTypeGetMaxInstancesPerVm),
  /** nvmlGetVgpuVersion */
  NVML_OVERRIDE_ENUM(nvmlGetVgpuVersion),
  /** nvmlSetVgpuVersion */
  NVML_OVERRIDE_ENUM(nvmlSetVgpuVersion),
  /** nvmlDeviceGetGridLicensableFeatures_v3 */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetGridLicensableFeatures_v3),
  /** nvmlDeviceGetHostVgpuMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetHostVgpuMode),
  /** nvmlDeviceGetPgpuMetadataString */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetPgpuMetadataString),
  /** nvmlVgpuInstanceGetEccMode */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetEccMode),
  /** nvmlComputeInstanceDestroy */
  NVML_OVERRIDE_ENUM(nvmlComputeInstanceDestroy),
  /** nvmlComputeInstanceGetInfo */
  NVML_OVERRIDE_ENUM(nvmlComputeInstanceGetInfo),
  /** nvmlDeviceCreateGpuInstance */
  NVML_OVERRIDE_ENUM(nvmlDeviceCreateGpuInstance),
  /** nvmlDeviceGetArchitecture */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetArchitecture),
  /** nvmlDeviceGetAttributes */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetAttributes),
  /** nvmlDeviceGetAttributes_v2 */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetAttributes_v2),
  /** nvmlDeviceGetComputeInstanceId */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetComputeInstanceId),
  /** nvmlDeviceGetCpuAffinityWithinScope */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetCpuAffinityWithinScope),
  /** nvmlDeviceGetDeviceHandleFromMigDeviceHandle */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetDeviceHandleFromMigDeviceHandle),
  /** nvmlDeviceGetGpuInstanceById */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetGpuInstanceById),
  /** nvmlDeviceGetGpuInstanceId */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetGpuInstanceId),
  /** nvmlDeviceGetGpuInstancePossiblePlacements */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetGpuInstancePossiblePlacements),
  /** nvmlDeviceGetGpuInstanceProfileInfo */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetGpuInstanceProfileInfo),
  /** nvmlDeviceGetGpuInstanceRemainingCapacity */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetGpuInstanceRemainingCapacity),
  /** nvmlDeviceGetGpuInstances */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetGpuInstances),
  /** nvmlDeviceGetMaxMigDeviceCount */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetMaxMigDeviceCount),
  /** nvmlDeviceGetMemoryAffinity */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetMemoryAffinity),
  /** nvmlDeviceGetMigDeviceHandleByIndex */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetMigDeviceHandleByIndex),
  /** nvmlDeviceGetMigMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetMigMode),
  /** nvmlDeviceGetRemappedRows */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetRemappedRows),
  /** nvmlDeviceGetRowRemapperHistogram */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetRowRemapperHistogram),
  /** nvmlDeviceIsMigDeviceHandle */
  NVML_OVERRIDE_ENUM(nvmlDeviceIsMigDeviceHandle),
  /** nvmlDeviceSetMigMode */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetMigMode),
  /** nvmlEventSetWait_v2 */
  NVML_OVERRIDE_ENUM(nvmlEventSetWait_v2),
  /** nvmlGpuInstanceCreateComputeInstance */
  NVML_OVERRIDE_ENUM(nvmlGpuInstanceCreateComputeInstance),
  /** nvmlGpuInstanceDestroy */
  NVML_OVERRIDE_ENUM(nvmlGpuInstanceDestroy),
  /** nvmlGpuInstanceGetComputeInstanceById */
  NVML_OVERRIDE_ENUM(nvmlGpuInstanceGetComputeInstanceById),
  /** nvmlGpuInstanceGetComputeInstanceProfileInfo */
  NVML_OVERRIDE_ENUM(nvmlGpuInstanceGetComputeInstanceProfileInfo),
  /** nvmlGpuInstanceGetComputeInstanceRemainingCapacity */
  NVML_OVERRIDE_ENUM(nvmlGpuInstanceGetComputeInstanceRemainingCapacity),
  /** nvmlGpuInstanceGetComputeInstances */
  NVML_OVERRIDE_ENUM(nvmlGpuInstanceGetComputeInstances),
  /** nvmlGpuInstanceGetInfo */
  NVML_OVERRIDE_ENUM(nvmlGpuInstanceGetInfo),
  /** nvmlVgpuInstanceClearAccountingPids */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceClearAccountingPids),
  /** nvmlVgpuInstanceGetMdevUUID */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetMdevUUID),
  /** nvmlComputeInstanceGetInfo_v2 */
  NVML_OVERRIDE_ENUM(nvmlComputeInstanceGetInfo_v2),
  /** nvmlDeviceGetComputeRunningProcesses_v2 */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetComputeRunningProcesses_v2),
  /** nvmlDeviceGetGraphicsRunningProcesses_v2 */
  NVML_OVERRIDE_ENUM(nvmlDeviceGetGraphicsRunningProcesses_v2),
  /** nvmlDeviceSetTemperatureThreshold */
  NVML_OVERRIDE_ENUM(nvmlDeviceSetTemperatureThreshold),
  /** nvmlRetry_NvRmControl */
  //NVML_OVERRIDE_ENUM(nvmlRetry_NvRmControl),
  /** nvmlVgpuInstanceGetGpuInstanceId */
  NVML_OVERRIDE_ENUM(nvmlVgpuInstanceGetGpuInstanceId),
  /** nvmlVgpuTypeGetGpuInstanceProfileId */
  NVML_OVERRIDE_ENUM(nvmlVgpuTypeGetGpuInstanceProfileId),
  NVML_ENTRY_END
} NVML_OVERRIDE_ENUM_t;

#endif
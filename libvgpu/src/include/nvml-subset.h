/*
 * Copyright 1993-2018 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO LICENSEE:
 *
 * This source code and/or documentation ("Licensed Deliverables") are
 * subject to NVIDIA intellectual property rights under U.S. and
 * international Copyright laws.
 *
 * These Licensed Deliverables contained herein is PROPRIETARY and
 * CONFIDENTIAL to NVIDIA and is being provided under the terms and
 * conditions of a form of NVIDIA software license agreement by and
 * between NVIDIA and Licensee ("License Agreement") or electronically
 * accepted by Licensee.  Notwithstanding any terms or conditions to
 * the contrary in the License Agreement, reproduction or disclosure
 * of the Licensed Deliverables to any third party without the express
 * written consent of NVIDIA is prohibited.
 *
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, NVIDIA MAKES NO REPRESENTATION ABOUT THE
 * SUITABILITY OF THESE LICENSED DELIVERABLES FOR ANY PURPOSE.  IT IS
 * PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.
 * NVIDIA DISCLAIMS ALL WARRANTIES WITH REGARD TO THESE LICENSED
 * DELIVERABLES, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
 * NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY
 * SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THESE LICENSED DELIVERABLES.
 *
 * U.S. Government End Users.  These Licensed Deliverables are a
 * "commercial item" as that term is defined at 48 C.F.R. 2.101 (OCT
 * 1995), consisting of "commercial computer software" and "commercial
 * computer software documentation" as such terms are used in 48
 * C.F.R. 12.212 (SEPT 1995) and is provided to the U.S. Government
 * only as a commercial end item.  Consistent with 48 C.F.R.12.212 and
 * 48 C.F.R. 227.7202-1 through 227.7202-4 (JUNE 1995), all
 * U.S. Government End Users acquire the Licensed Deliverables with
 * only those rights set forth herein.
 *
 * Any use of the Licensed Deliverables in individual and commercial
 * software must include, in the user documentation and internal
 * comments to the code, the above Disclaimer and U.S. Government End
 * Users Notice.
 */
#ifndef HIJACK_DRIVER_SUBSET_H
#define HIJACK_DRIVER_SUBSET_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Buffer size guaranteed to be large enough for pci bus id
 */
#define NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE 16
#define NVML_GRID_LICENSE_BUFFER_SIZE 128
#define NVML_GRID_LICENSE_FEATURE_MAX_COUNT 3
#define NVML_VGPU_NAME_BUFFER_SIZE 64

/**
 * Buffer size guaranteed to be large enough for \ref nvmlSystemGetDriverVersion
 */
#define NVML_SYSTEM_DRIVER_VERSION_BUFFER_SIZE 80

/**
 * Maximum limit on Physical Bridges per Board
 */
#define NVML_MAX_PHYSICAL_BRIDGE (128)

typedef struct nvmlDevice_st *nvmlDevice_t;

typedef struct nvmlProcessInfo_st {
  unsigned int pid;                  //!< Process ID
  unsigned long long usedGpuMemory;  //!< Amount of used GPU memory in bytes.
  //! Under WDDM, \ref NVML_VALUE_NOT_AVAILABLE is always reported
  //! because Windows KMD manages all the memory and not the NVIDIA driver
} nvmlProcessInfo_t;

/**
 * Return values for NVML API calls.
 */
typedef enum nvmlReturn_enum {
  NVML_SUCCESS = 0,  //!< The operation was successful
  NVML_ERROR_UNINITIALIZED =
  1,  //!< NVML was not first initialized with nvmlInit()
  NVML_ERROR_INVALID_ARGUMENT = 2,  //!< A supplied argument is invalid
  NVML_ERROR_NOT_SUPPORTED =
  3,  //!< The requested operation is not available on target device
  NVML_ERROR_NO_PERMISSION =
  4,  //!< The current user does not have permission for operation
  NVML_ERROR_ALREADY_INITIALIZED = 5,  //!< Deprecated: Multiple initializations
  //! are now allowed through ref counting
  NVML_ERROR_NOT_FOUND = 6,  //!< A query to find an object was unsuccessful
  NVML_ERROR_INSUFFICIENT_SIZE = 7,  //!< An input argument is not large enough
  NVML_ERROR_INSUFFICIENT_POWER =
  8,  //!< A device's external power cables are not properly attached
  NVML_ERROR_DRIVER_NOT_LOADED = 9,  //!< NVIDIA driver is not loaded
  NVML_ERROR_TIMEOUT = 10,           //!< User provided timeout passed
  NVML_ERROR_IRQ_ISSUE =
  11,  //!< NVIDIA Kernel detected an interrupt issue with a GPU
  NVML_ERROR_LIBRARY_NOT_FOUND =
  12,  //!< NVML Shared Library couldn't be found or loaded
  NVML_ERROR_FUNCTION_NOT_FOUND =
  13,  //!< Local version of NVML doesn't implement this function
  NVML_ERROR_CORRUPTED_INFOROM = 14,  //!< infoROM is corrupted
  NVML_ERROR_GPU_IS_LOST = 15,        //!< The GPU has fallen off the bus or has
  //! otherwise become inaccessible
  NVML_ERROR_RESET_REQUIRED =
  16,  //!< The GPU requires a reset before it can be used again
  NVML_ERROR_OPERATING_SYSTEM = 17,  //!< The GPU control device has been
  //! blocked by the operating system/cgroups
  NVML_ERROR_LIB_RM_VERSION_MISMATCH =
  18,                  //!< RM detects a driver/library version mismatch
  NVML_ERROR_IN_USE = 19,  //!< An operation cannot be performed because the GPU
  //! is currently in use
  NVML_ERROR_NO_DATA = 20,  //!< No data
  NVML_ERROR_UNKNOWN = 999  //!< An internal driver error occurred
} nvmlReturn_t;

/**
 * PCI information about a GPU device.
 */
typedef struct nvmlPciInfo_st {
  char busId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];  //!< The tuple
  //!< domain:bus:device.function
  //!< PCI identifier (&amp;
  //!< NULL terminator)
  unsigned int domain;  //!< The PCI domain on which the device's bus resides, 0
  //!< to 0xffff
  unsigned int bus;     //!< The bus on which the device resides, 0 to 0xff
  unsigned int device;  //!< The device's id on the bus, 0 to 31
  unsigned int
      pciDeviceId;  //!< The combined 16-bit device id and 16-bit vendor id

  // Added in NVML 2.285 API
  unsigned int pciSubSystemId;  //!< The 32-bit Sub System Device ID

  // NVIDIA reserved for internal use only
  unsigned int reserved0;
  unsigned int reserved1;
  unsigned int reserved2;
  unsigned int reserved3;
} nvmlPciInfo_t;

/**
 * Structure to store utilization value and process Id
 */
typedef struct nvmlProcessUtilizationSample_st {
  unsigned int pid;              //!< PID of process
  unsigned long long timeStamp;  //!< CPU Timestamp in microseconds
  unsigned int smUtil;           //!< SM (3D/Compute) Util Value
  unsigned int memUtil;          //!< Frame Buffer Memory Util Value
  unsigned int encUtil;          //!< Encoder Util Value
  unsigned int decUtil;          //!< Decoder Util Value
} nvmlProcessUtilizationSample_t;

/**
 * ECC counter types.
 *
 * Note: Volatile counts are reset each time the driver loads. On Windows this
 * is once per boot. On Linux this can be more frequent. On Linux the driver
 * unloads when no active clients exist. If persistence mode is enabled or there
 * is always a driver client active (e.g. X11), then Linux also sees per-boot
 * behavior. If not, volatile counts are reset each time a compute app is run.
 */
typedef enum nvmlEccCounterType_enum {
  NVML_VOLATILE_ECC =
  0,  //!< Volatile counts are reset each time the driver loads.
  NVML_AGGREGATE_ECC = 1,  //!< Aggregate counts persist across reboots (i.e.
  //!< for the lifetime of the device)

  // Keep this last
  NVML_ECC_COUNTER_TYPE_COUNT  //!< Count of memory counter types
} nvmlEccCounterType_t;

/**
 * Generic enable/disable enum.
 */
typedef enum nvmlEnableState_enum {
  NVML_FEATURE_DISABLED = 0,  //!< Feature disabled
  NVML_FEATURE_ENABLED = 1    //!< Feature enabled
} nvmlEnableState_t;

/**
 * Describes accounting statistics of a process.
 */
typedef struct nvmlAccountingStats_st {
  unsigned int
      gpuUtilization;  //!< Percent of time over the process's lifetime during
  //!< which one or more kernels was executing on the GPU.
  //! Utilization stats just like returned by \ref nvmlDeviceGetUtilizationRates
  //! but for the life time of a process (not just the last sample period). Set
  //! to NVML_VALUE_NOT_AVAILABLE if nvmlDeviceGetUtilizationRates is not
  //! supported

  unsigned int memoryUtilization;  //!< Percent of time over the process's
  //!< lifetime during which global (device)
  //!< memory was being read or written.
  //! Set to NVML_VALUE_NOT_AVAILABLE if nvmlDeviceGetUtilizationRates is not
  //! supported

  unsigned long long maxMemoryUsage;  //!< Maximum total memory in bytes that
  //!< was ever allocated by the process.
  //! Set to NVML_VALUE_NOT_AVAILABLE if nvmlProcessInfo_t->usedGpuMemory is not
  //! supported

  unsigned long long time;  //!< Amount of time in ms during which the compute
  //!< context was active. The time is reported as 0 if
  //!< the process is not terminated

  unsigned long long startTime;  //!< CPU Timestamp in usec representing start
  //!< time for the process

  unsigned int isRunning;  //!< Flag to represent if the process is running (1
  //!< for running, 0 for terminated)

  unsigned int reserved[5];  //!< Reserved for future use
} nvmlAccountingStats_t;

typedef unsigned int nvmlVgpuInstance_t;

/**
 * API types that allow changes to default permission restrictions
 */
typedef enum nvmlRestrictedAPI_enum {
  NVML_RESTRICTED_API_SET_APPLICATION_CLOCKS =
  0,  //!< APIs that change application clocks, see
  //!< nvmlDeviceSetApplicationsClocks
  //!< and see nvmlDeviceResetApplicationsClocks
  NVML_RESTRICTED_API_SET_AUTO_BOOSTED_CLOCKS =
  1,  //!< APIs that enable/disable Auto Boosted clocks
  //!< see nvmlDeviceSetAutoBoostedClocksEnabled
  // Keep this last
  NVML_RESTRICTED_API_COUNT
} nvmlRestrictedAPI_t;

/**
 * Clock types.
 *
 * All speeds are in Mhz.
 */
typedef enum nvmlClockType_enum {
  NVML_CLOCK_GRAPHICS = 0,  //!< Graphics clock domain
  NVML_CLOCK_SM = 1,        //!< SM clock domain
  NVML_CLOCK_MEM = 2,       //!< Memory clock domain
  NVML_CLOCK_VIDEO = 3,     //!< Video encoder/decoder clock domain

  // Keep this last
  NVML_CLOCK_COUNT  //<! Count of clock types
} nvmlClockType_t;

/**
 * BAR1 Memory allocation Information for a device
 */
typedef struct nvmlBAR1Memory_st {
  unsigned long long bar1Total;  //!< Total BAR1 Memory (in bytes)
  unsigned long long bar1Free;   //!< Unallocated BAR1 Memory (in bytes)
  unsigned long long bar1Used;   //!< Allocated Used Memory (in bytes)
} nvmlBAR1Memory_t;

/**
 *  * The Brand of the GPU
 *   */
typedef enum nvmlBrandType_enum {
  NVML_BRAND_UNKNOWN = 0,
  NVML_BRAND_QUADRO = 1,
  NVML_BRAND_TESLA = 2,
  NVML_BRAND_NVS = 3,
  NVML_BRAND_GRID = 4,
  NVML_BRAND_GEFORCE = 5,
  NVML_BRAND_TITAN = 6,

  // Keep this last
  NVML_BRAND_COUNT
} nvmlBrandType_t;

/**
 * Enum to represent type of bridge chip
 */
typedef enum nvmlBridgeChipType_enum {
  NVML_BRIDGE_CHIP_PLX = 0,
  NVML_BRIDGE_CHIP_BRO4 = 1
} nvmlBridgeChipType_t;

/**
 * Information about the Bridge Chip Firmware
 */
typedef struct nvmlBridgeChipInfo_st {
  nvmlBridgeChipType_t type;  //!< Type of Bridge Chip
  unsigned int fwVersion;     //!< Firmware Version. 0=Version is unavailable
} nvmlBridgeChipInfo_t;

/**
 * This structure stores the complete Hierarchy of the Bridge Chip within the
 * board. The immediate bridge is stored at index 0 of bridgeInfoList, parent to
 * immediate bridge is at index 1 and so forth.
 */
typedef struct nvmlBridgeChipHierarchy_st {
  unsigned char bridgeCount;  //!< Number of Bridge Chips on the Board
  nvmlBridgeChipInfo_t
      bridgeChipInfo[NVML_MAX_PHYSICAL_BRIDGE];  //!< Hierarchy of Bridge Chips
  //!< on the board
} nvmlBridgeChipHierarchy_t;

/**
 * Clock Ids.  These are used in combination with nvmlClockType_t
 * to specify a single clock value.
 */
typedef enum nvmlClockId_enum {
  NVML_CLOCK_ID_CURRENT = 0,             //!< Current actual clock value
  NVML_CLOCK_ID_APP_CLOCK_TARGET = 1,    //!< Target application clock
  NVML_CLOCK_ID_APP_CLOCK_DEFAULT = 2,   //!< Default application clock target
  NVML_CLOCK_ID_CUSTOMER_BOOST_MAX = 3,  //!< OEM-defined maximum clock rate

  // Keep this last
  NVML_CLOCK_ID_COUNT  //<! Count of Clock Ids.
} nvmlClockId_t;

/**
 * Compute mode.
 *
 * NVML_COMPUTEMODE_EXCLUSIVE_PROCESS was added in CUDA 4.0.
 * Earlier CUDA versions supported a single exclusive mode,
 * which is equivalent to NVML_COMPUTEMODE_EXCLUSIVE_THREAD in CUDA 4.0 and
 * beyond.
 */
typedef enum nvmlComputeMode_enum {
  NVML_COMPUTEMODE_DEFAULT =
  0,  //!< Default compute mode -- multiple contexts per device
  NVML_COMPUTEMODE_EXCLUSIVE_THREAD = 1,  //!< Support Removed
  NVML_COMPUTEMODE_PROHIBITED =
  2,  //!< Compute-prohibited mode -- no contexts per device
  NVML_COMPUTEMODE_EXCLUSIVE_PROCESS =
  3,  //!< Compute-exclusive-process mode -- only one context per device,
  //!< usable from multiple threads at a time

  // Keep this last
  NVML_COMPUTEMODE_COUNT
} nvmlComputeMode_t;

typedef unsigned int nvmlVgpuTypeId_t;

#define nvmlEccBitType_t nvmlMemoryErrorType_t

/**
 * Detailed ECC error counts for a device.
 *
 * @deprecated  Different GPU families can have different memory error counters
 *              See \ref nvmlDeviceGetMemoryErrorCounter
 */
typedef struct nvmlEccErrorCounts_st {
  unsigned long long l1Cache;       //!< L1 cache errors
  unsigned long long l2Cache;       //!< L2 cache errors
  unsigned long long deviceMemory;  //!< Device memory errors
  unsigned long long registerFile;  //!< Register file errors
} nvmlEccErrorCounts_t;

/**
 * Driver models.
 *
 * Windows only.
 */
typedef enum nvmlDriverModel_enum {
  NVML_DRIVER_WDDM =
  0,  //!< WDDM driver model -- GPU treated as a display device
  NVML_DRIVER_WDM =
  1  //!< WDM (TCC) model (recommended) -- GPU treated as a generic device
} nvmlDriverModel_t;

/*
 * Represents type of encoder for capacity can be queried
 */
typedef enum nvmlEncoderQueryType_enum {
  NVML_ENCODER_QUERY_H264 = 0,
  NVML_ENCODER_QUERY_HEVC = 1,
} nvmlEncoderType_t;

/*
 * Struct to hold encoder session data
 */
typedef struct nvmlEncoderSessionInfo_st {
  unsigned int sessionId;           //!< Unique session ID
  unsigned int pid;                 //!< Owning process ID
  nvmlVgpuInstance_t vgpuInstance;  //!< Owning vGPU instance ID (only valid on
  //!< vGPU hosts, otherwise zero)
  nvmlEncoderType_t codecType;  //!< Video encoder type
  unsigned int hResolution;     //!< Current encode horizontal resolution
  unsigned int vResolution;     //!< Current encode vertical resolution
  unsigned int averageFps;      //!< Moving average encode frames per second
  unsigned int
      averageLatency;  //!< Moving average encode latency in microseconds
} nvmlEncoderSessionInfo_t;

/**
 * Represents the type for sample value returned
 */
typedef enum nvmlValueType_enum {
  NVML_VALUE_TYPE_DOUBLE = 0,
  NVML_VALUE_TYPE_UNSIGNED_INT = 1,
  NVML_VALUE_TYPE_UNSIGNED_LONG = 2,
  NVML_VALUE_TYPE_UNSIGNED_LONG_LONG = 3,
  NVML_VALUE_TYPE_SIGNED_LONG_LONG = 4,

  // Keep this last
  NVML_VALUE_TYPE_COUNT
} nvmlValueType_t;

/**
 * Union to represent different types of Value
 */
typedef union nvmlValue_st {
  double dVal;                //!< If the value is double
  unsigned int uiVal;         //!< If the value is unsigned int
  unsigned long ulVal;        //!< If the value is unsigned long
  unsigned long long ullVal;  //!< If the value is unsigned long long
  signed long long sllVal;    //!< If the value is signed long long
} nvmlValue_t;

/**
 * Information for a Field Value Sample
 */
typedef struct nvmlFieldValue_st {
  unsigned int fieldId;  //!< ID of the NVML field to retrieve. This must be set
  //!< before any call that uses this struct. See the
  //!< constants starting with NVML_FI_ above.
  unsigned int unused;  //!< Currently unused. This should be initialized to 0
  //!< by the caller before any API call
  long long
      timestamp;  //!< CPU Timestamp of this value in microseconds since 1970
  long long
      latencyUsec;  //!< How long this field value took to update (in usec)
  //!< within NVML. This may be averaged across several
  //!< fields that are serviced by the same driver call.
  nvmlValueType_t valueType;  //!< Type of the value stored in value
  nvmlReturn_t
      nvmlReturn;  //!< Return code for retrieving this value. This must
  //!< be checked before looking at value, as value is
  //!< undefined if nvmlReturn != NVML_SUCCESS
  nvmlValue_t
      value;  //!< Value for this field. This is only valid if nvmlReturn
  //!< == NVML_SUCCESS
} nvmlFieldValue_t;

/**
 * GPU Operation Mode
 *
 * GOM allows to reduce power usage and optimize GPU throughput by disabling GPU
 * features.
 *
 * Each GOM is designed to meet specific user needs.
 */
typedef enum nvmlGom_enum {
  NVML_GOM_ALL_ON = 0,  //!< Everything is enabled and running at full speed

  NVML_GOM_COMPUTE =
  1,  //!< Designed for running only compute tasks. Graphics operations
  //!< are not allowed

  NVML_GOM_LOW_DP = 2  //!< Designed for running graphics applications that
  //!< don't require high bandwidth double precision
} nvmlGpuOperationMode_t;

// GRID license feature code
typedef enum {
  NVML_GRID_LICENSE_FEATURE_CODE_VGPU = 1,         // Virtual GPU
  NVML_GRID_LICENSE_FEATURE_CODE_VWORKSTATION = 2  // Virtual Workstation
} nvmlGridLicenseFeatureCode_t;

/**
 * Structure to store GRID licensable features
 */
typedef struct nvmlGridLicensableFeature_st {
  nvmlGridLicenseFeatureCode_t featureCode;  //<! Licensed feature code
  unsigned int featureState;  //<! Non-zero if feature is currently licensed,
  // otherwise zero
  char licenseInfo[NVML_GRID_LICENSE_BUFFER_SIZE];
} nvmlGridLicensableFeature_t;

typedef struct nvmlGridLicensableFeatures_st {
  int isGridLicenseSupported;  //<! Non-zero if GRID Software Licensing is
  // supported on the system, otherwise zero
  unsigned int licensableFeaturesCount;  //<! Entries returned in \a
  // gridLicensableFeatures array
  nvmlGridLicensableFeature_t
      gridLicensableFeatures[NVML_GRID_LICENSE_FEATURE_MAX_COUNT];
} nvmlGridLicensableFeatures_t;

/**
 * Available infoROM objects.
 */
typedef enum nvmlInforomObject_enum {
  NVML_INFOROM_OEM = 0,  //!< An object defined by OEM
  NVML_INFOROM_ECC =
  1,  //!< The ECC object determining the level of ECC support
  NVML_INFOROM_POWER = 2,  //!< The power management object

  // Keep this last
  NVML_INFOROM_COUNT  //!< This counts the number of infoROM objects the driver
  //!< knows about
} nvmlInforomObject_t;

/**
 * Memory error types
 */
typedef enum nvmlMemoryErrorType_enum {
  /**
   * A memory error that was corrected
   *
   * For ECC errors, these are single bit errors
   * For Texture memory, these are errors fixed by resend
   */
  NVML_MEMORY_ERROR_TYPE_CORRECTED = 0,
  /**
   * A memory error that was not corrected
   *
   * For ECC errors, these are double bit errors
   * For Texture memory, these are errors where the resend fails
   */
  NVML_MEMORY_ERROR_TYPE_UNCORRECTED = 1,

  // Keep this last
  NVML_MEMORY_ERROR_TYPE_COUNT  //!< Count of memory error types

} nvmlMemoryErrorType_t;

/**
 * Memory locations
 *
 * See \ref nvmlDeviceGetMemoryErrorCounter
 */
typedef enum nvmlMemoryLocation_enum {
  NVML_MEMORY_LOCATION_L1_CACHE = 0,        //!< GPU L1 Cache
  NVML_MEMORY_LOCATION_L2_CACHE = 1,        //!< GPU L2 Cache
  NVML_MEMORY_LOCATION_DEVICE_MEMORY = 2,   //!< GPU Device Memory
  NVML_MEMORY_LOCATION_REGISTER_FILE = 3,   //!< GPU Register File
  NVML_MEMORY_LOCATION_TEXTURE_MEMORY = 4,  //!< GPU Texture Memory
  NVML_MEMORY_LOCATION_TEXTURE_SHM = 5,     //!< Shared memory
  NVML_MEMORY_LOCATION_CBU = 6,             //!< CBU

  // Keep this last
  NVML_MEMORY_LOCATION_COUNT  //!< This counts the number of memory locations
  //!< the driver knows about
} nvmlMemoryLocation_t;

/**
 * Memory allocation information for a device.
 */
typedef struct nvmlMemory_st {
  unsigned long long total;  //!< Total installed FB memory (in bytes)
  unsigned long long free;   //!< Unallocated FB memory (in bytes)
  unsigned long long
      used;  //!< Allocated FB memory (in bytes). Note that the driver/GPU
  //!< always sets aside a small amount of memory for bookkeeping
} nvmlMemory_t;

/**
 * Memory allocation information for a device (v2).
 * 
 * Version 2 adds versioning for the struct and the amount of system-reserved memory as an output.
 * @note The \ref nvmlMemory_v2_t.used amount also includes the \ref nvmlMemory_v2_t.reserved amount.
 */
typedef struct nvmlMemory_v2_st
{
    unsigned int version;            //!< Structure format version (must be 2)
    unsigned long long total;        //!< Total physical device memory (in bytes)
    unsigned long long reserved;     //!< Device memory (in bytes) reserved for system use (driver or firmware)
    unsigned long long free;         //!< Unallocated device memory (in bytes)
    unsigned long long used;         //!< Allocated device memory (in bytes). Note that the driver/GPU always sets aside a small amount of memory for bookkeeping
} nvmlMemory_v2_t;


/**
 * Enum to represent NvLink queryable capabilities
 */
typedef enum nvmlNvLinkCapability_enum {
  NVML_NVLINK_CAP_P2P_SUPPORTED = 0,   // P2P over NVLink is supported
  NVML_NVLINK_CAP_SYSMEM_ACCESS = 1,   // Access to system memory is supported
  NVML_NVLINK_CAP_P2P_ATOMICS = 2,     // P2P atomics are supported
  NVML_NVLINK_CAP_SYSMEM_ATOMICS = 3,  // System memory atomics are supported
  NVML_NVLINK_CAP_SLI_BRIDGE = 4,      // SLI is supported over this link
  NVML_NVLINK_CAP_VALID = 5,           // Link is supported on this device
  // should be last
  NVML_NVLINK_CAP_COUNT
} nvmlNvLinkCapability_t;

/**
 * Enum to represent NvLink queryable error counters
 */
typedef enum nvmlNvLinkErrorCounter_enum {
  NVML_NVLINK_ERROR_DL_REPLAY = 0,  // Data link transmit replay error counter
  NVML_NVLINK_ERROR_DL_RECOVERY =
  1,  // Data link transmit recovery error counter
  NVML_NVLINK_ERROR_DL_CRC_FLIT =
  2,  // Data link receive flow control digit CRC error counter
  NVML_NVLINK_ERROR_DL_CRC_DATA =
  3,  // Data link receive data CRC error counter

  // this must be last
  NVML_NVLINK_ERROR_COUNT
} nvmlNvLinkErrorCounter_t;

/**
 * Enum to represent the NvLink utilization counter packet units
 */
typedef enum nvmlNvLinkUtilizationCountUnits_enum {
  NVML_NVLINK_COUNTER_UNIT_CYCLES = 0,   // count by cycles
  NVML_NVLINK_COUNTER_UNIT_PACKETS = 1,  // count by packets
  NVML_NVLINK_COUNTER_UNIT_BYTES = 2,    // count by bytes

  // this must be last
  NVML_NVLINK_COUNTER_UNIT_COUNT
} nvmlNvLinkUtilizationCountUnits_t;

/**
 * Enum to represent the NvLink utilization counter packet types to count
 *  ** this is ONLY applicable with the units as packets or bytes
 *  ** as specified in \a nvmlNvLinkUtilizationCountUnits_t
 *  ** all packet filter descriptions are target GPU centric
 *  ** these can be "OR'd" together
 */
typedef enum nvmlNvLinkUtilizationCountPktTypes_enum {
  NVML_NVLINK_COUNTER_PKTFILTER_NOP = 0x1,      // no operation packets
  NVML_NVLINK_COUNTER_PKTFILTER_READ = 0x2,     // read packets
  NVML_NVLINK_COUNTER_PKTFILTER_WRITE = 0x4,    // write packets
  NVML_NVLINK_COUNTER_PKTFILTER_RATOM = 0x8,    // reduction atomic requests
  NVML_NVLINK_COUNTER_PKTFILTER_NRATOM = 0x10,  // non-reduction atomic requests
  NVML_NVLINK_COUNTER_PKTFILTER_FLUSH = 0x20,   // flush requests
  NVML_NVLINK_COUNTER_PKTFILTER_RESPDATA = 0x40,    // responses with data
  NVML_NVLINK_COUNTER_PKTFILTER_RESPNODATA = 0x80,  // responses without data
  NVML_NVLINK_COUNTER_PKTFILTER_ALL = 0xFF          // all packets
} nvmlNvLinkUtilizationCountPktTypes_t;

/**
 * Struct to define the NVLINK counter controls
 */
typedef struct nvmlNvLinkUtilizationControl_st {
  nvmlNvLinkUtilizationCountUnits_t units;
  nvmlNvLinkUtilizationCountPktTypes_t pktfilter;
} nvmlNvLinkUtilizationControl_t;

/* P2P Capability Index*/
typedef enum nvmlGpuP2PCapsIndex_enum {
  NVML_P2P_CAPS_INDEX_READ = 0,
  NVML_P2P_CAPS_INDEX_WRITE,
  NVML_P2P_CAPS_INDEX_NVLINK,
  NVML_P2P_CAPS_INDEX_ATOMICS,
  NVML_P2P_CAPS_INDEX_PROP,
  NVML_P2P_CAPS_INDEX_UNKNOWN
} nvmlGpuP2PCapsIndex_t;

/* P2P Capability Index Status*/
typedef enum nvmlGpuP2PStatus_enum {
  NVML_P2P_STATUS_OK = 0,
  NVML_P2P_STATUS_CHIPSET_NOT_SUPPORED,
  NVML_P2P_STATUS_GPU_NOT_SUPPORTED,
  NVML_P2P_STATUS_IOH_TOPOLOGY_NOT_SUPPORTED,
  NVML_P2P_STATUS_DISABLED_BY_REGKEY,
  NVML_P2P_STATUS_NOT_SUPPORTED,
  NVML_P2P_STATUS_UNKNOWN

} nvmlGpuP2PStatus_t;

/**
 * Represents the queryable PCIe utilization counters
 */
typedef enum nvmlPcieUtilCounter_enum {
  NVML_PCIE_UTIL_TX_BYTES = 0,  // 1KB granularity
  NVML_PCIE_UTIL_RX_BYTES = 1,  // 1KB granularity

  // Keep this last
  NVML_PCIE_UTIL_COUNT
} nvmlPcieUtilCounter_t;

/**
 * Allowed PStates.
 */
typedef enum nvmlPStates_enum {
  NVML_PSTATE_0 = 0,        //!< Performance state 0 -- Maximum Performance
  NVML_PSTATE_1 = 1,        //!< Performance state 1
  NVML_PSTATE_2 = 2,        //!< Performance state 2
  NVML_PSTATE_3 = 3,        //!< Performance state 3
  NVML_PSTATE_4 = 4,        //!< Performance state 4
  NVML_PSTATE_5 = 5,        //!< Performance state 5
  NVML_PSTATE_6 = 6,        //!< Performance state 6
  NVML_PSTATE_7 = 7,        //!< Performance state 7
  NVML_PSTATE_8 = 8,        //!< Performance state 8
  NVML_PSTATE_9 = 9,        //!< Performance state 9
  NVML_PSTATE_10 = 10,      //!< Performance state 10
  NVML_PSTATE_11 = 11,      //!< Performance state 11
  NVML_PSTATE_12 = 12,      //!< Performance state 12
  NVML_PSTATE_13 = 13,      //!< Performance state 13
  NVML_PSTATE_14 = 14,      //!< Performance state 14
  NVML_PSTATE_15 = 15,      //!< Performance state 15 -- Minimum Performance
  NVML_PSTATE_UNKNOWN = 32  //!< Unknown performance state
} nvmlPstates_t;

/**
 * Causes for page retirement
 */
typedef enum nvmlPageRetirementCause_enum {
  NVML_PAGE_RETIREMENT_CAUSE_MULTIPLE_SINGLE_BIT_ECC_ERRORS =
  0,  //!< Page was retired due to multiple single bit ECC error
  NVML_PAGE_RETIREMENT_CAUSE_DOUBLE_BIT_ECC_ERROR =
  1,  //!< Page was retired due to double bit ECC error

  // Keep this last
  NVML_PAGE_RETIREMENT_CAUSE_COUNT
} nvmlPageRetirementCause_t;

/**
 *  Represents Type of Sampling Event
 */
typedef enum nvmlSamplingType_enum {
  NVML_TOTAL_POWER_SAMPLES = 0,  //!< To represent total power drawn by GPU
  NVML_GPU_UTILIZATION_SAMPLES =
  1,  //!< To represent percent of time during which one or more kernels was
  //!< executing on the GPU
  NVML_MEMORY_UTILIZATION_SAMPLES =
  2,  //!< To represent percent of time during which global (device) memory
  //!< was being read or written
  NVML_ENC_UTILIZATION_SAMPLES =
  3,  //!< To represent percent of time during which NVENC remains busy
  NVML_DEC_UTILIZATION_SAMPLES =
  4,  //!< To represent percent of time during which NVDEC remains busy
  NVML_PROCESSOR_CLK_SAMPLES = 5,  //!< To represent processor clock samples
  NVML_MEMORY_CLK_SAMPLES = 6,     //!< To represent memory clock samples

  // Keep this last
  NVML_SAMPLINGTYPE_COUNT
} nvmlSamplingType_t;

/**
 * Information for Sample
 */
typedef struct nvmlSample_st {
  unsigned long long timeStamp;  //!< CPU Timestamp in microseconds
  nvmlValue_t sampleValue;       //!< Sample Value
} nvmlSample_t;

/**
 * Temperature sensors.
 */
typedef enum nvmlTemperatureSensors_enum {
  NVML_TEMPERATURE_GPU = 0,  //!< Temperature sensor for the GPU die

  // Keep this last
  NVML_TEMPERATURE_COUNT
} nvmlTemperatureSensors_t;

/**
 * Temperature thresholds.
 */
typedef enum nvmlTemperatureThresholds_enum {
  NVML_TEMPERATURE_THRESHOLD_SHUTDOWN = 0,  // Temperature at which the GPU will
  // shut down for HW protection
  NVML_TEMPERATURE_THRESHOLD_SLOWDOWN =
  1,  // Temperature at which the GPU will begin HW slowdown
  NVML_TEMPERATURE_THRESHOLD_MEM_MAX =
  2,  // Memory Temperature at which the GPU will begin SW slowdown
  NVML_TEMPERATURE_THRESHOLD_GPU_MAX =
  3,  // GPU Temperature at which the GPU can be throttled below base clock
  // Keep this last
  NVML_TEMPERATURE_THRESHOLD_COUNT
} nvmlTemperatureThresholds_t;

/**
 * Represents level relationships within a system between two GPUs
 * The enums are spaced to allow for future relationships
 */
typedef enum nvmlGpuLevel_enum {
  NVML_TOPOLOGY_INTERNAL = 0,  // e.g. Tesla K80
  NVML_TOPOLOGY_SINGLE =
  10,  // all devices that only need traverse a single PCIe switch
  NVML_TOPOLOGY_MULTIPLE =
  20,  // all devices that need not traverse a host bridge
  NVML_TOPOLOGY_HOSTBRIDGE =
  30,  // all devices that are connected to the same host bridge
  NVML_TOPOLOGY_NODE = 40,    // all devices that are connected to the same NUMA
  // node but possibly multiple host bridges
  NVML_TOPOLOGY_SYSTEM = 50,  // all devices in the system

  // there is purposefully no COUNT here because of the need for spacing above
} nvmlGpuTopologyLevel_t;

/**
 * Utilization information for a device.
 * Each sample period may be between 1 second and 1/6 second, depending on the
 * product being queried.
 */
typedef struct nvmlUtilization_st {
  unsigned int gpu;  //!< Percent of time over the past sample period during
  //!< which one or more kernels was executing on the GPU
  unsigned int
      memory;  //!< Percent of time over the past sample period during which
  //!< global (device) memory was being read or written
} nvmlUtilization_t;

/**
 * Physical GPU metadata structure
 */
typedef struct nvmlVgpuPgpuMetadata_st {
  unsigned int version;   //!< Current version of the structure
  unsigned int revision;  //!< Current revision of the structure
  char hostDriverVersion
  [NVML_SYSTEM_DRIVER_VERSION_BUFFER_SIZE];  //!< Host driver version
  unsigned int
      pgpuVirtualizationCaps;   //!< Pgpu virtualizaion capabilities bitfileld
  unsigned int reserved[7];     //!< Reserved for internal use
  unsigned int opaqueDataSize;  //!< Size of opaque data field in bytes
  char opaqueData[4];           //!< Opaque data
} nvmlVgpuPgpuMetadata_t;

/**
 * Structure to store Utilization Value, vgpuInstance and subprocess information
 */
typedef struct nvmlVgpuProcessUtilizationSample_st {
  nvmlVgpuInstance_t vgpuInstance;  //!< vGPU Instance
  unsigned int pid;  //!< PID of process running within the vGPU VM
  char processName[NVML_VGPU_NAME_BUFFER_SIZE];  //!< Name of process running
  //!< within the vGPU VM
  unsigned long long timeStamp;  //!< CPU Timestamp in microseconds
  unsigned int smUtil;           //!< SM (3D/Compute) Util Value
  unsigned int memUtil;          //!< Frame Buffer Memory Util Value
  unsigned int encUtil;          //!< Encoder Util Value
  unsigned int decUtil;          //!< Decoder Util Value
} nvmlVgpuProcessUtilizationSample_t;

/**
 * Structure to store Utilization Value and vgpuInstance
 */
typedef struct nvmlVgpuInstanceUtilizationSample_st {
  nvmlVgpuInstance_t vgpuInstance;  //!< vGPU Instance
  unsigned long long timeStamp;     //!< CPU Timestamp in microseconds
  nvmlValue_t smUtil;               //!< SM (3D/Compute) Util Value
  nvmlValue_t memUtil;              //!< Frame Buffer Memory Util Value
  nvmlValue_t encUtil;              //!< Encoder Util Value
  nvmlValue_t decUtil;              //!< Decoder Util Value
} nvmlVgpuInstanceUtilizationSample_t;

/**
 * Represents type of perf policy for which violation times can be queried
 */
typedef enum nvmlPerfPolicyType_enum {
  NVML_PERF_POLICY_POWER = 0,  //!< How long did power violations cause the GPU
  //!< to be below application clocks
  NVML_PERF_POLICY_THERMAL = 1,  //!< How long did thermal violations cause the
  //!< GPU to be below application clocks
  NVML_PERF_POLICY_SYNC_BOOST = 2,   //!< How long did sync boost cause the GPU
  //!< to be below application clocks
  NVML_PERF_POLICY_BOARD_LIMIT = 3,  //!< How long did the board limit cause the
  //!< GPU to be below application clocks
  NVML_PERF_POLICY_LOW_UTILIZATION =
  4,  //!< How long did low utilization cause the GPU to be below
  //!< application clocks
  NVML_PERF_POLICY_RELIABILITY =
  5,  //!< How long did the board reliability limit cause the GPU to be
  //!< below application clocks

  NVML_PERF_POLICY_TOTAL_APP_CLOCKS =
  10,  //!< Total time the GPU was held below application clocks by any
  //!< limiter (0 - 5 above)
  NVML_PERF_POLICY_TOTAL_BASE_CLOCKS =
  11,  //!< Total time the GPU was held below base clocks

  // Keep this last
  NVML_PERF_POLICY_COUNT
} nvmlPerfPolicyType_t;

/**
 * Struct to hold perf policy violation status data
 */
typedef struct nvmlViolationTime_st {
  unsigned long long referenceTime;  //!< referenceTime represents CPU timestamp
  //!< in microseconds
  unsigned long long violationTime;  //!< violationTime in Nanoseconds
} nvmlViolationTime_t;

/*!
 * GPU virtualization mode types.
 */
typedef enum nvmlGpuVirtualizationMode {
  NVML_GPU_VIRTUALIZATION_MODE_NONE = 0,  //!< Represents Bare Metal GPU
  NVML_GPU_VIRTUALIZATION_MODE_PASSTHROUGH =
  1,  //!< Device is associated with GPU-Passthorugh
  NVML_GPU_VIRTUALIZATION_MODE_VGPU =
  2,  //!< Device is associated with vGPU inside virtual machine.
  NVML_GPU_VIRTUALIZATION_MODE_HOST_VGPU =
  3,  //!< Device is associated with VGX hypervisor in vGPU mode
  NVML_GPU_VIRTUALIZATION_MODE_HOST_VSGA =
  4,  //!< Device is associated with VGX hypervisor in vSGA mode
} nvmlGpuVirtualizationMode_t;

/**
 * Handle to an event set
 */
typedef struct nvmlEventSet_st *nvmlEventSet_t;

/**
 *  Is the GPU device to be removed from the kernel by nvmlDeviceRemoveGpu()
 */
typedef enum nvmlDetachGpuState_enum {
  NVML_DETACH_GPU_KEEP = 0,
  NVML_DETACH_GPU_REMOVE,
} nvmlDetachGpuState_t;

/**
 *  Parent bridge PCIe link state requested by nvmlDeviceRemoveGpu()
 */
typedef enum nvmlPcieLinkState_enum {
  NVML_PCIE_LINK_KEEP = 0,
  NVML_PCIE_LINK_SHUT_DOWN,
} nvmlPcieLinkState_t;

/**
 * Information about occurred event
 */
typedef struct nvmlEventData_st {
  nvmlDevice_t device;  //!< Specific device where the event occurred
  unsigned long long
      eventType;  //!< Information about what specific event occurred
  unsigned long long
      eventData;  //!< Stores last XID error for the device in the
  //!< event of nvmlEventTypeXidCriticalError,
  //  eventData is 0 for any other event. eventData is set as 999 for unknown
  //  xid error.
} nvmlEventData_t;

// vGPU GUEST info state.
typedef enum nvmlVgpuGuestInfoState_enum {
  NVML_VGPU_INSTANCE_GUEST_INFO_STATE_UNINITIALIZED =
  0,  //<! Guest-dependent fields uninitialized
  NVML_VGPU_INSTANCE_GUEST_INFO_STATE_INITIALIZED =
  1,  //<! Guest-dependent fields initialized
} nvmlVgpuGuestInfoState_t;

/**
 * vGPU metadata structure.
 */
typedef struct nvmlVgpuMetadata_st {
  unsigned int version;   //!< Current version of the structure
  unsigned int revision;  //!< Current revision of the structure
  nvmlVgpuGuestInfoState_t
      guestInfoState;  //!< Current state of Guest-dependent fields
  char guestDriverVersion
  [NVML_SYSTEM_DRIVER_VERSION_BUFFER_SIZE];  //!< Version of driver
  //!< installed in guest
  char hostDriverVersion[NVML_SYSTEM_DRIVER_VERSION_BUFFER_SIZE];  //!< Version
  //!< of driver
  //!< installed in host
  unsigned int reserved[8];     //!< Reserved for internal use
  unsigned int opaqueDataSize;  //!< Size of opaque data field in bytes
  char opaqueData[4];           //!< Opaque data
} nvmlVgpuMetadata_t;

/**
 * vGPU VM compatibility codes
 */
typedef enum nvmlVgpuVmCompatibility_enum {
  NVML_VGPU_VM_COMPATIBILITY_NONE = 0x0,  //!< vGPU is not runnable
  NVML_VGPU_VM_COMPATIBILITY_COLD =
  0x1,  //!< vGPU is runnable from a cold / powered-off state (ACPI S5)
  NVML_VGPU_VM_COMPATIBILITY_HIBERNATE =
  0x2,  //!< vGPU is runnable from a hibernated state (ACPI S4)
  NVML_VGPU_VM_COMPATIBILITY_SLEEP =
  0x4,  //!< vGPU is runnable from a sleeped state (ACPI S3)
  NVML_VGPU_VM_COMPATIBILITY_LIVE =
  0x8,  //!< vGPU is runnable from a live/paused (ACPI S0)
} nvmlVgpuVmCompatibility_t;

/**
 *  vGPU-pGPU compatibility limit codes
 */
typedef enum nvmlVgpuPgpuCompatibilityLimitCode_enum {
  NVML_VGPU_COMPATIBILITY_LIMIT_NONE = 0x0,  //!< Compatibility is not limited.
  NVML_VGPU_COMPATIBILITY_LIMIT_HOST_DRIVER =
  0x1,  //!< Compatibility is limited by host driver version.
  NVML_VGPU_COMPATIBILITY_LIMIT_GUEST_DRIVER =
  0x2,  //!< Compatibility is limited by guest driver version.
  NVML_VGPU_COMPATIBILITY_LIMIT_GPU =
  0x4,  //!< Compatibility is limited by GPU hardware.
  NVML_VGPU_COMPATIBILITY_LIMIT_OTHER =
  0x80000000,  //!< Compatibility is limited by an undefined factor.
} nvmlVgpuPgpuCompatibilityLimitCode_t;

/**
 * vGPU-pGPU compatibility structure
 */
typedef struct nvmlVgpuPgpuCompatibility_st {
  nvmlVgpuVmCompatibility_t
      vgpuVmCompatibility;  //!< Compatibility of vGPU VM. See \ref
  //!< nvmlVgpuVmCompatibility_t
  nvmlVgpuPgpuCompatibilityLimitCode_t
      compatibilityLimitCode;  //!< Limiting factor for vGPU-pGPU compatibility.
  //!< See \ref
  //!< nvmlVgpuPgpuCompatibilityLimitCode_t
} nvmlVgpuPgpuCompatibility_t;

/**
 * Description of HWBC entry
 */
typedef struct nvmlHwbcEntry_st {
  unsigned int hwbcId;
  char firmwareVersion[32];
} nvmlHwbcEntry_t;

typedef struct nvmlUnit_st *nvmlUnit_t;

/**
 * Fan state enum.
 */
typedef enum nvmlFanState_enum {
  NVML_FAN_NORMAL = 0,  //!< Fan is working properly
  NVML_FAN_FAILED = 1   //!< Fan has failed
} nvmlFanState_t;

/**
 * Fan speed reading for a single fan in an S-class unit.
 */
typedef struct nvmlUnitFanInfo_st {
  unsigned int speed;  //!< Fan speed (RPM)
  nvmlFanState_t
      state;  //!< Flag that indicates whether fan is working properly
} nvmlUnitFanInfo_t;
/**
 * Fan speed readings for an entire S-class unit.
 */
typedef struct nvmlUnitFanSpeeds_st {
  nvmlUnitFanInfo_t fans[24];  //!< Fan speed data for each fan
  unsigned int count;          //!< Number of fans in unit
} nvmlUnitFanSpeeds_t;

/**
 * Led color enum.
 */
typedef enum nvmlLedColor_enum {
  NVML_LED_COLOR_GREEN = 0,  //!< GREEN, indicates good health
  NVML_LED_COLOR_AMBER = 1   //!< AMBER, indicates problem
} nvmlLedColor_t;

/**
 * LED states for an S-class unit.
 */
typedef struct nvmlLedState_st {
  char cause[256];       //!< If amber, a text description of the cause
  nvmlLedColor_t color;  //!< GREEN or AMBER
} nvmlLedState_t;

/**
 * Power usage information for an S-class unit.
 * The power supply state is a human readable string that equals "Normal" or
 * contains a combination of "Abnormal" plus one or more of the following:
 *
 *    - High voltage
 *    - Fan failure
 *    - Heatsink temperature
 *    - Current limit
 *    - Voltage below UV alarm threshold
 *    - Low-voltage
 *    - SI2C remote off command
 *    - MOD_DISABLE input
 *    - Short pin transition
 */
typedef struct nvmlPSUInfo_st {
  char state[256];       //!< The power supply state
  unsigned int current;  //!< PSU current (A)
  unsigned int voltage;  //!< PSU voltage (V)
  unsigned int power;    //!< PSU power draw (W)
} nvmlPSUInfo_t;

/**
 * Static S-class unit info.
 */
typedef struct nvmlUnitInfo_st {
  char name[96];             //!< Product name
  char id[96];               //!< Product identifier
  char serial[96];           //!< Product serial number
  char firmwareVersion[96];  //!< Firmware version
} nvmlUnitInfo_t;

/*!
 * Types of VM identifiers
 */
typedef enum nvmlVgpuVmIdType {
  NVML_VGPU_VM_ID_DOMAIN_ID = 0,  //!< VM ID represents DOMAIN ID
  NVML_VGPU_VM_ID_UUID = 1,       //!< VM ID represents UUID
} nvmlVgpuVmIdType_t;

/**
 * Represents frame buffer capture session type
 */
typedef enum nvmlFBCSessionType_enum {
  NVML_FBC_SESSION_TYPE_UNKNOWN = 0,  //!< Unknwon
  NVML_FBC_SESSION_TYPE_TOSYS,        //!< ToSys
  NVML_FBC_SESSION_TYPE_CUDA,         //!< Cuda
  NVML_FBC_SESSION_TYPE_VID,          //!< Vid
  NVML_FBC_SESSION_TYPE_HWENC,        //!< HEnc
} nvmlFBCSessionType_t;

/**
 * Structure to hold FBC session data
 */
typedef struct nvmlFBCSessionInfo_st {
  unsigned int sessionId;           //!< Unique session ID
  unsigned int pid;                 //!< Owning process ID
  nvmlVgpuInstance_t vgpuInstance;  //!< Owning vGPU instance ID (only valid on
  //!< vGPU hosts, otherwise zero)
  unsigned int displayOrdinal;       //!< Display identifier
  nvmlFBCSessionType_t sessionType;  //!< Type of frame buffer capture session
  unsigned int sessionFlags;         //!< Session flags (one or more of
  //!< NVML_NVFBC_SESSION_FLAG_XXX).
  unsigned int hMaxResolution;  //!< Max horizontal resolution supported by the
  //!< capture session
  unsigned int vMaxResolution;  //!< Max vertical resolution supported by the
  //!< capture session
  unsigned int hResolution;  //!< Horizontal resolution requested by caller in
  //!< capture call
  unsigned int
      vResolution;  //!< Vertical resolution requested by caller in capture call
  unsigned int averageFPS;  //!< Moving average new frames captured per second
  unsigned int averageLatency;  //!< Moving average new frame capture latency in
  //!< microseconds
} nvmlFBCSessionInfo_t;

/**
 * Structure to hold frame buffer capture sessions stats
 */
typedef struct nvmlFBCStats_st {
  unsigned int sessionsCount;  //!< Total no of sessions
  unsigned int averageFPS;  //!< Moving average new frames captured per second
  unsigned int averageLatency;  //!< Moving average new frame capture latency in
  //!< microseconds
} nvmlFBCStats_t;

/**
 * Buffer size guaranteed to be large enough for \ref nvmlDeviceGetUUID
 */
#define NVML_DEVICE_UUID_BUFFER_SIZE 80

/**
 * Blacklist GPU device information
 **/
typedef struct nvmlBlacklistDeviceInfo_st {
  nvmlPciInfo_t pciInfo;  //!< The PCI information for the blacklisted GPU
  char uuid[NVML_DEVICE_UUID_BUFFER_SIZE];  //!< The ASCII string UUID for the
  //!< blacklisted GPU
} nvmlBlacklistDeviceInfo_t;

/**
 * Structure representing a range of vGPU version
 */
typedef struct nvmlVgpuVersion_st {
  unsigned int minVersion;  //!< Minimum vGPU version.
  unsigned int maxVersion;  //!< Maximum vGPU version.
} nvmlVgpuVersion_t;

/**
 * Host vGPU modes
 */
typedef enum nvmlHostVgpuMode_enum {
  NVML_HOST_VGPU_MODE_NON_SRIOV = 0,  //!< Non SR-IOV mode
  NVML_HOST_VGPU_MODE_SRIOV = 1       //!< SR-IOV mode
} nvmlHostVgpuMode_t;

typedef struct nvmlGpuInstance_st *nvmlGpuInstance_t;

typedef struct nvmlComputeInstanceInfo_st {
  nvmlDevice_t device;           //!< Parent device
  nvmlGpuInstance_t gpuInstance; //!< Parent GPU instance
  unsigned int id;               //!< Unique instance ID within the GPU instance
  unsigned int profileId;        //!< Unique profile ID within the GPU instance
} nvmlComputeInstanceInfo_t;

typedef struct nvmlComputeInstance_st *nvmlComputeInstance_t;

typedef unsigned int nvmlDeviceArchitecture_t;

typedef struct nvmlDeviceAttributes_st {
  unsigned int multiprocessorCount;   //!< Streaming Multiprocessor count
  unsigned int sharedCopyEngineCount; //!< Shared Copy Engine count
  unsigned int sharedDecoderCount;    //!< Shared Decoder Engine count
  unsigned int sharedEncoderCount;    //!< Shared Encoder Engine count
  unsigned int sharedJpegCount;       //!< Shared JPEG Engine count
  unsigned int sharedOfaCount;        //!< Shared OFA Engine count
} nvmlDeviceAttributes_t;

typedef unsigned int nvmlAffinityScope_t;

typedef struct nvmlGpuInstancePlacement_st {
  unsigned int start;
  unsigned int size;
} nvmlGpuInstancePlacement_t;

typedef struct nvmlGpuInstanceProfileInfo_st {
  unsigned int id;                  //!< Unique profile ID within the device
  unsigned int isP2pSupported;      //!< Peer-to-Peer support
  unsigned int sliceCount;          //!< GPU Slice count
  unsigned int instanceCount;       //!< GPU instance count
  unsigned int multiprocessorCount; //!< Streaming Multiprocessor count
  unsigned int copyEngineCount;     //!< Copy Engine count
  unsigned int decoderCount;        //!< Decoder Engine count
  unsigned int encoderCount;        //!< Encoder Engine count
  unsigned int jpegCount;           //!< JPEG Engine count
  unsigned int ofaCount;            //!< OFA Engine count
  unsigned long long memorySizeMB;  //!< Memory size in MBytes
} nvmlGpuInstanceProfileInfo_t;

typedef struct nvmlComputeInstanceProfileInfo_st {
  unsigned int id;                    //!< Unique profile ID within the GPU instance
  unsigned int sliceCount;            //!< GPU Slice count
  unsigned int instanceCount;         //!< Compute instance count
  unsigned int multiprocessorCount;   //!< Streaming Multiprocessor count
  unsigned int sharedCopyEngineCount; //!< Shared Copy Engine count
  unsigned int sharedDecoderCount;    //!< Shared Decoder Engine count
  unsigned int sharedEncoderCount;    //!< Shared Encoder Engine count
  unsigned int sharedJpegCount;       //!< Shared JPEG Engine count
  unsigned int sharedOfaCount;        //!< Shared OFA Engine count
} nvmlComputeInstanceProfileInfo_t;

typedef struct nvmlGpuInstanceInfo_st {
  nvmlDevice_t device;                  //!< Parent device
  unsigned int id;                      //!< Unique instance ID within the device
  unsigned int profileId;               //!< Unique profile ID within the device
  nvmlGpuInstancePlacement_t placement; //!< Placement for this instance
} nvmlGpuInstanceInfo_t;

/**
 * Possible values that classify the remap availability for each bank. The max
 * field will contain the number of banks that have maximum remap availability
 * (all reserved rows are available). None means that there are no reserved
 * rows available.
 */
typedef struct nvmlRowRemapperHistogramValues_st {
  unsigned int max;
  unsigned int high;
  unsigned int partial;
  unsigned int low;
  unsigned int none;
} nvmlRowRemapperHistogramValues_t;

#ifdef __cplusplus
}
#endif

#endif  // HIJACK_DRIVER_SUBSET_H

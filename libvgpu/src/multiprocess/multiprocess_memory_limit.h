#ifndef __MULTIPROCESS_MEMORY_LIMIT_H__
#define __MULTIPROCESS_MEMORY_LIMIT_H__

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <cuda.h>
#include <pthread.h>

#include "static_config.h"
#include "include/log_utils.h"


#define MULTIPROCESS_SHARED_REGION_MAGIC_FLAG  19920718
#define MULTIPROCESS_SHARED_REGION_CACHE_ENV   "CUDA_DEVICE_MEMORY_SHARED_CACHE"
#define MULTIPROCESS_SHARED_REGION_CACHE_DEFAULT  "/tmp/cudevshr.cache"
#define ENV_OVERRIDE_FILE "/overrideEnv"
#define CUDA_TASK_PRIORITY_ENV "CUDA_TASK_PRIORITY"

#define CUDA_DEVICE_MAX_COUNT 16
#define CUDA_DEVICE_MEMORY_UPDATE_SUCCESS 0
#define CUDA_DEVICE_MEMORY_UPDATE_FAILURE 1
#define MEMORY_LIMIT_TOLERATION_RATE 1.1

#define SHARED_REGION_SIZE_MAGIC  sizeof(shared_region_t)
#define SHARED_REGION_MAX_PROCESS_NUM 1024

// macros for debugging
#define SEQ_FIX_SHRREG_ACQUIRE_FLOCK_OK 0
#define SEQ_FIX_SHRREG_UPDATE_OWNER_OK 1
#define SEQ_FIX_SHRREG_RELEASE_FLOCK_OK 2
#define SEQ_ACQUIRE_SEMLOCK_OK 3
#define SEQ_UPDATE_OWNER_OK 4
#define SEQ_RESET_OWNER_OK 5
#define SEQ_RELEASE_SEMLOCK_OK 6
#define SEQ_BEFORE_UNLOCK_SHRREG 7

#define SEQ_AFTER_INC 8
#define SEQ_AFTER_DEC 9

#ifndef SEQ_POINT_MARK
    #define SEQ_POINT_MARK(s) 
#endif

#define FACTOR 32

typedef struct {
    uint64_t context_size;
    uint64_t module_size;
    uint64_t data_size;
    uint64_t offset;
    uint64_t total;
} device_memory_t;

typedef struct {
    int32_t pid;
    int32_t hostpid;
    device_memory_t used[CUDA_DEVICE_MAX_COUNT];
    uint64_t monitorused[CUDA_DEVICE_MAX_COUNT];
    int32_t status;
} shrreg_proc_slot_t;

typedef char uuid[96];

typedef struct {
    int32_t initialized_flag;
    int32_t sm_init_flag;
    size_t owner_pid;
    sem_t sem;
    uint64_t num;
    uuid uuids[CUDA_DEVICE_MAX_COUNT];
    uint64_t limit[CUDA_DEVICE_MAX_COUNT];
    uint64_t sm_limit[CUDA_DEVICE_MAX_COUNT];
    shrreg_proc_slot_t procs[SHARED_REGION_MAX_PROCESS_NUM];
    int proc_num;
    int utilization_switch;
    int recent_kernel;
    int priority;
} shared_region_t;

typedef struct {
    int32_t pid;
    int fd;
    pthread_once_t init_status;
    shared_region_t* shared_region; 
} shared_region_info_t;


typedef struct {
  size_t tid;
  CUcontext ctx;
} thread_context_map;

void ensure_initialized();

int get_current_device_sm_limit(int dev);
uint64_t get_current_device_memory_limit(const int dev);
int set_current_device_memory_limit(const int dev,size_t newlimit);
int set_current_device_sm_limit(int dev,int scale);
int set_current_device_sm_limit_scale(int dev,int scale);
int update_host_pid();
int set_host_pid(int hostpid);

uint64_t get_current_device_memory_monitor(const int dev);
uint64_t get_current_device_memory_usage(const int dev);
size_t get_gpu_memory_usage(const int dev);

// Priority-related
int get_current_priority();
int set_recent_kernel(int value);
int get_recent_kernel();
int get_utilization_switch();
int set_env_utilization_switch();

int set_gpu_device_memory_monitor(int32_t pid,int dev,size_t monitor);
int add_gpu_device_memory_usage(int32_t pid,int dev,size_t usage,int type);
int rm_gpu_device_memory_usage(int32_t pid,int dev,size_t usage,int type);

shrreg_proc_slot_t *find_proc_by_hostpid(int hostpid);
int active_oom_killer();

//void inc_current_device_memory_usage(const int dev, const uint64_t usage);
//void decl_current_device_memory_usage(const int dev, const uint64_t usage);

//int oom_check(const int dev,int addon);

void lock_shrreg();
void unlock_shrreg();

//Setspec of the corresponding device
int setspec();
//Remove quitted process
int rm_quitted_process();

void suspend_all();
void resume_all();
int wait_status_self(int status);
int wait_status_all(int status);

int load_env_from_file(char *filename);
int comparelwr(const char *s1,char *s2);
int put_device_info();
unsigned int nvml_to_cuda_map(unsigned int nvmldev);

#endif  // __MULTIPROCESS_MEMORY_LIMIT_H__


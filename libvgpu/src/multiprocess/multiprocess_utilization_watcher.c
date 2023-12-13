#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include <cuda.h>
#include "include/nvml_prefix.h"
#include <nvml.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "multiprocess/multiprocess_memory_limit.h"
#include "multiprocess/multiprocess_utilization_watcher.h"
#include "include/log_utils.h"
#include "include/nvml_override.h"


static int g_sm_num;
static int g_max_thread_per_sm;
static volatile int g_cur_cuda_cores = 0;
static volatile int g_total_cuda_cores = 0;
extern int pidfound;
int cuda_to_nvml_map[16];

void rate_limiter(int grids, int blocks) {
  int before_cuda_cores = 0;
  int after_cuda_cores = 0;
  int kernel_size = grids;

  while (get_recent_kernel()<0) {
    sleep(1);
  }
  set_recent_kernel(2);
  if ((get_current_device_sm_limit(0)>=100) || (get_current_device_sm_limit(0)==0))
    	return;
  if (get_utilization_switch()==0)
      return;
  LOG_DEBUG("grid: %d, blocks: %d", grids, blocks);
  LOG_DEBUG("launch kernel %d, curr core: %d", kernel_size, g_cur_cuda_cores);
  //if (g_vcuda_config.enable) {
    do {
CHECK:
      before_cuda_cores = g_cur_cuda_cores;
      LOG_DEBUG("current core: %d", g_cur_cuda_cores);
      if (before_cuda_cores < 0) {
        nanosleep(&g_cycle, NULL);
        goto CHECK;
      }
      after_cuda_cores = before_cuda_cores - kernel_size;
    } while (!CAS(&g_cur_cuda_cores, before_cuda_cores, after_cuda_cores));
  //}
}

static void change_token(int delta) {
  int cuda_cores_before = 0, cuda_cores_after = 0;

  LOG_DEBUG("delta: %d, curr: %d", delta, g_cur_cuda_cores);
  do {
    cuda_cores_before = g_cur_cuda_cores;
    cuda_cores_after = cuda_cores_before + delta;

    if (cuda_cores_after > g_total_cuda_cores) {
      cuda_cores_after = g_total_cuda_cores;
    }
  } while (!CAS(&g_cur_cuda_cores, cuda_cores_before, cuda_cores_after));
}

int delta(int up_limit, int user_current, int share) {
  int utilization_diff =
      abs(up_limit - user_current) < 5 ? 5 : abs(up_limit - user_current);
  int increment =
      g_sm_num * g_sm_num * g_max_thread_per_sm * utilization_diff / 2560;
    
  /* Accelerate cuda cores allocation when utilization vary widely */
  if (utilization_diff > up_limit / 2) {
    increment = increment * utilization_diff * 2 / (up_limit + 1);
  }

  if (user_current <= up_limit) {
    share = share + increment > g_total_cuda_cores ? g_total_cuda_cores
                                                   : share + increment;
  } else {
    share = share - increment < 0 ? 0 : share - increment;
  }

  return share;
}

unsigned int nvml_to_cuda_map(unsigned int nvmldev){
    unsigned int devcount;
    CHECK_NVML_API(nvmlDeviceGetCount_v2(&devcount));
    int i=0;
    for (i=0;i<devcount;i++){
        if (cuda_to_nvml_map[i]==nvmldev)
          return i;
    }
    return -1;
}

int setspec() {
    CHECK_NVML_API(nvmlInit());
    CHECK_CU_RESULT(cuDeviceGetAttribute(&g_sm_num,CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT,0));
    CHECK_CU_RESULT(cuDeviceGetAttribute(&g_max_thread_per_sm,CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_MULTIPROCESSOR,0));
    g_total_cuda_cores = g_max_thread_per_sm * g_sm_num * FACTOR;
    return 0;
}

int get_used_gpu_utilization(int *userutil,int *sysprocnum) {
    nvmlProcessUtilizationSample_t processes_sample[SHARED_REGION_MAX_PROCESS_NUM];
    unsigned int processes_num = SHARED_REGION_MAX_PROCESS_NUM;
    nvmlDevice_t device;
    struct timeval cur;
    size_t microsec;

    int i,sum=0;
    unsigned int infcount;
    size_t summonitor=0;
    nvmlProcessInfo_v1_t infos[SHARED_REGION_MAX_PROCESS_NUM];

    unsigned int nvmlCounts;
    CHECK_NVML_API(nvmlDeviceGetCount(&nvmlCounts));

    int devi,cudadev;
    for (devi=0;devi<nvmlCounts;devi++){
      sum=0;
      summonitor=0;
      shrreg_proc_slot_t *proc;
      cudadev = nvml_to_cuda_map((unsigned int)(devi));
      if (cudadev<0)
        continue;
      CHECK_NVML_API(nvmlDeviceGetHandleByIndex(cudadev, &device));
      nvmlReturn_t res = nvmlDeviceGetComputeRunningProcesses(device,&infcount,infos);
      if (res==NVML_ERROR_INSUFFICIENT_SIZE){
        continue;
      }
      gettimeofday(&cur,NULL);
      microsec = (cur.tv_sec - 1) * 1000UL * 1000UL + cur.tv_usec;
      res = nvmlDeviceGetProcessUtilization(device,processes_sample,&processes_num,microsec);
      if (res==NVML_ERROR_INSUFFICIENT_SIZE){
        userutil[cudadev] = 0;
        for (i=0; i<infcount; i++){
          proc = find_proc_by_hostpid(infos[i].pid);
          if (proc != NULL){
              LOG_DEBUG("pid=%u monitor=%lld\n",infos[i].pid,infos[i].usedGpuMemory);
              summonitor += infos[i].usedGpuMemory;
          }
          set_gpu_device_memory_monitor(infos[i].pid,cudadev,summonitor);
        } 
        continue;
      }
      for (i=0; i<processes_num; i++){
          //if (processes_sample[i].timeStamp >= microsec){
          proc = find_proc_by_hostpid(processes_sample[i].pid);
          if (proc != NULL){
              //LOG_WARN("pid=%u num=%d\n",processes_sample[i].pid,processes_num);
              //proc = find_proc_by_hostpid(processes_sample[i].pid);
              //if (proc!=NULL) {
              //    printf("inner pid=%u\n",proc->pid);
              sum += processes_sample[i].smUtil;
              summonitor += infos[i].usedGpuMemory;
              //LOG_WARN("monitorused=%lld %d %d %d",infos[i].usedGpuMemory,proc->hostpid,proc->pid,pidfound);
              //LOG_WARN("smutil=%d %d %lu %u %u %u\n",virtual_map[devi],devi,summonitor,processes_sample[i].smUtil,processes_sample[i].encUtil,processes_sample[i].decUtil);
              //}
          }
          set_gpu_device_memory_monitor(processes_sample[i].pid,cudadev,summonitor);
      }
      if (sum < 0)
        sum = 0;
      userutil[cudadev] = sum;
    }
    return 0;
}

void* utilization_watcher() {
    nvmlInit();
    int userutil[CUDA_DEVICE_MAX_COUNT];
    int sysprocnum;
    int share = 0;
    int upper_limit = get_current_device_sm_limit(0);
    LOG_DEBUG("upper_limit=%d\n",upper_limit);
    while (1){
        nanosleep(&g_wait, NULL);
        if (pidfound==0) {
          update_host_pid();
          if (pidfound==0)
            continue;
        }
        get_used_gpu_utilization(userutil,&sysprocnum);
        //if (sysprocnum == 1 &&
        //    userutil < upper_limit / 10) {
        //    g_cur_cuda_cores =
        //        delta(upper_limit, userutil, share);
        //    continue;
        //}
        if ((share==g_total_cuda_cores) && (g_cur_cuda_cores<0)) {
          g_total_cuda_cores *= 2;
          share = g_total_cuda_cores;
        }
        share = delta(upper_limit, userutil[0], share);
        LOG_DEBUG("userutil=%d currentcores=%d total=%d limit=%d share=%d\n",userutil[0],g_cur_cuda_cores,g_total_cuda_cores,upper_limit,share);
        change_token(share);
    }
}

void init_utilization_watcher() {
    LOG_INFO("set core utilization limit to  %d",get_current_device_sm_limit(0));
    setspec();
    pthread_t tid;
    if ((get_current_device_sm_limit(0)<100) && (get_current_device_sm_limit(0)>0)){
        pthread_create(&tid, NULL, utilization_watcher, NULL);
    }
    return;
}

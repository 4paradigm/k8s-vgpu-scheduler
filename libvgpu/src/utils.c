#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include "include/utils.h"
#include "include/log_utils.h"
#include "include/nvml_prefix.h"
#include <nvml.h>
#include "include/nvml_override.h"
#include "include/libcuda_hook.h"
#include "multiprocess/multiprocess_memory_limit.h"

const char* unified_lock="/tmp/vgpulock/lock";
extern int context_size;
extern int cuda_to_nvml_map[16];


int try_lock_unified_lock() {
    int fd = open(unified_lock,O_CREAT | O_EXCL,S_IRWXU);
    int cnt = 0;
    LOG_INFO("try_lock_unified_lock:%d",fd);
    while (fd == -1) {
        if (cnt == 20) {
            LOG_MSG("unified_lock expired,removing...");
            remove("/tmp/vgpulock/lock");
        }else{
            LOG_MSG("unified_lock locked, waiting 1 second...")
            sleep(rand()%5 + 1);
        }
        cnt++;
        fd = open(unified_lock,O_CREAT | O_EXCL,S_IRWXU); 
    }
    return 0;
}

int try_unlock_unified_lock() {
    int res = remove(unified_lock);
    LOG_INFO("try unlock_unified_lock:%d",res);
    return 0;
}

int mergepid(unsigned int *prev, unsigned int *current, nvmlProcessInfo_t1 *sub, nvmlProcessInfo_t1 *merged) {
    int i,j;
    int found=0;
    for (i=0;i<*prev;i++){
        found=0;
        for (j=0;j<*current;j++) {
            LOG_INFO("merge pid=%d",sub[i].pid);
            if (sub[i].pid == merged[j].pid) {
                found = 1;
                break;
            } 
        }
        if (!found) {
            LOG_DEBUG("merged pid=%d\n",sub[i].pid);
            merged[*current].pid = sub[i].pid;
            (*current)++;
        }
    }
    return 0;
}

int getextrapid(unsigned int prev, unsigned int current, nvmlProcessInfo_t1 *pre_pids_on_device, nvmlProcessInfo_t1 *pids_on_device) {
    int i,j;
    int found = 0;
    for (i=0; i<prev; i++){
        LOG_INFO("prev pids[%d]=%d",i,pre_pids_on_device[i].pid);
    }
    for (i=0; i< current; i++) {
        LOG_INFO("current pids[%d]=%d",i,pids_on_device[i].pid);
    }
    if (current-prev<=0)
        return 0;
    for (i=0; i<current; i++) {
        found = 0;
        for (j=0; j<prev; j++) {
            if (pids_on_device[i].pid == pre_pids_on_device[j].pid) {
                found = 1;
                break;
            }
        }
        if (!found)
            return pids_on_device[i].pid;
    }
    return 0;
}

nvmlReturn_t set_task_pid() {
    unsigned int running_processes=0,previous=0,merged_num=0;
    nvmlProcessInfo_v1_t tmp_pids_on_device[SHARED_REGION_MAX_PROCESS_NUM];
    nvmlProcessInfo_t1 pre_pids_on_device[SHARED_REGION_MAX_PROCESS_NUM];
    nvmlProcessInfo_t1 pids_on_device[SHARED_REGION_MAX_PROCESS_NUM];
    nvmlDevice_t device;
    nvmlReturn_t res;
    CUcontext pctx;
    int i;
    CHECK_NVML_API(nvmlInit());
    CHECK_NVML_API(nvmlDeviceGetHandleByIndex(0, &device));
    
    unsigned int nvmlCounts;
    CHECK_NVML_API(nvmlDeviceGetCount(&nvmlCounts));
    
    int cudaDev;
    for (i=0;i<nvmlCounts;i++){
        cudaDev=nvml_to_cuda_map(i);
        if (cudaDev<0) {
            continue;
        }
        CHECK_NVML_API(nvmlDeviceGetHandleByIndex(i, &device));
        do{
            res = nvmlDeviceGetComputeRunningProcesses(device, &previous, tmp_pids_on_device);
            if ((res != NVML_SUCCESS) && (res != NVML_ERROR_INSUFFICIENT_SIZE)) {
                LOG_WARN("Device2GetComputeRunningProcesses failed %d,%d\n",res,i);
                return res;
            }
        }while(res==NVML_ERROR_INSUFFICIENT_SIZE); 
        mergepid(&previous,&merged_num,(nvmlProcessInfo_t1 *)tmp_pids_on_device,pre_pids_on_device);
    }
    previous = merged_num;
    merged_num = 0;
    memset(tmp_pids_on_device,0,sizeof(nvmlProcessInfo_v1_t)*SHARED_REGION_MAX_PROCESS_NUM);
    CHECK_CU_RESULT(cuDevicePrimaryCtxRetain(&pctx,0));
    for (i=0;i<nvmlCounts;i++) {
        cudaDev=nvml_to_cuda_map(i);
        if (cudaDev<0) {
            continue;
        }
        CHECK_NVML_API(nvmlDeviceGetHandleByIndex (i, &device)); 
        do{
            res = nvmlDeviceGetComputeRunningProcesses(device, &running_processes, tmp_pids_on_device);
            if ((res != NVML_SUCCESS) && (res != NVML_ERROR_INSUFFICIENT_SIZE)) {
                LOG_WARN("Device2GetComputeRunningProcesses failed %d\n",res);
                return res;
            }
        }while(res == NVML_ERROR_INSUFFICIENT_SIZE);
        mergepid(&running_processes,&merged_num,(nvmlProcessInfo_t1 *)tmp_pids_on_device,pids_on_device);
    }
    running_processes = merged_num;
    LOG_INFO("current processes num = %u %u",previous,running_processes);
    for (i=0;i<merged_num;i++){
        LOG_INFO("current pid in use is %d %d",i,pids_on_device[i].pid);
        //tmp_pids_on_device[i].pid=0;
    }
    unsigned int hostpid = getextrapid(previous,running_processes,pre_pids_on_device,pids_on_device); 
    if (hostpid==0) {
        LOG_WARN("host pid is error!");
        return NVML_ERROR_DRIVER_NOT_LOADED;
    }
    LOG_INFO("hostPid=%d",hostpid);
    if (set_host_pid(hostpid)==0) {
        for (i=0;i<running_processes;i++) {
            if (pids_on_device[i].pid==hostpid) {
                LOG_INFO("Primary Context Size==%lld",tmp_pids_on_device[i].usedGpuMemory);
                context_size = tmp_pids_on_device[i].usedGpuMemory; 
                break;
            }
        }
    }
    CHECK_CU_RESULT(cuDevicePrimaryCtxRelease(0));
    return NVML_SUCCESS; 
}

int parse_cuda_visible_env() {
    int i,count,tmp;
    char *s = getenv("CUDA_VISIBLE_DEVICES");
    count = 0;
    for (i=0; i<16; i++) {
        cuda_to_nvml_map[i] = i;
    }   

    if (need_cuda_virtualize()) {
        for (i=0; i<strlen(s); i++){
            if ((s[i] == ',') || (i == 0)){
                tmp = (i==0) ? atoi(s) : atoi(s + i +1);
                cuda_to_nvml_map[count] = tmp; 
                count++;
            }
        } 
    }
    for (i=0;i<16;i++){
        LOG_INFO("device %d -> %d",i,cuda_to_nvml_map[i]);
    }
    LOG_WARN("get default cuda from %s",getenv("CUDA_VISIBLE_DEVICES"));
    return count;
}

int map_cuda_visible_devices() {
    parse_cuda_visible_env();
    return 0;
}

int getenvcount() {
    char *s = getenv("CUDA_VISIBLE_DEVICES");
    if ((s == NULL) || (strlen(s)==0)){
        return -1;
    }
    LOG_DEBUG("get from env %s",s);
    int i,count=0;
    for (i=0;i<strlen(s);i++){
        if (s[i]==',')
            count++;
    }
    return count+1;
}

int need_cuda_virtualize() {
    int count1 = -1;
    char *s = getenv("CUDA_VISIBLE_DEVICES");
    if ((s == NULL) || (strlen(s)==0)){
        return 0;
    }
    int fromenv = getenvcount();
    CUresult res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuDeviceGetCount,&count1);
    if (res != CUDA_SUCCESS) {
        return 1;
    }
    LOG_WARN("count1=%d",count1);
    if (fromenv ==count1) {
        return 1;
    }
    return 0;
}
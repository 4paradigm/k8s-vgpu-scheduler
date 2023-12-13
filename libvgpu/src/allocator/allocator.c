#include "allocator.h"
#include "include/log_utils.h"
#include "include/libcuda_hook.h"
#include "multiprocess/multiprocess_memory_limit.h"


size_t BITSIZE = 512;
size_t IPCSIZE = 2097152;
size_t OVERSIZE = 134217728;
//int pidfound;


#define ALIGN       2097152
#define MULTI_PARAM 1

#define CHUNK_SIZE  (OVERSIZE/BITSIZE)
#define __CHUNK_SIZE__  CHUNK_SIZE

extern size_t initial_offset;
extern CUresult cuMemoryAllocate(CUdeviceptr* dptr, size_t bytesize, size_t* bytesallocated,void* data);
extern CUresult cuMemoryFree(CUdeviceptr dptr);

pthread_once_t allocator_allocate_flag = PTHREAD_ONCE_INIT;

size_t round_up(size_t size,size_t unit){
    if (size & (unit-1))
        return ((size / unit) + 1 ) * unit;
    return size;
}

int oom_check(const int dev,size_t addon) {
    int count1=0;
    CUDA_OVERRIDE_CALL(cuda_library_entry,cuDeviceGetCount,&count1);
    CUdevice d;
    if (dev==-1)
        cuCtxGetDevice(&d);
    else
        d=dev;
    uint64_t limit = get_current_device_memory_limit(d);
    size_t _usage = get_gpu_memory_usage(d);

    if (limit == 0) {
        return 0;
    }

    size_t new_allocated = _usage + addon;
    LOG_INFO("_usage=%lu limit=%lu new_allocated=%lu",_usage,limit,new_allocated);
    if (new_allocated > limit) {
        LOG_ERROR("Device %d OOM %lu / %lu", d, new_allocated, limit);

        if (rm_quitted_process() > 0)
            return oom_check(dev,addon);
        return 1;
    }
    return 0;
}

CUresult view_vgpu_allocator(){
    allocated_list_entry *al;
    size_t total;
    total=0;
    LOG_INFO("[view1]:overallocated:");
    for (al=device_overallocated->head;al!=NULL;al=al->next){
        LOG_INFO("(%p %lu)\t",(void *)al->entry->address,al->entry->length);
        total+=al->entry->length;
    }
    LOG_INFO("total=%lu",total);
    size_t t = get_current_device_memory_usage(0);
    LOG_INFO("current_device_memory_usage:%lu",t);
    return 0;
}

CUresult get_listsize(allocated_list *al,size_t *size){
    if (al->length == 0){
        *size = 0;
        return CUDA_SUCCESS;
    }
    size_t count=0;
    allocated_list_entry *val;
    for (val=al->head;val!=NULL;val=val->next){
        count+=val->entry->length;
    }
    *size = count;
    return CUDA_SUCCESS;
}

void allocator_init(){
    LOG_DEBUG("Allocator_init\n");
    
    device_overallocated = malloc(sizeof(allocated_list));
    LIST_INIT(device_overallocated);
    
    pthread_mutex_init(&mutex,NULL);
}

int add_chunk(CUdeviceptr *address,size_t size){
    size_t addr=0;
    size_t allocsize;
    CUresult res = CUDA_SUCCESS;
    CUdevice dev;
    cuCtxGetDevice(&dev);
    if (oom_check(dev,size))
        return -1;
    
    allocated_list_entry *e;
    INIT_ALLOCATED_LIST_ENTRY(e,addr,size);
    if (size <= IPCSIZE)
        res = CUDA_OVERRIDE_CALL(cuda_library_entry,cuMemAlloc_v2,&e->entry->address,size);
    else{
        //size = round_up(size,ALIGN);
        e->entry->length = size;
        res = cuMemoryAllocate(&e->entry->address,size,&e->entry->length,e->entry->allocHandle);
    }
    if (res!=CUDA_SUCCESS){
        LOG_ERROR("cuMemoryAllocate failed res=%d",res);
        return res;
    }
    LIST_ADD(device_overallocated,e);
    //uint64_t t_size;
    *address = e->entry->address;
    allocsize = size;
    cuCtxGetDevice(&dev);
    add_gpu_device_memory_usage(getpid(),dev,allocsize,2);
    return 0;
}

int add_chunk_only(CUdeviceptr address,size_t size){
    pthread_mutex_lock(&mutex);
    size_t addr=0;
    size_t allocsize;
    CUdevice dev;
    cuCtxGetDevice(&dev);
    if (oom_check(dev,size)){
        pthread_mutex_unlock(&mutex);
        return -1;
    }
    allocated_list_entry *e;
    INIT_ALLOCATED_LIST_ENTRY(e,addr,size);
    LIST_ADD(device_overallocated,e);
    //uint64_t t_size;
    e->entry->address=address;
    allocsize = size;
    cuCtxGetDevice(&dev);
    add_gpu_device_memory_usage(getpid(),dev,allocsize,2);
    pthread_mutex_unlock(&mutex);
    return 0;
}

int check_memory_type(CUdeviceptr address) {
    allocated_list_entry *cursor;
    cursor = device_overallocated->head;
    for (cursor=device_overallocated->head;cursor!=NULL;cursor=cursor->next){
        if ((cursor->entry->address <= address) && (cursor->entry->address+cursor->entry->length>=address))
            return CU_MEMORYTYPE_DEVICE;
    }
    return CU_MEMORYTYPE_HOST;
}

int remove_chunk(allocated_list *a_list, CUdeviceptr dptr){
    size_t t_size;
    if (a_list->length==0) {
        return -1;
    }
    allocated_list_entry *val;
    for (val=a_list->head;val!=NULL;val=val->next){
        if (val->entry->address==dptr){
            t_size=val->entry->length;
            cuMemoryFree(dptr);
            LIST_REMOVE(a_list,val);
        
            CUdevice dev;
            cuCtxGetDevice(&dev);
            rm_gpu_device_memory_usage(getpid(),dev,t_size,2);
            return 0;
        }
    }
    return -1;
}

int allocate_raw(CUdeviceptr *dptr, size_t size){
    int tmp;
    pthread_mutex_lock(&mutex);
    tmp = add_chunk(dptr,size);
    pthread_mutex_unlock(&mutex);
    return tmp;
}

int free_raw(CUdeviceptr dptr){
    pthread_mutex_lock(&mutex);
    unsigned int tmp = remove_chunk(device_overallocated,dptr);
    pthread_mutex_unlock(&mutex);
    return tmp;
}


#include <stdio.h>
#include <cuda.h>
#include <assert.h>
#include <memory.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>

struct allocated_device_memory_struct{
    CUdeviceptr address;
    size_t length;
    CUcontext ctx;
    CUmemGenericAllocationHandle *allocHandle;
};
typedef struct allocated_device_memory_struct allocated_device_memory;

struct allocated_list_entry_struct{
    allocated_device_memory *entry;;
    struct allocated_list_entry_struct *next,*prev;
};
typedef struct allocated_list_entry_struct allocated_list_entry;

struct allocated_list_struct{
    allocated_list_entry *head;
    allocated_list_entry *tail;
    size_t length;
};
typedef struct allocated_list_struct allocated_list;

struct region_struct{
    size_t start;
    size_t freemark;
    size_t freed_map;
    size_t length;
    CUcontext ctx;
    allocated_list *region_allocs;
    char *bitmap;
    CUmemGenericAllocationHandle *allocHandle;
};
typedef struct region_struct region;

struct region_list_entry_struct{
    region *entry;
    struct region_list_entry_struct *next,*prev;
};
typedef struct region_list_entry_struct region_list_entry;

struct region_list_struct{
    region_list_entry   *head;
    region_list_entry   *tail;
    size_t length;
};
typedef struct region_list_struct region_list;

region_list *r_list;
allocated_list *device_overallocated;
allocated_list *array_list;
pthread_mutex_t mutex;

#define LIST_INIT(list) {   \
    list->head=NULL;         \
    list->tail=NULL;         \
    list->length=0;          \
    }
#define __LIST_INIT(list) LIST_INIT(list)

#define QUIT_WITH_ERROR(__message) {    \
    LOG_ERROR("%s\n",#__message);  \
    return -1;                          \
}

#define LIST_REMOVE(list,val) {             \
    if (val->prev!=NULL)                    \
        val->prev->next=val->next;          \
    if (val->next!=NULL)                    \
        val->next->prev=val->prev;          \
    if (val == list->tail)                  \
        list->tail = val->prev;             \
    if (val == list->head)                  \
        list->head = val->next;             \
    free(val->entry->allocHandle);          \
    free(val->entry);                       \
    free(val);                              \
    list->length--;                         \
}   

#define INIT_ALLOCATED_LIST_ENTRY(__list_entry,__address,__size) {             \
    CUcontext __ctx;                                                           \
    CUresult __res=cuCtxGetCurrent(&__ctx);                                    \
    if (__res!=CUDA_SUCCESS) QUIT_WITH_ERROR("cuCtxGetCurrent failed");        \
    __list_entry = malloc(sizeof(allocated_list_entry));                       \
    if (__list_entry == NULL) QUIT_WITH_ERROR("malloc failed");                \
    __list_entry->entry = malloc(sizeof(allocated_device_memory));             \
    if (__list_entry->entry == NULL) QUIT_WITH_ERROR("malloc failed");         \
    __list_entry->entry->address=__address;                                    \
    __list_entry->entry->length=__size;                                        \
    __list_entry->entry->allocHandle=malloc(sizeof(CUmemGenericAllocationHandle)); \
    __list_entry->entry->ctx=__ctx;                                            \
    __list_entry->next=NULL;                                                   \
    __list_entry->prev=NULL;                                                   \
}

#define INIT_REGION_LIST_ENTRY(__list_entry,__address,__size)                      \
    do{                                                                            \
        CUcontext __ctx;                                                           \
        CUresult __res;                                                            \
        __res = cuCtxGetCurrent(&__ctx);                                           \
        if (__res!=CUDA_SUCCESS) QUIT_WITH_ERROR("cuCtxGetCurrent failed");        \
        __list_entry = malloc(sizeof(region_list_entry));                          \
        if (__list_entry == NULL) QUIT_WITH_ERROR("malloc failed");                \
        __list_entry->entry = malloc(sizeof(region));                              \
        if (__list_entry->entry == NULL) QUIT_WITH_ERROR("malloc failed")          \
        __list_entry->entry->region_allocs = malloc(sizeof(allocated_list));       \
        if (__list_entry->entry->region_allocs == NULL) QUIT_WITH_ERROR("malloc failed") \
        __list_entry->entry->start=__address;                                      \
        __list_entry->entry->freed_map=__CHUNK_SIZE__;                             \
        __list_entry->entry->freemark=0;                                           \
        __list_entry->entry->length=0;                                             \
        __list_entry->entry->ctx=__ctx;                                            \
        __list_entry->entry->allocHandle=malloc(sizeof(CUmemGenericAllocationHandle)); \
        __list_entry->entry->bitmap=malloc(__CHUNK_SIZE__);                        \
        memset(__list_entry->entry->bitmap,0,__CHUNK_SIZE__);                      \
        __LIST_INIT(__list_entry->entry->region_allocs);                           \
        region_fill(__list_entry->entry,0,__size);                                 \
        __list_entry->next=NULL;                                                   \
        __list_entry->prev=NULL;                                                   \
    }while(0);                                                                     

#define LIST_ADD(list,__entry) { \
    if (list->head == NULL) {    \
        list->head = __entry;    \
        list->tail = __entry;    \
        list->length=1;          \
    }else{                      \
        __entry->prev = list->tail; \
        list->tail->next=__entry;   \
        list->tail = __entry;       \
        list->length++;             \
    }                               \
}                                   


int getallochandle(CUmemGenericAllocationHandle *handle, size_t size, size_t *allocsize);

// Check result of Allocator
CUresult view_vgpu_allocator();

// Checks if oom
int oom_check(const int dev,size_t addon);

// Allocate and free device memory
int allocate_raw(CUdeviceptr *dptr, size_t size);
int free_raw(CUdeviceptr dptr);
int add_chunk_only(CUdeviceptr address,size_t size);

// Checks memory type
int check_memory_type(CUdeviceptr address);


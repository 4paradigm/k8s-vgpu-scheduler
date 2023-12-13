#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>


int try_lock_unified_lock();
int try_unlock_unified_lock();

//Nvml part utils
void sort(int vmap[16]);
int initial_virtual_devices();
int parser(char *str);
int need_cuda_virtualize();

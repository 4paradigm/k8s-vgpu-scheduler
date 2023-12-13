#define MAX_PIDS 1024

#define CAS(ptr, old, new) __sync_bool_compare_and_swap((ptr), (old), (new))

#define MILLISEC (1000UL * 1000UL)

#define TIME_TICK (10)

static const struct timespec g_cycle = {
    .tv_sec = 0,
    .tv_nsec = TIME_TICK * MILLISEC,
};

static const struct timespec g_wait = {
    .tv_sec = 0,
    .tv_nsec = 120 * MILLISEC,
};


void rate_limiter(int grids, int blocks);
void init_utilization_watcher();
void* utilization_watcher();
int setspec();
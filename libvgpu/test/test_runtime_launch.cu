#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime.h>

#include "test_utils.h"
#include <unistd.h>


__global__ void add(float* a, float* b, float* c) {
    int idx = threadIdx.x;
    c[idx] = a[idx] + b[idx];
}


int main() {
    float *a, *b, *c;
    CHECK_RUNTIME_API(cudaMalloc(&a, 1024 * sizeof(float)));
    CHECK_RUNTIME_API(cudaMalloc(&b, 1024 * sizeof(float)));
    CHECK_RUNTIME_API(cudaMalloc(&c, 1024 * sizeof(float)));

    add<<<1, 1024>>>(a, b, c);
    return 0;
}

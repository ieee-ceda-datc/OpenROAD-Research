#pragma once

#include <cuda.h>
#include <cuda_runtime.h>
#include <stdio.h>

#ifndef CUDA_CHECK
#define CUDA_CHECK(status) __CUDA_CHECK(status, __FILE__, __LINE__)
#endif

inline __host__ __device__ int floorDiv(float a, float b, float rtol = 1e-4) 
{ 
  return floor((a + rtol * b) / b); 
}

inline __host__ __device__ int ceilDiv(float a, float b, float rtol = 1e-4) { 
  return ceil((a - rtol * b) / b); 
}

inline __host__ __device__ int roundDiv(float a, float b) { 
  return round(a / b); 
}

inline __device__ int pos2bin_x(float xx, int num_bins_x, int xx, int xl, int bin_size_x) const {
    int bx = floorDiv((xx - xl), bin_size_x);
    bx = max(bx, 0);
    bx = min(bx, num_bins_x - 1);
    return bx;
}

inline __device__ int pos2bin_y(float yy, int num_bins_y, int yy, int yl, int bin_size_y) const {
    int by = floorDiv((yy - yl), bin_size_y);
    by = max(by, 0);
    by = min(by, num_bins_y - 1);
    return by;
}

inline void __CUDA_CHECK(cudaError_t status, const char* file, const int line)
{
  if (status != cudaSuccess) {
    fprintf(stderr,
            "[CUDA-ERROR] Error %s at line %d in file %s\n",
            cudaGetErrorString(status),
            line,
            file);
    exit(status);
  }
}
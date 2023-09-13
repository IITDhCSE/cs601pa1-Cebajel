// CS601: example code to show how matrix multiplication code
// shows orders of magnitude of performance difference when
// using -O3, loop interchange, and parallelization optimizations.

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <immintrin.h>
#ifdef BLAS
#include<cblas.h>
#endif

#ifdef STACKALLOCATED
#define INPUTSIZE 256
#endif
int main(int argc, char *argv[])
{
    int n;
#ifdef STACKALLOCATED
    n = INPUTSIZE;
    float A[INPUTSIZE][INPUTSIZE], B[INPUTSIZE][INPUTSIZE], C[INPUTSIZE][INPUTSIZE];
#else
    n = atoi(argv[1]);
    float *A = new float[n * n];
    float *B = new float[n * n];
    float *C = new float[n * n];
#endif

    std::srand(std::time(NULL));
#ifdef STACKALLOCATED
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            A[i][j] = std::rand() / (float)(RAND_MAX);
            B[i][j] = std::rand() / (float)(RAND_MAX);
            C[i][j] = 0;
        }
    }

#else
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            A[i * n + j] = std::rand() / (float)(RAND_MAX);
            B[i * n + j] = std::rand() / (float)(RAND_MAX);
            C[i * n + j] = 0;
        }
    }
#endif

    const auto start = std::chrono::steady_clock::now();
#ifdef STACKALLOCATED
#ifdef LOOPINTERCHANGE
#if LOOPINTERCHANGE == 1
    for (int i = 0; i < n; i++)
        for (int k = 0; k < n; k++)
            for (int j = 0; j < n; j++)
#elif LOOPINTERCHANGE == 2
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
#elif LOOPINTERCHANGE == 3
    for (int k = 0; k < n; k++)
        for (int j = 0; j < n; j++)
            for (int i = 0; i < n; i++)
#elif LOOPINTERCHANGE == 4
    for (int j = 0; j < n; j++)
        for (int i = 0; i < n; i++)
            for (int k = 0; k < n; k++)
#elif LOOPINTERCHANGE == 5
    for (int j = 0; j < n; j++)
        for (int k = 0; k < n; k++)
            for (int i = 0; i < n; i++)
#elif LOOPINTERCHANGE == 0
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < n; k++)
#endif
        C[i][j] = C[i][j] + A[i][k] * B[k][j];
#endif
#else

#ifdef PARALLEL
#pragma omp parallel for
#endif

#ifdef LOOPINTERCHANGE
#if LOOPINTERCHANGE == 1
    for (int i = 0; i < n; i++)
        for (int k = 0; k < n; k++)
            for (int j = 0; j < n; j++)
#elif LOOPINTERCHANGE == 2
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
#elif LOOPINTERCHANGE == 3
    for (int k = 0; k < n; k++)
        for (int j = 0; j < n; j++)
            for (int i = 0; i < n; i++)
#elif LOOPINTERCHANGE == 4
    for (int j = 0; j < n; j++)
        for (int i = 0; i < n; i++)
            for (int k = 0; k < n; k++)
#elif LOOPINTERCHANGE == 5
    for (int j = 0; j < n; j++)
        for (int k = 0; k < n; k++)
            for (int i = 0; i < n; i++)
#elif LOOPINTERCHANGE == 0
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < n; k++)
#endif
C[i * n + j] = C[i * n + j] + A[i * n + k] * B[k * n + j];
#endif

#ifdef BLAS
#if BLAS == 1
for(int i=0; i<n; i++)
        for(int j=0; j<n; j++)
            C[i] = C[i] + cblas_ddot(n,(double*)A,1,(double*)B,n);
    std::cout<<"Using cblas_ddot function:"<<std::endl;
#else
    cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,n,n,n,1,A,n,B,n,1,C,n);
    std::cout<<"Using sgemm function:"<<std::endl;
#endif
#endif
#endif
    const auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<float> elapsedtime = end - start;

    double computation_cost = 2.0*n*n*n;
    double flops = static_cast<double>(computation_cost / elapsedtime.count());
    std::cout << "elapsed seconds: " << elapsedtime.count()<< " s" << std::endl;
    std::cout<<"throughput: "<<flops<< " flops" <<std::endl;
}

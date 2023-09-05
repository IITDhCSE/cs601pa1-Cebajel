//CS601: example code to show how matrix multiplication code
//shows orders of magnitude of performance difference when
//using -O3, loop interchange, and parallelization optimizations.

#include<iostream>
#include<cstdlib>
#include<ctime>
#include<chrono>
#include<immintrin.h>

#ifdef STACKALLOCATED
#define INPUTSIZE 256
#endif
int main(int argc, char* argv[]){
    int n;
#ifdef STACKALLOCATED
    n = INPUTSIZE;
    float A[INPUTSIZE][INPUTSIZE], B[INPUTSIZE][INPUTSIZE], C[INPUTSIZE][INPUTSIZE];
#else
    n=atoi(argv[1]);
    float* A=new float[n*n];
    float* B=new float[n*n];
    float* C=new float[n*n];
#endif

    std::srand(std::time(NULL));
#ifdef STACKALLOCATED
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            A[i][j] = std::rand() / (float)(RAND_MAX);
            B[i][j] = std::rand() / (float)(RAND_MAX);
            C[i][j]=0;
        }  
    }

#else    
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            A[i*n+j] = std::rand() / (float)(RAND_MAX);
            B[i*n+j] = std::rand() / (float)(RAND_MAX);
            C[i*n+j]=0;
        }  
    }
#endif

    const auto start=std::chrono::steady_clock::now();
#ifdef STACKALLOCATED
for(int i=0;i<n;i++)
        for(int k=0;k<n;k++)
            for(int j=0;j<n;j++)
                C[i][j] = C[i][j] + A[i][k] * B[k][j];
#endif
#else
#ifdef PARALLEL
#pragma omp parallel for
#endif
    for(int i=0;i<n;i++)
    #ifdef LOOPINTERCHANGE
        for(int k=0;k<n;k++)
            for(int j=0;j<n;j++)
    #else
        for(int j=0;j<n;j++)
            for(int k=0;k<n;k++)
    #endif
                C[i*n+j] = C[i*n+j] + A[i*n+k] * B[k*n+j];
#endif
    const auto end=std::chrono::steady_clock::now();
    const std::chrono::duration<float> elapsedtime = end-start;

    std::cout<<"elapsed seconds:"<<elapsedtime.count()<<std::endl;
}

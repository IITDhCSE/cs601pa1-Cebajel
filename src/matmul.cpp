// CS601: example code to show how matrix multiplication code
// shows orders of magnitude of performance difference when
// using -O3, loop interchange, and parallelization optimizations.

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <immintrin.h>
#ifdef BLAS
#include <cblas.h>
#endif

#ifdef STACKALLOCATED
#define INPUTSIZE 256
#endif

/*
    This function is used to compare two matrices element-wise and returns 1 if the maximum absolute difference between
    corresponding elements is less than 1E-6, indicating that the matrices are considered similar within the specified tolerance.
*/
static int
verify_result(int n, float *C_ref, float *C)
{
    float e_max;
    e_max = 0;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            e_max = std::max(std::abs(C[i * n + j] - C_ref[i * n + j]), e_max);
        }
    }

    printf("e_max: %.e\n", e_max);

    return e_max < 1E-6;
}

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
    float *C_ref = new float[n * n];
#endif

    std::srand(std::time(NULL));
#ifdef STACKALLOCATED
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            A[i][j] = std::rand() / (float)(RAND_MAX);
            /*
                If BLAS = 1 then B is stored in Column Major layout
            */
            #ifdef BLAS
            #if BLAS == 1
                B[j][i] = std::rand() / (float)(RAND_MAX);
            #else
                B[i][j] = std::rand() / (float)(RAND_MAX);
            #endif
            #else
                B[i][j] = std::rand() / (float)(RAND_MAX);
            #endif
            C[i][j] = 0;
        }
    }

#else
    /*
        Initializing matrices A, B, C and C_ref 
        This loop takes layout of matrices into account.
    */
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            A[i * n + j] = std::rand() / (float)(RAND_MAX);
            /*
                If BLAS = 1 then B is stored in Column Major layout
            */
            #ifdef BLAS
            #if BLAS == 1
                B[j * n + i] = std::rand() / (float)(RAND_MAX);
            #else
                B[i * n + j] = std::rand() / (float)(RAND_MAX);
            #endif
            #else
                B[i * n + j] = std::rand() / (float)(RAND_MAX);
            #endif
            C[i * n + j] = 0;
            C_ref[i * n + j] = 0;
        }
    }
#endif

    const auto start = std::chrono::steady_clock::now();
#ifdef STACKALLOCATED
#ifdef LOOPINTERCHANGE
/*
    LOOPINTERCHANGE macro is used to dertermine which loop ordering to be used:
    0 : ijk
    1 : ikj
    2 : kij
    3 : kji
    4 : jik
    5 : kji
*/
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
                C[i][j] += A[i][k] * B[k][j];
#endif
#else

/*
    This macro parallelizes the immediate for loop it encounters.
*/
#ifdef PARALLEL
#pragma omp parallel for
#endif

/*
    LOOPINTERCHANGE macro is used to dertermine which loop ordering to be used:
    0 : ijk
    1 : ikj
    2 : kij
    3 : kji
    4 : jik
    5 : kji
*/
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
                C[i * n + j] += A[i * n + k] * B[k * n + j];
#endif

/*
    BLAS macro is used to determine whether to use cblas library in computation and if so, then which function of library.
    If BLAS is equal to 1 we will use cblas_sdot() function to compute matrix matrix product. For any other value of BLAS macro we will compute
    matrix matrix product using cblas_sgemm() function.
*/
#ifdef BLAS

#if BLAS == 1
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
        /*
            cblas_sdot computes: returns A . B ; here . represents dot product between vectors A and B

            cblas_sdot syntax:
            cblas_sdot(<number of elements in vecotrs passed>, <float * of first vector A>, 
            <number to add in vector A to get next element>,
            <float * of second vector B>, <number to add in vector B to get next element>)
        */
            C[i * n + j] = cblas_sdot(n, &A[i * n], 1, &B[j * n], 1);
    std::cout << "Using cblas_sdot function:" << std::endl;
#else
    /*
        cblas_sgemm computes: C <- (alpha) * A * B + (beta) * C

        cblas_sgemm syntax:
            cblas_sgemm(<Layout of matrices passed>, <How matrix A should be used>,
            <How matrix B should be used>, <First dimension length of matrix C and A>,
            <Second dimension length of A and first dimension length of matrix B>, 
            <Second dimension length of C and first dimension length of B>, <value of alpha>,
            <float * of matrix A>, <leading dimensino of matrix A>, <float * of matrix B>, <leading dimension of matrix B>,
            <vlaue of beta>, <float * of matrix C>, <leading dimension of matrix C>)
    */
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n, n, n, 1, A, n, B, n, 1, C, n);
    std::cout << "Using sgemm function:" << std::endl;
#endif

#endif

#endif

    const auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<float> elapsedtime = end - start;

    /*
        Computation cost for matrix matrix multiplication is 2N^3. Throguput = <Computation Cost> / <Time Taken>
    */
    double computation_cost = 2.0 * n * n * n;
    double flops = static_cast<double>(computation_cost / elapsedtime.count());
    std::cout << "elapsed seconds: " << elapsedtime.count() << " s" << std::endl;
    std::cout << "throughput: " << flops << " flops" << std::endl;

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < n; k++)
            /*
                If BLAS = 1 then B is stored in Column Major layout
            */
            #ifdef BLAS
            #if BLAS == 1
                C_ref[i * n + k] += A[i * n + j] * B[k * n + j];
            #else
                C_ref[i * n + k] += A[i * n + j] * B[j * n + k];
            #endif
            #else
                C_ref[i * n + k] += A[i * n + j] * B[j * n + k];
            #endif

    if (verify_result(n, C_ref, C))
        printf("Result OK\n");
    else
        printf("Result MISMATCH\n");
}

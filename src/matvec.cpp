#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <omp.h>

// #include <pmmintrin.h>
#include <immintrin.h>

#ifndef __SSE3__
#error This example requires SSE3
#endif

#include "../inc/timeutil.h"

/* Size of the matrices to multiply */
#ifndef SIZE2
#define SIZE2 14
#endif
#define SIZE (1 << SIZE2)

/* 
    Defining a macro to compute index for a by passing row index and column index for a two dimensional matrix
    stored in a single vector.
*/
#define MINDEX(n, m) (((n) << SIZE2) | (m))

// #define XMM_ALIGNMENT_BYTES 16
/*
    64 bytes are required in alignment while allocating memory to a matrix to use __m512 register.
    Similarly, 16 bytes are required in alignment in memory of matrix to use __m128 register.
*/
#define XMM_ALIGNMENT_BYTES 64

/* Declaring a matrices along with their alignment bytes*/
static float *mat_a __attribute__((aligned(XMM_ALIGNMENT_BYTES)));
/*
    Macro matmul is used to determine whether to do matrix vector multiplication or matrix matrix multiplication.
    If macro matmul is not defined then code will compute matrix vector multiplication else matrix matrix multiplication.
*/
#ifndef MATMUL
static float *vec_b __attribute__((aligned(XMM_ALIGNMENT_BYTES)));
static float *vec_c __attribute__((aligned(XMM_ALIGNMENT_BYTES)));
static float *vec_ref __attribute__((aligned(XMM_ALIGNMENT_BYTES)));

static void
matvec_intrinsics()
{
    /* Assume that the data size is an even multiple of the 512 bit
     * SSE vectors (i.e. 16 floats) */
    assert(!(SIZE & 0xf));

    /*
        This macro parallelizes the immediate for loop.
    */
    #pragma omp parallel for
    for (int i = 0; i < SIZE; i++)
    {
        /*
            __m512 is a class which stores 16 floats or 512 bits in a 512 bit register.
            _mm512_setzero_ps() : returns __m512 object with all 512 bits as 0.
            _mm512_load_ps(float*) : returns __m512 object with 512 bits equal to 512 bits stored in memory pointed by passed pointer.
            _mm512_mul_ps(__m512, __m512) : returns __m512 object with 16 floats, each float is a product of corresponding
                numbers passed in __m512 registers.
            _mm512_reduce_add_ps(__m512) : returns a float which is sum of all 16 floats stored in passed __m512 object.
        */
        __m512 result = _mm512_setzero_ps();
        for (int j = 0; j < SIZE; j += 16)
        {
            __m512 register1 = _mm512_load_ps(&mat_a[MINDEX(i, j)]);
            __m512 register2 = _mm512_load_ps(&vec_b[j]);
            __m512 temp = _mm512_mul_ps(register1, register2);
            result = _mm512_add_ps(result, temp);
        }
        vec_c[i] = _mm512_reduce_add_ps(result);
    }

    return;
}

/**
 * Reference implementation of the matvec used to verify the answer. Do NOT touch this function.
 */
static void
matvec_ref()
{
    int i, j;

    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            vec_ref[i] += mat_a[MINDEX(i, j)] * vec_b[j];
}

/**
 * Function used to verify the result. Do not touch this function.
 */
static int
verify_result()
{
    float e_sum;
    int i;

    e_sum = 0;
    for (i = 0; i < SIZE; i++)
    {
        e_sum += vec_c[i] < vec_ref[i] ? vec_ref[i] - vec_c[i] : vec_c[i] - vec_ref[i];
    }

    printf("e_sum: %.e\n", e_sum);

    return e_sum < 1E-6;
}

/**
 * Initialize mat_a and vec_b with "random" data. Write to every
 * element in mat_c to make sure that the kernel allocates physical
 * memory to every page in the matrix before we start doing
 * benchmarking.
 */
static void
init()
{
    int i, j;

    mat_a = (float *)_mm_malloc(sizeof(*mat_a) * SIZE * SIZE, XMM_ALIGNMENT_BYTES);
    vec_b = (float *)_mm_malloc(sizeof(*vec_b) * SIZE, XMM_ALIGNMENT_BYTES);
    vec_c = (float *)_mm_malloc(sizeof(*vec_c) * SIZE, XMM_ALIGNMENT_BYTES);
    vec_ref = (float *)_mm_malloc(sizeof(*vec_ref) * SIZE, XMM_ALIGNMENT_BYTES);

    if (!mat_a || !vec_b || !vec_c || !vec_ref)
    {
        fprintf(stderr, "Memory allocation failed\n");
        abort();
    }

    for (i = 0; i < SIZE; i++)
    {
        for (j = 0; j < SIZE; j++)
            mat_a[MINDEX(i, j)] = ((7 * i + j) & 0x0F) * 0x1P-2F;
        vec_b[i] = ((i * 17) & 0x0F) * 0x1P-2F;
    }

    memset(vec_c, 0, sizeof(vec_c));
    memset(vec_ref, 0, sizeof(vec_ref));
}

static void
run_multiply()
{
    struct timespec ts_start, ts_stop;
    double runtime_ref, runtime_sse;

    get_time_now(&ts_start);
    matvec_intrinsics();
    get_time_now(&ts_stop);
    runtime_sse = get_time_diff(&ts_start, &ts_stop);
    printf("Matvec using intrinsics completed in %.2f s\n",
           runtime_sse);

    /*
        Computation cost for matrix matrix multiplication is 2N^2. Throguput = <Computation Cost> / <Time Taken>
    */
    double computation_cost = 2.0 * SIZE * SIZE;
    double flops = static_cast<double>(computation_cost / runtime_sse);
    printf("Throughtput: %5.4e flops\n", flops);

    get_time_now(&ts_start);
    matvec_ref();
    get_time_now(&ts_stop);
    runtime_ref = get_time_diff(&ts_start, &ts_stop);
    printf("Matvec reference code completed in %.2f s\n",
           runtime_ref);

    printf("Speedup: %0.2f\n",
           runtime_ref / runtime_sse);

    if (verify_result())
        printf("Result OK\n");
    else
        printf("Result MISMATCH\n");
}

int main(int argc, char *argv[])
{
    /* Initialize the matrices with some "random" data. */
    init();

    run_multiply();

    _mm_free(mat_a);
    _mm_free(vec_b);
    _mm_free(vec_c);
    _mm_free(vec_ref);

    return 0;
}

#else

static float *mat_b __attribute__((aligned(XMM_ALIGNMENT_BYTES)));
static float *mat_c __attribute__((aligned(XMM_ALIGNMENT_BYTES)));
static float *mat_ref __attribute__((aligned(XMM_ALIGNMENT_BYTES)));

static void
matmul_intrinsics()
{
    /* Assume that the data size is an even multiple of the 512 bit
     * SSE vectors (i.e. 16 floats) */
    assert(!(SIZE & 0xf));

/* HINT: Read the documentation about the following. You might find at least the following instructions
 * useful:
 *  - _mm_setzero_ps
 *  - _mm_load_ps
 *  - _mm_hadd_ps
 *  - _mm_mul_ps
 *  - _mm_cvtss_f32
 *
 * HINT: You can create the sum of all elements in a vector
 * using two hadd instructions.
 */

/* Implement your SSE version of the matrix-vector
 * multiplication here.
 */

/*
    This macro parallelizes the immediate for loop.
*/
#pragma omp parallel for
    for (int i = 0; i < SIZE; i++)
    {
        for (int k = 0; k < SIZE; k++)
        {
            /*
                __m512 is a class which stores 16 floats or 512 bits in a 512 bit register.
                _mm512_setzero_ps() : returns __m512 object with all 512 bits as 0.
                _mm512_load_ps(float*) : returns __m512 object with 512 bits equal to 512 bits stored in memory pointed by passed pointer.
                _mm512_mul_ps(__m512, __m512) : returns __m512 object with 16 floats, each float is a product of corresponding
                    numbers passed in __m512 registers.
                _mm512_reduce_add_ps(__m512) : returns a float which is sum of all 16 floats stored in passed __m512 object.
            */
            __m512 result = _mm512_setzero_ps();
            for (int j = 0; j < SIZE; j += 16)
            {
                __m512 register1 = _mm512_load_ps(&mat_a[MINDEX(i, j)]);
                __m512 register2 = _mm512_load_ps(&mat_b[MINDEX(k, j)]); // mat_b is column major
                __m512 temp = _mm512_mul_ps(register1, register2);
                result = _mm512_add_ps(result, temp);
            }
            mat_c[MINDEX(i, k)] = _mm512_reduce_add_ps(result);
        }
    }

    return;
}

/**
 * Reference implementation of the matmul used to verify the answer. Do NOT touch this function.
 */

/*
    Matrix B is stored in column major layout.
*/
static void
matmul_ref()
{
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            for (int k = 0; k < SIZE; k++)
                mat_ref[MINDEX(i, k)] += mat_a[MINDEX(i, j)] * mat_b[MINDEX(k, j)];
}

/**
 * Function used to verify the result. Do not touch this function.
 */
static int
verify_result()
{
    float e_sum;
    e_sum = 0;
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            e_sum += mat_c[MINDEX(i, j)] < mat_ref[MINDEX(i, j)] ? mat_ref[MINDEX(i, j)] - mat_c[MINDEX(i, j)] : mat_c[MINDEX(i, j)] - mat_ref[MINDEX(i, j)];
        }
    }

    printf("e_sum: %.e\n", e_sum);

    return e_sum < 1E-6;
}

/**
 * Initialize mat_a and mul_b with "random" data. Write to every
 * element in mat_c to make sure that the kernel allocates physical
 * memory to every page in the matrix before we start doing
 * benchmarking.
 */
static void
init()
{
    mat_a = (float *)_mm_malloc(sizeof(*mat_a) * SIZE * SIZE, XMM_ALIGNMENT_BYTES);
    mat_b = (float *)_mm_malloc(sizeof(*mat_b) * SIZE * SIZE, XMM_ALIGNMENT_BYTES);
    mat_c = (float *)_mm_malloc(sizeof(*mat_c) * SIZE * SIZE, XMM_ALIGNMENT_BYTES);
    mat_ref = (float *)_mm_malloc(sizeof(*mat_ref) * SIZE * SIZE, XMM_ALIGNMENT_BYTES);

    if (!mat_a || !mat_b || !mat_c || !mat_ref)
    {
        fprintf(stderr, "Memory allocation failed\n");
        abort();
    }

    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            mat_a[MINDEX(i, j)] = ((7 * i + j) & 0x0F) * 0x1P-2F;
            mat_b[MINDEX(j, i)] = ((7 * i + j) & 0x0F) * 0x1P-2F;
        }
    }

    memset(mat_c, 0, sizeof(mat_c));
    memset(mat_ref, 0, sizeof(mat_ref));
}

static void
run_multiply()
{
    struct timespec ts_start, ts_stop;
    double runtime_ref, runtime_sse;

    get_time_now(&ts_start);
    matmul_intrinsics();
    get_time_now(&ts_stop);
    runtime_sse = get_time_diff(&ts_start, &ts_stop);
    printf("Matmul using intrinsics completed in %.2f s\n",
           runtime_sse);

    /*
        Computation cost for matrix matrix multiplication is 2N^3. Throguput = <Computation Cost> / <Time Taken>
    */
    double computation_cost = 2.0 * SIZE * SIZE * SIZE;
    double flops = static_cast<double>(computation_cost / runtime_sse);
    printf("Throughtput: %5.4e flops\n", flops);
    get_time_now(&ts_start);
    matmul_ref();
    get_time_now(&ts_stop);
    runtime_ref = get_time_diff(&ts_start, &ts_stop);
    printf("Matmul reference code completed in %.2f s\n",
           runtime_ref);

    printf("Speedup: %0.2f\n",
           runtime_ref / runtime_sse);

    if (verify_result())
        printf("Result OK\n");
    else
        printf("Result MISMATCH\n");
}

int main(int argc, char *argv[])
{
    /* Initialize the matrices with some "random" data. */
    init();

    run_multiply();

    _mm_free(mat_a);
    _mm_free(mat_b);
    _mm_free(mat_c);
    _mm_free(mat_ref);

    return 0;
}

#endif

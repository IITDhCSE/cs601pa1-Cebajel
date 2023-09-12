#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <immintrin.h>
#include <omp.h> // For OpenMP parallelization

#define MATRIX_SIZE 256 // Default matrix size

int main(int argc, char* argv[]) {
    int n = MATRIX_SIZE;
    if (argc > 1) {
        n = atoi(argv[1]);
    }

    float* A = new float[n * n];
    float* B = new float[n * n];
    float* C = new float[n * n];

    // Initialize matrices A and B with random values
    std::srand(std::time(NULL));
    for (int i = 0; i < n * n; i++) {
        A[i] = static_cast<float>(std::rand()) / RAND_MAX;
        B[i] = static_cast<float>(std::rand()) / RAND_MAX;
        C[i] = 0.0f;
    }

    const auto start = std::chrono::steady_clock::now();

#ifdef LOOP_ORDER_IJK
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                C[i * n + j] += A[i * n + k] * B[k * n + j];
            }
        }
    }
#endif

#ifdef LOOP_ORDER_JIK
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            for (int k = 0; k < n; k++) {
                C[i * n + j] += A[i * n + k] * B[k * n + j];
            }
        }
    }
#endif

#ifdef LOOP_ORDER_KIJ
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                C[i * n + j] += A[i * n + k] * B[k * n + j];
            }
        }
    }
#endif

#ifdef LOOP_ORDER_IKJ
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            for (int j = 0; j < n; j++) {
                C[i * n + j] += A[i * n + k] * B[k * n + j];
            }
        }
    }
#endif

#ifdef LOOP_ORDER_KJI
    for (int k = 0; k < n; k++) {
        for (int j = 0; j < n; j++) {
            for (int i = 0; i < n; i++) {
                C[i * n + j] += A[i * n + k] * B[k * n + j];
            }
        }
    }
#endif

#ifdef LOOP_ORDER_JKI
    for (int j = 0; j < n; j++) {
        for (int k = 0; k < n; k++) {
            for (int i = 0; i < n; i++) {
                C[i * n + j] += A[i * n + k] * B[k * n + j];
            }
        }
    }
#endif

    const auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<float> elapsedtime = end - start;

    std::cout << "elapsed seconds:" << elapsedtime.count() << std::endl;

    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}

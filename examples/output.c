#include <stdlib.h>

int sumsquares(int n){
    int s = 0;
    #pragma omp parallel for reduction(+:s)
    for (int i = 0; i < n; i++){
    s += (i * i);
    }
    return s;
}

int factorial(int n){
    int p = 1;
    for (int i = 1; i < (n + 1); i++){
    p *= i;
    }
    return p;
}

int modchecksum(int n, int k){
    int s = 0;
    #pragma omp parallel for reduction(+:s)
    for (int i = 0; i < n; i++){
    s += (i % k);
    }
    return s;
}

int prefixsum(int n){
    int* a = malloc(n * sizeof(int));
    a[0] = 0;
    for (int i = 1; i < n; i++){
    a[i] = (a[(i - 1)] + i);
    }
    return a[(n - 1)];
}

float dot(int n){
    float* x = malloc(n * sizeof(float));
    float* y = malloc(n * sizeof(float));
    #pragma omp parallel for
    for (int i = 0; i < n; i++){
    x[i] = 1.5;
    y[i] = 2.0;
    }
    float s = 0.0;
    #pragma omp parallel for reduction(+:s)
    for (int i = 0; i < n; i++){
    s += (x[i] * y[i]);
    }
    return s;
}

float saxpy(int n){
    float* x = malloc(n * sizeof(float));
    float* y = malloc(n * sizeof(float));
    #pragma omp parallel for
    for (int i = 0; i < n; i++){
    x[i] = 1.0;
    y[i] = 0.5;
    }
    #pragma omp parallel for
    for (int i = 0; i < n; i++){
    y[i] += (2.5 * x[i]);
    }
    return y[(n - 1)];
}

float stencil(int n){
    float* a = malloc(n * sizeof(float));
    float* b = malloc(n * sizeof(float));
    #pragma omp parallel for
    for (int i = 0; i < n; i++){
    a[i] = 3.0;
    }
    #pragma omp parallel for
    for (int i = 1; i < (n - 1); i++){
    b[i] = ((a[(i - 1)] + a[i]) + a[(i + 1)]);
    }
    return b[1];
}

int matvec(int n){
    int* m = malloc((n * n) * sizeof(int));
    int* x = malloc(n * sizeof(int));
    int* y = malloc(n * sizeof(int));
    #pragma omp parallel for
    for (int i = 0; i < n; i++){
    x[i] = 1;
    y[i] = 0;
    for (int j = 0; j < n; j++){
    m[((i * n) + j)] = (i + j);
    }
    }
    #pragma omp parallel for
    for (int i = 0; i < n; i++){
    for (int j = 0; j < n; j++){
    y[i] += (m[((i * n) + j)] * x[j]);
    }
    }
    return y[(n - 1)];
}

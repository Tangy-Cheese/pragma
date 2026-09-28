#include <stdlib.h>

int parsum(int n){
    int* a = malloc(n * sizeof(int));
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++){
    sum += a[i];
    }
    return sum;
}

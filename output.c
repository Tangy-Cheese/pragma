#include <stdlib.h>

int arrsum(int n){
    int* a = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++){
    a[i] = i;
    }
    int sum = 0;
    for (int i = 0; i < n; i++){
    sum = (sum + a[i]);
    }
    return sum;
}

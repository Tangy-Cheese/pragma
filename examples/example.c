#include <stdint.h>
#include <stdio.h>
#include <omp.h>

void race(long number){

    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i=0;i<number;i++) {
        sum += 1;
    }
    printf("%d\n", sum);
}

void data_race(long number){

    int sum = 0;
    #pragma omp parallel for 
    for (int i=0;i<number;i++) {
        sum += 1;
    }
    printf("%d\n", sum);
}

int main(){

    data_race(10000000000); 
    race(10000000000);
    return 0;
}

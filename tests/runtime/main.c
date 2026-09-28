/* Driver for the runtime test: calls the generated function named by the
 * KERNEL macro with n, and compares the result against an expected value
 * computed independently by the test script. Exit status 0 means a match. */

#include <stdio.h>
#include <stdlib.h>

int KERNEL(int n);

int main(int argc, char** argv){

    if (argc != 3){
        fprintf(stderr, "usage: %s n expected\n", argv[0]);
        return 2;
    }

    int n = atoi(argv[1]);
    long expected = atol(argv[2]);
    long got = KERNEL(n);

    printf("n=%d got=%ld expected=%ld\n", n, got, expected);

    return got == expected ? 0 : 1;
}

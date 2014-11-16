#include <stdio.h>
#include <stdlib.h>

#pragma MIHPS vcheck (loop_queue)

int N = 10;
void loop_queue(int *A, int *B, int *C) {
    int i;

    for ( i = 0 ; i < N ; ++i )
        A[i] += B[i] + C[i];
    for ( i = 1 ; i < N ; ++i )
        B[i] += C[i-1] * A[i];
    for ( i = 2 ; i < N ; ++i )
        C[i] += A[i] - B[i-2];

    return;
}

int main ( int argc, char **argv ) {
    int *A, *B, *C;

    A = (int*)malloc(N*sizeof(int));
    B = (int*)malloc(N*sizeof(int));
    C = (int*)malloc(N*sizeof(int));

    loop_queue(A, B, C);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>

#pragma MIHPS vcheck (nestedLoops, notHere, notThere)

int N = 10;
void nestedLoops(int *A, int *B, int *C) {
    int i, j, k;

    for ( i = 0 ; i < 5 ; ++i ) {
        for ( j = 0 ; j < 4 ; ++j ) {
            for ( k = 0 ; k < 3 ; ++ k )
                C[k] += A[i]*B[j];
        }
    }

    return;
}

int main ( int argc, char **argv ) {
    int *A, *B, *C;
    int i;

    A = (int*)malloc(N*sizeof(int));
    B = (int*)malloc(N*sizeof(int));
    C = (int*)malloc(N*sizeof(int));

    nestedLoops(A, B, C);

    return 0;
}

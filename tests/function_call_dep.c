#include <stdio.h>
#include <stdlib.h>

#pragma MIHPS vcheck (function_call)

int N = 10;

int function_dep(int *A, int i){
    if(i==0) return 1;
    if(i==1) return 1;
    return A[i-2];
}

void function_call(int *A) {
    int i;
    for ( i = 0 ; i < N ; ++i ) {
        A[i] = function_dep(A, i);
    }

    return;
}

int main (int argc, char **argv) {

    int *A;
    A = (int*)malloc(N*sizeof(int));
    function_call(A);

    return 0;
}

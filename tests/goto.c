#include <stdio.h>
#include <stdlib.h>

#pragma MIHPS vcheck (addArrays)

int N = 10;

void addArrays(int *A, int *B, int *C) {
  int i;
  for ( i = 0 ; i < N ; ++i ) {
    goto indirection;
  loop:
    C[i] = A[i] + B[i];
  }

  return;

indirection:
  C[1] = A[1] + B[1];
  goto loop;
}

int main (int argc, char **argv) {
  int *A, *B, *C;

  A = (int*)malloc(N*sizeof(int));
  B = (int*)malloc(N*sizeof(int));
  C = (int*)malloc(N*sizeof(int));

  int i;
  for ( i = 0 ; i < N ; ++i ) {
    A[i] = i;
    B[i] = i*i;
  }

  addArrays(A, B, C);

  return 0;
}

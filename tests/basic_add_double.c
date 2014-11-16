#include <stdio.h>
#include <stdlib.h>

#pragma MIHPS vcheck (addArrays)

void addArrays(double *A, double *B, double *C, int n) {
  int i;
  for ( i = 0 ; i < n ; ++i ) {
    C[i] = A[i] + B[i];
    printf("%d\n", C[i]);
  }

  C[n/2] = A[0] * B[0];

  return;
}

int main (int argc, char **argv) {
  double *A, *B, *C;
  int n = 10;

  A = (double*)malloc(n*sizeof(double));
  B = (double*)malloc(n*sizeof(double));
  C = (double*)malloc(n*sizeof(double));

  int i;
  for ( i = 0 ; i < n ; ++i ) {
    A[i] = i;
    B[i] = i*i;
  }

  addArrays(A, B, C, n);

  return 0;
}

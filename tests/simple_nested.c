#include <stdio.h>
#include <stdlib.h>

#pragma MIHPS vcheck (nestedLoops, notHere, notThere)

int N = 3;
void nestedLoops(int *A, int *B, int *C) {
  int i, j, k;

  for ( i = 0 ; i < N ; ++i ) {
    for ( j = 0 ; j < N ; ++j ) {
      for ( k = 0 ; k < N ; ++ k )
	C[k] += A[i]*B[j];
      for ( k = 0 ; k < N ; ++ k )
	C[k] += A[i]*B[j];
      printf("\n");
    }
  }

  return;
}

int main ( int argc, char **argv ) {
  int *A, *B, *C;
  int i;

  A = malloc(N*sizeof(int));
  B = malloc(N*sizeof(int));
  C = malloc(N*sizeof(int));

  nestedLoops(A, B, C);

  return 0;
}

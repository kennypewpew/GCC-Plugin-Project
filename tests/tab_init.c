#include <stdio.h>
#include <stdlib.h>

#pragma MIHPS vcheck dependencies

int N = 20;

void tab_init(int *A) {
  int i;

  for ( i = 0 ; i < N ; ++i ) {
    A[i] = i;
  }
  return;
}

int main ( int argc, char **argv ) {
  int *A;

  A = (int*)malloc(N*sizeof(int));

  tab_init(A);

  return 0;
}

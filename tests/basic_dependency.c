#include <stdio.h>
#include <stdlib.h>

#pragma MIHPS vcheck dependencies

int N = 20;

void dependencies(int *A) {
  int i;

  A[0] = 1;

  for ( i = 1 ; i < N ; ++i ) {
    A[i] = A[i-1];
  }
  return;
}

//void dependencies(int *A) {
//  int i;

//  A[0] = 1;
//  A[1] = 1;

//  for ( i = 2 ; i < N ; ++i ) {
//    A[i] = A[i-2];
//  }
//  return;
//}

//void dependencies(int *A) {
//  int i;

//  A[0] = 1;
//  A[1] = 1;
//  A[2] = 1;

//  for ( i = 3 ; i < N ; ++i ) {
//    A[i] = A[i-3];
//  }
//  return;
//}

int main ( int argc, char **argv ) {
  int *A;

  A = (int*)malloc(N*sizeof(int));

  dependencies(A);

  return 0;
}

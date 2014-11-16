#include <stdio.h>
#include <stdlib.h>

#pragma MIHPS vcheck _static

int N = 20;

//void _static() {
//  int i;
//  int A[N];
//  int B[N];

//  for ( i = 1 ; i < N ; ++i ) {
//    A[i] = B[i];
//  }
//  return;
//}

void _static() {
  int i;

  int A[N];
  A[0] = 1;
  A[1] = 1;

  for ( i = 2 ; i < N ; ++i ) {
    A[i] = A[i-2];
  }
  return;
}

int main ( int argc, char **argv ) {

  _static();

  return 0;
}

#include <stdio.h>
#include <stdlib.h>

#pragma MIHPS vcheck branches

void branches (int *A, int n) {
  int i;
  for ( i = 0 ; i < n ; ++i ) {
    if ( A[i] == 1 ) goto exit1;
    //if ( A[i] == 2 ) goto exit2;
    //if ( A[i] == 3 ) goto exit3;
    if ( A[i] == 4 ) goto exit4;
    A[i] += 10;
  }

  printf("Exit 0\n");
  return;

 exit1:
  printf("Exit 1\n");
  return;

 exit2:
  printf("Exit 2\n");
  return;
  
 exit3:
  printf("Exit 3\n");
  return;

 exit4:
  printf("Exit 4\n");
  return;
}

int main(void) {
  printf("Beginning multiple exit test\n");

  int n = 10;
  int *A = (int*)malloc(n*sizeof(int));

  int i;
  for ( i = 0 ; i < n ; ++i )
    A[i] = i;

  branches(A, n);
  A[1] = 0;
  /*
    branches(A,n);
    A[2] = 0;
    branches(A,n);
    A[3] = 0;
   */
  branches(A, n);
  A[4] = 0;
  branches(A, n);

  return 0;
}

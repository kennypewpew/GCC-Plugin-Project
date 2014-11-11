#include <stdio.h>

#pragma MIHPS vcheck dummy
int sum;

void dummy(int * A) {
  int i;
  sum = 0;

  for ( i = 0 ; i < 10 ; ++i ) {
    sum += A[i];
    printf("%d\n", sum);
  }

  
  return;
}
int main(void) {
  printf("Beginning single_array test\n");

  int i, *A;
  A = malloc(10*sizeof(int));
  for ( i = 0 ; i < 10 ; ++i )
    A[i] = 2*i;
  
  dummy(A);

  return 0;
}

#include <stdio.h>

#pragma MIHPS vcheck dummy
void dummy() {
  int i;
  int sum = 0;

  for ( i = 0 ; i < 10 ; ++i ) {
    sum += i;
    printf("%d\n", sum);
  }

  /*
  for ( i = 0 ; i < 10 ; ++i ) {
    printf("%d\n", i);
  }
  printf("\n");
  */

  
  return;
}
int main(void) {
  printf("Beginning basic_loop test\n");

  dummy();

  return 0;
}

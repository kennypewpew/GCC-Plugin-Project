#include <stdio.h>
#include <stdlib.h>

#pragma MIHPS vcheck (function_call)

int function(int n){
    return n+1;
}

void function_call() {
  int i;
  for ( i = 0 ; i < 10 ; ++i ) {
     int res = function(i);
     printf("%d\n",res);
  }

  return;
}

int main (int argc, char **argv) {

  function_call();

  return 0;
}

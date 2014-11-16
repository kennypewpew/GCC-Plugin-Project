#include <stdio.h>

#pragma MIHPS vcheck (f1, f2, f2, f2, f4, f5)

#pragma MIHPS vcheck f3

#pragma MIHPS vcheck (f3) ignored

#pragma MIHPS vcheck f3, notIgnored

void f1() {
  return;
}
void f2() {
  return;
}
void f3() {
  return;
}
void f4() {
  return;
}

int main(int argc, char **argv) {




  return 0;
}

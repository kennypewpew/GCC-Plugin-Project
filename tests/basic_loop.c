

#pragma MIHPS vcheck dummy
void dummy() {
  int i;
  int sum = 0;
  for ( i = 0 ; i < 10 ; ++i ) {
    sum += i;
  }
  return;
}
int main(void) {
  dummy();

  return 0;
}

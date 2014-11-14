#include <stdio.h>
#include <stdarg.h>

#include <vector>

void test_function(const char *txt, ...) {
  va_list ap;
  va_start(ap, 1);

  unsigned n = 1;
  n = va_arg ( ap, int ) ;
  va_end(ap);
  printf("[test_function] %s %d\n", txt, n);
  printf("[test_function]\n");
  return;
}

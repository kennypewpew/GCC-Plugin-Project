#include <stdio.h>
#include <stdarg.h>
#include <vector>

extern "C" {

  enum IO {
    RD,
    WR
  };
  
  struct access {
    void* start;
    void* end;
    enum IO type;
  };

  std::vector<std::vector<struct access>* > full_loop;
  
  void test_function(const char *txt, ...) {
    va_list ap;
    va_start(ap, txt);
    
    unsigned n = 1;
    n = va_arg ( ap, int ) ;
    va_end(ap);
    printf("[test_function] %s %d\n", txt, n);
    printf("[test_function]\n");
    return;
  }


  std::vector<struct access> *alloc_loop_vector() {
    std::vector<struct access> *vec = new std::vector<struct access>;
    full_loop.push_back(vec);
    printf("Allocating vector at %p\n", vec);
    return vec;
  }

  void insert_info(std::vector<struct access> *vec) {
    printf("Using vector at %p\n", vec);


  }


} // end extern "C"

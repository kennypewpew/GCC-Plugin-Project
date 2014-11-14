#include <stdio.h>
#include <stdarg.h>
#include <vector>

extern "C" {

  enum IO {
    RD = 0,
    WR = 1
  };
  
  struct access {
    void* start;
    void* end;
    enum IO type;
  };

  std::vector<std::vector<struct access>* > full_loop;
  std::vector<struct access> *current_loop;
  
  // Add vectors to store info on loops
  // If possible, wait until end of execution to say if loop is vectorizable

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
    current_loop = new std::vector<struct access>;
    full_loop.push_back(current_loop);
    //printf("Allocating vector at %p\n", current_loop);
    return current_loop;
  }

  void insert_info(void* address, enum IO type, int size) {
    void *tmp = address;
    tmp += size;
    //printf("%d access type from address %p - %p\n", type, address, tmp);
    struct access acc = { address, tmp, type };
    current_loop->push_back(acc);
  }

  void clean_full_loop() {
    printf("Deleting loop info\n");
    for ( int i = 0 ; i < full_loop.size() ; ++i ) 
      delete full_loop[i];

    full_loop.clear();
    return;
  }

  void analyze_loop() {
    printf("Analyzing loop\n");


    clean_full_loop();
    return;
  }
  

} // end extern "C"

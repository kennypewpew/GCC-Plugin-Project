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

  void insert_info(void *address, enum IO type, int size) {
    void *tmp = address;
    tmp += size/8; // because size is in bits and it's easier to modify here
    printf("%d access type from address %p - %p\n", type, address, tmp);

    // How do structs get copied? Should we be allocating on the head instead?
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


bool check_overlap(void* start1, void* end1, void* start2, void* end2){
    if( (start1 > start2 && start1 < end2) ||
            (end1 > start2 && end1 < end2) ||
            (start2 > start1 && start2 < end1)
            ) return true;
    return false;
}
  void analyze_loop() {
    printf("Analyzing loop\n");
        int i, j, k, l;

    std::vector<struct access> *current_loop_2;

    int full_loop_size = full_loop.size();
    int current_loop_size;
    int current_loop_size_2;

    int vector_max_size = full_loop_size;
	printf("LOOP SIZE : %d\n",vector_max_size);

    for(i=1; i<full_loop_size; i++){ //all iterations in the loop except the first
        current_loop = full_loop[i];
        current_loop_size = current_loop->size();
        for(j=0; j<current_loop_size; j++){ //all access for the current loop iteration
            for(k=i-1; k>=0; k--){ //look for dependencies in previous iterations
                current_loop_2 = full_loop[k];
                current_loop_size_2 = current_loop_2->size();
                for(l=0; l<current_loop_size_2; l++){ //check all access in iteration k
                    if((*current_loop)[j].type == WR){ //every previous acces can be a problem
                        if(check_overlap((*current_loop)[j].start, (*current_loop)[j].end,
                                      (*current_loop_2)[l].start, (*current_loop_2)[l].end)
						  ){
								vector_max_size = i-k;
								if(vector_max_size == 1) goto end;
						   }
                    }
                    else if((*current_loop_2)[l].type == WR){ //check only for RAW dependencies
                        if(check_overlap((*current_loop)[j].start, (*current_loop)[j].end,
                                      (*current_loop_2)[l].start, (*current_loop_2)[l].end)
						  ){
								vector_max_size = i-k;
								if(vector_max_size == 1) goto end;
						   }
                    }
                }
            }
        }
    }
end:
    printf("POSSIBLE VECTOR SIZE : %d\n",vector_max_size);

    clean_full_loop();
    return;
  }
  

} // end extern "C"

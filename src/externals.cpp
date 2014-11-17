#include <stdio.h>
#include <stdarg.h>
#include <vector>

extern "C" {

  enum IO {
    RD = 0,
    WR = 1
  };
  
  struct access_t {
    void* start;
    void* end;
    enum IO type;
  };

  std::vector<std::vector<struct access_t>* > full_loop;
  std::vector<struct access_t> *current_loop;

  // Add vectors to store info on loops
  // If possible, wait until end of execution to say if loop is vectorizable

  std::vector<struct access_t> *alloc_loop_vector() {
    current_loop = new std::vector<struct access_t>;
    full_loop.push_back(current_loop);
    return current_loop;
  }

  void insert_info(void *address, int index, enum IO type, int size) {
    void *start = address + index*size/8;
    void *tmp = start;
    tmp += size/8; // because size is in bits and it's easier to modify here
    //printf("%d access_t type from address %p - %p\n", type, start, tmp);

    // How do structs get copied? Should we be allocating on the head instead?
    struct access_t acc = { start, tmp, type };
    current_loop->push_back(acc);
  }

  void clean_full_loop() {
    for ( int i = 0 ; i < full_loop.size() ; ++i ) 
      delete full_loop[i];

    full_loop.clear();
    return;
  }



  bool check_overlap(void* start1, void* end1, void* start2, void* end2){
      if( (end2 > start1 && end2 <= end1) ||
          (start2 < end1 && start2 >= start1 ) ||
          (start1 >= start2 && start1 < end2) ||
          (end1 > start2 && end1 <= end2)
        ) return true;

      return false;
  }

  void analyze_loop() {
        int i, j, k, l;

    std::vector<struct access_t> *current_loop_2;

    int full_loop_size = full_loop.size();
    int current_loop_size;
    int current_loop_size_2;

    int vector_max_size = full_loop_size;
    printf("[Analysis] Loop size : %d\n",vector_max_size);

    for(i=1; i<full_loop_size; i++){ //all iterations in the loop except the first
        current_loop = full_loop[i];
        current_loop_size = current_loop->size();
        for(j=0; j<current_loop_size; j++){ //all access_t for the current loop iteration
            for(k=i-1; k>=0; k--){ //look for dependencies in previous iterations
                current_loop_2 = full_loop[k];
                current_loop_size_2 = current_loop_2->size();
                for(l=0; l<current_loop_size_2; l++){ //check all access_t in iteration k
                    if((*current_loop)[j].type == WR){ //every previous acces can be a problem
                        if(check_overlap((*current_loop)[j].start, (*current_loop)[j].end,
                                      (*current_loop_2)[l].start, (*current_loop_2)[l].end)
                          ){
                                if(vector_max_size > (i-k)) vector_max_size = i-k;
								if(vector_max_size == 1) goto end;
						   }
                    }
                    else if((*current_loop_2)[l].type == WR){ //check only for RAW dependencies
                        if(check_overlap((*current_loop)[j].start, (*current_loop)[j].end,
                                      (*current_loop_2)[l].start, (*current_loop_2)[l].end)
						  ){
                                if(vector_max_size > (i-k)) vector_max_size = i-k;
								if(vector_max_size == 1) goto end;
						   }
                    }


                }
            }
        }
    }
end:
    printf("[Analysis] Possible Vector Size : %d\n",vector_max_size);
    if(vector_max_size == 1) printf("[Analysis] All iterations are dependant with each other\n");
    if(vector_max_size == full_loop_size) printf("[Analysis] All iterations are independant from each other\n");
    printf("\n");


    clean_full_loop();
    return;
  }


} // end extern "C"

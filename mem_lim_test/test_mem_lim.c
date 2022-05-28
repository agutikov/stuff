
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

int main (int argc, const char* argv[])
{
    uint64_t total = 0;
    size_t size = 1024;
    int rise = 1;
    uint32_t* ptr = 0;
    
    srand(time(0));
    
    while (size > sizeof(*ptr) ) {
        printf("%d ", size);
        ptr = malloc(size);
        
        if (ptr == 0) {
            printf("failed, total=%llu\n", total);
            size /= 2;
            rise = 0;
        } else {
            total += size;
            printf("allocated, total=%llu\n", total);
            
            uint32_t* p = ptr;
            uint32_t* end = ptr + size/sizeof(*p);
            while (p < end) {
                *p++ = rand();
            }
            printf("initialized\n");
            
            if (rise) {
                if (size < 1024*1024*1024) {
                    size *= 2;
                }
            }
        }
        
        //sleep(1);
    }
    


	return 0;
}

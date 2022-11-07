// R. Jesse Chaney
// rchaney@pdx.edu

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include <time.h>
#include <sys/time.h>

#include "vikalloc.h"

#define MICROSECONDS_PER_SECOND 1000000.0

#ifndef NUM_PTRS
# define NUM_PTRS 25000
#endif // NUM_PTRS
#ifndef CHUNK_SIZE
# define CHUNK_SIZE 4096
#endif // CHUNK_SIZE
#define PTR "0x%07lx"
#define PTR_T PTR "\t"
#define PTR_N PTR "\n"

// If you are feeling like your vikalloc is really performing well,
// enable this to compare it to the regular malloc. You will be
// humbled. Sure, this eliminates a few things for the regular malloc,
// but not enough to make THIS difference.
//#define REAL_MALLOC
#ifdef REAL_MALLOC
# define vikalloc malloc
# define vikcalloc calloc
# define vikfree free
# define vikstrdup strdup
# define vikrealloc realloc

# define vikalloc_dump2(_a)
# define vikalloc_reset()
#endif // REAL_MALLOC

#define TEXT_BLOCK \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" \
    "aaaaaaaaaaaaaaaaaaaaaaaaa" "aaaaaaaaaaaaaaaaaaaaaaaaa" 

static size_t alloc_chunk_size = 0;
static FILE *log_stream = NULL;
static char *base = NULL;

void *pointers[NUM_PTRS] = {NULL};

void coalesce1(int testno);

static void init_streams(void) __attribute__((constructor));

static void 
init_streams(void)
{
    log_stream = stderr;
}

int
main(int argc, char **argv)
{
    int num_ptrs = NUM_PTRS;
    int chunk_size = CHUNK_SIZE;
    struct timeval tv0;
    struct timeval tv1;

    {
        int opt = -1;
        
        while ((opt = getopt(argc, argv, "hs:")) != -1) {
            switch (opt) {
            case 's':
                chunk_size = atoi(optarg);;
                break;
            case 'h':
                // HAHAHAHA!!!
                break;
            default:
                fprintf(stderr, "Usage: %s [-c chunk_size] [-h]\n"
                        , argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }
    base = sbrk(0);
    
    alloc_chunk_size = vikalloc_set_min(chunk_size);
    fprintf(log_stream, "allocating in chunks of %lu\n", alloc_chunk_size);

    gettimeofday (&tv0, NULL);
    coalesce1(1);
    vikalloc_reset();

    for(int i = 0, j = 1; i < num_ptrs; i++, j = ((j + 1) % 4) + 1) {
        pointers[i] = vikalloc(alloc_chunk_size * j);
    }

    for(int i = num_ptrs - 1; i >= 0; i -= 3) {
        vikfree(pointers[i]);
        pointers[i] = vikcalloc(1, alloc_chunk_size * 3);
    }

    for(int i = num_ptrs - 1; i >= 0; i -= 4) {
        vikfree(pointers[i]);
        pointers[i] = vikstrdup(TEXT_BLOCK);
    }

    for(int i = 0, j = 3; i < num_ptrs; i += 2, j = ((j + 1) % 7) + 1) {
        pointers[i] = vikrealloc(pointers[i], (alloc_chunk_size * j) + 1000);
    }

    for(int i = num_ptrs - 1; i >= 0; i -= 3) {
        vikfree(pointers[i]);
    }

    for(int i = num_ptrs - 2; i >= 0; i -= 3) {
        vikfree(pointers[i]);
    }

    for(int i = num_ptrs - 3; i >= 0; i -= 3) {
        vikfree(pointers[i]);
    }
    
    vikalloc_dump2((long) base);
    vikalloc_reset();
    coalesce1(2);
    vikalloc_reset();

    gettimeofday (&tv1, NULL);
    {
        double total_time =
            (((double) (tv1.tv_usec - tv0.tv_usec)) / MICROSECONDS_PER_SECOND)
            + ((double) (tv1.tv_sec - tv0.tv_sec));

        fprintf(stdout, "array size:  %d\n", num_ptrs);
        fprintf(stdout, "elapse time: %.4lf\n", total_time);
    }

    return EXIT_SUCCESS;
}

void 
coalesce1(int testno)
{
#ifndef REAL_MALLOC
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;
    char *ptr5 = NULL;
    size_t chunk = alloc_chunk_size / 2;

    fprintf(log_stream,"*** Begin %d\n", testno);
    fprintf(log_stream,"      begin coalesce\n");

    ptr1 = vikalloc(chunk + 10);
    ptr2 = vikalloc(chunk + 30);
    ptr3 = vikalloc(chunk + 50);
    ptr4 = vikalloc(chunk + chunk);
    ptr5 = vikalloc(chunk + chunk + alloc_chunk_size);

    assert(base < ptr1);
    assert(ptr1 < ptr2);
    assert(ptr2 < ptr3);
    assert(ptr3 < ptr4);
    assert(ptr4 < ptr5);
    //fprintf(log_stream, "  ptr1 : %p\n  ptr2 : %p\n  ptr3 : %p\n  ptr4 : %p\n  ptr5 : %p\n"
    //        , ptr1, ptr2, ptr3, ptr4, ptr5);
    fprintf(log_stream
            , "  ptr1 : " PTR_N
              "  ptr2 : " PTR_N
              "  ptr3 : " PTR_N
              "  ptr4 : " PTR_N
              "  ptr5 : " PTR_N
            , (long) (ptr1 - base)
            , (long) (ptr2 - base)
            , (long) (ptr3 - base)
            , (long) (ptr4 - base)
            , (long) (ptr5 - base));

    vikalloc_dump2((long) base);

    vikfree(ptr2);
    vikfree(ptr4);
    vikalloc_dump2((long) base);

    fprintf(log_stream,"-- coalesce up\n");
    vikfree(ptr1);
    vikalloc_dump2((long) base);

    fprintf(log_stream,"-- coalesce down\n");
    vikfree(ptr5);
    vikalloc_dump2((long) base);

    fprintf(log_stream,"-- coalesce up and down\n");
    vikfree(ptr3);
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
#endif // REAL_MALLOC
    fprintf(log_stream,"*** End %d\n", testno);
}

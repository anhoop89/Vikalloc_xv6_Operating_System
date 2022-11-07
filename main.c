// R. Jesse Chaney
// rchaney@pdx.edu

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

//#define NDEBUG
#include <assert.h>

#include "vikalloc.h"

#ifndef NUM_PTRS
# define NUM_PTRS 100
#endif // NUM_PTRS

#define PTR "0x%07lx"
#define PTR_T PTR "\t"
#define PTR_N PTR "\n"

#define OPTIONS "hvt:a:o:s:"

#define FIRST_FIT_STR "ff"
#define BEST_FIT_STR  "bf"
#define WORST_FIT_STR "wf"
#define NEXT_FIT_STR  "nf"

#define VIKTEST(_numb,_func) \
    if (test_number == 0 || test_number == _numb ) { \
        _func ( _numb ); \
    }

//#define vikalloc_dump2(_param) vikalloc_dump()

vikalloc_fit_algorithm_t algo = FIRST_FIT;

//extern char end, etext, edata;
static unsigned test_number = 0;
static FILE *log_stream = NULL;
static char *base = NULL;
static size_t alloc_chunk_size = 0;

void first_fit_tests(void);
void best_fit_tests(void);
void worst_fit_tests(void);
void next_fit_tests(void);

void basic1(int);
void basic2(int);
void bytes10(int);
void bytes2000(int);
void bytes10000(int);

void alloc5(int);
void alloc5free3(int);
void alloc5free2(int);

void coalesce1(int);
void memset1(int);
void memset2(int);
void memset3(int);
void memset4(int);
void split1(int);
void splitcoalesce1(int);

void freefree(int);

void calloc1(int);
void calloc2(int);
void calloc3(int);
void realloc1(int);

void realloc2(int);
void realloc3(int);
void realloc4(int);
void realloc5(int);

void stress1(int);
void stress2(int);
void stress3(int);
void stress4(int);
void stress5(int);

void strdup1(int);

void bestfit1(int);
void bestfit2(int);
void bestfit3(int);
void bestfit4(int);
void bestfit5(int);
void bestfit6(int);

static void init_streams(void) __attribute__((constructor));

static void 
init_streams(void)
{
    log_stream = stderr;
}

int
main(int argc, char **argv)
{
    uint8_t isVerbose = FALSE;

    //base = sbrk(0);
    //printf("base: %p\n", base);
    {
        int opt = -1;

        while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
            switch (opt) {
            case 'h':
                fprintf(log_stream, "%s %s\n", argv[0], OPTIONS);
                fprintf(log_stream, "  -h        : print help and exit\n");
                fprintf(log_stream, "  -v        : verbose output\n");
                fprintf(log_stream, "  -t #      : test number to run, 0 for all\n");
                fprintf(log_stream, "  -o <file> : name of file for diagnostics\n");
                fprintf(log_stream, "  -s #      : set the size of the allocation chunk\n");
                fprintf(log_stream, "  -a <opt>  : algorithm to use for finding room\n");
                fprintf(log_stream, "     ff     : first fit (default)\n");
                fprintf(log_stream, "     bf     : best fit\n");
                fprintf(log_stream, "     wf     : worst fit\n");
                fprintf(log_stream, "     nf     : next fit\n");
                
                exit(EXIT_SUCCESS);
                break;
            case 'a':
                if (strcmp(optarg, FIRST_FIT_STR) == 0) {
                    algo = FIRST_FIT;
                    vikalloc_set_algorithm(FIRST_FIT);
                }
                else if (strcmp(optarg, BEST_FIT_STR) == 0) {
                    algo = BEST_FIT;
                    vikalloc_set_algorithm(BEST_FIT);
                }
                else if (strcmp(optarg, WORST_FIT_STR) == 0) {
                    algo = WORST_FIT;
                    vikalloc_set_algorithm(WORST_FIT);
                }
                else if (strcmp(optarg, NEXT_FIT_STR) == 0) {
                    algo = NEXT_FIT;
                    vikalloc_set_algorithm(NEXT_FIT);
                }
                else {
                    fprintf(log_stream, "**** Algorithm not recognized %s\n", optarg);
                    algo = FIRST_FIT;
                }
                break;
            case 'o':
                log_stream = fopen(optarg, "w");
                if (NULL == log_stream) {
                    perror("cannot open log file");
                    exit(EXIT_FAILURE);
                }
                vikalloc_set_log(log_stream);
                break;
            case 'v':
                isVerbose = isVerbose;
                vikalloc_set_verbose(TRUE);
                fprintf(log_stream, "Verbose enabled\n");
                break;
            case 't':
                test_number = atoi(optarg);
                break;
            case 's':
                alloc_chunk_size = strtol(optarg, NULL, 10);
                vikalloc_set_min(alloc_chunk_size);
                break;
            default: /* '?' */
                fprintf(log_stream, "%s\n", argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }

    // Get the beginning address of the start of the stack.
    base = sbrk(0);
    //printf("base: %p\n", base);
    alloc_chunk_size = vikalloc_set_min(0);
    fprintf(log_stream, "sbrk() should be called in multiples of %lu\n"
            , (unsigned long) alloc_chunk_size);

    if (algo == FIRST_FIT) {
        first_fit_tests();
    }
    else if (algo == BEST_FIT) {
        best_fit_tests();
    }
    else if (algo == WORST_FIT) {
        worst_fit_tests();
    }
    else if (algo == NEXT_FIT) {
        next_fit_tests();
    }
    else {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void
first_fit_tests(void)
{
    fprintf(log_stream, "vikalloc first fit tests starting\n");
    if (test_number == 0) {
        fprintf(log_stream, "  running all tests\n");
    }
    else {
        fprintf(log_stream, "  running only test %d\n", test_number);
    }

    VIKTEST(1,basic1);
    VIKTEST(2,basic2);

    VIKTEST(3,bytes10);
    VIKTEST(4,bytes2000);
    VIKTEST(5,bytes10000);

    VIKTEST(6,alloc5);
    VIKTEST(7,alloc5free2);
    VIKTEST(8,alloc5free3);

    VIKTEST(9,coalesce1);
    VIKTEST(10,splitcoalesce1);

    VIKTEST(11,freefree);

    VIKTEST(12,memset1);
    VIKTEST(13,memset2);
    VIKTEST(14,memset3);
    VIKTEST(15,memset4);

    VIKTEST(16,calloc1);
    VIKTEST(17,calloc2);
    VIKTEST(18,calloc3);

    VIKTEST(19,realloc1);
    VIKTEST(20,realloc2);
    VIKTEST(21,realloc3);
    VIKTEST(22,realloc4);
    VIKTEST(23,realloc5);

    VIKTEST(24,stress1);
    VIKTEST(25,stress2);
    VIKTEST(26,stress3);
    VIKTEST(27,stress4);
    VIKTEST(28,stress5);

    VIKTEST(29,strdup1);

    if (test_number == 0) {
        fprintf(log_stream, "\n\nWoooooooHooooooo!!! "
                "All tests done and you survived. This only means it did not seg-fault.\n\n"
                "\tNow, make sure they are correct\n\n\n");
    }
    else {
        fprintf(log_stream, "\n\nWoooooooHooooooo!!! "
                "You survived test %d. This only means it did not seg-fault.\n\n"
                "\tNow, make sure it is correct.\n\n\n", test_number);
    }
}

void
best_fit_tests(void)
{
    fprintf(log_stream, "vikalloc best fit tests starting\n");

    if (test_number == 0) {
        fprintf(log_stream, "  running all tests\n");
    }
    else {
        fprintf(log_stream, "  running only test %d\n", test_number);
    }

    if (test_number == 0 || test_number == 1) {
        bestfit1(test_number);
    }
    if (test_number == 0 || test_number == 2) {
        bestfit2(test_number);
    }
    if (test_number == 0 || test_number == 3) {
        bestfit3(test_number);
    }
    if (test_number == 0 || test_number == 4) {
        bestfit4(test_number);
    }    
    if (test_number == 0 || test_number == 5) {
        bestfit5(test_number);
    }    
    if (test_number == 0 || test_number == 6) {
        bestfit6(test_number);
    }    

    if (test_number == 0) {
        fprintf(log_stream, "\n\nWoooooooHooooooo!!! "
                "All tests done and you survived. This only means it did not seg-fault.\n\n"
                "\tNow, make sure they are correct\n\n\n");
    }
    else {
        fprintf(log_stream, "\n\nWoooooooHooooooo!!! "
                "You survived test %d. This only means it did not seg-fault.\n\n"
                "\tNow, make sure it is correct.\n\n\n", test_number);
    }
}

void
worst_fit_tests(void)
{
    fprintf(log_stream, "vikalloc worst fit tests starting\n");

    if (test_number == 0) {
        fprintf(log_stream, "  running all tests\n");
    }
    else {
        fprintf(log_stream, "  running only test %d\n", test_number);
    }
    if (test_number == 0 || test_number == 1) {
        bestfit1(test_number);
    }
    if (test_number == 0 || test_number == 2) {
        bestfit2(test_number);
    }
    if (test_number == 0 || test_number == 3) {
        bestfit3(test_number);
    }
    if (test_number == 0 || test_number == 4) {
        bestfit4(test_number);
    }    

    if (test_number == 0) {
        fprintf(log_stream, "\n\nWoooooooHooooooo!!! "
                "All tests done and you survived. This only means it did not seg-fault.\n\n"
                "\tNow, make sure they are correct\n\n\n");
    }
    else {
        fprintf(log_stream, "\n\nWoooooooHooooooo!!! "
                "You survived test %d. This only means it did not seg-fault.\n\n"
                "\tNow, make sure it is correct.\n\n\n", test_number);
    }
}

void
next_fit_tests(void)
{
    // at the moment, this is, mostly, just a copy of the first-fit tests.
    // more validation is needed.
    fprintf(log_stream, "vikalloc next fit tests starting\n");

        if (test_number == 0) {
        fprintf(log_stream, "  running all tests\n");
    }
    else {
        fprintf(log_stream, "  running only test %d\n", test_number);
    }

    VIKTEST(1,basic1);
    VIKTEST(2,basic2);

    VIKTEST(3,bytes10);
    VIKTEST(4,bytes2000);
    VIKTEST(5,bytes10000);

    VIKTEST(6,alloc5);
    VIKTEST(7,alloc5free2);
    VIKTEST(8,alloc5free3);

    VIKTEST(9,coalesce1);
    VIKTEST(10,splitcoalesce1);

    VIKTEST(11,freefree);

    VIKTEST(12,memset1);
    VIKTEST(13,memset2);
    VIKTEST(14,memset3);
    VIKTEST(15,memset4);

    VIKTEST(16,calloc1);
    VIKTEST(17,calloc2);
    VIKTEST(18,calloc3);

    VIKTEST(19,realloc1);
    VIKTEST(20,realloc2);
    VIKTEST(21,realloc3);
    VIKTEST(22,realloc4);
    VIKTEST(23,realloc5);

    VIKTEST(24,stress1);
    VIKTEST(25,stress2);
    VIKTEST(26,stress3);
    VIKTEST(27,stress4);
    VIKTEST(28,stress5);

    VIKTEST(29,strdup1);

    VIKTEST(30,split1);

    
    if (test_number == 0) {
        fprintf(log_stream, "\n\nWoooooooHooooooo!!! "
                "All tests done and you survived. This only means it did not seg-fault.\n\n"
                "\tNow, make sure they are correct\n\n\n");
    }
    else {
        fprintf(log_stream, "\n\nWoooooooHooooooo!!! "
                "You survived test %d. This only means it did not seg-fault.\n\n"
                "\tNow, make sure it is correct.\n\n\n", test_number);
    }

}

void 
basic1(int testno)
{
    char *ptr1 = NULL;

    // Does the heap reset function worK?
    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      Basic reset\n");

    vikalloc_reset();
    ptr1 = sbrk(0);
    fprintf(log_stream, "  ptr : " PTR_N, (long) (ptr1 - base));
    assert(ptr1 == base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    fprintf(log_stream, "  ptr : " PTR_N, (long) (ptr1 - base));
    assert(ptr1 == base);

    fprintf(log_stream, "*** End %d\n", testno);
}

void 
basic2(int testno)
{
    char *ptr1 = NULL;

    // Allocate zero bytes and see if that works.
    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      zero bytes\n");

    ptr1 = vikalloc(0);
    assert(ptr1 == NULL);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
bytes10(int testno)
{
    char *ptr1 = NULL;

    // Just allocate 10 bytes.
    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      10 bytes\n");

    ptr1 = vikalloc(10);
    assert(ptr1 != NULL);
    assert(base < ptr1);

    fprintf(log_stream, "  ptr1 : " PTR_N
            , (long) (ptr1 -  base));

    //vikalloc_dump();
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
bytes2000(int testno)
{
    char *ptr1 = NULL;

    // Just allocate 2000 bytes.
    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      2000 bytes\n");

    ptr1 = vikalloc(2000);
    assert(ptr1 != NULL);
    assert(base < ptr1);

    //fprintf(log_stream, "  ptr1 : %p\n", ptr1);
    fprintf(log_stream, "  ptr : " PTR_N, (long) (ptr1 - base));

    //vikalloc_dump();
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
bytes10000(int testno)
{
    char *ptr1 = NULL;

    // Just allocate 10000 bytes.
    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      10000 bytes\n");

    ptr1 = vikalloc(10000);
    assert(ptr1 != NULL);
    assert(base < ptr1);

    //fprintf(log_stream, "  ptr1 : %p\n", ptr1);
    fprintf(log_stream, "  ptr : " PTR_N, (long) (ptr1 - base));

    //vikalloc_dump();
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
alloc5(int testno)
{
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;
    char *ptr5 = NULL;
    size_t chunk = alloc_chunk_size / 2;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      5 alloc\n");

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

    //vikalloc_dump();
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
alloc5free3(int testno)
{
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;
    char *ptr5 = NULL;
    size_t chunk = alloc_chunk_size / 2;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      5 alloc 3 frees\n");
    ptr1 = vikalloc(chunk + 10);
    ptr2 = vikalloc(chunk + 30);
    ptr3 = vikalloc(chunk + 50);
    ptr4 = vikalloc(chunk + chunk);
    ptr5 = vikalloc(chunk + chunk + alloc_chunk_size);

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

    assert(base < ptr1);
    assert(ptr1 < ptr2);
    assert(ptr2 < ptr3);
    assert(ptr3 < ptr4);
    assert(ptr4 < ptr5);

    //vikalloc_dump();
    vikalloc_dump2((long) base);

    vikfree(ptr1);
    vikfree(ptr3);
    vikfree(ptr5);

    //vikalloc_dump();
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
alloc5free2(int testno)
{
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;
    char *ptr5 = NULL;
    size_t chunk = alloc_chunk_size / 2;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      5 alloc 2 frees\n");

    ptr1 = vikalloc(chunk + 10);
    ptr2 = vikalloc(chunk + 30);
    ptr3 = vikalloc(chunk + 50);
    ptr4 = vikalloc(chunk + chunk);
    ptr5 = vikalloc(chunk + chunk + alloc_chunk_size);

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

    assert(base < ptr1);
    assert(ptr1 < ptr2);
    assert(ptr2 < ptr3);
    assert(ptr3 < ptr4);
    assert(ptr4 < ptr5);

    //vikalloc_dump();
    vikalloc_dump2((long) base);

    vikfree(ptr2);
    vikfree(ptr4);

    //vikalloc_dump();
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
coalesce1(int testno)
{
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
    fprintf(log_stream,"*** End %d\n", testno);
}

void 
memset1(int testno)
{
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;
    char *ptr5 = NULL;
    int alloc[] = {900, 3000, 5000, 7000, 10000};

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      memset 1\n");

    ptr1 = vikalloc(alloc[0]);
    ptr2 = vikalloc(alloc[1]);
    ptr3 = vikalloc(alloc[2]);
    ptr4 = vikalloc(alloc[3]);
    ptr5 = vikalloc(alloc[4]);

    assert(base < ptr1);
    assert(ptr1 < ptr2);
    assert(ptr2 < ptr3);
    assert(ptr3 < ptr4);
    assert(ptr4 < ptr5);

    vikalloc_dump2((long) base);

    memset(ptr1, 0x1, alloc[0]);
    memset(ptr2, 0x2, alloc[1]);
    memset(ptr3, 0x3, alloc[2]);
    memset(ptr4, 0x4, alloc[3]);
    memset(ptr5, 0x5, alloc[4]);

    vikalloc_dump2((long) base);

    {
        char ch[10000] = {0};

        memset(ch, 0x1, alloc[0]);
        assert(memcmp(ptr1, ch, alloc[0]) == 0);

        memset(ch, 0x2, alloc[1]);
        assert(memcmp(ptr2, ch, alloc[1]) == 0);

        memset(ch, 0x3, alloc[2]);
        assert(memcmp(ptr3, ch, alloc[2]) == 0);

        memset(ch, 0x4, alloc[3]);
        assert(memcmp(ptr4, ch, alloc[3]) == 0);

        memset(ch, 0x5, alloc[4]);
        assert(memcmp(ptr5, ch, alloc[4]) == 0);
    }

    vikfree(ptr1);
    vikfree(ptr2);
    vikfree(ptr3);
    vikfree(ptr4);
    vikfree(ptr5);

    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
memset2(int testno)
{
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;
    char *ptr5 = NULL;
    int alloc[] = {2000, 4000, 6000, 8000, 12000};

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      memset 2\n");

    ptr1 = vikalloc(alloc[0]);
    ptr2 = vikalloc(alloc[1]);
    ptr3 = vikalloc(alloc[2]);
    ptr4 = vikalloc(alloc[3]);
    ptr5 = vikalloc(alloc[4]);

    assert(base < ptr1);
    assert(ptr1 < ptr2);
    assert(ptr2 < ptr3);
    assert(ptr3 < ptr4);
    assert(ptr4 < ptr5);

    vikalloc_dump2((long) base);

    memset(ptr1, 0x1, alloc[0]);
    memset(ptr2, 0x2, alloc[1]);
    memset(ptr3, 0x3, alloc[2]);
    memset(ptr4, 0x4, alloc[3]);
    memset(ptr5, 0x5, alloc[4]);

    vikalloc_dump2((long) base);

    vikfree(ptr1);
    vikfree(ptr3);
    vikfree(ptr5);

    {
        char ch[12000] = {0};

        memset(ch, 0x2, alloc[1]);
        assert(memcmp(ptr2, ch, alloc[1]) == 0);

        memset(ch, 0x4, alloc[3]);
        assert(memcmp(ptr4, ch, alloc[3]) == 0);
    }
    vikalloc_dump2((long) base);

    vikfree(ptr2);
    vikfree(ptr4);

    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
memset3(int testno)
{
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;
    char *ptr5 = NULL;
    int alloc[] = {500, 700, 900, 1100, 1300};

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      memset 3\n");

    ptr1 = vikalloc(alloc[0]);
    ptr2 = vikalloc(alloc[1]);
    ptr3 = vikalloc(alloc[2]);
    ptr4 = vikalloc(alloc[3]);
    ptr5 = vikalloc(alloc[4]);

    assert(base < ptr1);
    assert(ptr1 < ptr2);
    assert(ptr2 < ptr3);
    assert(ptr3 < ptr4);
    assert(ptr4 < ptr5);

    vikalloc_dump2((long) base);

    memset(ptr1, 0x1, alloc[0]);
    memset(ptr2, 0x2, alloc[1]);
    memset(ptr3, 0x3, alloc[2]);
    memset(ptr4, 0x4, alloc[3]);
    memset(ptr5, 0x5, alloc[4]);

    vikalloc_dump2((long) base);

    vikfree(ptr1);
    vikfree(ptr2);
    vikfree(ptr5);
    vikfree(ptr4);

    {
        char ch[2000] = {0};

        memset(ch, 0x3, alloc[2]);
        assert(memcmp(ptr3, ch, alloc[2]) == 0);
    }
    vikalloc_dump2((long) base);

    vikfree(ptr3);

    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
memset4(int testno)
{
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;
    char *ptr5 = NULL;
    int alloc[] = {1 * alloc_chunk_size
                    , 2 * alloc_chunk_size
                    , 3 * alloc_chunk_size
                    , 4 * alloc_chunk_size
                    , 5 * alloc_chunk_size};

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      memset 4\n");

    ptr1 = vikalloc(alloc[0]);
    ptr2 = vikalloc(alloc[1]);
    ptr3 = vikalloc(alloc[2]);
    ptr4 = vikalloc(alloc[3]);
    ptr5 = vikalloc(alloc[4]);

    assert(base < ptr1);
    assert(ptr1 < ptr2);
    assert(ptr2 < ptr3);
    assert(ptr3 < ptr4);
    assert(ptr4 < ptr5);

    vikalloc_dump2((long) base);

    memset(ptr1, 0x1, alloc[0]);
    memset(ptr2, 0x2, alloc[1]);
    memset(ptr3, 0x3, alloc[2]);
    memset(ptr4, 0x4, alloc[3]);
    memset(ptr5, 0x5, alloc[4]);

    vikalloc_dump2((long) base);

    vikfree(ptr2);
    vikfree(ptr4);

    {
        char ch[5 * alloc_chunk_size];

        memset(ch, 0x1, alloc[0]);
        assert(memcmp(ptr1, ch, alloc[0]) == 0);

        memset(ch, 0x3, alloc[2]);
        assert(memcmp(ptr3, ch, alloc[2]) == 0);

        memset(ch, 0x5, alloc[4]);
        assert(memcmp(ptr5, ch, alloc[4]) == 0);
    }
    vikalloc_dump2((long) base);

    vikfree(ptr1);
    vikfree(ptr3);
    vikfree(ptr5);

    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void
split1(int testno)
{
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;
    char *ptr5 = NULL;
    char *ptr6 = NULL;
    size_t chunk = alloc_chunk_size / 2;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      split1\n");

    ptr1 = vikalloc(chunk + 100);
    ptr2 = vikalloc(chunk + 300);
    ptr3 = vikalloc(chunk + 500);
    ptr4 = vikalloc(alloc_chunk_size);
    ptr5 = vikalloc(alloc_chunk_size + alloc_chunk_size);
    ptr6 = vikalloc(alloc_chunk_size + alloc_chunk_size);

    assert(base < ptr1);
    assert(ptr1 < ptr2);
    assert(ptr2 < ptr3);
    assert(ptr3 < ptr4);
    assert(ptr4 < ptr5);
    assert(ptr5 < ptr6);

    vikalloc_dump2((long) base);
    vikfree(ptr2);
    vikfree(ptr4);
    vikfree(ptr6);
    vikalloc_dump2((long) base);
    ptr2 = vikalloc(chunk);
    
    vikalloc_dump2((long) base);
    
    vikfree(ptr1);
    vikalloc_dump2((long) base);
    
    ptr4 = vikalloc(chunk);
    vikalloc_dump2((long) base);
    
    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
splitcoalesce1(int testno)
{
    char *ptr1 = NULL;
    char *ptr2 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      split and coalesce\n");

    ptr1 = vikalloc(10 * alloc_chunk_size);
    vikalloc_dump2((long) base);
    vikfree(ptr1);
    vikalloc_dump2((long) base);

    ptr1 = vikalloc(100);
    vikalloc_dump2((long) base);

    // should force a split
    ptr2 = vikalloc(200);
    vikalloc_dump2((long) base);

    fprintf(log_stream, "  ptr1 : " PTR_N "  ptr2 : " PTR_N
            , (long) (ptr1 - base)
            , (long) (ptr2 - base));

    vikfree(ptr2);
    vikalloc_dump2((long) base);

    // coalesce up
    vikfree(ptr1);
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);

    fprintf(log_stream, "*** End %d\n", testno);
}

void 
stress1(int testno)
{
    const ushort num_ptrs = NUM_PTRS;
    void *ptrs[num_ptrs];
    int i = 0;
    char *ptr1 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      stress 1  %u\n", num_ptrs);

    for (i = 0; i < num_ptrs; i++) {
        ptrs[i] = vikalloc(i + 100);
    }
    vikalloc_dump2((long) base);

    for (i = 3; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }

    for (i = 0; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }

    for (i = 1; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }
    for (i = 2; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }

    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);

    fprintf(log_stream, "*** End %d\n", testno);
}

void 
stress2(int testno)
{
    const ushort num_ptrs = NUM_PTRS;
    void *ptrs[num_ptrs];
    int i = 0;
    char *ptr1 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      stress 2  %u\n", num_ptrs);

    for (i = 0; i < num_ptrs; i++) {
        ptrs[i] = vikalloc((i * 15) + alloc_chunk_size);
        memset(ptrs[i], 0x0, (i * 15) + alloc_chunk_size);
    }
    vikalloc_dump2((long) base);

    for (i = 3; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    for (i = 2; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    for (i = 1; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    for (i = 0; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    /////////////////////
    for (i = 0; i < num_ptrs; i++) {
        ptrs[i] = vikalloc((i * 15) + 500);
        memset(ptrs[i], 0x0, (i * 15) + 500);
    }
    vikalloc_dump2((long) base);

    for (i = 0; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    for (i = 1; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    for (i = 2; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    for (i = 3; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    ///////////////////
    for (i = 0; i < num_ptrs; i++) {
        ptrs[i] = vikalloc((i * 15) + alloc_chunk_size + alloc_chunk_size);
        memset(ptrs[i], 0x00, (i * 15) + alloc_chunk_size + alloc_chunk_size);
    }
    vikalloc_dump2((long) base);

    for (i = 3; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    for (i = 0; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    for (i = 2; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    for (i = 1; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);

    fprintf(log_stream, "*** End %d\n", testno);
}

void 
stress3(int testno)
{
    const ushort num_ptrs = NUM_PTRS;
    void *ptrs[num_ptrs];
    int i = 0;
    char *ptr1 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      stress 3  %u\n", num_ptrs);

    for (i = 0; i < num_ptrs; i++) {
        ptrs[i] = vikalloc((i * 15) + alloc_chunk_size);
    }
    vikalloc_dump2((long) base);

    for (i = 3; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
        ptrs[i] = vikalloc((i * 5) + 100);
    }

    for (i = 2; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
        ptrs[i] = vikalloc((i * 5) + alloc_chunk_size + alloc_chunk_size);
    }

    for (i = 1; i < num_ptrs; i += 4) {
        vikfree(ptrs[i]);
        ptrs[i] = vikalloc(i + alloc_chunk_size);
    }
    vikalloc_dump2((long) base);

    for (i = 0; i < num_ptrs; i++) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);

    fprintf(log_stream, "*** End %d\n", testno);
}

void 
stress4(int testno)
{
    const ushort num_ptrs = NUM_PTRS;
    void *ptrs[num_ptrs];
    int i = 0;
    char *ptr1 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      stress 4  %u\n", num_ptrs);

    for (i = 0; i < num_ptrs; i++) {
        ptrs[i] = vikalloc(i + 10);
    }
    vikalloc_dump2((long) base);
    for (i = 0; i < num_ptrs; i ++) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    //vikalloc_set_verbose(TRUE);
    for (i = 0; i < num_ptrs; i ++) {
        ptrs[i] = vikalloc(i + alloc_chunk_size);
        //fprintf(log_stream, "**** i = %d   %p\n", i, ptrs[i]);
        //vikalloc_dump2((long) base);
    }
    vikalloc_dump2((long) base);
    for (i = 0; i < num_ptrs; i ++) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    //return;

    for (i = 0; i < num_ptrs; i ++) {
        ptrs[i] = vikalloc(i + 107);
    }
    vikalloc_dump2((long) base);
    for (i = 0; i < num_ptrs; i ++) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    for (i = 0; i < num_ptrs; i ++) {
        ptrs[i] = vikalloc(i + alloc_chunk_size + alloc_chunk_size);
    }
    vikalloc_dump2((long) base);
    for (i = 0; i < num_ptrs; i ++) {
        vikfree(ptrs[i]);
    }

    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);

    fprintf(log_stream, "*** End %d\n", testno);
}

void 
stress5(int testno)
{
    const ushort num_ptrs = NUM_PTRS;
    void *ptrs[num_ptrs];
    int i = 0;
    char *ptr1 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      stress 5  %u\n", num_ptrs);

    ptrs[0] = vikalloc(25 * alloc_chunk_size);
    vikalloc_dump2((long) base);
    vikfree(ptrs[0]);
    vikalloc_dump2((long) base);

    for (i = 0; i < num_ptrs; i++) {
        ptrs[i] = vikalloc(i + 1);
    }
    vikalloc_dump2((long) base);

    for (i = 0; i < num_ptrs; i++) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    for (i = 0; i < num_ptrs; i++) {
        ptrs[i] = vikalloc(i + 1);
    }
    vikalloc_dump2((long) base);

    for (i = NUM_PTRS - 1; i >= 0; i--) {
        vikfree(ptrs[i]);
    }
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);

    fprintf(log_stream, "*** End %d\n", testno);
}

void 
freefree(int testno)
{
    char *ptr1 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      Free the free\n");

    ptr1 = vikalloc(10);
    vikfree(ptr1);
    vikalloc_set_verbose(TRUE);
    vikfree(ptr1);
    vikalloc_set_verbose(FALSE);
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
calloc1(int testno)
{
    char *ptr1 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      vikcalloc 1\n");

    ptr1 = vikcalloc(1000, 17);
    assert(ptr1 != NULL);
    {
        char ch[1000 * 17] = {0};

        //memset(ch, 0x1, 1000);
        assert(memcmp(ptr1, ch, 2000) == 0);
    }
    vikalloc_dump2((long) base);

    vikfree(ptr1);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
calloc2(int testno)
{
    char *ptr1 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      vikcalloc 2\n");

    ptr1 = vikcalloc(0, 17);
    assert(ptr1 == NULL);
    //vikalloc_dump();
    vikalloc_dump2((long) base);

    vikfree(ptr1);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
calloc3(int testno)
{
    char *ptr1 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      vikcalloc 3\n");

    ptr1 = vikcalloc(1000, 0);
    assert(ptr1 == NULL);
    vikalloc_dump2((long) base);

    vikfree(ptr1);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
realloc1(int testno)
{
    char *ptr1 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      vikrealloc 1\n");

    ptr1 = vikrealloc(ptr1, 100);
    assert(ptr1 != NULL);
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
realloc2(int testno)
{
    char *ptr1 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      vikrealloc 2\n");

    ptr1 = vikrealloc(ptr1, alloc_chunk_size);
    assert(ptr1 != NULL);
    memset(ptr1, 0x1, alloc_chunk_size);

    ptr1 = vikrealloc(ptr1, 5 * alloc_chunk_size);
    assert(ptr1 != NULL);
    memset(ptr1, 0x1, 5 * alloc_chunk_size);
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
realloc3(int testno)
{
    char *ptr1 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      vikrealloc 3\n");

    ptr1 = vikrealloc(ptr1, 5 * alloc_chunk_size);
    assert(ptr1 != NULL);

    ptr1 = vikrealloc(ptr1, alloc_chunk_size);
    assert(ptr1 != NULL);
    memset(ptr1, 0x1, alloc_chunk_size);
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
realloc4(int testno)
{
    char **ptr1 = NULL;
    char *ptrs[10] = {0};
    int i = 0;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      vikrealloc 4\n");

    for (i = 0; i < 10; i++) {
        ptrs[i] = vikalloc((i + 5) * 10);
        //vikalloc_dump2((long) base);
    }

    ptr1 = vikrealloc(ptr1, 5 * (sizeof(char *)));
    assert(ptr1 != NULL);

    for (i = 0; i < 5; i++) {
        ptr1[i] = ptrs[i];
    }

    assert(memcmp(ptrs, ptr1, 5 * sizeof(char *)) == 0);
    vikalloc_dump2((long) base);

    ptr1 = vikrealloc(ptr1, 2000 * (sizeof(char *)));
    assert(memcmp(ptrs, ptr1, 5 * sizeof(char *)) == 0);

    for (i = 5; i < 10; i++) {
        ptr1[i] = ptrs[i];
    }
    assert(memcmp(ptrs, ptr1, 10 * sizeof(char *)) == 0);
    vikalloc_dump2((long) base);

    ptr1 = vikrealloc(ptr1, 2 * (sizeof(char *)));
    assert(memcmp(ptrs, ptr1, 2 * sizeof(char *)) == 0);

    vikalloc_dump2((long) base);

    for (i = 0; i < 10; i++) {
        vikfree(ptrs[i]);
    }
    vikfree(ptr1);
    vikalloc_dump2((long) base);

    vikalloc_reset();
    {
        char *ptr0 = sbrk(0);
        assert(ptr0 == base);
    }
    fprintf(log_stream, "*** End %d\n", testno);
}

void 
realloc5(int testno)
{
    char **ptr1 = NULL;
    char *ptrs[10] = {0};
    int i = 0;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      vikrealloc 5\n");

    ptr1 = vikrealloc(ptr1, 5 * (sizeof(char *)));
    assert(ptr1 != NULL);

    for (i = 0; i < 10; i++) {
        ptrs[i] = vikalloc((i + 2) * 1000);
    }

    ptr1 = vikrealloc(ptr1, 5 * (sizeof(char *)));
    assert(ptr1 != NULL);

    for (i = 0; i < 5; i++) {
        ptr1[i] = ptrs[i];
    }

    assert(memcmp(ptrs, ptr1, 5 * sizeof(char *)) == 0);
    vikalloc_dump2((long) base);

    ptr1 = vikrealloc(ptr1, 2000 * (sizeof(char *)));
    assert(memcmp(ptrs, ptr1, 5 * sizeof(char *)) == 0);

    for (i = 5; i < 10; i++) {
        ptr1[i] = ptrs[i];
    }
    assert(memcmp(ptrs, ptr1, 10 * sizeof(char *)) == 0);
    vikalloc_dump2((long) base);

    ptr1 = vikrealloc(ptr1, 2 * (sizeof(char *)));
    assert(memcmp(ptrs, ptr1, 2 * sizeof(char *)) == 0);

    vikalloc_dump2((long) base);

    for (i = 0; i < 10; i++) {
        vikfree(ptrs[i]);
    }
    vikfree(ptr1);
    vikalloc_dump2((long) base);

    vikalloc_reset();
    {
        char *ptr0 = sbrk(0);
        assert(ptr0 == base);
    }
    fprintf(log_stream, "*** End %d\n", testno);
}

void
strdup1(int testno)
{
    char ch1[] = "";
    char ch2[] = "a";
    char ch3[] = "hello world";
    char ch4[50] = "goodbye world";
    char *ptr1 = NULL;
    char *ptr2 = NULL;

    fprintf(log_stream,"*** Begin %d\n", testno);
    fprintf(log_stream,"      begin vikstrdup\n");

    ptr1 = vikstrdup(ch1);
    assert(strcmp(ptr1, ch1) == 0);
    vikfree(ptr1);

    ptr1 = vikstrdup(ch2);
    assert(strcmp(ptr1, ch2) == 0);
    vikfree(ptr1);

    ptr1 = vikstrdup(ch3);
    assert(strcmp(ptr1, ch3) == 0);
    vikfree(ptr1);

    ptr1 = vikstrdup(ch4);
    assert(strcmp(ptr1, ch4) == 0);

    ptr2 = vikstrdup(ptr1);
    assert(strcmp(ptr1, ptr2) == 0);

    vikalloc_dump2((long) base);
    vikfree(ptr1);
    vikfree(ptr2);
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);

    fprintf(log_stream,"*** End %d\n", testno);
}

void
bestfit1(int testno)
{
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;
    char *ptr5 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      best/worst fit 1\n");
    ptr1 = vikalloc(2000);
    ptr2 = vikalloc(570);
    ptr3 = vikalloc(3000);
    ptr4 = vikalloc(530);
    ptr5 = vikalloc(510);

    assert(base < ptr1);
    assert(ptr1 < ptr2);
    assert(ptr2 < ptr3);
    assert(ptr3 < ptr4);
    assert(ptr4 < ptr5);

    vikalloc_dump2((long) base);

    vikfree(ptr1);
    vikfree(ptr3);
    vikfree(ptr5);
    vikalloc_dump2((long) base);

    ptr1 = vikalloc(500);
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void
bestfit2(int testno)
{
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;
    char *ptr5 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      best/worst fit 2\n");
    ptr1 = vikalloc(2000);
    ptr2 = vikalloc(570);
    ptr3 = vikalloc(510);
    ptr4 = vikalloc(530);
    ptr5 = vikalloc(3000);

    assert(base < ptr1);
    assert(ptr1 < ptr2);
    assert(ptr2 < ptr3);
    assert(ptr3 < ptr4);
    assert(ptr4 < ptr5);

    vikalloc_dump2((long) base);

    vikfree(ptr1);
    vikfree(ptr3);
    vikfree(ptr5);
    vikalloc_dump2((long) base);

    ptr1 = vikalloc(500);
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void
bestfit3(int testno)
{
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;
    char *ptr5 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      best/worst fit 3\n");
    ptr1 = vikalloc(510);
    ptr2 = vikalloc(570);
    ptr3 = vikalloc(3000);
    ptr4 = vikalloc(530);
    ptr5 = vikalloc(2000);

    assert(base < ptr1);
    assert(ptr1 < ptr2);
    assert(ptr2 < ptr3);
    assert(ptr3 < ptr4);
    assert(ptr4 < ptr5);

    vikalloc_dump2((long) base);

    vikfree(ptr1);
    vikfree(ptr3);
    vikfree(ptr5);
    vikalloc_dump2((long) base);

    ptr1 = vikalloc(500);
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void
bestfit4(int testno)
{
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;
    char *ptr5 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      best/worst fit 4\n");
    ptr1 = vikalloc(510);
    ptr2 = vikalloc(570);
    ptr3 = vikalloc(510);
    ptr4 = vikalloc(530);
    ptr5 = vikalloc(510);

    assert(base < ptr1);
    assert(ptr1 < ptr2);
    assert(ptr2 < ptr3);
    assert(ptr3 < ptr4);
    assert(ptr4 < ptr5);

    vikalloc_dump2((long) base);

    vikfree(ptr1);
    vikfree(ptr3);
    vikfree(ptr5);
    vikalloc_dump2((long) base);

    ptr1 = vikalloc(500);
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void
bestfit5(int testno)
{
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;
    char *ptr5 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      best/worst fit 5\n");
    ptr1 = vikalloc(5000);
    ptr2 = vikalloc(4000);
    ptr3 = vikalloc(3000);
    ptr4 = vikalloc(2000);
    ptr5 = vikalloc(1000);

    assert(base < ptr1);
    assert(ptr1 < ptr2);
    assert(ptr2 < ptr3);
    assert(ptr3 < ptr4);
    assert(ptr4 < ptr5);

    vikalloc_dump2((long) base);

    ptr1 = vikalloc(10);
    ptr3 = vikalloc(11);
    ptr5 = vikalloc(12);
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

void
bestfit6(int testno)
{
    char *ptr1 = NULL;
    char *ptr2 = NULL;
    char *ptr3 = NULL;
    char *ptr4 = NULL;
    char *ptr5 = NULL;

    fprintf(log_stream, "*** Begin %d\n", testno);
    fprintf(log_stream, "      best/worst fit 6\n");
    ptr1 = vikalloc(510);
    ptr2 = vikalloc(570);
    ptr3 = vikalloc(510);
    ptr4 = vikalloc(530);
    ptr5 = vikalloc(510);

    assert(base < ptr1);
    assert(ptr1 < ptr2);
    assert(ptr2 < ptr3);
    assert(ptr3 < ptr4);
    assert(ptr4 < ptr5);

    vikalloc_dump2((long) base);

    ptr1 = vikalloc(10);
    ptr1 = vikalloc(50);
    ptr1 = vikalloc(100);
    ptr1 = vikalloc(150);
    ptr1 = vikalloc(200);
    ptr1 = vikalloc(250);
    vikalloc_dump2((long) base);

    vikalloc_reset();
    ptr1 = sbrk(0);
    assert(ptr1 == base);
    fprintf(log_stream, "*** End %d\n", testno);
}

// R. Jesse Chaney
// rchaney@pdx.edu

//reference: Vikalloc, vikfree: I got help from a very good TA, Sean. 


#include "vikalloc.h"

#define BLOCK_SIZE (sizeof(mem_block_t))
#define BLOCK_DATA(__curr) (((void *)__curr) + (BLOCK_SIZE))
#define DATA_BLOCK(__data) ((mem_block_t *)(__data - BLOCK_SIZE))

#define IS_FREE(__curr) ((__curr->size) == 0)

#define PTR "0x%07lx"
#define PTR_T PTR "\t"

static mem_block_t *block_list_head = NULL;
static mem_block_t *block_list_tail = NULL;

static void *low_water_mark = NULL;
static void *high_water_mark = NULL;
// only used in next-fit algorithm
static mem_block_t *prev_fit = NULL;

static uint8_t isVerbose = FALSE;
static vikalloc_fit_algorithm_t fit_algorithm = FIRST_FIT;
static FILE *vikalloc_log_stream = NULL;

static void init_streams(void) __attribute__((constructor));

static size_t min_sbrk_size = MIN_SBRK_SIZE;

static void
init_streams(void)
{
    vikalloc_log_stream = stderr;
}

size_t
vikalloc_set_min(size_t size)
{
    if (0 == size)
    {
        // just return the current value
        return min_sbrk_size;
    }
    if (size < (BLOCK_SIZE + BLOCK_SIZE))
    {
        // In the event that it is set to something silly small.
        size = MAX(BLOCK_SIZE + BLOCK_SIZE, SILLY_SBRK_SIZE);
    }
    min_sbrk_size = size;

    return min_sbrk_size;
}

void vikalloc_set_algorithm(vikalloc_fit_algorithm_t algorithm)
{
    fit_algorithm = algorithm;
    if (isVerbose)
    {
        switch (algorithm)
        {
        case FIRST_FIT:
            fprintf(vikalloc_log_stream, "** First fit selected\n");
            break;
        case BEST_FIT:
            fprintf(vikalloc_log_stream, "** Best fit selected\n");
            break;
        case WORST_FIT:
            fprintf(vikalloc_log_stream, "** Worst fit selected\n");
            break;
        case NEXT_FIT:
            fprintf(vikalloc_log_stream, "** Next fit selected\n");
            break;
        default:
            fprintf(vikalloc_log_stream, "** Algorithm not recognized %d\n", algorithm);
            fit_algorithm = FIRST_FIT;
            break;
        }
    }
}

void vikalloc_set_verbose(uint8_t verbosity)
{
    isVerbose = verbosity;
    if (isVerbose)
    {
        fprintf(vikalloc_log_stream, "Verbose enabled\n");
    }
}

void vikalloc_set_log(FILE *stream)
{
    vikalloc_log_stream = stream;
}

void *
vikalloc(size_t size)
{
    mem_block_t *curr = NULL;
    mem_block_t *new = NULL;
    size_t amount_alc = 0;
    size_t amount_left = 0;

    // initialize curr

    if (size == 0)
        return NULL;

    if (block_list_head == NULL)
    {
        amount_alc = ((size + BLOCK_SIZE) / min_sbrk_size + 1) * min_sbrk_size;
        // create a new node with curr
        curr = (mem_block_t *)sbrk(amount_alc);
        curr->prev = curr->next = NULL;

        curr->size = size;
        curr->capacity = amount_alc - BLOCK_SIZE;

        block_list_head = block_list_tail = curr;

        // set up low water mark and high water mark
        low_water_mark = curr;
        high_water_mark = low_water_mark + amount_alc;
    }
    else // when its not empty
    {
        for (curr = block_list_head; curr != NULL; curr = curr->next)
        {
            // check if IS_Free, resure the block
            if (IS_FREE(curr) && curr->capacity >= size)
            {
                curr->size = size;
                return BLOCK_DATA(curr);
            }

            // check if not IS_free, split the block
            amount_left = curr->capacity - curr->size;
            if (!IS_FREE(curr) && amount_left >= size + BLOCK_SIZE)
            {
                // create a split node and set up a new node
                mem_block_t *split_node = (mem_block_t *)(curr->size + BLOCK_DATA(curr));

                split_node->capacity = curr->capacity - curr->size - BLOCK_SIZE;
                split_node->size = size;

                curr->capacity = curr->size;
                split_node->prev = curr;
                split_node->next = curr->next;

                if (curr->next == NULL) // if it's the last node
                    block_list_tail = split_node;
                else
                    curr->next->prev = split_node;

                curr->next = split_node;
                curr = split_node;
                return BLOCK_DATA(curr);
            }
        }

        // what if it doesn't have any block fit, we need to create a new block at the end.
        if (curr == NULL)
        {
            size_t new_amount_alc = ((size + BLOCK_SIZE) / min_sbrk_size + 1) * min_sbrk_size;
            new = (mem_block_t *)sbrk(new_amount_alc);
            new->next = NULL;
            new->size = size;
            new->capacity = new_amount_alc - BLOCK_SIZE;

            block_list_tail->next = new;
            new->prev = block_list_tail;

            block_list_tail = new;
            
            high_water_mark += new_amount_alc; // another way: high_water_mark += new->capacity + BLOCK_SIZE;
            return BLOCK_DATA(block_list_tail);
        }
    }

    if (isVerbose)
    {
        fprintf(vikalloc_log_stream, ">> %d: %s entry: size = %lu\n", __LINE__, __FUNCTION__, size);
    }

    return BLOCK_DATA(curr);
}

static void
coalesce(mem_block_t *curr)
{
    mem_block_t *remove_node = curr->next;
    
    curr->capacity += remove_node->capacity + BLOCK_SIZE;

    // doing DLL stuff

    // middle
    if (remove_node != block_list_tail && remove_node != block_list_head)
    {
        curr->next = remove_node->next;
        remove_node->next->prev = curr;
    }

    // handle the head
    else if (remove_node == block_list_head)
    {

        if (!remove_node->next)
        {
            curr->next = remove_node->next;
            remove_node->next->prev = curr;
        }
        else
        {
            curr->next = NULL;
            block_list_tail = curr;
        }

        block_list_head = curr;
    }

    // handle the tail
    else if (remove_node == block_list_tail)
    {

        curr->next = NULL;
        block_list_tail = curr;
    }

    return;
}

void vikfree(void *ptr)
{
    mem_block_t *curr = NULL;

    if (ptr == NULL)
        return;
    else
    {
        curr = DATA_BLOCK(ptr);

        if (!IS_FREE(curr))
            curr->size = 0;
        else 
        {
            if (isVerbose) {
                fprintf(vikalloc_log_stream, "Block is already free: ptr = " PTR "\n"
                        , (long) (ptr - low_water_mark));
            }
            return;
        }

        if (curr->next != NULL)
        {
            if (IS_FREE(curr->next))
                coalesce(curr);
        }

        if (curr->prev != NULL)
        {
            if (IS_FREE(curr->prev))
                coalesce(curr->prev);
        }
    
    }

    return;
}

void vikalloc_reset(void)
{
    if (isVerbose)
    {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n", __LINE__, __FUNCTION__);
    }

    if (low_water_mark != NULL)
    {
        if (isVerbose)
        {
            fprintf(vikalloc_log_stream, "*** Resetting all vikalloc space ***\n");
        }
        brk(low_water_mark);
        low_water_mark = high_water_mark = NULL;
        block_list_head = block_list_tail = NULL;
    }
}

// not done

void *
vikcalloc(size_t nmemb, size_t size)
{
    void *ptr = NULL;

    // check overflow
    size_t mem_alc = nmemb * size;
    ptr = vikalloc(mem_alc);
    // set all the arr to 0
    memset(ptr, 0, mem_alc);

    if (isVerbose)
    {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n", __LINE__, __FUNCTION__);
    }

    return ptr;
}

//
void *
vikrealloc(void *ptr, size_t size)
{
    mem_block_t *curr = NULL;
    void * new_block = NULL;
    curr = DATA_BLOCK(ptr);

    // If ptr  is NULL,  then  the  call  is equivalent to malloc(size)
    if (!ptr)
        return vikalloc(size);

    // if size is equal to zero, and ptr is not NULL, then the call is equivalent to free(ptr).
    if (ptr && size == 0)
    {
        vikfree(ptr);
        return NULL;
    }
    // If the new size exceeds the capacity of the existing
    //  block, a new block will be allocated, the old contents will be copied
    //  into the new block, and the old block deallocated.
   
    // if the new size fit in the existing capacity
    if (curr->capacity >= size)
    {
        curr->size = size;
        return ptr;
    }

    // What if the size doesn't fit and qualify with all the previous conditions, 
    // add more block here.
    new_block = vikalloc(size);
    memcpy(new_block, ptr, curr->capacity);
    vikfree(ptr); // old block deallocated
    
    if (isVerbose)
    {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n", __LINE__, __FUNCTION__);
    }
    return new_block;
}

void *
vikstrdup(const char *s)
{
    void *ptr = NULL;

    if (s != NULL)
        ptr = (char *)vikalloc(strlen(s) + 1);

    if (ptr != NULL)
    {
        strcpy(ptr, s);
    }

    if (isVerbose)
    {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n", __LINE__, __FUNCTION__);
    }

    return ptr;
}

#include "vikalloc_dump.c"

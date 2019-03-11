/************************************************
 *      mymalloc.c                              *
 *      Authors:  Seth Karten, Yash Shah        *
 *      2019                                    *
 ************************************************/
#include "mymalloc.h"
#include <string.h>

#define IN_USE 'Y'
#define NOT_IN_USE 'N'
#define HEAP_SIZE 4096
#define METADATA_SIZE 3
#define METADATA_FLAG_SIZE 1
#define FIRST_NODE_INDEX 4
#define MAX_FREE_SPACE 4088
#define SUPERBLOCK_SPACE_INDEX 2

/*
 *      Converts x from an integer to a psuedo-base 64 character stored at
 *      HEAP[address] and HEAP[address+1]. The two byte representation can store
 *      values 0-4095.
 */
void dec_to_base64(int x, int address)
{
        int y;
        int i = 0;

        /* before writing the HEX value to the address, zero out the values stored in the bytes first. */
        HEAP[address++] = '\0';
        HEAP[address] = '\0';

        while (x != 0 && i < 2)
        {
                y = x % 64;
                HEAP[address--] = (char) y;
                x /= 64;
                i++;
        }
}

/*
 *      Converts the psuedo-base 64 characters stored and HEAP[address] and
 *      HEAP[address+1] to an integer and returns the integer.
 */
int base64_to_dec(int address)
{
        int i, out = 0, multiplier = 1;
        for (i = 1; i >= 0; i--)
        {
                int y = (int) HEAP[address + i];
                out += y * multiplier;
                multiplier *= 64;
        }
        return out;
}

/*
 *      Converts the psuedo-base 64 characters stored and address[0] and
 *      adresss[1] to an integer and returns the integer.
 */
int base64_to_dec_pointer(char * address)
{
        int i, out = 0, multiplier = 1;
        for (i = 1; i >= 0; i--)
        {
                int y = (int) address[i];
                out += y * multiplier;
                multiplier *= 64;
        }
        return out;
}

/*
 *      Returns 1 if heap is initialized, 0 otherwise.
 */
int heap_initialized()
{
        if (HEAP[0] == IN_USE && HEAP[1] == IN_USE)
                return 1;
        else
                return 0;
}

/*
 *      Prints indices of the heap for debugging
 */
void print_heap(int max)
{
        int i = 0;
        for (; i < max; i++)
        {
                if (DEBUG) printf("Index: %d\tValue: %d, %c\n", i, HEAP[i], HEAP[i]);
        }
}

/*
 *      Given a valid size and enough contiguous memory located on the HEAP,
 *      mymalloc will return a pointer to the beginning of the block.
 *      Otherwise, mymalloc will return NULL and print an error to stderr.
 */
void * mymalloc(size_t size, char * file, int line)
{
        /* Check if superblock metadata was initialized. */
        if (!heap_initialized())
        {
                HEAP[0] = IN_USE;                  /* Mark first byte as IN_USE to denote initialization */
                HEAP[1] = IN_USE;
                dec_to_base64(MAX_FREE_SPACE, SUPERBLOCK_SPACE_INDEX);               /* Set available space to 4088 bytes */

                /* Initialize first node */
                HEAP[FIRST_NODE_INDEX] = NOT_IN_USE;              /* Bytes 0-3 are superblock, 4-7 is metadata for first node */
                dec_to_base64(MAX_FREE_SPACE, FIRST_NODE_INDEX+METADATA_FLAG_SIZE);               /* 4088 bytes remain for usage. */
        }

        /* Check if requested size is greater than 0 */
        int s = (int) size;
        if (s < 1 || size < 1)
        {
                fprintf(stderr, "[malloc] Error in malloc: Invalid size requested. FILE: %s\tLINE:%d\n", file, line);
                return NULL;
        }

        /* Check to see if there is enough space available to allocate */
        int available_space = base64_to_dec(SUPERBLOCK_SPACE_INDEX);
        if (DEBUG) printf("[malloc] found available space of %d bytes\n", available_space);


        /* if total available space is less than requested size, don't bother looking for a spot */
        if (available_space < size)
        {
                clean();
                available_space = base64_to_dec(SUPERBLOCK_SPACE_INDEX);
                if (available_space < size)
                {
                        fprintf(stderr, "[malloc] Not enough space remaining. Requested: %ld. Available: %d FILE: %s\tLINE: %d\n", size, available_space, file, line);
                        return NULL;
                }
        }

        if (DEBUG) printf("[malloc] received valid size request of %ld bytes\n", size);

        /* Fetch optimal node to store data in */
        int block_index = fetch_optimal_location(size);

        if (DEBUG) printf("[malloc] found optimal index to store data at index %d val: %4s end\n", block_index, &HEAP[block_index]);

        /*      if returned index is -1, it failed to find free space
                Time to combine contiguous blocks
        */
        if (block_index == -1)
        {
                if (DEBUG) printf("Cleaning...\n");
                clean();
                if (DEBUG) printf("Cleaned.\n");
                /* Fetch optimal node to store data in */
                block_index = fetch_optimal_location(size);

                if (DEBUG) printf("[malloc] found optimal index to store data at index %d val: %c end\n", block_index, HEAP[block_index]);
                if (block_index == -1)
                {
                        fprintf(stderr, "[malloc] Error in malloc: No space available. Available: %d. Requested: %ld. FILE: %s\tLINE: %d\n", base64_to_dec(2), size, file, line);
                        return NULL;
                }
        }

        /* Fetch block size */
        int block_size = base64_to_dec(block_index+METADATA_FLAG_SIZE);
        if (block_size == 0)
        {
                fprintf(stderr, "[malloc] Error in malloc: block size 0. FILE: %s\tLINE: %d\n", file, line);
                return NULL;
        }
        if (DEBUG) printf("[malloc] block size: %d == %3s, index %d == %ld\n", block_size, &HEAP[block_index+METADATA_FLAG_SIZE], block_index, &HEAP[block_index] - &HEAP[0]);

        /* if fetched data block's size is equal to requested size,
         return pointer to block without any modifications
        */
        if (block_size == size)
        {
                update_available_space(size);
                HEAP[block_index] = IN_USE;
                if (DEBUG) printf("[malloc] returning pointer to index %d, pointer: %p end\n", block_index+METADATA_SIZE, &HEAP[block_index+METADATA_SIZE]);
                return &HEAP[block_index+METADATA_SIZE];
        }

        /* if fetched data block's size is greater than requested size,
        split the data block into requested size and remainder bytes
        */
        else if (block_size > size)
        {
                /* Keep track of remainder bytes */
                int remainder_bytes = block_size - size;
                /* Set data block metadata */
                HEAP[block_index] = IN_USE;
                dec_to_base64(size, block_index+METADATA_FLAG_SIZE);

                if (DEBUG) printf("[malloc] block_size=%d, block_index=%d, size=%ld end\n", block_size, block_index, size);
                /* create a new data node on the remainder bytes if there's enough space */
                if (remainder_bytes > METADATA_SIZE)
                {
                        if (DEBUG) printf("[malloc] creating new data node at index %ld, with size %d end\n", block_index+METADATA_SIZE+size, remainder_bytes-METADATA_SIZE);
                        if (!create_data_node(block_index + METADATA_SIZE + size, remainder_bytes - METADATA_SIZE))
                        {
                                fprintf(stderr, "[malloc] Error in malloc: New data node could not be created. FILE: %s\tLINE: %d\n", file, line);
                                return NULL;
                        }
                        update_available_space(size + METADATA_SIZE);
                }
                else
                {
                        /* if enough space wasn't available to allocate a new block, mark the entire block as used space */
                        if (DEBUG) printf("Adding extra space to prev block\n");
                        update_available_space(size + remainder_bytes);
                        dec_to_base64(size + remainder_bytes, block_index+METADATA_FLAG_SIZE);
                }

                if (DEBUG) printf("[malloc] returning pointer to index %d, pointer: %p end\n", block_index+METADATA_SIZE, &HEAP[block_index+METADATA_SIZE]);
                return &HEAP[block_index + METADATA_SIZE];
        }
        fprintf(stderr, "[malloc] Error in malloc. FILE: %s\tLINE: %d\n", file, line);
        return NULL;
}

/*
 *      Combines contiguous free blocks in HEAP and updates superblock metadata
 *      to reclaim space taken up by block metadata.
 */
void clean()
{
        int prev_contig_free_ptr = -1;
        int ptr = FIRST_NODE_INDEX;
        for (; ptr < HEAP_SIZE - METADATA_SIZE; )
        {
                if (HEAP[ptr] == NOT_IN_USE)
                {
                        /* Join free space of block at ptr with start of free block */
                        if (prev_contig_free_ptr != -1)
                        {

                                int next_ptr = ptr + base64_to_dec(ptr+METADATA_FLAG_SIZE) + METADATA_SIZE;
                                dec_to_base64(base64_to_dec(ptr + METADATA_FLAG_SIZE) + base64_to_dec(prev_contig_free_ptr + METADATA_FLAG_SIZE) + METADATA_SIZE, prev_contig_free_ptr + METADATA_FLAG_SIZE);
                                int i = 0;
                                for (; i < METADATA_SIZE && ptr+i < HEAP_SIZE; i++)
                                {
                                        HEAP[ptr+i] = '\0';
                                }
                                mark_free_space(METADATA_SIZE);
                                ptr = next_ptr;
                        }
                        /* Mark ptr as start of new free block */
                        else
                        {
                                prev_contig_free_ptr = ptr;
                                ptr += base64_to_dec(ptr+METADATA_FLAG_SIZE) + METADATA_SIZE;
                        }
                }
                /* Block used. Reset start of new free block ptr. */
                else
                {
                        prev_contig_free_ptr = -1;
                        ptr += base64_to_dec(ptr+METADATA_FLAG_SIZE) + METADATA_SIZE;
                }

        }
}

/*
 *      Function to fetch most optimal data block to store data in using first fit selection.
 *
 *      Returns -1 if it is unable to find a block, otherwise it returns
 *      index in HEAP[] of start of data block.
 *
 */
int fetch_optimal_location(size_t size)
{

        if (size < 1)
                return -1;
        /* Keep track of index pointer */
        int ptr = FIRST_NODE_INDEX;

        /* Keep track of */
        int optimal_index = -1;
        int optimal_size = 4096;

        /* Traverse through HEAP until last node is visited or pointer has reached
         last possible array index (leaving space for data block of size 0)
        */
        while (ptr < HEAP_SIZE - METADATA_SIZE - 1)
        {
                char block_status = HEAP[ptr];
                int block_size = base64_to_dec(ptr+METADATA_FLAG_SIZE);
                /*if (DEBUG) printf("Looking at %d, %c\n", ptr, HEAP[ptr]); */

                /* if block isn't in use, compare size difference against current optimal */
                if (block_status == NOT_IN_USE)
                {
                        int block_compare   = block_size - size;
                        int optimal_compare = optimal_size - size;
                        if (block_compare >= 0)
                                return ptr;
                        /* bounded space optimality used for time efficiency */
                        /*
                        if (block_compare <= 16 && block_compare >= 0)
                                return ptr;
                        if (block_compare >= 0 && block_compare < optimal_compare)
                        {
                                optimal_index = ptr;
                                optimal_size = block_size;
                        }
                        */

                }

                if (block_size == 0) break;

                ptr += METADATA_SIZE + block_size;     /* set pointer to heap array to next data block */
        }

        return optimal_index;
}

/*
 *      Removes total available space in the metadata in superblock of heap
 */
void update_available_space(int bytes_used)
{
        int available_space = base64_to_dec(SUPERBLOCK_SPACE_INDEX);
        available_space -= bytes_used;
        dec_to_base64(available_space, SUPERBLOCK_SPACE_INDEX);
}

/*
 *      Reclaims available space metadata in superblock
 */
void mark_free_space(int size)
{
        int available_space = base64_to_dec(SUPERBLOCK_SPACE_INDEX);
        available_space += size;
        dec_to_base64(available_space, SUPERBLOCK_SPACE_INDEX);
}

/*
 *      Function used to create new unused data node in HEAP structure given
 *      the index and size.
 *
 *       Returns 1 if operation succeeded, and 0 if there was an error.
 *
 */
int create_data_node(int index, size_t size)
{
        /* Don't create a new data node where the index is too large */
        if (index > HEAP_SIZE-METADATA_SIZE-size)
                return 0;
        HEAP[index] = NOT_IN_USE;
        dec_to_base64(size, index+METADATA_FLAG_SIZE);
        return 1;
}

/*
 *      Given a valid pointer with a flag set to IN_USE, myfree will mark the
 *      flag as NOT_IN_USE and update the superblock with the reclaimed space
 *      from the no longer in use block.
 *      On all other inputs, it will print an error to stderr explaining the
 *      possible error.
 */
void myfree(void * pointer, char * file, int line)
{
        if (!heap_initialized())
        {
                fprintf(stderr, "[free] Error in free. Nothing has been allocated yet. FILE: %s\tLINE: %d\n", file, line);
                return;
        }
        char * heap_pointer = (char *) pointer;
        if (heap_pointer < &HEAP[FIRST_NODE_INDEX] || heap_pointer > &HEAP[4095] || pointer == NULL)
        {
                fprintf(stderr, "[free] Error in free: Invalid pointer passed. Too low?: %d\t Too high? %d\t NULL? %d? FILE: %s\tLINE: %d\n", (int)(heap_pointer < &HEAP[FIRST_NODE_INDEX]), (int)(heap_pointer > &HEAP[4095]), (int)(pointer == NULL), file, line);
                return;
        }

        if (heap_pointer[-METADATA_SIZE] == IN_USE)
        {
                /* Fetch block size */
                int block_size = base64_to_dec_pointer(&heap_pointer[METADATA_FLAG_SIZE-METADATA_SIZE]);
                /* Zero data */
                int j = 0;
                for (; j < block_size; j++)
                {
                        heap_pointer[j] = '\0';
                }
                /* Mark as free */
                heap_pointer[-METADATA_SIZE] = NOT_IN_USE;
                mark_free_space(block_size);
        }
        else
        {
                fprintf(stderr, "[free] Error in free: Found: %c. Expected: Y. Pointer: %ld FILE: %s\tLINE: %d\n", heap_pointer[-METADATA_SIZE], heap_pointer - &HEAP[0], file, line);
                return;
        }
        if (DEBUG) printf("[free] Successfully freed pointer %p\t %ld\n\n", pointer, heap_pointer - &HEAP[0]);
        return;
}

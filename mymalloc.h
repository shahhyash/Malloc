/************************************************
 *      mymalloc.h                              *
 *      Authors:  Seth Karten, Yash Shah        *
 *      2019                                    *
 ************************************************/
#ifndef __mymalloc_h_
#define __mymalloc_h_

#include <stdlib.h>
#include <stdio.h>

#define DEBUG 0

#define malloc(x) mymalloc(x, __FILE__, __LINE__)
#define free(x) myfree(x, __FILE__, __LINE__)

static char HEAP[4096];

/*
 *      Converts x from an integer to a psuedo-base 64 character stored at
 *      HEAP[address] and HEAP[address+1]. The two byte representation can store
 *      values 0-4095.
 */
void dec_to_base64(int x, int address);
/*
 *      Converts the psuedo-base 64 characters stored and HEAP[address] and
 *      HEAP[address+1] to an integer and returns the integer.
 */
int base64_to_dec(int address);
/*
 *      Converts the psuedo-base 64 characters stored and address[0] and
 *      adresss[1] to an integer and returns the integer.
 */
int base_to_dec_pointer(char * address);
/*
 *      Returns 1 if heap is initialized, 0 otherwise.
 */
int heap_initialized();
/*
 *      Prints indices of the heap for debugging
 */
void print_heap(int max);
/*
 *      Combines contiguous free blocks in HEAP and updates superblock metadata
 *      to reclaim space taken up by block metadata.
 */
void clean();
/*
 *      Function to fetch most optimal data block to store data in using first fit selection.
 *
 *      Returns -1 if it is unable to find a block, otherwise it returns
 *      index in HEAP[] of start of data block.
 *
 */
int fetch_optimal_location(size_t size);
/*
 *      Removes total available space in the metadata in superblock of heap
 */
void update_available_space(int bytes_used);
/*
 *      Reclaims available space metadata in superblock
 */
void mark_free_space(int size);
/*
 *      Function used to create new unused data node in HEAP structure given
 *      the index and size.
 *
 *       Returns 1 if operation succeeded, and 0 if there was an error.
 *
 */
int create_data_node(int index, size_t size);
/*
 *      Given a valid size and enough contiguous memory located on the HEAP,
 *      mymalloc will return a pointer to the beginning of the block.
 *      Otherwise, mymalloc will return NULL and print an error to stderr.
 */
void * mymalloc(size_t size, char * file, int line);
/*
 *      Given a valid pointer with a flag set to IN_USE, myfree will mark the
 *      flag as NOT_IN_USE and update the superblock with the reclaimed space
 *      from the no longer in use block.
 *      On all other inputs, it will print an error to stderr explaining the
 *      possible error.
 */
void myfree(void * pointer, char * file, int line);

#endif

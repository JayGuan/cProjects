#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stddef.h>
#ifndef MYMALLOC_INCLUDED
#define MYMALLOC_INCLUDED
////////////////////////////////////////////////////////////////////////////////
////
////  File          : mymalloc.h
////  Description   : This is a set of general-purpose utility functions we use
////                  for the 257 assignment #2.
////
////  Author   : DongChen Guan
////  Created  : 24 Oct 2016
//
////
//// Functional Prototypes
void display(); 
void *nonfree_malloc(size_t size);
 struct block_meta {
  size_t size;
  struct block_meta *next;
  struct block_meta *prev;
  void *ptr;
  int free;
  char data[1];
 int magic;    // For debugging only. TODO: remove this in non-debug mode.
}; 
// void *global_base = NULL;
void *getGlobalBase(void);
typedef struct block_meta *block_meta_ptr;
struct block_meta *find_free_block(struct block_meta **last, size_t size);
struct block_meta *request_space(struct block_meta* last, size_t size);
void split_block(block_meta_ptr block, size_t size); 
block_meta_ptr merge(block_meta_ptr block);
void *malloc(size_t size); 
void *calloc(size_t nelem, size_t elsize); 
struct block_meta *get_block_ptr(void *ptr); 
int valid_addr(void *p);
void free(void *ptr);
void *realloc(void *ptr, size_t size); 
#endif

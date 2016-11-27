#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
// Don't include stdlb since the names will conflict?
typedef struct block_meta *block_meta_ptr;

// TODO: align
#define align(k) (((((k)-1)>>2)<<2)+4)

// sbrk some extra space every time we need it.
// This does no bookkeeping and therefore has no ability to free, realloc, etc.
void *nofree_malloc(size_t size) {
  void *p = sbrk(0);
  void *request = sbrk(size);
  if (request == (void*) -1) { 
    return NULL; // sbrk failed
  } else {
    assert(p == request); // Not thread safe.
    return p;
  }
}


struct block_meta {
  size_t size;
  struct block_meta *next;
  struct block_meta *prev;
  int free;
  void *ptr;
  char data[1];
 int magic;    // For debugging only. TODO: remove this in non-debug mode.
};

#define META_SIZE sizeof(struct block_meta)

void *global_base = NULL;
block_meta_ptr answer = NULL;
// Iterate through blocks until we find one that's large enough.
struct block_meta *find_free_block(struct block_meta **last, size_t size) {
  struct block_meta *current = global_base;
  size_t diff;
  size_t minDiff;  
// find first block
while (current && !(current->free && current->size >= size)) {
    *last = current;
    current = current->next;
  }
// set best fit to first block as default
if (!(current == NULL)) {
  diff = (current->size)-size;
  minDiff = diff;
  answer = current;
} 
  // find difference of found block size comparing to needed size
  //while has next block

  while(!(current == NULL) && current->next) {
  while (current && !(current->free && current->size >= size)) {
    *last = current;
    current = current->next;
  }
  diff = (current->size) - size;
  if (diff<minDiff) {
    answer = current;
    minDiff = diff;
	}
  }
return answer;
}

struct block_meta *request_space(struct block_meta* last, size_t size) {
  struct block_meta *block;
  block = sbrk(0);
  void *request = sbrk(size + META_SIZE);
  assert((void*)block == request); // Not thread safe.
  if (request == (void*) -1) {
    return NULL; // sbrk failed.
  }
  
  if (last) { // NULL on first request.
    last->next = block;
  }
  block->size = size;
  block->next = NULL;
  block->free = 0;
  block->magic = 0x12345678;
  return block;
}

//split block
void split_block(block_meta_ptr block, size_t size) {
	block_meta_ptr newBlock;
	newBlock =(block_meta_ptr)(block->data + size);
	newBlock->size = block->size - size - META_SIZE;
	newBlock->next = block->next;
	newBlock->free = 1;
	block->size = size;
	block->next = newBlock;

}

//merge blocks
block_meta_ptr merge(block_meta_ptr block) {
	if (block->next && block->next->free) {
		block->size += META_SIZE + block->next->size;
		block->next = block->next->next;
		if (block->next)
		  block->next->prev = block;
	}
	return (block);
}

// If it's the first ever call, i.e., global_base == NULL, request_space and set global_base.
// Otherwise, if we can find a free block, use it.
// If not, request_space.
void *malloc(size_t size) {
  struct block_meta *block;
  // TODO: align size?
  size = (size_t)align(size);
  if (size <= 0) {
    return NULL;
  }

  if (!global_base) { // First call.
    block = request_space(NULL, size);
    if (!block) {
      return NULL;
    }
    global_base = block;
  } else {
    struct block_meta *last = global_base;
    block = find_free_block(&last, size);
    if (!block) { // Failed to find free block.
      block = request_space(last, size);
      if (!block) {
	return NULL;
      }
    } else {      // Found free block
      // TODO: consider splitting block here.
      if((block->size - size)>=(META_SIZE+4)) {
	split_block(block,size);
	}
      block->free = 0;
      block->magic = 0x77777777;
    }
  }
  
  return(block+1);
}

void *calloc(size_t nelem, size_t elsize) {
  size_t size = nelem * elsize;
  size = (size_t) align(size);
  void *ptr = malloc(size);
  memset(ptr, 0, size);
  return ptr;
}

// TODO: maybe do some validation here.

struct block_meta *get_block_ptr(void *ptr) {
  return (struct block_meta*)ptr - 1;
}

int valid_addr(void *p) {
        if(global_base) {
          if(p>global_base && p<sbrk(0) ) {
                return (p == (get_block_ptr(p))->ptr);
                }
        }
        return (0);
}


void free(void *ptr) {
  if (!ptr) {
    return;
  }

  // TODO: consider merging blocks once splitting blocks is implemented.
  struct block_meta* block_ptr = get_block_ptr(ptr);
  assert(block_ptr->free == 0);
  assert(block_ptr->magic == 0x77777777 || block_ptr->magic ==0x12345678);  
  block_ptr->free = 1;
  block_ptr->magic = 0x55555555;  
  // merge if there are smaller blocks free
  if (block_ptr->prev && block_ptr->prev->free)
	{
	block_ptr = merge(block_ptr->prev);}
  if (block_ptr->next) 
	merge(block_ptr);
  else {
	if(block_ptr->prev)
	  block_ptr->prev->next = NULL;
	else 
	  global_base = NULL;
	brk(block_ptr);
	}
}

void *realloc(void *ptr, size_t size) {
  if (!ptr) { 
    // NULL ptr. realloc should act like malloc.
    return malloc(size);
  }
  if (valid_addr(ptr)) {
  struct block_meta* block_ptr = get_block_ptr(ptr);
  size = (size_t)align(size); 
  if (block_ptr->size >= size) {
    // We have enough space. Could free some once we implement split.
    if (block_ptr->size - size >= (META_SIZE + 4))
	split_block(block_ptr,size); 
  return ptr;
  }

  // Need to really realloc. Malloc new space and free old space.
  // Then copy old data to new space.
  if (block_ptr->next && block_ptr->next->free
    &&(block_ptr->size + META_SIZE + block_ptr->next->size) >= size) {
	merge(block_ptr);
	if (block_ptr->size - size >= (META_SIZE + 4))
	  split_block(block_ptr,size);
	}
  else {
  void *new_ptr;
  new_ptr = malloc(size);
  if (!new_ptr) {
    return NULL; // TODO: set errno on failure.
  }
  memcpy(new_ptr, ptr, block_ptr->size);
  free(ptr);
  return new_ptr;	
	}
	return(block_ptr);
  }
  return (NULL);
}
int check_memory(void) {
        printf("Meta size: %d\n",(int)(META_SIZE));
	struct block_meta *current = global_base;
        // loop and print only if there is something in the heap
        int count = 0;
        if (current) {
       // loop until top of the heap
        while(!(current == NULL) && current->next) {
        printf("Block [%d] at (%p) want size %d has size %d free = %d\n",
                                                                                                         //count,current,(unsigned) (current->next-current));
        count, current, (int)current->size, (int) ((char*)current->next - (char*)current), current->free);
	count++;
	current=current->next;
	}
	printf("Block [%d] at(%p) want size %d has size %d free = %d\n", 
	count, current, (int)current->size, (int)((char*)sbrk(0)-(char*)current), current->free);
	}	
}


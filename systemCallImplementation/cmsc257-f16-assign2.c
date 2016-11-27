#include "cmsc257-f16-assign2-support.h"
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>

void *base = 0x0;
/*
int check_memory(void) {
	//print memory address of the pointer used for malloc instead!!
	void *ptr = sbrk(0);
	printf("Top of heap: [%p] (%x)\n", ptr,(unsigned)(ptr-last));
	return (0);
}
*/
/*
int check_memory(void) {
	struct block_meta *current = global_base;
	// loop and print only if there is something in the heap
		int count = 0;
		if (current) {
		// loop until top of the heap
		while(!(current == NULL) && current->next && (void*)(current)<sbrk(0) ) {
		printf("Block [%d] at (%p) has size %x\n",
		count,current,(unsigned) (current->next-current));
		count ++;
		current = current->next;
		}
		// reached top of the heap
		printf("Block [%d] at (%p) has size %x\n",
		count,current, (unsigned) ((struct block_meta *)sbrk(0)-current));
	}
}
*/
int main(void) {
	void *ptr[15], *ptr2[15], *ptr3[15];
	int i;
	// malloc test
	for (i = 0 ; i < 5; i++) {
	  ptr[i] = malloc(51);
	}
	  for (i = 5 ; i < 10; i++) {
          ptr[i] = malloc(200);
	}
          for (i = 10 ; i < 15; i++) {
          ptr[i] = malloc(300);
	 }
	printf("malloc 5 [51] bytes, 5 [200] bytes, 5 [300] bytes\n");
	//calloc test
	  for (i = 0 ; i < 5; i++) {
          ptr2[i] = calloc(5,4);
        }
          for (i = 5 ; i < 10; i++) {
          ptr2[i] = calloc(5,5);
        }
          for (i = 10 ; i < 15; i++) {
          ptr2[i] = calloc(5,7);
        }
	
	for (i=0; i<15; i++) {
	  ptr3[i] = realloc(ptr3[i],4);
	}
	printf("calloc 5 [4] bytes, 5 [5] bytes, 5 [7] bytes\n");
	check_memory();
	printf("\n\nAfter 5 free()\n\n");
	// 5 calls to free
	free(ptr[7]);
	free(ptr[6]);
	free(ptr[5]);
	free(ptr[4]);
	free(ptr2[10]);
	free(ptr2[9]);
	check_memory();
}

/*
int main(void) {
printf("\nhelloWorld\n");
}
*/

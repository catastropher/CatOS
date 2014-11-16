#include "CatOS.h"

#include <string.h>

#define RAM_PAGES 8

#define BLOCK_FREE 127		// Used to indicate that a block of memory is free
#define BLOCK_SIZE 128		// The size of a memory block
#define TOTAL_BLOCKS (16384 / BLOCK_SIZE)	// The total number of blocks in a single RAM page

// Allocation table for the entire operating system (across all 8 RAM pages)
uchar malloc_tab_full[RAM_PAGES * TOTAL_BLOCKS];

// Allocates a chunk of memory that is at least 'size' bytes in length. This function
// allows you to customize the allocation: between which allocation blocks (start and end)
// to search for, which process ID to allocate it for, and which of the 8 RAM pages it
// should be allocated for.
// Note: this is meant as an internal function and shouldn't be used by user programs.
void *malloc_from_range(ushort size, uchar start, uchar end, uchar pid, uchar page) {
	uchar i;
	uchar count = 0;
	uchar needed = size / 128;
	uchar begin = start;
	uchar d;
	uchar *malloc_tab = &malloc_tab_full[(ushort)page * TOTAL_BLOCKS];
	uchar allow_int = allow_interrupts;
	
	allow_interrupts = 0;
	
	if(size > 32000)
		return NULL;
	
	if((size % 128) != 0)
		needed++;
	
	needed++;
	
	// Search for a sequence of blocks which is at least equal to 'needed' blocks
	for(i = start;i <= end;i++) {
		if(malloc_tab[i] == BLOCK_FREE) {
			if(count == 0)
				begin = i;
			
			
			count++;
			
			if(count == needed) {
				malloc_tab[begin] = pid;
				
				for(d = 1;d < needed;d++) {
					malloc_tab[d + begin] = pid + 128;
				}
				
				allow_interrupts = 1;
				//allow_interrupts = allow_int;
				
				if(pid != 0)
					return (void *)((ushort)begin * BLOCK_SIZE + 0xC000);
				else
					return (void *)((ushort)begin * BLOCK_SIZE + 0x8000);
			}
		}
		else {
			count = 0;
		}
	}
	
	allow_interrupts = 1;
	
	HALT();
	
	return NULL;
}

// Allocates a chunk of memory for the filesystem
// Note: this sets the chunk of memory to 0
void *fs_alloc(ushort size) {
  void *data = malloc_from_range(size, 0, TOTAL_BLOCKS - 1, 0, 7);
  
  if(!data)
    return NULL;
  
  memset(data, 0, size);
  
  return data;
}

// General memory allocation routine for user programs (exactly like the C standard library
// function). Note that the size may be larger than what was asked for. Also, the process
// that calls this function owns the memory allocated. If the allocation fails, it returns NULL.
void *malloc(ushort size) {
	return malloc_from_range(size, 0, TOTAL_BLOCKS - 1, process_id, current_process->ram_page);
}

// Frees a block of memory allocated by any of the allocation routines.
// TODO: finish implementing!
void free(void *ptr) {
	//uchar block = ((ushort)ptr - 0x8000) / BLOCK_SIZE;
	ushort base;
	uchar block;
	
	if(ptr <= 0xC000)
		base = 0xC000;
	else
		base = 0x8000;
	
	//block = (ptr - base) / BLOCK_SIZE;
	
	
	
	
	
	
	
	
	
	
	
	
	
	//uchar *malloc_tab = &malloc_tab_full[process_tab[process_id].ram_page * TOTAL_BLOCKS];
	
	/*if(ptr >= (void *)0x8000 && malloc_tab[block] == current_process->id) {
		do {
			malloc_tab[block++] = BLOCK_FREE;
		} while(malloc_tab[block] >= 128);
	}
	*/
}

// Returns the number of blocks which are free on a given RAM page
uchar get_free_blocks(uchar page) {
	uchar i;
	uchar *malloc_tab = &malloc_tab_full[(ushort)page * TOTAL_BLOCKS];
	uchar count = 0;
	
	for(i = 0;i < TOTAL_BLOCKS;i++) {
		if(malloc_tab[i] == BLOCK_FREE)
			++count;
	}
	
	return count;
}

// Returns the total amount of free memory available to the system.
unsigned long get_free_mem() {
	unsigned long sum = 0;
	uchar i;
	
	for(i = 0;i < 8;i++) {
		if(i != 1)
			sum += get_free_blocks(i) * BLOCK_SIZE;
	}
	
	return sum;
}


// Selects the RAM page with the most free blocks
uchar get_best_ram_page() {
	uchar best_blocks = 0;
	uchar best_page = 0;
	uchar i;
	uchar count;
	
	for(i = 0;i < RAM_PAGES;i++) {
		if(i != 1) {
			count = get_free_blocks(i);
			
			if(count > best_blocks) {
				best_blocks = count;
				best_page = i;
			}
		}
	}
	
	return best_page;
}

// Allocates memory for the given process
void *malloc_for_pid(uchar pid, ushort size) {
	return malloc_from_range(size, 0, TOTAL_BLOCKS - 1, pid, process_tab[pid].ram_page);
}

// Allocates kernal memory.
// Note: the memory allocated is owned by the system process!
void *system_alloc(ushort size) {
	return malloc_from_range(size, 0, TOTAL_BLOCKS - 1, process_id, 1);
}

// Initializes the memory manager
void init_memory() {
	uchar i;
	
	// Set every block in the allocation table to be free
	memset(malloc_tab_full, BLOCK_FREE, RAM_PAGES * TOTAL_BLOCKS);
	
	// Reserve some kernal RAM for global variables
	malloc_tab_full[TOTAL_BLOCKS] = 128;
	memset(malloc_tab_full + TOTAL_BLOCKS + 1, 0, 25);
}

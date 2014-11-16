#include "CatOS.h"

#include <string.h>

#define RAM_PAGES 8

#define BLOCK_FREE 127
#define BLOCK_SIZE 128
#define TOTAL_BLOCKS (16384 / BLOCK_SIZE)

uchar malloc_tab_full[RAM_PAGES * TOTAL_BLOCKS];

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

void *fs_alloc(ushort size) {
  void *data = malloc(size);//malloc_from_range(size, 0, TOTAL_BLOCKS - 1, 0, 7);
  
  if(!data)
    return NULL;
  
  memset(data, 0, size);
  
  return data;
}

void *malloc(ushort size) {
	return malloc_from_range(size, 0, TOTAL_BLOCKS - 1, process_id, current_process->ram_page);
}

void free(void *ptr) {
	uchar block = ((ushort)ptr - 0x8000) / BLOCK_SIZE;
	//uchar *malloc_tab = &malloc_tab_full[process_tab[process_id].ram_page * TOTAL_BLOCKS];
	
	/*if(ptr >= (void *)0x8000 && malloc_tab[block] == current_process->id) {
		do {
			malloc_tab[block++] = BLOCK_FREE;
		} while(malloc_tab[block] >= 128);
	}
	*/
}

ushort get_mem_usage(uchar pid) {
	ushort sum = 0;
	uchar i;

	/*for(i = 0;i < 255;i++) {
		if((malloc_tab[i] & 0x7F) == pid)
			sum += BLOCK_SIZE;
	}*/
	
	return sum;
}

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

ushort get_total_mem_usage() {
	ushort sum = 0;
	uchar i;
	
	/*for(i = 0;i < 255;i++) {
			if(malloc_tab[i] != BLOCK_FREE)
				sum += BLOCK_SIZE;
	}*/
	
	return sum;
}

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

void *malloc_for_pid(uchar pid, ushort size) {
	return malloc_from_range(size, 0, TOTAL_BLOCKS - 1, pid, process_tab[pid].ram_page);
}

void *system_alloc(ushort size) {
	return malloc_from_range(size, 0, TOTAL_BLOCKS - 1, process_id, 1);
}


void init_memory() {
	uchar i;
	
	memset(malloc_tab_full, BLOCK_FREE, RAM_PAGES * TOTAL_BLOCKS);
	
	//malloc_tab = malloc_tab_full;
	
	malloc_tab_full[TOTAL_BLOCKS] = 128;
	
	
	memset(malloc_tab_full + TOTAL_BLOCKS + 1, 0, 25);
	//for(i = 1;i < 25;i++) {
	//	malloc_tab_full[(ushort)i + TOTAL_BLOCKS] = 128;
	//}
	
	/*
	malloc_tab[0] = 0;
	
	for(i = 1;i < 200;i++)
		malloc_tab[i] = 128;
	
	*/
}

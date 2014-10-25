#include "CatOS.h"

unsigned char malloc_tab[256];

#define BLOCK_FREE 127
#define BLOCK_SIZE 128

void *malloc_from_range(ushort size, uchar start, uchar end) {
	uchar i;
	uchar count = 0;
	uchar needed = size / 128;
	uchar begin = start;
	uchar d;
	
	if(size > 32000)
		return NULL;
	
	if((size % 128) != 0)
		needed++;
	
	for(i = start;i <= end;i++) {
		if(malloc_tab[i] == BLOCK_FREE) {
			if(count == 0)
				begin = i;
			
			
			count++;
			
			if(count == needed) {
				malloc_tab[begin] = current_process->id;
				
				for(d = 1;d < needed;d++) {
					malloc_tab[d + start] = current_process->id + 128;
				}
				
				return (void *)((ushort)begin * BLOCK_SIZE + 0x8000);
			}
		}
		else {
			count = 0;
		}
	}
	
	return NULL;
}

void *malloc(ushort size) {
	return malloc_from_range(size, 0, 255);
}

void *calloc(ushort size) {
	uchar *data = malloc(size);
	
	while(size > 0) {
		*data = 0;
		data++;
	}
	
	return data;
}

void free(void *ptr) {
	uchar block = ((ushort)ptr - 0x8000) / BLOCK_SIZE;
	
	if(ptr >= (void *)0x8000 && malloc_tab[block] == current_process->id) {
		do {
			malloc_tab[block++] = BLOCK_FREE;
		} while(malloc_tab[block] >= 128);
	}
}

ushort get_mem_usage(uchar pid) {
	ushort sum = 0;
	uchar i;

	for(i = 0;i < 255;i++) {
		if((malloc_tab[i] & 0x7F) == pid)
			sum += BLOCK_SIZE;
	}
	
	return sum;
}

ushort get_total_mem_usage() {
	ushort sum = 0;
	uchar i;
	
	for(i = 0;i < 255;i++) {
			if(malloc_tab[i] != BLOCK_FREE)
				sum += BLOCK_SIZE;
	}
	
	return sum;
}

void init_memory() {
	uchar i;
	
	malloc_tab[0] = 0;
	malloc_tab[1] = 128;
	malloc_tab[2] = 128;
	malloc_tab[3] = 128;
	
	for(i = 4;i < 255;i++) {
		malloc_tab[i] = BLOCK_FREE;
	}
}

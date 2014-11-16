#ifndef MEMORY_H
#define MEMORY_H

void *malloc_from_range(ushort size, uchar start, uchar end, uchar pid, uchar page);
void *malloc(ushort size);
void free(void *ptr);
ushort get_mem_usage(uchar pid);
ushort get_total_mem_usage();
void init_memory();
uchar get_best_ram_page();
void *malloc_for_pid(uchar pid, ushort size);
unsigned long get_free_mem();
void *system_alloc(ushort size);
uchar get_free_blocks(uchar page);
void *fs_alloc(ushort size);

#endif

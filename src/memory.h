#ifndef MEMORY_H
#define MEMORY_H

void *malloc_from_range(ushort size, uchar start, uchar end);
void *malloc(ushort size);
void free(void *ptr);
ushort get_mem_usage(uchar pid);
ushort get_total_mem_usage();
void init_memory();

#endif

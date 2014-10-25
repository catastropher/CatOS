//unsigned char screen[768];

#include <string.h>
#include <stdio.h>

#include "CatOS.h"

void cls() {
	ushort i;
	
	for(i = 0;i < 768;i++)
		system_screen[i] = 0;
}

void main() {
	uchar buf[16];
	void *mem;
	ushort i = 0;
	
	system_screen = 0xA000;
	init_multitask();
	init_memory();
	current_process->id = 0;
	
	mem = malloc(128);
	
	free(mem);
	
	sprintf(buf, "%u", get_total_mem_usage());
	
	cls();
	
	for(i = 35000; i < 35000+1;i++) {
		mem = malloc(i);
		
		if(!mem) {
			draw_string("failed", 0, 0);
			break;
		}
		
		free(mem);
	}
	
	
	//draw_string(buf, 0, 0);
	draw_string("done", 0, 20);
	cps();
	
	while(1) ;
	
	
	
}














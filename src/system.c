#include <stdio.h>

#include "CatOS.h"

Event *system_event_queue;
uchar system_event_queue_start;
uchar system_event_queue_end;
uchar system_items_in_queue;

#define SYSTEM_QUEUE_SIZE (128 / sizeof(Event))

void system_process4();

void set_screen_constrast(uchar value) __naked {
	__asm
		push hl
		push af
		ld hl,#6
		add hl,sp
		
		ld a,(hl)
		out (#0x10),a
	
		nop
		nop
		nop
		nop
	
		pop af
		pop hl
		ret
	__endasm;
}

void delay(ushort n) {
	volatile ushort a = n;
	
	while(a > 0)
		a--;
}

void screen_fade_in() {
	uchar i;
	
	for(i = 0xC0;i < 0xF0;i++) {
		set_screen_constrast(i);
		delay(3000);
	}
}

void screen_fade_out() {
	uchar i;
	
	for(i = 0xF0;i > 0xC0;i--) {
		set_screen_constrast(i);
		delay(3000);
	}
}

void boot_screen() {
	ushort i;
	
	init_visible_console(0, 255);
	switch_console(0);
	
	set_screen_constrast(0xC0);
	
	delay(30000);
	
	printf("       CatOS v1.0      \n\n\n");
	printf("    Duncan Campbell\n");
	printf("     Jason McGough\n");
	printf("      Eric Rieman\n");
	printf("    Michael Wilder\n");
	
	for(i = 0;i < LINE_SIZE - 12;i++) {
		system_screen[i] = ~system_screen[i];
	}
	
	cps();
	screen_fade_in();
	delay(65535);
	delay(65535);
	delay(65535);
	delay(65535);
	screen_fade_out();
	
	free(console_tab[0]);
	console_tab[0] = NULL;
	
	start_process(system_process4, "console");
	init_visible_console(1, 255);
	switch_console(0);
	
	start_program(system_process4, "console", 255);
	start_program(system_process4, "console", 255);
	start_program(system_process4, "console", 255);
	start_program(system_process4, "console", 255);
	
	
	force_context_switch();
	
	//wait(10);
	delay(65535);
	
	screen_fade_in();
	
	allow_interrupts = 1;
	
}

void generate_system_event(uchar event, uchar arg, void *data) {
	EI();
	
	// If the system queue is full, let some other processes run unti there is room
	while(system_items_in_queue == SYSTEM_QUEUE_SIZE)
		force_context_switch();
	
	DI();
	
	system_event_queue[system_event_queue_end].event_id = event;
	system_event_queue[system_event_queue_end].pid = process_id;
	system_event_queue[system_event_queue_end].arg = arg;
	system_event_queue[system_event_queue_end].data = data;
	
	system_event_queue_end = (system_event_queue_end + 1) % SYSTEM_QUEUE_SIZE;
	system_items_in_queue++;
	
	if(allow_interrupts)
		EI();
}

void catos_system_process() {
	Event *e;
	Console *save;
	uchar save_id;
	
	boot_screen();
	
	//start_process(system_process4, "console");
	//init_visible_console(1, 255);
	//switch_console(0);
	allow_interrupts = 1;
	
	//start_program(system_process4, "console", 255);
	
	system_event_queue = system_alloc(128);
	system_event_queue_start = 0;
	system_event_queue_end = 0;
	system_items_in_queue = 0;
	
	do {
		if(system_items_in_queue == 0)
			force_context_switch();
		else {
			DI();
			e = &system_event_queue[system_event_queue_start];
			
			switch(e->event_id) {
				case EV_KILL_PROCESS:
					if(console_pid == e->arg) {
						console_pid = process_tab[e->arg].parent_id;
						console_tab[process_tab[e->arg].parent_id] = console_tab[e->arg];
					}
					
					free_all_for_pid(e->arg);
					process_tab[e->arg].id = 255;
					console_tab[e->arg] = NULL;
					
					if(process_tab[process_tab[e->arg].parent_id].flags & PROC_WAIT_RETURN) {
						process_tab[process_tab[e->arg].parent_id].flags |= PROC_RUN;
						process_tab[process_tab[e->arg].parent_id].flags &=
							(~(PROC_WAIT_RETURN | PROC_WAIT_KEY));
					}
					
					break;
			}
			
#if 0
			allow_interrupts = 0;
			DI();
			save = current_con;
			save_id = console_pid;
			console_pid = 5;
			current_con = console_tab[5];
			printf("Killed %d,%u\n", e->arg, e->data);
			current_con = save;
			console_pid = save_id;
			e->arg = 9;
			//EI();
#endif		
			
			system_event_queue_start = (system_event_queue_start + 1) % SYSTEM_QUEUE_SIZE;
			system_items_in_queue--;
			allow_interrupts = 1;
			EI();
		}
	} while(1);
}







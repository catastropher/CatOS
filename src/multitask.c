#include <stdio.h>
#include <string.h>

#include "CatOS.h"

uchar process_id;

void cls();

volatile unsigned long int_counter;
volatile unsigned long int_counter2;

uchar isr_flags;

void wait(ushort ticks) {
	unsigned long start = int_counter;
	
	__asm
		ei
	__endasm;
	
	while(int_counter < start + ticks) ;
}

void do_switch(uchar new_pid) {
	current_process = &process_tab[new_pid];
	process_id = new_pid;
	current_con = console_tab[new_pid];
	//printf("_Switch process\n");
	//printf("Resume address: %u\n", ((ushort *)current_process->sp)[8]);
	//wait(200);
}

ushort switch_process(uchar new_pid) __naked{
	do_switch(new_pid);
	
	__asm
		;halt
		ld hl, (_current_process)
		ld e,(hl)
		inc hl
		ld d,(hl)
		ex de,hl
		ld sp,hl
		;pop hl
		;halt
		jp isr_process_resume
		;ret
	__endasm;
}

void force_context_switch() __naked {
	DI();
	allow_interrupts = 1;
	isr_flags = FORCE_SWITCH;
	
	__asm
		rst #0x38
		ret
	__endasm;
}


uchar pid;
uchar key_counter;

#define MAX_PROC 16

uchar check_function_keys() __naked {
	__asm
		push af
		push bc
		ld b,#5
		ld a,#0xBF
		out (#1),a
		nop
		nop
		in a,(#1)
		
	check_function_key_loop:
		bit #4,a
		jr Z,check_function_key_done
		rlca
		djnz check_function_key_loop
	check_function_key_done:
		ld a,#5
		sub b
		ld l,a
		pop bc
		pop af
		ret
	__endasm;
}






uchar transition_mode;
uchar transition_pos;
uchar *transition_screen;

uchar cursor_counter;

void interrupt_handler(uchar flags) {
	uchar i;
	uchar key;
	
	//printf("Interrupt!\n");
	
	if(key_counter++ == 1) {
		key_counter = 0;
		
		key = check_function_keys();
		
		if(key != 5) {
			switch_console(key);
			//switch_console(0);
		}
	}
	
	memcpy(&current_process->old_state, &current_process->new_state, sizeof(KeyState));
	read_keypad(current_process->new_state.key_map);
	
	key = compare_key_matrix(&current_process->old_state, &current_process->new_state);
	
	if(key != 0) {
		current_process->key = key;
	}
	
	if(flags & 2) {
		int_counter++;
		int_counter2++;
	}
	
	/*
	if(key != 0) {
		console_printc(active_con, key);
		console_printc(active_con, '\n');
	}
	*/
	
	if((int_counter%2) == 0 || (flags & FORCE_SWITCH)) {
		/*if(pid == 1) {
			printf("HALT\n");
			
			while(1) ;
		}*/
		
		
		i = pid;
		
		do {
			++i;
			
			if(i >= MAX_PROC) {
				i = 0;
			}
			
			if(process_tab[i].id != 255 && (process_tab[i].flags & PROC_RUN) != 0) {
				pid = i;
				//int_counter = 0;
				switch_process(pid);
			}
		} while(1);
		
		//printf("Switch!\n");
		//
	}
}

void init_process() __naked {
	__asm
		di
		;halt
		ld hl,(_current_process)
		inc hl
		inc hl
		ld a,(hl)
		;inc a
		
		out (#5),a
		ret
	__endasm;
}

void write_short_far(uchar page, ushort *addr, ushort val) __naked {
	__asm
	
	push bc
	push de
	push hl
	push ix
	
	ld ix,#0
	add ix,sp
	
	ld c,10(ix)
	
	ld l,11(ix)
	ld h,12(ix)
	
	ld e,13(ix)
	ld d,14(ix)
	
	in a,(#5)
	ld b,a
	
	ld a,c
	out (#5),a
	
	ld (hl),e
	inc hl
	ld (hl),d
	
	ld a,b
	out (#5),a
	
	pop ix
	pop hl
	pop de
	pop bc
	ret
	
	__endasm;
}

uchar start_process(void *function, uchar *name) {
	uchar i;
	ushort *ptr;
	uchar page;
	
	allow_interrupts = 0;
	
	for(i = 0;i < MAX_PROC;i++) {
		if(process_tab[i].id == 255) {
			//printf("Start process %d\n", i);
			strcpy(process_tab[i].name, name);
			
			if(i != 0)
				process_tab[i].ram_page = get_best_ram_page();
			else
				process_tab[i].ram_page = 1;
			
			process_tab[i].id = i;
			process_tab[i].sp = (ushort)system_alloc(256) + 200;//(ushort)malloc_for_pid(i, 500) + 450;
			
			
			if(process_tab[i].sp - 450 == 0xD000) {
			}
			
			//process_tab[i].sp = 0xB000 + (ushort)i*500 + 450;//(ushort)malloc(128) + 60;
			process_tab[i].flags = PROC_RUN;
			
			memset(&process_tab[i].new_state, 0, sizeof(KeyState));
			
			ptr = (ushort *)process_tab[i].sp;
			
			//ptr[8] = (ushort)init_process;
			//ptr[9] = (ushort)function;

			
			write_short_far(process_tab[i].ram_page, &ptr[0], (ushort)process_tab[i].ram_page * 256 + process_tab[i].ram_page);
			
				write_short_far(process_tab[i].ram_page, &ptr[1], (ushort)process_tab[i].ram_page * 256 + process_tab[i].ram_page);
			
			//write_short_far(process_tab[i].ram_page, &ptr[1], 1 | 256);
			write_short_far(process_tab[i].ram_page, &ptr[7], (ushort)init_process);
			write_short_far(process_tab[i].ram_page, &ptr[8], (ushort)function);

			
			return i;
		}
	}
	
	return 255;
}

void begin_run_process() {
	current_process = &process_tab[0];

	
	
	__asm
		;di
		;halt
		ld a,#1
		ld (_allow_interrupts),a
		ei
		ld hl,(_current_process)
		inc hl
		inc hl
		ld a,(hl)
		out (#5),a
	__endasm;
	
	//HALT();
	switch_process(0);
}

uchar start_program(void *function, uchar *name, uchar parent_id) {
	uchar allow_int = allow_interrupts;
	uchar pid;
	
	allow_interrupts = 0;
	DI();
	pid = start_process(function, name);
	init_visible_console(pid, 255);
	allow_interrupts = allow_int;
	
	if(allow_int) {
		EI();
	}
	
	return pid;
}
	

void init_multitask() {
	uchar i;
	current_process = &process_tab[0];
	
	int_counter2 = 0;
	isr_flags = 0;
	
	//printf("Init multitask\n");
	
	for(i = 0; i < 16;i++) {
		process_tab[i].id = 255;
	}
	
	key_counter = 0;
	
	/*start_process(function_sum);
	start_process(function_sum);
	start_process(function_sum);
	start_process(function_sum);
	start_process(function_sum);
	start_process(my_process);
	
	//start_process(function);	
	//start_process(function4);
	//start_process(function);*/
	
	//begin_run_process();
}














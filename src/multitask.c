#include <stdio.h>
#include <string.h>

#include "CatOS.h"

uchar process_id;

void cls();

volatile unsigned long int_counter;
volatile unsigned long int_counter2;

uchar isr_flags;

// Waits for at least 'ticks' interrupts to occur before returning. Note: it's possible
// for more interrupts to happenen before it returns. This is almost like Sleep(), but
// instead the time is measured in interrupts, not ms.
void wait(ushort ticks) {
	unsigned long start = int_counter;
	
	__asm
		ei
	__endasm;
	
	while(int_counter < start + ticks) {
		force_context_switch();
	}
}

// Switches the kernal global variables to reflect what the new process is.
// This has to be outside of switch_process because switch_process modifies
// the stack pointer, so we don't want it to have a stack frame.
void do_switch(uchar new_pid) {
	current_process = &process_tab[new_pid];
	process_id = new_pid;
	current_con = console_tab[new_pid];
}

// Switches the currently running process to be new_pid
// Warning: it is assumed that the process control block has already been saved,
// so this will modify the stack pointer... do not call this directly!
ushort switch_process(uchar new_pid) __naked{
	do_switch(new_pid);
	
	__asm
		ld hl, (_current_process)
		ld e,(hl)
		inc hl
		ld d,(hl)
		ex de,hl
		ld sp,hl
		jp isr_process_resume
	__endasm;
}

// Forces a context switch
// Typically used in key waiting routines to increase throughput
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

// Checks the value of the function key, which is used to switch windows
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


// These next four variables should not be used
// TODO: remove them
uchar transition_mode;
uchar transition_pos;
uchar *transition_screen;
uchar cursor_counter;

// The "high-level" interrup handler, which is called from the assembly routine 'isr'
// (see startup.z80)
void interrupt_handler(uchar flags) {
	uchar i;
	uchar key;
	
	// Check if we should switch to a different window
	if(key_counter++ == 1) {
		key_counter = 0;
		
		key = check_function_keys();
		
		if(key != 5) {
			switch_console(key);
		}
	}
	
	// Copy the new key state to the old one and read in the new one
	memcpy(&current_process->old_state, &current_process->new_state, sizeof(KeyState));
	read_keypad(current_process->new_state.key_map);
	
	// Compare the key states to see if a key was pressed
	key = compare_key_matrix(&current_process->old_state, &current_process->new_state);
	
	if(key != 0) {
		current_process->key = key;
	}
	
	// If a timer interrupt happened, increase the timers
	if(flags & 2) {
		int_counter++;
		int_counter2++;
	}
	
	if((int_counter%2) == 0 || (flags & FORCE_SWITCH)) {
		// Use round-robin scheduling to find which process we should run next
		i = pid;
		
		do {
			++i;
			
			if(i >= MAX_PROC) {
				i = 0;
			}
			
			if(process_tab[i].id != 255 && (process_tab[i].flags & PROC_RUN) != 0) {
				pid = i;
				
				// We do not ever return from this...
				switch_process(pid);
			}
		} while(1);
	}
}

// This routine is the first thing a process calls when it starts running
// It sets the RAM page to what is stored in the process table
void init_process() __naked {
	__asm
		di
		ld hl,(_current_process)
		inc hl
		inc hl
		ld a,(hl)
		
		out (#5),a
		
		ld a,#1
		ld (_allow_interrupts),a
		ei
		ret
	__endasm;
}

// Write a ushort to a RAM page other than the current one (used to write return
// addresses to the stack when launching a process)
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

// Launches the process pointed to by 'function' with the given name
// Note: all processes must (unfortunately) be stored on flash page 0
// It was intended for each process to be stored on a separate flash page
// but due to a lack of time, all processes are built into the operating
// system on page 0.
// Returns the ID of the process if successful, 255 otherwise
uchar start_process(void *function, uchar *name) {
	uchar i;
	ushort *ptr;
	uchar page;
	uchar allow_int = allow_interrupts;
	
	allow_interrupts = 0;
	
	// Find a free entry in the process table
	// And ID of 255 indicates that a slot is empty
	for(i = 0;i < MAX_PROC;i++) {
		if(process_tab[i].id == 255) {
			strcpy(process_tab[i].name, name);
			
			// If we're not the system process, find the best RAM page (i.e. the one with the
			// most free RAM blocks) to run in.
			if(i != 0)
				process_tab[i].ram_page = get_best_ram_page();
			else
				process_tab[i].ram_page = 1;
			
			// Allocate 256 bytes of stack space, but leave some room for the register values
			// and return address we're going to inject
			process_tab[i].id = i;
			process_tab[i].sp = (ushort)system_alloc(256) + 200;//(ushort)malloc_for_pid(i, 500) + 450;
			
			// Set the process to be in the running state
			process_tab[i].flags = PROC_RUN;
			
			// Reset the key state (i.e. no keys are pressed)
			memset(&process_tab[i].new_state, 0, sizeof(KeyState));
			
			ptr = (ushort *)process_tab[i].sp;
			
			//ptr[8] = (ushort)init_process;
			//ptr[9] = (ushort)function;

			// Inject values for the flash and RAM page
			write_short_far(process_tab[i].ram_page, &ptr[0], (ushort)process_tab[i].ram_page * 256 + process_tab[i].ram_page);
			write_short_far(process_tab[i].ram_page, &ptr[1], (ushort)process_tab[i].ram_page * 256 + process_tab[i].ram_page);
			
			// Inject the address of init_process, which should be the first thing the process calls once it
			// starts running
			write_short_far(process_tab[i].ram_page, &ptr[7], (ushort)init_process);
			
			// Finally, inject the actual starting address of the function, which should be called after
			// init_process
			write_short_far(process_tab[i].ram_page, &ptr[8], (ushort)function);

			allow_interrupts = allow_int;
			
			return i;
		}
	}
	
	allow_interrupts = allow_int;
	
	return 255;
}

// Begins multitasking by allowing the first process to execute (the system process)
// Note: there is no returning from this function!
void begin_run_process() {
	current_process = &process_tab[0];

	__asm
		di
		;halt
		ld a,#1
		ld (_allow_interrupts),a
		ld hl,(_current_process)
		inc hl
		inc hl
		ld a,(hl)
		out (#5),a
		ei
	__endasm;
	
	//HALT();
	switch_process(0);
}

// Starts a program and creates a new visible console for it
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
	
// Initializes multitasking, but does not actually run the first process yet
// To do so, call begin_run_process()
void init_multitask() {
	uchar i;
	current_process = &process_tab[0];
	
	int_counter2 = 0;
	isr_flags = 0;
	
	for(i = 0; i < 16;i++) {
		process_tab[i].id = 255;
	}
	
	key_counter = 0;
}














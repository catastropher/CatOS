#include "CatOS.h"

#ifndef MULTITASK_H
#define MULTITASK_H

#define PROC_RUN 1
#define PROC_WAIT_KEY 2
#define PROC_WAIT_RETURN 4

#define FORCE_SWITCH 64

typedef struct {
	ushort sp;
	uchar ram_page;
	uchar id;
	uchar flags;
	
	uchar name[9];
	uchar parent_id;
	
	KeyState old_state;
	KeyState new_state;
	
	uchar key;
	
	uchar *screen;
} ProcessEntry;

void init_multitask();
uchar start_process(void *function, uchar *name);
void begin_run_process();
uchar check_function_keys();
void force_context_switch();
uchar start_program(void *function, uchar *name, uchar parent_id);
ushort switch_process(uchar new_pid);
void end_process();
void call_program(void *function, uchar *name);

void set_ram_page(uchar page);
uchar get_ram_page();

extern uchar process_id;
extern uchar allow_interrupts;

extern uchar transition_mode;
extern uchar transition_pos;
extern uchar *transition_screen;

extern uchar isr_flags;
extern uchar cursor_counter;

#endif

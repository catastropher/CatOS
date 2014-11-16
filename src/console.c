#include <string.h>
#include <stdio.h>

#include "CatOS.h"


Console *active_con;		// The console that is currently displayed on the screen
Console *current_con;		// The console being used by the current process
uchar console_pid;			// The process ID of the process which owns the console that is active
uchar allow_interrupts;		// Whether interrupts are allowed to happen

// Table for all of the consoles in the operating system
Console *console_tab[MAX_PROCS];

// Copies the screen into a side-by-side window buffer for the screen transition
// effect.
void copy_half(uchar *dest, uchar *src) {
	uchar x, y;
	
	for(y = 0;y < 64;y++) {
		for(x = 0;x < 12;x++) {
			*dest = *src;
			dest++;
			src++;
		}
		
		dest += 12;
	}
}

// Shifts the entire screen so that it performs the window transition effect
void copy_screen_half(uchar *dest, uchar *src_left, uchar *src_right, uchar x_pos) __naked {
	__asm
		;push af
		push bc
		push de
		push hl
		push ix
		
		;10 - 11 -> dest
		;12 - 13 -> src_left
		;14 - 15 -> src_right
		;16      -> x_pos
		
		ld ix,#0
		add ix,sp
		
		ld e,16(ix)
		ld a,e
		srl e
		srl e
		srl e
		
		;e = byte offset
		ld d,#0
		ld l,12(ix)
		ld h,13(ix)
		add hl,de
		
		and #7
		ld c,a
		
		;c = shift amount
		;de = dest
		;hl = src
		
		;b' = count x
		;c' = count y
		
		ld a,#12
		;sub e
		
		ld e,10(ix)
		ld d,11(ix)
		
		exx
		
		ld c,#64
		ld b,a
		
	copy_screen_half_outer:
		push bc
		exx
		push de
		push hl
		exx
		
	copy_screen_half_inner:
		exx
		ld b,c

		ld a,b
		or a
		ld a,(hl)
		jr Z,copy_screen_half_skip
		
	copy_screen_half_shift:
		add a,a
		djnz copy_screen_half_shift
	
	copy_screen_half_skip:
		ld (de),a
		inc hl
		exx
		ld a,b
		exx
		cp #1
		jr Z,copy_screen_half_skip2
		
		ld a,#8
		sub c
		ld b,a
		
		ld a,(hl)
	copy_screen_half_shift2:
		srl a
		djnz copy_screen_half_shift2
		
		ex de,hl
		or (hl)
		ld (hl),a
		ex de,hl
		
	copy_screen_half_skip2:
		inc de
		exx
		djnz copy_screen_half_inner
		
		exx
		
		pop hl
		pop de
		push bc
		ld bc,#24
		add hl,bc
		ex de,hl
		ld bc,#12
		add hl,bc
		ex de,hl
		pop bc
		
		exx
		pop bc
		dec c
		jr NZ,copy_screen_half_outer
		
		pop ix
		pop hl
		pop de
		pop bc
		;pop af
		ret
	__endasm;
}
		


/*
void copy_screen_half(uchar *dest, uchar *src_left, uchar *src_right, uchar x_pos) {
	uchar shift_left = x_pos % 8;
	uchar y = 0;
	uchar byte;
	uchar x;
	uchar start_x = x_pos / 8;
	uchar real_x;
	
	for(y = 0;y < 64;y++) {
		for(x = start_x;x < 96 / 8;x++) {
			real_x = x - start_x;
			byte = src_left[(ushort)y * 12 + x];
			byte <<= shift_left;
			
			if(x < 96 / 8 - 1)
				byte |= (src_left[(ushort)y * 12 + x + 1]) >> (8 - shift_left);
			
			dest[(ushort)y * 12 + real_x] = byte;
		}
	}
}
*/

// Switches to the nth visible console. If it doesn't exists, the console
// isn't switched. Note: this should only be called in the ISR to prevent
// race conditions! This also does the cool screen transition effect! All
// processes are paused while this transition happens!
void switch_console(uchar id) {
	uchar i;
	uchar d;
	uchar old_id;
	ushort off;
	uchar x, y;
	uchar buf[20];
	
	for(i = 0;i < MAX_PROCS;i++) {
		if(console_tab[i] && console_tab[i]->visible_id == id) {
			if(console_tab[i] == active_con)
 				return;
			
			process_tab[console_pid].key = 0;
			memset(&process_tab[i].new_state, 0, sizeof(KeyState));
			process_tab[i].key = 0;
			memset(&process_tab[console_pid].new_state, 0, sizeof(KeyState));
			
			__asm
				push af
				ld a,#0xFF
				out (#1),a
				pop af
			__endasm;
				
			
			if(active_con != NULL) {
				if(transition_screen == NULL)
					transition_screen = system_alloc(768 * 2);
			
				old_id = active_con->visible_id;				
				
				if(id > old_id) {
					off = 0;
				}
				else {
					off = 12;
				}
				
				copy_half(transition_screen + off, system_screen);
				active_con = console_tab[i];
				transition_mode = 1;
				console_pid = i;
				draw_console(active_con);
				transition_mode = 0;
				copy_half(transition_screen + 12 - off, system_screen);
				transition_pos = 0;
				
				sprintf(buf, "Terminal %d", active_con->visible_id + 1);
				
				if(id > old_id) {
					for(d = 0;d <= 96;d += 8) {
						cls();
						copy_screen_half(system_screen, transition_screen, NULL, d);
						fill_rect(3, 5);
						draw_string_inverse(buf, 96 / 2 - strlen(buf) * 2, 64 / 2 - 2);
						cps();
						
					}
				}
				else {
					for(d = 96;d < 128;d -= 8) {
						cls();
						copy_screen_half(system_screen, transition_screen, NULL, d);
						fill_rect(3, 5);
						draw_string_inverse(buf, 96 / 2 - strlen(buf) * 2, 64 / 2 - 2);
						cps();
					}
				}
						
					
				
				//free(transition_screen);
			}
			
			active_con = console_tab[i];
			//draw_console(active_con);	
				
			console_pid = i;
			
			if(process_tab[console_pid].flags & PROC_WAIT_KEY) {
				process_tab[console_pid].flags = (process_tab[console_pid].flags & (~PROC_WAIT_KEY)) | PROC_RUN;
			}
				
				
			//cls();
			cls();
			draw_console(console_tab[i]);
			return;
		}
	}
}

// Clears the console (resets the position of the cursor and sets it to spaces)
void clear_console(Console *c) {
	uchar i;
	
	c->posx = 0;
	c->posy = 0;
	memset(c->data, ' ', CONSOLE_SIZE);
	
	for(i = 0; i < CONSOLE_HEIGHT;i++) {
		c->data[CONSOLE_WIDTH * i + CONSOLE_WIDTH - 1] = 0;
	}
}

// Initializes the console for the given process
// Returns the address of the console if successful, NULL otherwise
Console *init_console(uchar pid) {
	Console *c;
	uchar i;
	
	if(!process_valid(pid))
		return NULL;
	
	c = system_alloc(sizeof(Console));//malloc_for_pid(pid, sizeof(Console));
	
	clear_console(c);
	
	c->visible_id = 255;
	c->flags = 0;
	console_tab[pid] = c;
	
	return c;
}

// Creates a new console for a process and attemps to map it to a window
// (0 - 4). Note that the creation of the console can succeed, but mapping
// it to a window can fail if all of the windows are taken,
Console *init_visible_console(uchar pid, uchar parent_pid) {
	Console *c = NULL;
	uchar found[5];
	uchar i;
	
	
	for(i = 0;i < 5;i++)
		found[i] = 0;
	
	if(parent_pid == 255) {
		c = init_console(pid);
		c->visible_id = 255;
		
		for(i = 0;i < MAX_PROCS;i++) {
			if(console_tab[i] && console_tab[i]->visible_id != 255) {
				found[console_tab[i]->visible_id] = 1;
			}
		}
		
		for(i = 0;i < 5;i++) {
			if(!found[i]) {
				c->visible_id = i;
				return c;
			}
		}
	}
	
	return c;
}

// The amount of space it takes 
#define LINE_SIZE (96 * 7 / 8)

// Prints a character to the given console
// Special characters:
// 		\1 - reset the cursor to the beginning of the current line
//		\2 - set the cursor to the end of the current line
//		\3 - advance the cursor one character (without printing)
//		\4 - clear the current line and reset cursor to the beginning of the line
//		\5 - move the cursor to the previous line
//		\6 - print a newline without redrawing the console
//		\7 - forces a redraw of the current line
//		\8 - clears the console
//		\b - move the cursor back one character
//		\r - force redraw of the console
//		\n - print a newline and force redraw of the console
void console_printc(Console *c, uchar ch) {
	uchar i;
	
	DI();
	if(ch == '\n' || ch == '\6') {
		if(++c->posy == CONSOLE_HEIGHT) {
			DI();
			c->posy--;
			memmove(c->data, c->data + CONSOLE_WIDTH, CONSOLE_SIZE - CONSOLE_WIDTH);
		
			for(i = 0;i < CONSOLE_WIDTH - 1;i++) {
				c->data[(CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH + i] = ' ';
			}
			
			if(console_pid == process_id) {
				memmove(system_screen, system_screen + LINE_SIZE, 768 - LINE_SIZE * 2);
				memset(system_screen + 768 - LINE_SIZE * 2, 0, LINE_SIZE);
			}
		}
		
		if(ch == '\n' && console_pid == process_id) {
			draw_string(&c->data[(c->posy - 1) * CONSOLE_WIDTH], 0, (c->posy - 1) * 7);
			cps();
			//draw_console(c);
		}
		
		c->posx = 0;
		
		//draw_console(c);
	}
	else if(ch == '\r') {
		if(console_pid == process_id)
			draw_console(c);
	}
	else if(ch == '\1') {
		c->posx = 0;
	}
	else if(ch == '\2') {
		c->posx = CONSOLE_WIDTH - 2;
	}
	else if(ch == '\3') {
		c->posx++;
	}
	else if(ch == '\4') {
		for(i = 0;i < CONSOLE_WIDTH - 1;i++) {
			c->data[c->posy * CONSOLE_WIDTH + i] = ' ';
		}
		
		c->posx = 0;
	}
	else if(ch == '\5') {
		if(c->posy-- == 0)
			c->posy = 0;
		
		c->posx = 0;
	}
	else if(ch == '\7') {
		if(console_pid == process_id) {
			memset(system_screen + (ushort)c->posy * 12 * 7, 0, LINE_SIZE);
			draw_string(&c->data[c->posy * CONSOLE_WIDTH], 0, c->posy * 7);
			cps();
		}
	}
	else if(ch == '\b') {
		if(c->posx-- == 0) {
			c->posx = 0;
			
			if(c->posy != 0)
				c->posy--;
		}
	}
	else {
		if(c->posx == CONSOLE_WIDTH - 1) {
			console_printc(c, '\n');
		}
		
		c->data[c->posy * CONSOLE_WIDTH + c->posx++] = ch;
	}
	
	if(allow_interrupts)
		EI();
}

// Redraws a console
// Note: this should only be called if the console is active one (the one in the
// current window)
void draw_console(Console *c) {
	uchar i;
	uchar buf[25];
	
	DI();
	
	cls();
	
	for(i = 0;i < CONSOLE_HEIGHT;i++) {
		draw_string(&c->data[i * CONSOLE_WIDTH], 0, i * 7);
	}
	
	sprintf(buf, "CatOS v1.0 - %s", process_tab[console_pid].name);

	draw_string(buf, 0, 63 - 5);
	
	for(i = 0;i < LINE_SIZE;i++) {
		system_screen[768 - LINE_SIZE + i] = ~system_screen[768 - LINE_SIZE + i];
	}
	
	if(transition_mode == 0)
		cps();
	
	if(allow_interrupts) {
		EI();
	}
}

// SDCC's callback routine to print a character from printf
void putchar(char tx_data) {
	if(tx_data != 0) {
		if (current_con != NULL) {
			console_printc(current_con, tx_data);
		}
	}
}

// Initialize all of the consoles
void init_consoles() {
	uchar i;
	
	for(i = 0;i < 16;i++) {
		console_tab[i] = NULL;
	}
	
	transition_screen = NULL;
}

// Like the gets() function in the C standard library. Has the user input a string
// into buf. Returns when the user presses enter.
void get_input(uchar *buf) {
	uchar key;
	uchar pos = 0;
	
	printf("_\7");

	do {
		key = wait_key();	
		
		if(key == KEY_DEL) {
			if(pos != 0) {
				DI();
				printf("\b\b_ \b\7");
				
				if(allow_interrupts)
					EI();
				
				buf[--pos] = 0;
			}
		}
		else if(key == KEY_ENTER) {
			break;
		}
		else {
			buf[pos++] = key;
			buf[pos] = 0;
			printf("\b%c_\7", key);
		}
		
	} while(1);
	
	current_con->flags = 0;
	printf("\b \n");
}
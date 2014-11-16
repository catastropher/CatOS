//unsigned char screen[768];

#include <string.h>
#include <stdio.h>

#include "CatOS.h"

SYSCALL void cls() {
	memset(system_screen, 0, 768);
}

void interrupt_handler();
void unlock_flash();
void lock_flash();


SYSCALL void set_ram_page(uchar page) __naked {
	page;
	
	__asm
		push hl
		push af
		ld hl,#6
		add hl,sp
		
		ld a,(hl)
		;and a,#6
		
		;ld a,#2
		;or a,#12
		out (#5),a
		
		pop af
		pop hl
		
		ret
		
	__endasm;
}

SYSCALL void set_flash_page(uchar page) __naked {
	page;
	
	__asm
		push hl
		push af
		ld hl,#6
		add hl,sp
		
		ld a,(hl)
		out (#6),a
		
		pop af
		pop hl
		
		ret
		
	__endasm;
}

SYSCALL uchar get_flash_page() __naked {
	__asm
		push af
		in a,(#6)
		ld l,a
		pop af
		ret
	__endasm;
}

SYSCALL uchar get_ram_page() __naked {
	__asm
		push af
		in a,(#5)
		ld l,a
		pop af
		ret
	__endasm;
}

SYSCALL ushort clear_sector(uchar sector) __naked {
	__asm
		push bc
		push de
		push ix
	
		ld hl,#8
		add hl,sp
		ld a,(hl)
		call clear_sector
		
		pop ix
		pop de
		pop bc
		ret
	__endasm;
		
}

ushort flash_write_block(void *data, uchar page, uchar block, uchar block_size);

ushort test() {
	return 5;
}

uchar get_calc_status() {
	__asm
		push af
		in a,(#2)
		ld l,a
		pop af
		ret
	__endasm;
}



/*
void putchar (char tx_data)
{
// write to registers to tx the data.  TX buffering, etc

}
*/

void wait(ushort ticks);

extern unsigned long int_counter;

//uchar system_screen_buffer[768];

void system_process() {
	ushort i = 0;
	
	EI();
	
	EI();
	
	while(1) {
		printf("%d\n", i++);
		//wait(100);
		EI();
	}
	
	while(1) ;
}

void system_process3() {
	long sum = 0;
	long i;
	char buf[20];
	
	printf("Calculating sum...\n");

	for(i = 0;i < 100000;i++) {
		sum += i;
	}
	
	printf("Sum: %ld\n", sum);
	
	while(1) {
		get_input(buf);
	}
}
	
extern long int_counter2;

void system_process2() {
	uchar str[] = "hello\n";
	uchar pos = 0;
	unsigned long start;
	
	//allow_interrupts = 0;
	//current_con = console_tab[0];
	//process_id = 1;
	
	//printf("World\nTHere\n");
	
	//console_printc(console_tab[0], '\n');

	
	allow_interrupts = 1;
	
	__asm
		ei
	__endasm;
	
	start = int_counter2;
	
	while(1) {
		EI();
		if(int_counter - start > 10) {
			printf("%c\r", str[pos]);
			pos = (pos + 1) % (sizeof(str) - 1);
			start = int_counter;
		}
		else {
			force_context_switch();
		}
	}
}

ushort parse_num(uchar **buf2, uchar *flag) {
	ushort a = 0;
	uchar *buf = *buf2;
	
	if(!(buf[0] >= '0' && buf[0] <= '9')) {
		flag[0] = 0;
		return 0;
	}
	
	while(buf[0] >= '0' && buf[0] <= '9') {
		a = a * 10 + buf[0] - '0';
		buf++;
	}
	
	flag[0] = 1;
	buf2[0] = buf;
	
	return a;
}



void math_process() {
	short res;
	uchar buf[50];
	uchar *bufp = buf;
	uchar flag;
	short a;
	uchar op;
	
	
	while(1) {
		printf("Expr: ");
		get_input(buf);
		bufp = buf;
		
		res = parse_num(&bufp, &flag);
		
		if(!flag) {
			printf("Error: bad expr\n");
			continue;
		}
		
		while(*bufp != 0) {
			if(*bufp == '+' || *bufp == '-' || *bufp == '/' || *bufp == '*') {
				op = *bufp;
				bufp++;
			}
			else {
				printf("Error: bad expr\n");
				break;
			}
			
			a = parse_num(&bufp, &flag);
			
			if(!flag) {
				printf("Error: bad expr\n");
				break;
			}
			
			if(op == '+')
				res += a;
			else if(op == '-')
				res -= a;
			else if(op == '*')
				res *= a;
			else if(op == '/') {
				if(a != 0) {
					res /= a;
				}
				else {
					printf("Error: div by 0\n");
					break;
				}
			}
		}
		
		printf("%d\n", res);
	}
}

void game_process() {
	uchar board[3][3];
	uchar i, d;
	uchar posx = 0, posy = 0;
	uchar under_cursor;
	uchar key;
	uchar piece = 'X';
	
	for(i = 0;i < 3;i++)
		for(d = 0;d < 3;d++)
			board[i][d] = ' ';
	
	do {
		under_cursor = board[posy][posx];
		
		board[posy][posx] = '*';
		
		clear_console(current_con);
		
		printf("%c|%c|%c\6", board[0][0], board[0][1], board[0][2]);
		printf("-----\6");
		printf("%c|%c|%c\6", board[1][0], board[1][1], board[1][2]);
		printf("-----\6");
		printf("%c|%c|%c\6\r", board[2][0], board[2][1], board[2][2]);
	
		key = wait_key();
		
		board[posy][posx] = under_cursor;
		
		if(key == KEY_LEFT) {
			if(posx == 0)
				posx = 2;
			else
				posx--;
		}
		else if(key == KEY_DOWN) {
			if(posy == 2)
				posy = 0;
			else
				posy++;
		}
		else if(key == KEY_UP) {
			if(posy == 0)
				posy = 2;
			else
				posy--;
		}
		else if(key == KEY_RIGHT) {
			if(posx == 2)
				posx = 0;
			else
				posx++;
		}
		else if(key == KEY_ENTER) {
			if(board[posy][posx] == ' ') {
				board[posy][posx] = piece;
				
				if(piece == 'X')
					piece = 'O';
				else
					piece = 'X';
			}
		}
				
				
	} while(1);
}
	
	
void system_process4() {
	uchar buf[32];
	uchar pid = 0;
	uchar i;
	
	printf("CatOS console\n\n");
	
	do {
		printf("$\r");
		get_input(buf);
		EI();
		
		if(strcmp("mem", buf) == 0) {
			printf("Free RAM: %ld\n", get_free_mem());
			printf("Free kernal RAM: %d\n", (ushort)get_free_blocks(1) * 128);
		}
		else if(strcmp("cmd", buf) == 0) {
			pid = start_program(system_process4, "console", 255);
		}
		else if(strcmp("fs", buf) == 0) {
			fs_test();
		}
		else if(strcmp("hello", buf) == 0) {
			pid = start_program(system_process2, "hello", 255);	
		}
		else if(strcmp("count", buf) == 0) {
			pid = start_program(system_process, "count", 255);
		}
		else if(strcmp("math", buf) == 0) {
			pid = start_program(math_process, "math", 255);
		}
		else if(strcmp("game", buf) == 0) {
			pid = start_program(game_process, "game", 255);
		}
		else if(strcmp("proc", buf) == 0) {
			for(i = 0;i < MAX_PROCS;i++) {
				if(process_tab[i].id != 255) {
					printf("*%d: %s (%s)", i, process_tab[i].name, (process_tab[i].flags & PROC_RUN) ? "run" : "sleep");
					
					printf("%d\n", (console_tab[i] != NULL) ? console_tab[i]->visible_id : 9);
					
				}
			}
		}
		else {
			printf("Unknown command\n");
		}
		
		if(pid != 0) {
			printf("Forked process\n");
			printf("PID: %d\n", pid);
			printf("RAM page: %d\n", process_tab[pid].ram_page);
			pid = 0;
		}
	} while(1);
}

void copy_screen_half(uchar *dest, uchar *src_left, uchar *src_right, uchar x_pos);

void copy_half(uchar *dest, uchar *src);

uchar system_screen_full[768];

void sys_process() {
	//boot_os();
	start_process(system_process4, "console");
	init_visible_console(1, 255);
	switch_console(0);
	
	EI();
	allow_interrupts = 1;
	
	
	while(1) {
		//force_context_switch();
	}
}
			
			
					
		


void main() {
	uchar buf[16];
	uchar *other_screen;
	uchar i;
	uchar *big_screen;
	volatile long ii;
	uchar key;
	
	__asm
		di
		im 1
	__endasm;
	
	system_screen = system_screen_full;
	
	allow_interrupts = 0;
	
	init_memory();
	init_consoles();
	init_multitask();
	process_id = 0;
	
	//big_screen = malloc(768 * 2);
	
	/*if(big_screen == NULL) {
		__asm
			di
			halt
		__endasm;
	}*/
	//console_tab[0] = malloc(sizeof(Console));
	//sprintf(buf, "%u, %u", system_screen, console_tab[0]);
	
	
	
	/*cls();
	draw_string("Hello there world!", 0, 0);
	draw_string("More text", 0, 7);
	draw_string("Even more text is here!", 0, 14);
	memcpy(other_screen, system_screen, 768);
	cps();*/
	
	DI();
	
	cls();
	//copy_half(big_screen, other_screen);
	//copy_half(big_screen + 12, other_screen);
	
	//system_screen = big_screen;
	
	//cps();
	
	//while(1) ;
	/*
	for(i = 0;i < 96;i += 1) {
		cls();
		copy_screen_half(system_screen, big_screen, NULL, i);
		cps();
		
		//for(ii = 0;ii < 1000;ii++) ;
	}
	*/
	
	
	//while(1) ;
	
	/*start_process(system_process4);
	memset(&current_process->new_state, 0, sizeof(KeyState));
	init_visible_console(0, 255);
	switch_console(0);
	
	allow_interrupts = 0;
	active_con = console_tab[0];
	current_con = console_tab[0];
	
	printf("Hello, World!\n");
	
	current_process = &process_tab[0];*/
	
	
#if 0
	do {
		memcpy(&current_process->old_state, &current_process->new_state, sizeof(KeyState));
		read_keypad(current_process->new_state.key_map);
		
		/*for(i = 0;i < 8;i++) {
			printf("%d ", current_process->new_state.key_map[i]);
		}
		
		printf("\n");*/
		
		key = compare_key_matrix(&current_process->old_state, &current_process->new_state);
		
		if(key != 0) {
			printf("%c\r", key);
		}
	} while(1);
#endif
	
	
	
	
	
	
	
	
	
	
	start_process(sys_process, "system");
	
	
	//switch_console(0);
	
	//printf("PID: %d\n", pid);
	
	//console_pid = 0;
	//current_con = console_tab[0];
	active_con = NULL;//console_tab[0];
	
	/*__asm
		di
		halt
	__endasm;*/
	
	DI();
	allow_interrupts = 1;
	begin_run_process();
	
	/*__asm
		di
		di
		di
		di
		halt
	__endasm;*/
	
	while(1) ;
}





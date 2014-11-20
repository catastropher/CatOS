#ifndef CONSOLE_H

#define CONSOLE_WIDTH 24
#define CONSOLE_HEIGHT 8
#define CONSOLE_SIZE CONSOLE_WIDTH*CONSOLE_HEIGHT

#define CONSOLE_VISIBLE 1

#define MAX_PROCS 16

typedef struct {
	uchar data[CONSOLE_SIZE];
	uchar posx, posy;
	uchar flags;
	uchar flags2;
	uchar flags3;
	uchar visible_id;
} Console;

Console *init_console(uchar pid);
Console *init_visible_console(uchar pid, uchar parent_pid);
void switch_console(uchar id);
void console_printc(Console *c, uchar ch);
void draw_console(Console *c);
void init_consoles();
void console_printc(Console *c, uchar ch);
void get_input(uchar *buf);
void clear_console(Console *c);

extern Console *active_con;
extern Console *current_con;
extern uchar console_pid;
extern Console *console_tab[MAX_PROCS];
extern uchar allow_interrupts;

// The number of bytes a single printed line of text takes
#define LINE_SIZE (96 * 7 / 8)

#endif
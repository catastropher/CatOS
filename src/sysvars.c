#include "CatOS.h"

// Pointer to the virtual screen (an array of 768 bytes which holds a
// copy of the screen in memory aka a double buffer). Call cps() to
// upload the screen to the LCD
volatile unsigned char *system_screen;

// 
ProcessEntry *current_process;

ProcessEntry process_tab[16];

// Memory allocation table for malloc. Keeps track of which blocks in memory
// are currently in use.

#ifndef SYSVARS_H
#define SYSVARS_H

#include "graphics.h"

// Pointer to the virtual screen (an array of 768 bytes which holds a
// copy of the screen in memory aka a double buffer). Call cps() to
// upload the screen to the LCD
extern volatile unsigned char *system_screen;

// The system font
const extern Character *system_font;

extern ProcessEntry *current_process;

extern ProcessEntry process_tab[16];

#endif

#ifndef CATOS_H
#define CATOS_H

#define SYSCALL

//=============================================================================

// It gets annoying typing these out all the time, so they've been
// shortened to uchar and ushort.
typedef unsigned char uchar;
typedef unsigned short ushort;

//=============================================================================

#define ASMSYMBOL(_sym, _type) ((_type)(&_sym))
#define NULL (void *)0

//=============================================================================


#include "keypad.h"
#include "multitask.h"
#include "sysvars.h"
#include "graphics.h"
#include "memory.h"
#include "console.h"
#include "filesystem.h"

#define DI() __asm\
	di\
	__endasm;
	
#define EI() __asm\
	ei\
	__endasm;
	
#define HALT() __asm\
	di\
	halt\
	__endasm;

#endif
	
#define TRUE 1
#define FALSE 0

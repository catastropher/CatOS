#ifndef CATOS_H
#define CATOS_H

//=============================================================================

// It gets annoying typing these out all the time, so they've been
// shortened to uchar and ushort.
typedef unsigned char uchar;
typedef unsigned short ushort;

//=============================================================================

#define ASMSYMBOL(_sym, _type) ((_type)(&_sym))
#define NULL (void *)0

//=============================================================================


#include "multitask.h"
#include "sysvars.h"
#include "graphics.h"
#include "memory.h"

#endif

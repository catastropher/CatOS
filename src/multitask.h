#ifndef MULTITASK_H
#define MULTITASK_H

#include "CatOS.h"

typedef struct {
	uchar id;
	uchar flags;
	uchar flash_page;
	
	ushort af;
	ushort bc;
	ushort de;
	ushort ix;
	ushort iy;
	ushort pc;
	ushort sp;
	
	uchar name[8];
	uchar parent_id;
	
	uchar *screen;
} ProcessEntry;

void init_multitask();

#endif

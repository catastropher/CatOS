#include "CatOS.h"

#include <stdio.h>

// Queries the keypad hardware to determine which keys in the key matrix
// are being pressed. This is read into the 8-byte buffer pointed to by
// dest
void read_keypad(uchar *dest) __naked {
	__asm
	push bc
	push de
	push hl
	push ix
	
	ld ix,#0
	add ix,sp
	ld l,10(ix)
	ld h,11(ix)
	
	ld a,#0xBF
	out (#1),a
	nop
	nop
	in a,(#1)
	cpl
	ld (hl),a
	inc hl

	ld a,#0xDF
	out (#1),a
	nop
	nop
	in a,(#1)
	cpl
	ld (hl),a
	inc hl
	
	ld a,#0xEF
	out (#1),a
	nop
	nop
	in a,(#1)
	cpl
	ld (hl),a
	inc hl
	
	ld a,#0xF7
	out (#1),a
	nop
	nop
	in a,(#1)
	cpl
	ld (hl),a
	inc hl
	
	ld a,#0xFB
	out (#1),a
	nop
	nop
	in a,(#1)
	cpl
	ld (hl),a
	inc hl
	
	ld a,#0xFD
	out (#1),a
	nop
	nop
	in a,(#1)
	cpl
	ld (hl),a
	inc hl
	
	ld a,#0xFE
	out (#1),a
	nop
	nop
	in a,(#1)
	cpl
	ld (hl),a
	inc hl
	
	pop ix
	pop hl
	pop de
	pop bc
	ret
	__endasm;
}

// Attemps to determine the ASCII representation of indicated key
// (indicated by the position in the key matrix). This takes into account
// whether 2nd or alpha has been pressed) Note that 2nd is shift and
// alpha is used for symbols
uchar get_key_code(uchar flags, uchar x, uchar y) {
	uchar key[] = "xsnida";
	uchar add = (flags == 1 ? ('A' - 'a') : 0);
	uchar key_num[] = "147";
	
	if(flags == 0 || flags == 1) {
		if(x == 0 && y == 2)
			return ' ';
		else if(x > 0 && x < 7) {
			if(x != 6 || (x == 6 && y < 4))
				return key[x - 1] + y - 1 + add;
		}
	}
	else {
		if(x == 4 && y == 3)
			return '(';
		else if(x == 4 && y == 4)
			return ')';
		else if(y == 2 && x == 0)
			return '0';
		else if(x == 4 && y == 5)
			return '/';
		else if(y == 5 && x == 1)
			return '+';
		else if(y == 5 && x == 2)
			return '-';
		else if(y == 5 && x == 3)
			return '*';
		else if((y > 1 && y < 5) && (x > 0 && x < 4))
			return key_num[x - 1] + y - 2;
			
	}
	
	return 0;
}

// Macro for low-level key matrix checking
#define KEY(_s, _x, _y) ((_s->key_map[_y] & (1 << _x)) != 0)

// Compares two key matricies to determine which key has been pressed. A
// key is considered pressed if it was pressed in the old key matrix but
// isn't pressed in the new one.
uchar compare_key_matrix(KeyState *old, KeyState *new) {
	uchar x, y;
	uchar o;
	uchar flags = new->flags;
	
	DI();
	
	if(KEY(old, 5, 0) && !KEY(new, 5, 0)) {
		if(new->flags & 1)
			new->flags = 0;
		else
			new->flags = 1;
		
		return 0;
	}
	else if(KEY(old, 7, 1) && !KEY(new, 7, 1)) {
		if(new->flags & 2)
			new->flags = 0;
		else
			new->flags = 2;
		
		return 0;
	}
	else if(KEY(old, 1, 6) && !KEY(new, 1, 6)) {
		new->flags = 0;
		
		return KEY_LEFT;
	}
	else if(KEY(old, 6, 0) && !KEY(new, 6, 0)) {
		new->flags = 0;
		
		return KEY_MODE;
	}
	else if(KEY(old, 0, 6) && !KEY(new, 0, 6)) {
		new->flags = 0;
		
		return KEY_DOWN;
	}
	else if(KEY(old, 2, 6) && !KEY(new, 2, 6)) {
		new->flags = 0;
		
		return KEY_RIGHT;
	}
	else if(KEY(old, 3, 6) && !KEY(new, 3, 6)) {
		new->flags = 0;
		
		return KEY_UP;
	}
	else if(KEY(old, 7, 0) && !KEY(new, 7, 0)) {
		new->flags = 0;
		return KEY_DEL;
	}
	else if(KEY(old, 0, 5) && !KEY(new, 0, 5)) {
		new->flags = 0;
		return KEY_ENTER;
	}
	
	for(y = 1;y < 7;y++) {
		o = old->key_map[y] & (~new->key_map[y]);
		
		if(o) {
			for(x = 0;x < 8;x++) {
				if(o & 1) {
					new->flags = 0;
					
					if(allow_interrupts)
						EI();
					
					return get_key_code(flags, x, y);
				}
				
				o >>= 1;
			}
		}
	}
	
	if(allow_interrupts)
		EI();
	
	return 0;
}

// Waits for a key to be pressed and returns the key that was pressed. Note: this
// will put the process to sleep if no key was pressed or if it's not the active console
uchar wait_key() {
	uchar key;
	
	do {
		while(console_pid != process_id) {
			current_process->flags = (current_process->flags & (~1)) | PROC_WAIT_KEY;
			force_context_switch();
			//printf("Wait\n");
		}
		
		key = current_process->key;
		
		if(key == 0) {
			force_context_switch();
		}
		
	} while(key == 0);
	
	current_process->key = 0;
	
	return key;
}


















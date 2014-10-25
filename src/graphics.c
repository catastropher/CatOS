#include "CatOS.h"

// Draws a character on the screen
void draw_char(uchar c, uchar x, uchar y) {
	draw_sprite(ASMSYMBOL(system_font, Character *)[(unsigned short)c].sprite, 5, x, y);
}

// Draws a string on the screen
// Note: this routine will NOT stop drawing if the characters are off the screen
// In other words, it will crash!
void draw_string(uchar *str, uchar x, uchar y) {
	while(*str != 0) {
		draw_char(*str, x, y);
		str++;
		x += 4;
	}
}











// Copies the virtual screen in memory to the LCD
// The screen is pointed to by the global variable system_screen
void cps() __naked {
	__asm
	push bc
	push de
	push hl
;-----> Copy the gbuf to the screen (fast)
;Input: nothing
;Output: graph buffer is copied to the screen
fastCopy:
 di
 ld a,#0x80
 out (#0x10),a
 ld hl,(_system_screen)
 ld de,#-12-(-(12*64)+1)
 add hl,de
 ld a,#0x20
 ld c,a
 inc hl
 dec hl
fastCopyAgain:
 ld b,#64
 inc c
 ld de,#-767
 out (#0x10),a
 add hl,de
 ld de,#10
fastCopyLoop:
 add hl,de
 inc hl
 inc hl
 inc de
 ld a,(hl)
 out (#0x11),a
 dec de
 djnz fastCopyLoop
 ld a,c
 cp #0x2C
 jr nz,fastCopyAgain
 
 pop hl
 pop de
 pop bc
 ret
	__endasm;
}

// Draws an 8xH sprite on the screen, where H is the height of the sprite
// Note: the sprite is not clipped (i.e. if it is partially off screen, it
// will corrupt other things in memory!)
void draw_sprite(uchar *sprite, uchar height, uchar x, uchar y) __naked {

	sprite; height; x; y;

__asm
	push bc
	push de
	push hl
	push ix
	
	ld ix,#0
	add ix,sp
	ld l,10(ix)
	ld h,11(ix)
	ld b,12(ix)
	ld a,13(ix)
	ld e,14(ix)
	
	push hl
	pop ix

;-----> Draw a sprite
; input:	a=x	e=y
;		b=height of sprite
;		ix holds pointer
; output:	
; destroys de,hl
putSprite8xb:
	ld	h,#0
	ld	d,h
	sla e
	sla e
	ld	l,e
	add	hl,de
	add	hl,de
	ld e,a
	srl	e
	srl	e
	srl	e
	add	hl,de
	ld	de,(_system_screen)
	add	hl,de
	and #7
	ld c,a
putSpriteLoop1:
	ld	d,0(ix)
	ld	e,#0
	ld a,c
	or a
	jr	z,putSpriteSkip1
putSpriteLoop2:
	srl	d
	rr	e
	dec	a
	jr	nz,putSpriteLoop2
putSpriteSkip1:
	ld	a,(hl)
	or	d
	ld	(hl),a
	inc	hl
	ld	a,(hl)
	or	e
	ld	(hl),a
	ld	de,#0x0B
	add	hl,de
	inc	ix
	djnz	putSpriteLoop1
	
	pop ix
	pop hl
	pop de
	pop bc
	
	ret
__endasm;
}

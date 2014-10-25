#ifndef GRAPHICS_H
#define GRAPHICS_H

typedef struct {
	unsigned char sprite[5];
} Character;

//=============================================================================

// Draws a character on the screen
void draw_char(uchar c, uchar x, uchar y);

// Draws a string on the screen
// Note: this routine will NOT stop drawing if the characters are off the screen
// In other words, it will crash!
void draw_string(uchar *str, uchar x, uchar y);

// Copies the virtual screen in memory to the LCD
// The screen is pointed to by the global variable system_screen
void cps();

// Draws an 8xH sprite on the screen, where H is the height of the sprite
// Note: the sprite is not clipped (i.e. if it is partially off screen, it
// will corrupt other things in memory!)
void draw_sprite(uchar *sprite, uchar height, uchar x, uchar y);

#endif

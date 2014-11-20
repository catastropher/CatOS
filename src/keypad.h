#ifndef KEYPAD_H
#define KEYPAD_H

typedef struct {
	uchar flags;
	uchar key_map[8];
} KeyState;

void read_keypad(uchar *dest);
uchar get_key_code(uchar flags, uchar x, uchar y);
uchar compare_key_matrix(KeyState *old, KeyState *new);
uchar wait_key();

#define KEY_DEL 128
#define KEY_ENTER 129
#define KEY_DOWN 130
#define KEY_UP 131
#define KEY_LEFT 132
#define KEY_RIGHT 133
#define KEY_MODE 134

#endif
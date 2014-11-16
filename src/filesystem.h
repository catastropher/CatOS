#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define TYPE_DIR 1
#define TYPE_FILE 0

typedef struct {
	void *next;
	uchar flags;
	uchar name[9 + 3];
} FileData;

typedef struct {
	FileData data;
	uchar pos;
} File;

typedef struct {
	void *next;
} FileBlock;
	

#define MAX_DIR_ITEMS 20

typedef struct Directory {
	FileData data;
	FileData *items[MAX_DIR_ITEMS];
} Directory;

typedef struct {
	File *file;
} FileDescriptor;

void *fs_get_pointer(uchar *name, uchar type);
uchar fs_directory_add_item(Directory *dir, void *item);
Directory *fs_create_directory(uchar *name);
void fs_print_dir(Directory *dir);
void fs_test();

extern Directory *root_dir;

#endif
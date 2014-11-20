#include <string.h>
#include <stdio.h>

#include "CatOS.h"


//Directory *root_dir;

// type = 0 to get a file
// type = 1 to get a directory
#if 0
void *fs_get_pointer(uchar *name, uchar type) {
	uchar temp_name[12];
	uchar pos = 0;
	uchar search_type;
	Directory *dir = root_dir;
	uchar i;
	uchar found;
	FileData *f = dir->items[0];
	
	if(*name == '/')
		++name;
	
	while(*name) {
		while(*name && *name != '/') {
			temp_name[pos++] = *name;
			name++;
		}
		
		temp_name[pos] = 0;
		
		if(*name == '/')
			name++;
		
		if(!*name || type == 1)
			search_type = 1;
		else
			search_type = 0;
		
		found = 0;
		for(i = 0;i < MAX_DIR_ITEMS;i++) {
			if(dir->items[i] && (dir->items[i]->flags & 1) == search_type &&
				strcmp(dir->items[i]->name, temp_name) == 0) {
					found = 1;
					dir = (Directory *)dir->items[i];
			}
		}
		
		if(!found)
			return NULL;
		
	}
	
	return dir;
}

uchar fs_directory_add_item(Directory *dir, void *item) {
	uchar i;
	uchar pos = 255;
	FileData *f = (FileData *)item;
	
	for(i = 0;i < MAX_DIR_ITEMS;i++) {
		if(dir->items[i] && (f->flags & 1) == TYPE_DIR && 
			strcmp(dir->items[i]->name, f->name) == 0) {
			
			return FALSE;
		}
		else if(dir->items[i] == NULL) {
			pos = i;
		}
	}
	
	if(pos == 255)
		return FALSE;
	
	dir->items[pos] = f;
	
	return TRUE;
}

Directory *fs_create_directory(uchar *name) {
	Directory *new_dir = fs_alloc(128);
	
	if(!new_dir)
		return NULL;
	
	new_dir->data.next = NULL;
	new_dir->data.flags = TYPE_DIR;
	strcpy(new_dir->data.name, name);
	
	return new_dir;
}

void fs_print_dir(Directory *dir) {
	uchar i;
	volatile uchar x;
	
	printf("===%s===\n\n", dir->data.name);
	
	for(i = 0;i < MAX_DIR_ITEMS;i++) {
		if(dir->items[i]) {
			x = 5;
			if((dir->items[i]->flags & 1) == TYPE_DIR) {
				printf("DIR: %s\n\n", dir->items[i]->name);
			}
			else {
				printf("FILE: %s\n\n", dir->items[i]->name);
			}
			
			//printf("%s: %s\n", (dir->items[i]->flags & 1) == TYPE_DIR ? "DIR" : "FILE", dir->items[i]->name);
			
		}
	}
}

void fs_test() {
	Directory *dir = fs_create_directory("direct");
	Directory *d2 = fs_create_directory("newdir");
	
	fs_directory_add_item(dir, d2);
	
	fs_print_dir(dir);
}

#endif


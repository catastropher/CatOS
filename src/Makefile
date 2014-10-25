CC=sdcc
CFLAGS=--no-std-crt0 -mz80 --code-loc 0x0500 "-Wa -g" -c
LDFLAGS=--no-std-crt0 -mz80 --data-loc 0x8000 --code-loc 0x0500 "-Wl -b_CODE=0x100" -o output.ihx "-Wl -b_DATA=0x8000"
SOURCES=score.c measure.c
OBJECTS=$(SOURCES:.c=.rel)
EXECUTABLE=OS.hex

all: CatOS.rom

CatOS.rom: font.rel startup.rel CatOS.rel graphics.rel sysvars.rel filesystem.rel memory.rel multitask.rel
	cp ../tools/CatOS.rom .
	$(CC) $(LDFLAGS) startup.rel CatOS.rel graphics.rel font.rel sysvars.rel filesystem.rel memory.rel multitask.rel
	../tools/multihex 00 output.ihx > OS.hex
	../tools/fixhex OS.hex
	../tools/rompatch CatOS.rom OS.hex


CatOS.rel: CatOS.c
	$(CC) $(CFLAGS) CatOS.c
	
graphics.rel: graphics.c
	$(CC) $(CFLAGS) graphics.c
	
sysvars.rel: sysvars.c
	$(CC) $(CFLAGS) sysvars.c

filesystem.rel: filesystem.c
	$(CC) $(CFLAGS) filesystem.c

memory.rel: memory.c
	$(CC) $(CFLAGS) memory.c
	
multitask.rel: multitask.c
	$(CC) $(CFLAGS) multitask.c

startup.rel: startup.z80
	cp startup.z80 startup.asm
	sdasz80 -l -o startup.asm

	
font.rel: font.z80
	cp font.z80 font.asm
	sdasz80 -l -o font.asm
	
	
clean:
	rm -f *.asm
	rm -f *.rel
	rm -f *.ihx
	rm -f *.sym
	rm -f *.lst
	rm -f *.rom
	rm -f *.map
	rm -f *.lk
	rm -f *.hex
	rm -f *.mem
	rm -f *.noi
	rm -f *.rst

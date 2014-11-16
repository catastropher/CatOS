CC=sdcc
CFLAGS=--no-std-crt0 -std-c99 --opt-code-size --max-allocs-per-node 3000 -mz80 --code-loc 0x0500 "-Wa -g" -c
LDFLAGS=--no-std-crt0 -mz80 --data-loc 0x8000 --code-loc 0x0500 "-Wl -b_CODE=0x500" -o output.ihx "-Wl -b_DATA=0x8000"
SOURCES=score.c measure.c
OBJECTS=$(SOURCES:.c=.rel)
EXECUTABLE=OS.hex

all: CatOS.rom

CatOS.rom: font.rel startup.rel CatOS.rel graphics.rel sysvars.rel memory.rel multitask.rel page1C.rel console.rel keypad.rel filesystem.rel
	cp ../tools/CatOS.rom .
	$(CC) $(LDFLAGS) startup.rel CatOS.rel graphics.rel font.rel sysvars.rel filesystem.rel memory.rel multitask.rel console.rel keypad.rel
	$(CC) --no-std-crt0 -mz80 -o output_page1C.ihx page1C.rel
	
	../tools/multihex 00 output.ihx 1C output_page1C.ihx > OS.hex
	../tools/fixhex OS.hex
	../tools/packxxu -v 9.99 -h 255 OS.hex -t 83p -o CatOS.8xu
	../tools/rompatch CatOS.rom OS.hex

page1C.rel:
	cp page1C.z80 page1C.asm
	sdasz80 -g -l -o page1C.asm


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
	
console.rel: console.c
	$(CC) $(CFLAGS) console.c
	
keypad.rel: keypad.c
	$(CC) $(CFLAGS) keypad.c

startup.rel: startup.z80
	cp startup.z80 startup.asm
	sdasz80 -g -l -o startup.asm

	
font.rel: font.z80
	cp font.z80 font.asm
	sdasz80 -g -l -o font.asm
	
	
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
	rm -f *.8xu

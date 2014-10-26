/*
 * ROM Patcher
 *
 * Copyright 2003 Benjamin Moody
 *
 * This program is free software; you can redistribute and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111
 * USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * This program takes as input one or more hex files, with type 2 page
 * records, and "installs" them into an existing binary file.
 *
 * Following -a8, page numbers are subtracted from the given BASE.
 * Following -a7, they are added.  Following -s or -4, page numbers
 * between 10 and 1F are relocated appropriately.
 *
 */

const char* usage="usage: %s ROM-FILE [-a8 BASE | -a7 BASE | -s | -4 ]"
" [HEX-FILE] ...\n";

int main(int argc, char **argv)
{
  char c;
  int nbytes, rectype, dl, dh, check, a;
  int pagenum=0;
  long position;
  FILE *hexfile;
  FILE *romfile;
  int i, j;
  int bp=0;
  int semode=0;

  if (argc>1) {
    if(!(romfile=fopen(argv[1],"r+b"))) {
      perror(argv[1]);
      fprintf(stderr,"%s: unable to open ROM file %s\n",argv[0],argv[1]);
      return 1;
    }
  }
  else {
    fprintf(stderr,usage,argv[0]);
    return 1;
  }

  for (i=2; i<argc; i++) {
    if (0==strcmp(argv[i],"-")) {
      hexfile=stdin;
    }
    else if (0==strcmp(argv[i],"-a8")) { /* TI-83+ App */
      sscanf(argv[++i],"%x",&bp);
      bp=bp<0?bp:-bp;
      semode=0;
      continue;
    }
    else if (0==strcmp(argv[i],"-a7")) { /* TI-73 App */
      sscanf(argv[++i],"%x",&bp);
      bp=bp<0?-bp:bp;
      semode=0;
      continue;
    }
    else if (0==strcmp(argv[i],"-s")) {	/* Silver Ed. OS */
      semode=0x60;
      continue;
    }
    else if (0==strcmp(argv[i],"-4")) {	/* TI-84 OS */
      semode=0x20;
      continue;
    }
    else if (!(hexfile = fopen(argv[i],"r"))) {
      perror(argv[i]);
      fprintf(stderr,"%s: unable to open hex file %s\n",argv[0],argv[i]);
      return 1;
    }

    pagenum=bp<0?-bp:bp;

    while (!feof(hexfile) && !ferror(hexfile)) {
      do
	c=fgetc(hexfile);
      while (c=='\n'||c=='\r'||c=='\t'||c=='\f'||c==' ');

      if (!feof(hexfile) && !ferror(hexfile)) {

	if (c!=':') {
	  fprintf(stderr,"%s: hex file %s not in valid Intel format",
		  argv[0],argv[i]);
	  return 1;
	}
	
	fscanf(hexfile,"%2X%2X%2X%2X",&nbytes,&dh,&dl,&rectype);
	check=nbytes+dh+dl+rectype;
	
	if (0==rectype) {
	  position = (pagenum * 0x4000l) + (((dh*0x100)+dl) & 0x3fff);
	  if (fseek(romfile, position, SEEK_SET)) {
	    perror("fseeking on rom file");
	    return 1;
	  }
	}
	
	for (j=0;j<nbytes;j++) {
	  fscanf(hexfile,"%2X",&a);
	  check+=a;
	  if (0==rectype)
	    fputc(a,romfile);
	  else if (2==rectype) {
	    if (semode) {
	      if (a>0x0f && a<0x20)
		pagenum = a | semode;
	      else
		pagenum = a;
	    }
	    else if (bp<0)
	      pagenum=(-bp)-a;
	    else
	      pagenum=bp+a;
	  }
	}
	
	fscanf(hexfile,"%2X",&a);
	check+=a;
	if (check&0xff) {
	  fprintf(stderr,
		  "%s: invalid checksum %X (at %02X%02X, type %02X) in %s\n",
		  check,dh,dl,rectype,argv[i]);
	}
      }
    }
    fclose(hexfile);
  }

  return 0;
}

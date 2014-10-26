/*
 * Intel Hex Multiplexer
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
 * This program does nothing particularly complicated.  For each page,
 * it adds a type 2 record containing the page address, then copies
 * over the contents of the hex file, omitting any empty records
 * (which are presumably end records.)
 *
 */

const char* usage="usage: %s [PAGE HEX-FILE] ...\n";

int main(int argc, char **argv)
{
  int pagenum, i;
  FILE *hexfile;
  char buf[256];

  if (0==(argc%2)) {
    fprintf(stderr,usage,argv[0]);
    return 1;
  }

  for (i=1; i<argc-1; i+=2) {

    if (0==sscanf(argv[i],"%x",&pagenum)) {
      fprintf(stderr,usage,argv[0]);
      return 1;
    }

    if (pagenum>0x1d) {
      fprintf(stderr,"%s: page number %s out of bounds.\n",argv[0],argv[i]);
      return 1;
    }

    printf(":0200000200%02X%02X\n",pagenum,0xFC-pagenum);

    hexfile = fopen(argv[i+1],"r");
    if (!hexfile) {
      perror(argv[i+1]);
      fprintf(stderr,"%s: unable to open hex file %s\n",argv[0],argv[i+1]);
      return 1;
    }
    while (!feof(hexfile)) {
      fgets(buf,256,hexfile);
      if (0!=strncmp(buf,":00",3))
	fputs(buf,stdout);
    }
    fclose(hexfile);
  }

  puts(":00000001FF");	// End of OS record
  return 0;
}

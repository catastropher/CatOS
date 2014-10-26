/*
 * XXU Packer
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
#include <time.h>

/*
 * This program generates and outputs a standard "xxu" (that is to
 * say, either 73u or 8xu) OS upgrade file.
 *
 * Notes:
 *  - Will not validate (obviously)
 *  - Should work with TI-Connect (but be prepared to accept that it
 *    may not be)
 *  - If you're using Unix, make sure you convert your hex file to DOS
 *    format first
 *  - You should probably use 32 bytes per record
 *
 */

const char* usage="usage: %s [OPTIONS] [HEX-FILE]\n"
"where OPTIONS may include:\n"
"  -c ID        set calculator (link device) ID\n"
"  -q ID        set certificate ID\n"
"  -t {83p|73}  set target calculator (i.e. use presets for the above)\n"
"  -d DATE      set date stamp\n"
"  -v VER       set OS version\n"
"  -s SIZE      set program size (apparently unnecessary)\n"
"  -i SIZE      set `image size' (almost certainly unnecessary)\n"
"  -h VER       set maximum compatible hardware version\n"
"  -o FILE      set output file\n";

FILE *infile, *outfile;
int v_major=1, v_minor=0, calc=0x73, certid=4, hardware=0xff;
long ossize=0, imsize=0;
struct tm *tm;

#define BCD(x) ((((x)/10)*16)+((x)%10))
#define RECSIZE(n) (13+(2*(n)))

#define PUTRHEX(x) fprintf(outfile,"%02X",x); c+=x

void putrec(int n, int a, int t, unsigned char *data);
void cmdline(char opt, char *val, char **argv);

int main(int argc, char** argv)
{
  int mon, day, yearh, yearl;
  int i;
  unsigned char c;
  long len;
  time_t t;

  unsigned char hdr_data[] = { 
    // Size doesn't matter
    0x80,0x0f, 0x00,0x00,0x00,0x00,
    // Certificate ID = 4 for 83+, 2 for 73
    0x80,0x11, 0x00,
    // Version major (may want to set this really high so all OS's will accept)
    0x80,0x21, 0x00,
    // Version minor
    0x80,0x31, 0x00,
    // Hardware compatibility (may want to set this to FF)
    0x80,0xa1, 0x00,
    // Size doesn't matter here either
    0x80,0x7f, 0x00,0x00,0x00,0x00};

  int hdr_size = 6+3+3+3+3+6;

  /* It doesn't matter what we put here -- it won't validate anyway --
     and using this data at least eliminates any copyright claims :) */

  unsigned char sig_data[]={0x02,0x0d,0x40,

                            0x03,0x14,0x15,0x92, 0x65,0x35,0x89,0x79,
			    0x32,0x38,0x46,0x26, 0x43,0x38,0x32,0x79,
			    0x50,0x28,0x84,0x19, 0x71,0x69,0x39,0x93,
			    0x75,0x10,0x58,0x20, 0x97,0x49,0x44,0x59,
			    0x23,0x07,0x81,0x64, 0x06,0x28,0x62,0x08,
			    0x99,0x86,0x28,0x03, 0x48,0x25,0x34,0x21,
			    0x17,0x06,0x79,0x82, 0x14,0x80,0x86,0x51,
			    0x32,0x82,0x30,0x66, 0x47,0x09,0x38,0x44,

                            0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
                            0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
                            0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
                            0xff,0xff,0xff,0xff, 0xff};

  /* I have no idea what this means. */

  char *end_str="   -- CONVERT 2.6 --\r\n\032";

  infile=stdin;
  outfile=stdout;

  time(&t);
  tm=localtime(&t);

  for (i=1; i<argc; i++) {
    if (argv[i][0]=='-') {
      if (argv[i][1]==0)
	infile = stdin;
      else if (argv[i][2])
	cmdline(argv[i][1],argv[i]+2, argv);
      else {
	cmdline(argv[i][1],argv[i+1], argv);
	i++;
      }
    }
    else {
      if (!(infile=fopen(argv[i],"rb"))) {
	perror(argv[i]);
	fprintf(stderr,"%s: unable to open hex file %s\n",argv[0],argv[i]);
	return 1;
      }
    }
  }

  hdr_data[2] = (ossize>>24)&0xff;
  hdr_data[3] = (ossize>>16)&0xff;
  hdr_data[4] = (ossize>>8)&0xff;
  hdr_data[5] = (ossize)&0xff;

  hdr_data[8] = certid;
  hdr_data[11] = v_major;
  hdr_data[14] = v_minor;
  hdr_data[17] = hardware;

  hdr_data[20] = (imsize>>24)&0xff;
  hdr_data[21] = (imsize>>16)&0xff;
  hdr_data[22] = (imsize>>8)&0xff;
  hdr_data[23] = (imsize)&0xff;

  fseek(infile,0l,SEEK_END);
  len = ftell(infile);
  fseek(infile,0l,SEEK_SET);

  len += RECSIZE(hdr_size) + RECSIZE(0) + 3*RECSIZE(32) + RECSIZE(0) - 2
    + strlen(end_str);

  day=tm->tm_mday;
  mon=1+tm->tm_mon;
  yearh=19+((tm->tm_year)/100);
  yearl=(tm->tm_year)%100;

  fprintf(outfile,"**TIFL**");

  fputc(0,outfile);
  fputc(0,outfile);

  fprintf(outfile,"\001\210%c%c%c%c\010basecode",
	 BCD(mon), BCD(day), BCD(yearh), BCD(yearl));

  for (i=0;i<23;i++)
    fputc(0,outfile);		/* A whole bunch of zeroes. */

  fputc(calc,outfile);		/* Type of calculator */
  fputc(0x23,outfile);		/* "Data type" for OS upgrade */

  for (i=0;i<24;i++)
    fputc(0,outfile);		/* And even more zeroes. */

  fputc(len&0xff,outfile);	/* The length of the entire rest of the file */
  fputc((len>>8)&0xff,outfile);
  fputc((len>>16)&0xff,outfile);
  fputc((len>>24)&0xff,outfile);

  /* OS header */
  putrec(hdr_size, 0, 0, hdr_data);
  putrec(0, 0, 1, NULL);

  /* Actual OS data */
  while (!(feof(infile) || ferror(infile))) {
    c = fgetc(infile);
    if (c < 128)
      fputc(c,outfile);
  }

  /* And a dumb signature */
  putrec(32, 0, 0, sig_data);
  putrec(32, 32, 0, sig_data+32);
  putrec(32, 64, 0, sig_data+64);
  putrec(0, 0, -1, NULL);
  fputs(end_str, outfile);

  fclose(infile);
  fclose(outfile);

  return 0;
}


void putrec(int n, int a, int t, unsigned char *data)
{
  // Use t=-1 to get the final record without newline
  int i, c=0;
  fputc(':',outfile);

  PUTRHEX(n);
  PUTRHEX((a>>8)&0xff);
  PUTRHEX(a&0xff);
  PUTRHEX(t<0?-t:t);

  for (i=0;i<n;i++) {
    PUTRHEX(data[i]);
  }

  PUTRHEX(256-(c&0xff));

  if (t>=0)
    fprintf(outfile,"\r\n");
}


void cmdline(char opt, char *val, char **argv)
{
  float foo;
  int i;

  switch (opt) {
  case 'd':
    if (strchr(val,'/'))
      sscanf(val,"%d/%d/%d",&(tm->tm_mon),&(tm->tm_mday),&(tm->tm_year));
    else
      sscanf(val,"%d%d%d",&(tm->tm_mday),&(tm->tm_mon),&(tm->tm_year));
    if (tm->tm_year>1900)
      tm->tm_year-=1900;
    break;
  case 'v':
    sscanf(val,"%f",&foo);
    v_major = (int) foo;
    v_minor = (int) (foo*100)%100;
    break;
  case 't':
    if (0==strcmp(val,"73")) {
      calc = 0x74;
      certid = 2;
    }
    else if (0==strcmp(val,"83p")) {
      calc = 0x73;
      certid = 4;
    }
    else {
      fprintf(stderr,"%s: unknown calculator type %s\nTry '73' or '83p'.\n",
	      argv[0],val);
      exit(1);
    }
    break;
  case 'c':
    sscanf(val,"%x",&calc);
    break;
  case 'q':
    sscanf(val,"%x",&certid);
    break;
  case 's':
    sscanf(val,"%li",&ossize);
    break;
  case 'i':
    sscanf(val,"%li",&imsize);
    break;
  case 'h':
    sscanf(val,"%i",&hardware);
    break;
  case 'o':
    if (0==strcmp(val,"-"))
      outfile=stdout;
    else
      if (!(outfile=fopen(val,"wb"))) {
	fprintf(stderr,"%s: unable to open XXU file %s\n",argv[0],val);
	perror(val);
	exit(1);
      }
    break;
  default:
    fprintf(stderr,"%s: unknown option '%c'\n", argv[0], opt);
    fprintf(stderr,usage,argv[0]);
    exit(1);
  }
}

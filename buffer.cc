/*
 *
 *  read and write to an intermediate buffer
 *
 *
 */
 

#include <stdio.h>
#include <string.h>
#include "buffer.h"
#include "dbg.h"
#define NEWLINE '\n'


unsigned char buffer::GetByte()
{
    unsigned char c;

    if (read_ptr >= num_bytes) {
	/* eof jump here */
	return((unsigned char )EOF);
    }
    c = bytes[read_ptr];
    read_ptr++;
    return(c);
}

void buffer::PutByte(const char c)
{
    bytes[read_ptr] = c;
    read_ptr++;
    num_bytes++;
    if (read_ptr > (size - 1)) {
	dbg2(Warning, "PutByte overflow %d %d\n",  
	     (void *)read_ptr, (void *)size);
	read_ptr = size - 1;
    }
    return;
}

char * buffer::GetLine(char *buf, int buflen)
{
  char *p;
  char q;

  p = buf;
  while ((*p = GetByte()) != NEWLINE) {
    if ((signed char)*p == (signed char)EOF) {
      if (p != buf) {
	*p = NEWLINE;
	p++;
	*p = '\0';
	if (buf[0] == 'e')
	  dbg0(Warning, 
	       "tab: buffer: file ends with no trailing newline\n");
	else
	  dbg1(Warning, "buffer: GetLine EOF at end of line %d\n", 
	       (void *)((int)*buf & 0xff)); 
                                /* *buf suggestion of insight */
      }
      else {
	dbg0(Flow, "buffer: GetLine EOF\n");
      }
      return (buf);
    }
    else {
#ifndef _WIN32
      if ( *p == '\r') {
	dbg0(Flow, 
	     "buffer: GetLine: discarding windows format return\n");
	*p = NEWLINE;
	p++;
	*p = '\0';
	q = GetByte();	// remove the real newline
	if ( q != '\n' )
	  unGet( q );
	return (buf);
      }
#endif
      p++;
      buflen--;
      }
  }
  *p = NEWLINE;
  p++;
  *p = '\0';
  return (buf);
}

void buffer::PutLine(const char * l)
{
    int i=0;
    while (l[i] != '\n' && l[i] != '\0') {
	PutByte(l[i]);
	i++;
    }
    PutByte('\n');
    return;
}
void buffer::PutString(const char * l)
{
    int i=0;
    while (l[i] != '\0') {
	PutByte(l[i]);
	i++;
    }
    return;
}
unsigned char buffer::unGet(const char c)
{
	read_ptr--;
        bytes[read_ptr] = c;
	return (c);
}

int buffer::Seek(const long offset, const int how)
{
    switch (how) {
      case rew:
	read_ptr = offset;
	break;
      case cur:
	read_ptr += offset;
	break;
      case end:
	read_ptr = size - offset;
	break;
    }
    if (read_ptr > size) return (0);
    if (read_ptr < 0)    return (0);
    return (1);
}

int buffer::Tell()
{
    return(read_ptr);
}

void buffer::dump(const char *fname, const mode mode)	// dump to a file
{
#ifdef MAC
	dbg0(Warning, "buffer::dump: undefined proceedure\n");
#else
    FILE *fp=0;

    Seek(rew, 0);
    if (strcmp(fname, "stdout")) {
      fp = fopen(fname, "wb");
      if (fp == NULL) {
	dbg1(Error, "tab: dump: can't open %s for output\n", (void *)fname);
      }
      fwrite(bytes, num_bytes, 1, fp);
      fclose(fp);
    }
    else {             /* really is stdout */
      fwrite(bytes, num_bytes, 1, stdout);
      //      fclose(fp);
    }
#endif
}

const int PLACES = 7;

void buffer::Put10(const int num)
{
    int nnum = num;
    int	i;
    char temp[PLACES+3];
    
    if (nnum < 0) {
	PutByte('-');
	nnum = -nnum;
    }
    temp[0] = '\0';
    for(i = 1; i <= PLACES; i++)  {
	temp[i] = nnum % 10 + '0';
	nnum /= 10;
    }
    for(i = PLACES; temp[i] == '0'; i--);
    if( i == 0 )
      i++;
    while( i > 0 ) {
	PutByte(temp[i--]);
    }
    PutByte(' ');
}

void buffer::Put16(const int num)	// format char as 2 character hex
{
    char hi, lo;
    
    hi = (num >> 4) & 0xf;
    lo = num & 0xf;
	if (hi < 10) PutChar(hi + '0');
	else PutChar(hi - 10 + 'A');
	if (lo < 10) PutChar(lo + '0');
	else PutChar(lo - 10 + 'A');
}

void rdiv(int a, int b, int *c, int *d);

void buffer::PutF(const double num, const int res)
{
    int	i, inum;
    double nnum = num;
    char temp[PLACES + 3];
    double rest;
    double round = 0.5;

    if (nnum < 0) {
	PutByte('-');
	nnum = -nnum;
    }
    inum = (int)nnum;
    rest = nnum - (double)inum;

    for (i=res; i > 0; i--) round = round/10.0;
    rest += round;
    if (rest > 1. ) {
	inum++;
	rest = 0.0;
    }
    temp[0] = '\0';
    for(i = 1; i <= PLACES; i++)  {
	temp[i] = inum % 10 + '0';
	inum /= 10;
    }
    for(i = PLACES; temp[i] == '0'; i--);
    if( i == 0 )
      i++;
    while( i > 0 )
      PutByte(temp[i--]);

    PutByte ('.');

    i = res;
    while (i--) rest *= 10;
    inum = (int)(rest+0.000001);
    for(i = 1; i <= res; i++)  {
	temp[i] = (int)inum % 10 + '0';
	inum /= 10;
    }
    i--;
    while( i > 0 ) {
//	printf("%c\n", temp[i]);
	PutByte(temp[i--]);
    }
    PutByte(' ');
}

/* this isn't any faster */

void rdiv(int a, int b, int *c, int *d)
/* numerator, divisor, divident, remainder */
{
    int counter=0;
    int res=0;
    
    while ( b <= a ) {
	counter++;
	b <<= 1;
    }

//    printf(" a is %d b is %d %d\n", a, b, (b>>1)); 

    while(counter > 0) {
	b >>= 1;
	counter --;
	if (a >= b) {
	    a -= b;
	    res |=(1<<counter); 
//	    printf(" a is %d b is %d res is %d\n", a, b, res);
	}
    }
    
    *c = res;
    *d = a;
}

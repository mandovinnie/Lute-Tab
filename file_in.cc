
/*
   This program is copyright 1991 by Wayne Cripps,
   P.O. Box 677 Hanover N.H. 03755.
   All rights reserved.  It is supplied "as is"
   without express or implied warranty.

   Permission is granted to use, copy, modify and distribute
   this software without fee, provided that this notice appears
   in all copies, and that a copy of this notice is provided to
   anyone who recieves a binary copy without sources.

   This software may not be used for commercial purposes
   without explicit, prior written permission.

   Please mail bug reports, suggestions, and improvements
   to wbc@sunapee.dartmouth.edu.

 */
#include "win.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "file_in.h"
#include "dbg.h"

/* private */
void _file_in(const char * fname, const char *mode );
#ifdef MAC
void c2p(const char * name, Str255 fname);
#endif
file_in::file_in(const char * fname, const char *mode)
{
    dbg1(Proceedure, "starting file_in %s\n", (void *)fname);
    fn = (char *)malloc(1024);
    strcpy (fn, fname);
    _file_in(fname, mode);
}

void file_in::_file_in(const char * fname, const char *mode)
{
#ifdef MAC
    OSErr error;
    short refNum;
    long int nread;
    Str255 volName;
    Str255 fString;
    short vRefNum;

    c2p(fname, fString);
    error = GetVol(volName, &vRefNum);
    error = FSOpen (fString, vRefNum, &refNum );
    if (error != noErr)
      dbg1(Error, "File in: open %s", (void *)fname);
    buffer::bytes = NULL;
    error = GetEOF(refNum, &nread);
    num_bytes = size = nread;
    bytes = (char *) malloc(size);
    read_ptr = 0;
    if (size < 1 || bytes == NULL) {
	dbg1 (Error, "file in: malloc failed %d bytes\n", (void *)size);
    }
    nread = size;
    error = FSRead(refNum, &nread, bytes);
    if (error != noErr)
      dbg1(Error, "File in: read %s", (void *)fname);
    error = FSClose(refNum);
    error = FlushVol(0,0);
#else /* MAC */
    FILE * fp;
    size_t  n_read;

    buffer::bytes = NULL;
    if (strlen (fname) == 0)
      dbg0(Error, "File In: no file name given\n");
    fp = fopen(fname, mode);
    if (fp == NULL )
      dbg1(Error, "File In: Can't open %s\n", (void *)fname);
    fseek(fp, 0L, 2);
    size = ftell(fp);
    if (size < 1) {
      dbg1(Error, "file_in: zero length file %s\n", (void *)fname);
    }
    num_bytes = size;
    fseek(fp, 0L, 0);
    bytes = (char *) malloc(size);
    read_ptr = 0;
    if (size < 1 || bytes == NULL) {
	dbg1 (Error, "file in: malloc failed, size = %d\n", (void *)size);
    }
    n_read = fread (bytes, 1, size, fp);
    if (n_read != (size_t)size) {
#ifndef _WIN32
      dbg2(Error, "file_in: file read problem size %d nread %d\n",
	   (void *)size, (void *)n_read);
#endif /* _WIN32 */
	}
    dbg2(File, "read file: %d %s\n", (void *)size, (void *)fname);
    fclose(fp);
#endif
}

file_in::~file_in()
{
    dbg1(Proceedure, "  ending file_in %s\n", (void *)fn);
    if (bytes) {
	free(bytes);
	bytes = NULL;
    }
#ifndef MAC
    if (fn) {
	free (fn);
	fn = 0;
    }
#endif
}

file_in::file_in()
{
    char fname[1024];
#ifdef MAC
    OSErr error;
    short refNum;
    long int nread;

    refNum = my_open();
    buffer::bytes = NULL;
    error = GetEOF(refNum, &nread);
    num_bytes = size = nread;
    bytes = (char *) malloc(size);
    read_ptr = 0;
    if (size < 1 || bytes == NULL) {
	dbg1 (Error, "file in: malloc failed for %d bytes\n",
	      (void *) size);
    }
    nread = size;
    error = FSRead(refNum, &nread, bytes);
    if (error != noErr)
	dbg1(Error, "File in: read %s", (void *)fname);
    error = FSClose(refNum);
    error = FlushVol(0,0);
#else
    (void) printf("enter file name: ");
    scanf ("%s", fname);
    _file_in((const char *)fname, "rb");
#endif
}

unsigned char file_in::GetByte()
{
  return ( buffer::GetByte() );
}

int file_in::Get2Bytes()
{
    unsigned int n, a,b;
    a = buffer::GetByte();   b = buffer::GetByte();
    n = (a<<8) | b;
    if (a & 0X80) n |=  0XFFFF0000;
    return n;
}

int file_in::Get3Bytes()
{
    unsigned int n, a,b,c;
    a = buffer::GetByte();   b = buffer::GetByte();   c = buffer::GetByte();
    n = (((a<<8) | b) << 8) | c;
    if (a & 0X80) n |=  0XFF000000;
    return n;
}

int file_in::Get4Bytes()
{
    unsigned int n, a,b,c,d;
    a = buffer::GetByte();   b = buffer::GetByte();
    c = buffer::GetByte();  d = buffer::GetByte();
    n = (((((a<<8) | b) << 8) | c) << 8) | d;
    return n;
}

unsigned char file_in::unGet(const char c)
{
	return (buffer::unGet(c));
}

void file_in::Skip(const long offset)
{
	read_ptr += offset;
}

int file_in::Seek(const long offset, const int how)
/* rew, cur, end */
{
    return(buffer::Seek(offset, how));
}

char * file_in::GetLine(char *buf, int buflen)
{
  int i = buflen;
  unsigned char *tbuf = (unsigned char *)buffer::GetLine(buf, buflen);

  while (--i) {
    if (tbuf[i] ==147 || tbuf[i] == 148 )
      tbuf[i] = '"';
    else if ( tbuf[i] == 146 || tbuf[i] == 147 )
      tbuf[i] = '\'';
  }
  //  return (buffer::GetLine(buf, buflen));

  //
  //  if ( c == 147 || c == 148 )
  //    c = '"';
  //  else if ( c == 146 )
  //    c = '\'';

  return ( (char *)tbuf );
}

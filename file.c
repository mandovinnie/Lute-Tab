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
/*
  file handling stuff
 */

#include <stdio.h>
#include <setjmp.h>
#include "tab.h"

int line_count=1;
struct b input, inter_file;

SIGNED_C 
  get()			/* return a char */
{
	SIGNED_C get_p();
	return(get_p(&input));
}

SIGNED_C
  geti()
{
	SIGNED_C get_p();
	return(get_p(&inter_file));
}

char * getl(buf, n)		/* fill a buffer, throw away the */
     char *buf;			/* rest of the line */
     int n;			/* size of the buffer */
{
    SIGNED_C *p, q;
    SIGNED_C get_p();

    p = ( SIGNED_C *)buf;

    while ((*p = get()) != '\n'
#ifdef COMMAS
	   && *p != ','
#endif /* COMMAS */
	   ) {
	if (*p == EOF) {
	    *p = '\0';
	    return (buf);
	}
	else if (!n) {
	    while ((q = get()) != '\n' && q != EOF)
	      ;
	    p++;
	    *p = '\0';
/*	    printf ("file.c 1: %s", buf);  */
	    return (buf);
	}
	else {
	    n--;
	    p++;
	}
    }
#ifdef COMMAS
    if (*p == ',' ) {
	*p = '\n';
	if ((q = get()) != '\n') 
	  unget(q);
    }
#endif /* COMMAS */
    p++;
    *p = '\0';
/*    printf ("file.c 2: %s", buf);   */
    return (buf);
}

SIGNED_C
  get_p(i)
struct b *i;
{
    SIGNED_C c;

    if (i->ib_p >= i->ib_s) { /* return (EOF); */
	printf ("tab: get_p: read error pointer_file %d size %d\n", 
		i->ib_p, i->ib_s);
	return(EOF);
    }
    c = i->ib[i->ib_p];
    i->ib_p++;
    if (c == '\n') line_count++;
    return(c);
}

reset_inter(f)
     struct file_info *f;
{
    rewind(f->i_p);
    inter_file.ib_p = 0;
}

unget(c)			/* put a char back on the input */
     SIGNED_C c;
{
	unget_p(c, &input);
}

ungeti(c)			/* put a char back on the input */
     SIGNED_C c;
{
	unget_p(c, &inter_file);
}

unget_p(c, i)
     SIGNED_C c;
     struct b *i;
{
    if (i->ib_p <= 0) {
	printf ("tab: unget: out of buffer space\n");
	exit (-1);
    }
    i->ib_p--;
    i->ib[i->ib_p] = c;
    if (c == '\n') line_count--;
}

rdfile(f, filetype)
struct file_info *f;
int filetype;
{
    int nread;
    char a_file[120];
    FILE *fp;
    struct b *i;

    strcpy (a_file, f->file);

    if (!filetype) {
	fp = fopen (a_file, "r");
	if (fp == NULL) {
	    strcat (a_file, ".tab");
	    fp = fopen (a_file, "r");
	}
    }
    else {			/* reopen intermediate */
	strcpy (a_file, f->i_file);
	fclose(f->i_p);
	fp = fopen (f->i_file, "r+");
    }
    if (fp == NULL) {
	printf ("tab: rdfile: can't open file %s\n", f->file);
	exit (-1);
    }
    if (f->flags & VERBOSE) printf (" opening %s\n", a_file);

    if (filetype) i = &inter_file;
    else i = &input;

    /* get size of file - this hack works on a vax */

    fseek(fp, 0L, 2);
    i->ib_s = ftell(fp);
    fseek(fp, 0L, 0);
    i->ib = (char *) malloc(i->ib_s);
    if (i->ib == 0) {
	printf ("tab: rdfile: can't allocate memory for input file %s\n",
		f->file);
    }
    i->ib_p = 0;

    if (f->flags & VERBOSE) printf ("input file size %d\n", i->ib_s);

    if ((nread = fread ( i->ib, 1, i->ib_s, fp)) != i->ib_s) {
#ifndef VAX
	printf ("tab: rdfile: error reading file %s, %d read %d wanted\n", 
		 f->file, nread, i->ib_s);
#endif /* VAX */
	if (nread <= 0) exit (-1); 
    }
    fclose (fp);
}


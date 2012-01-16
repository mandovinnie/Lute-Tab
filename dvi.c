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
 * dvi output stuff
 *
 * note that we use dvi units, and we use the tfm file for font
 * widths
 */

#include <stdio.h>
#include "dvi.h"
#include "tab.h"
#include <errno.h>
#include <string.h>

/* globals */


int cur_loc=0;
int last_bop= -1;
int page_num=1;
extern struct font_d *fonts[MAXFONTS];

FILE *fp;

#ifdef DVI

open_dvi_file(dvi_file)
char *dvi_file;
{   fp = fopen(dvi_file, "w");
    if (fp == NULL) {
	printf("tab: open_dvi_file: open failed\n");
	exit (-1); }}

put_byte(c)
unsigned char c;
{
    if (putc(c, fp) == EOF) {
	printf("tab: put_byte failed\n");
	exit (-1);
    }
    cur_loc++;
}

signed_byte(c)
     char c;
{    put_byte(c);}

put_two_bytes(c)
unsigned short c;
{
    put_byte((unsigned char)(c & 0xff00) >> 8 & 0xff);
    put_byte((unsigned char)(c & 0xff) & 0xff);
}

signed_pair(c)
unsigned short c;
{
    put_byte((unsigned)(c & 0xff00) >> 8 & 0xff);
    put_byte((unsigned)(c & 0xff) & 0xff);
}

put_three_bytes(){}

signed_trio(c)
long c;
{
    put_byte((unsigned)(c & 0xff0000) >> 16 & 0xff);
    put_byte((unsigned)(c & 0xff00) >> 8 & 0xff);
    put_byte((unsigned)(c & 0xff) & 0xff);
}

signed_quad(c)
long c;
{
    put_byte((unsigned)((c & 0xff000000) >> 24 & 0xff));
    put_byte((unsigned)((c & 0xff0000) >> 16 & 0xff));
    put_byte((unsigned)((c & 0xff00) >> 8 & 0xff));
    put_byte((unsigned)((c & 0xff) & 0xff));
}
dvi_length(){}

move_to_byte(n) long n; {    fseek(fp, n, 0);}

char tag[]=" TeX output by Wayne Cripps";
write_preamble()		/* sort of like #108 process preamble */
{
    int i;
    put_byte(PRE);		/* first byte - preamble opcode */
    put_byte(2);		/* version - identification*/
    signed_quad(NUM);		/* numerator */
    signed_quad(DENOM);		/* denominator */
    signed_quad(1000);		/* mag * 1000 */
    put_byte((unsigned char)strlen(tag));	/* length of comment */
    for (i=0;i< strlen(tag); i++) /* the comment */
      put_byte ((unsigned char)tag[i]);
}

write_postamble(f1, f2, f3, f4, f5/* , f6, f7 */)
char *f1, *f2, *f3, *f4, *f5/* , *f6, *f7 */;
{
    int loc;

    loc = cur_loc;
    put_byte(POST);
    signed_quad((long)last_bop);	/* pointer to last bop */
    signed_quad(NUM);	/* numerator */
    signed_quad(DENOM);	/* denominator */
    signed_quad(1000);		/* mag * 1000 */
    signed_quad((long)inch_to_dvi(11.0));	/* height plus depth tallest page */
    signed_quad((long)inch_to_dvi(8.5));	/* width of widest page */
    signed_pair(4);		/* max stack depth */
    signed_pair((short)page_num-1);		/* total num pages */
    /* font definitions */
    if (f1) define_font(0, f1);
    if (f2) define_font(1, f2);
    if (f3) define_font(2, f3);
    if (f4) define_font(3, f4);
    if (f5) define_font(4, f5);

    put_byte(POST_POST);	/* post-post */
    signed_quad((long)loc);	/* ptr to post that started postamble */
    put_byte(2);       	        /* identifier */
    put_byte(223);		/* filler 4 - 7 to pack file*/
    put_byte(223);put_byte(223);put_byte(223);
}


define_font(font_num, name)
int font_num;
char *name;
{
    int j, l = strlen(name);
    struct font_d *f;

    f = fonts[font_num];

    put_byte(FNT_DEF1);
    put_byte((unsigned char)font_num);	   /* font number */
    signed_quad((long)f->f_check_sum);   /* checksum */
    signed_quad((long)f->f_design_size); /* loaded at size, scale factor */
    signed_quad((long)f->f_design_size); /* design size */
    put_byte(0);		   /* a - the length of directory */
    put_byte((unsigned char)l);		   /* length of font name */
    for (j=0; j< l ; j++) 
      put_byte((unsigned char)name[j]);
    /* font name */
    
    /* font area, or directory goes here */
}

#endif /* DVI */


#ifdef DVI

do_dvi_pages(f)
struct file_info *f;
{
    int i, more=1;
    int prev_bop;

    last_bop = -1;

    while (more) {

	prev_bop=cur_loc; 
	put_byte(BOP);
	signed_quad(page_num);
	for (i=1; i < 10; i++)signed_quad(0);
	signed_quad(last_bop);	/* pointer to prev bop, -1 for first */
 	last_bop = prev_bop;  

	if (red == 1.0) 
	  define_font(0, "lute9"); /* does a font_def for lute9*/
	else if (red == 0.88880)
	  define_font(0, "lute8"); /* does a font_def for lute8*/
	else if (red == 0.777770)
	  define_font(0, "lute7"); /* does a font_def for lute8*/
	else
	  define_font(0, "lute6"); /* does a font_def for lute6*/
	define_font(1, "cmr12"); /* does a font_def for text*/

	define_font(2, "cmr10");

/*	printf ("do_dvi_page: ready to print a page of dvi \n"); */
	pagenum++;
/*	printf ("dvi.c about to format a page %d\n", pagenum); */
	more = format_page(f);
/* 	printf ("do_dvi_page: page printed - more = %d \n", more); */

	dvi_command(NO_OP, 0, 0);
	put_byte(EOP);

	reset_dvi_vh();
	page_num++;
    }
}


make_dvi(f)
struct file_info *f;
{

    /* load default fonts */

    if (red == 1.0) 
      open_tfm_file(0, "lute9.tfm", f);
    else if (red == 0.8888)
      open_tfm_file(0, "lute8.tfm", f);
    else if (red == 0.777770)
      open_tfm_file(0, "lute7.tfm", f);
    else
      open_tfm_file(0, "lute6.tfm", f);

    open_tfm_file(1, "cmr12.tfm", f);

    open_tfm_file(2, "cmr10.tfm", f);

    open_dvi_file(f->out_file);
    write_preamble();

    do_dvi_pages(f);

    if (red == 1.0) 
      write_postamble("lute9", "cmr12", "cmr10", (char *) 0, (char *)0);
    else if (red == 0.8888) 
      write_postamble("lute8", "cmr12", "cmr10", (char *) 0, (char *)0);
    else if (red == 0.777770) 
      write_postamble("lute7", "cmr12", "cmr10", (char *) 0, (char *)0);
    else
      write_postamble("lute6", "cmr12", "cmr10", (char *) 0, (char *)0);
}

#endif /* DVI */

str_to_dvi(string)
     char *string;
{
    char *s, str[15];
    extern double atof();
    extern char *strstr();

/*     strncpy(str, string, (size_t)sizeof(str)); */
    mystrncpy(str, string, (size_t)sizeof(str));

    s = strstr(str, "in");
    if (s) {			             /* inches */
	*s = 0;
	return (inch_to_dvi(atof(str)));
    }
    else if ((s = strstr(str, "pt"))) {          /* points */
	*s = 0;
	return (pt_to_dvi(atof(str)));
    }
    else if ((s = strstr(str, "mm"))) {          /* points */
	*s = 0;
	return (mm_to_dvi(atof(str)));
    }
    printf ("tab: str_to_dvi: unsupported dimen %s\n", str);
    exit(-1);
}

mystrncpy(s1, s2, n)		/* not as robust as the real thing */
char *s1, *s2;			/* but quite a bit faster! */
int n;
{
    while (n) {
	*s1 = *s2;
	s1++;
	s2++;
	if (!*s2) break;
	n--;
    }
}

inch_to_dvi(inch)
double inch;
{
    return ((int)(red * (inch * 254000.0) * ( (double)DENOM / (double)NUM)));
}

pt_to_dvi(pt)
double pt;
{
    return(red * (pt * 254000) * ( (double)DENOM / (double)NUM)) / 72.27;
}

/* dvi_to_inch is in tfm_file.c */
double
dvi_to_pt(dvi)
     int dvi;
{
    return ((72.27 / red ) * ( (double)NUM / (double)DENOM) 
	    * (double) dvi / 254000 );
}

mm_to_dvi(mm)
double mm;
{
    return (red * (mm * 254000) * ( (double)DENOM / (double)NUM)) / 2.54;
}


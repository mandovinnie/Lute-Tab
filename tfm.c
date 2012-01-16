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

#include <stdio.h>
#include <string.h>
#include "tab.h"
#include "dvi.h"

#ifndef TFM_PATH
#define TFM_PATH "./:/usr/lib/tex/fonts/tfm/"
#endif

#ifdef VAX
#undef TFM_PATH
#endif /* VAX */

/* int tfd; */
FILE *tfp;
double ps_fact[6]={/* 0 */ 1.0,
		   /* 1 */ 1.0,
		   /* 2 */ 1.0,
		   /* 3 */ 1.0,
		   /* 4 */ 1.0,
		   /* 6 */ 1.0};


open_tfm_file(fontnum, tfm_file, ff)
int fontnum;
char *tfm_file;
struct file_info *ff;
{
    char path[600];
    char file_path[120];
    char *p, *pp;

    path[0] = '\0'; file_path[0]='\0';

#ifdef TFM_PATH

    strcpy (path, TFM_PATH );
    p = path;

    while (tfp == NULL && p) {	/* try system location */
	pp = strchr(p, ':');
	if ( pp ) {
	    *(pp++) = '\0';
	}
	strcpy (file_path, p);
	strcat (file_path, tfm_file);
	tfp = fopen (file_path, "r");
	p = pp;
    }
#else
    strcpy (file_path, tfm_file);
    tfp = fopen(file_path, "r"); /* try local location first */
#endif /* TFM_PATH */

    if (tfp == NULL) {
	printf ("tab: open_tfm_file: open failed: %s\n", tfm_file);
	exit (-1);
    }
    if (ff->flags & DEBUG) 
      printf ("tab: opening tfm file %d %s %s\n", 
	      fontnum, tfm_file, file_path);

    in_TFM(0, fontnum, ff);

#ifndef hpux
    fclose(tfp);
#endif /* hpux */
    tfp = NULL;

    if (ff->flags & PS ) {
	switch(fontnum) {
	  case 0:
	    ps_fact[fontnum] = 1.0;
	    break;
	  case 1:		/* text */
	    ps_fact[fontnum] = PS_SCALE_1;
	    break;
	  case 2:		/* 12 pt titles */
	    ps_fact[fontnum] = PS_SCALE_2;
	    break;
	  case 3:
	    ps_fact[fontnum] = PS_SCALE_3;
	    break;
	  case 4:
	    ps_fact[fontnum] = PS_SCALE_4;
	    break;
	  case 5:		/* italic text */
	    ps_fact[fontnum] = PS_SCALE_5;
	    break;
	  default:
	    break;
	}
    }
}
read_tfm_word(b0, b1, b2, b3)
     char *b0, *b1, *b2, *b3;
{
    *b0 = getc(tfp);
    *b1 = getc(tfp);
    *b2 = getc(tfp);
    *b3 = getc(tfp);
}
read_tfm_short(b0, b1)
     char *b0, *b1;
{
    *b0 = getc(tfp); 
    *b1 = getc(tfp);
}

double tfm_conv;		/* dvi units per absolute TFM unit 1 for now */
extern int cur_loc;
extern int last_bop;
extern int page_num;
struct font_d *fonts[MAXFONTS];

in_TFM(z, nf, ff) 
int z;				/* scaling factor */ 
int nf;				/* font number */
struct file_info *ff;
{
    char  b0, b1, b2, b3; 
    int k;			/* index for loops */
    unsigned int x;
    struct font_d *f;
    long get_long();
    int get_short();
    double dvi_to_inch();
    char *p;

    tfm_conv = 1.0 / 8.0;
    /* (25400000 / NUM ) * ( resolution DENOM / DENOM) / 16.0 */

    /*  get a data structure  */
    if (! fonts[nf] ) 
      fonts[nf] = (struct font_d *)malloc (sizeof (struct font_d));
    if (! fonts[nf] ) {
	printf ("tab: can't allocate font def structure\n");
	exit (-1);
    }
    f = fonts[nf];
    /* read past header data */

    f->f_l  = get_short();	/* length */
    f->f_lh = get_short();	/* header len */

    /*
       if (ff->flags & DEBUG) 
       printf ("tab: in_TFM: length %d header_len %d\n",
       f->f_l, f->f_lh);
       */
    f->f_bc = get_short();	/* begin char */
    f->f_ec = get_short();	/* end char */

    if (ff->flags & DEBUG) 
      printf ("tab: in_TFM: begin char %d end char %d\n",
	      f->f_bc, f->f_ec);

    if (f->f_ec < f->f_bc) f->f_bc = f->f_ec +1;

    /* check for too many widths */
    if (( f->n_widths = f->f_ec - f->f_bc + 1) > MAXWIDTHS) {
	printf ("tab: in_TFM: too many widths\n");
	exit (-1);
    }

    if (f->n_widths < 5) {	/* oops! */
	printf ("tab: tfm_file: there are only %d characters in this font!!\n",
		f->n_widths);
	/*	exit (-3); hey - I might want to do it, anyways! */
    }
    f->f_nw = get_short();	/* number of widths */
    f->f_nh = get_short();	/* number of heights */
/*    printf ( "tab.c %d widths, %d heights\n", f->f_nw, f->f_nh); */
    f->f_nd = get_short();
    f->f_ics = get_short();
    f->f_ligproglen = get_short();
    f->f_kerns = get_short();
    f->f_extproglen = get_short();
    f->f_params = get_short();

    
    for (k=1;k <= f->f_lh; k++) { /* check sum and design size */
	if ( k == 1 ) {
	    f->f_check_sum = get_long();
	    pk_checksum[nf] = f->f_check_sum;
	}
	else if (k == 2) 
	  f->f_design_size = get_long() / 8;
	else if (k == 13 ) {	/* seems to be 1 byte size 19 name */
	    p = f->names;
	    read_tfm_word(&b0, &b1, &b2, &b3);
	    *p++ = b1; 
	    *p++ = b2; 
	    *p++ = b3; 
	    while (k < 17) {
		read_tfm_word(&b0, &b1, &b2, &b3);
		*p++ = b0; 
		*p++ = b1; 
		*p++ = b2; 
		*p++ = b3; 
		k++;
	    }
	}
	else (void) get_long();
    }
    /*
       printf ("in_TFM: fontnum %d ec %d nw %d %s\n",
       nf, f->f_ec, f->f_nw, f->names);
    */
    /* store char width indices at end of width table */
    for (k=0; k < f->n_widths; k++){
	read_tfm_word(&b0, &b1, &b2, &b3);
	f->f_widths[k] = (unsigned)(b0 & 0xff);
	f->f_heights[k] = (unsigned)(b1>>4 & 0xf);
	f->f_depths[k] = (unsigned)(b1 & 0xf);
    }

    /* read and convert width values */

    /*
       printf ("design size %X %d\n", f->f_design_size, f->f_design_size);
     */
    for (k=0; k < f->f_nw; k++){
	x = get_long();

	if (ff->flags & PS) {
	    f->f_width[k] = dvi_to_inch(x) * (double)f->f_design_size 
	      / (1048576.0 / red);

	    f->f_i_width[k] =( x * (( f->f_design_size * 16) / 0x100000 ))
	      / 16;
/*
	    if (nf == 1) printf("tfm: %d %d %f %d\n", 
	    k, f->f_i_width[k], 
	    f->f_width[k], inch_to_dvi(f->f_width[k] / red));
*/
	}
	else {
	    f->f_width[k] = dvi_to_inch(x) * f->f_design_size / 1048576.0;
	    f->f_i_width[k] =( x * 
			      (( f->f_design_size * 16) / 0x100000)) / 16;
	}

	/*  1048576 = 0x100000 design size for lute9 = 1179648 = 0x120000*/
    }    

    /* read and convert height values */

    for (k=0; k < f->f_nh; k++){
	x = get_long();
	f->f_i_height[k] = ( x * (( f->f_design_size * 16) / 0x100000)) / 16;
	f->f_height[k] = dvi_to_inch(x) * f->f_design_size / 1048576.0;
    }    

    /* read and convert depth values */

    for (k=0; k < f->f_nd; k++){
	x = get_long();
	f->f_depth[k] = dvi_to_inch(x) * f->f_design_size / 1048576.0;
    }    
}

long
get_long()
{
    char b0, b1, b2, b3;
    long val;
    read_tfm_word(&b0, &b1, &b2, &b3);

    val = ((unsigned long)b0 & 0xff) << 24;
    val += ((unsigned long)b1 & 0xff) << 16;
    val += ((unsigned long)b2 & 0xff) << 8;
    val += ((unsigned long)b3 & 0xff);
    return(val);
}
int
get_short()
{
    char b0, b1;

    read_tfm_short(&b0, &b1);
    return ((((unsigned long)b0 & 0xff) << 8) + (b1 & 0xff));
}

double
dvi_to_inch(dvi)
     int dvi;
{
    return ((double) dvi * ((double)NUM / (double)DENOM) / (254000.0 * red) );
}

double
  get_width(fontnum, c)
int fontnum;
unsigned char c;
{
    struct font_d *f;
    double result;

#ifdef X_WIN
    if (fontnum != 0) return (x_get_width(c));
#endif
    f = fonts[fontnum];
    if (c < f->n_widths) {
	if ( c >= f->f_bc ) {
	    result =  f->f_width[f->f_widths[(int)c - f->f_bc]];
	    return(ps_fact[fontnum] * result);
	}
	else {
	    printf ("tab: get_width: character %d font %d less than minimum\n", 
		    c, fontnum);
	    return(0.0);
	}
    }
    else {
	printf ("tab: get_width: unknown width for character %d font %d\n", 
		c, fontnum);
	return(0.0);
    }
}

double
  get_height(fontnum, c)
int fontnum;
unsigned char c;
{
    struct font_d *f;

#ifdef X_WIN
#else
    f = fonts[fontnum];
    return(ps_fact[fontnum] * f->f_height[f->f_heights[(int)c - f->f_bc]]);
#endif
}

double
  get_depth(fontnum, c)
int fontnum;
unsigned char c;
{
    struct font_d *f;

#ifdef X_WIN
#else
    f = fonts[fontnum];
    return(ps_fact[fontnum] * f->f_depth[f->f_depths[(int)c - f->f_bc]]);
#endif
}


p_get_w(fontnum, c)
     int fontnum;
     unsigned char c;
{
    struct font_d *f;

    f = fonts[fontnum];
    if ( c < f->f_bc ) {
	printf ("tab: p_get_w: character below minimum %d %d\n",
		c, fontnum);
	return (0);
    }
    return((int)(ps_fact[fontnum] * f->f_i_width[f->f_widths[(int)c - f->f_bc]]));
}

p_get_h(fontnum, c)
     int fontnum;
     unsigned char c;
{
    struct font_d *f;

    f = fonts[fontnum];
    return((int)(ps_fact[fontnum] * f->f_i_height[f->f_heights[(int)c - f->f_bc]]));
}

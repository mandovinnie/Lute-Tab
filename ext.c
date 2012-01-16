
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

#include <setjmp.h>
#include <stdio.h>
#include "dvi.h"
#define LIST 200
#define STAFF 11

char version[]="Version 3.0p, October 2 1994";
unsigned char list[LIST * STAFF];
struct list *listh=0;
char  inter[1024];
int inter_p=0;

int manuscript=0;
int paper=0;
int five=0;
jmp_buf env;
int u_bar = 0;			/* global */
int baroque=0;
double space;

double red=1.0;		/* scaling for the whole program */

int dvi=1;
int verbose=0;
int italian=0;
int i_flags=0;
int mod_flags=0;
int o_flags=0;
int mace=0;
int rob=0;
int marks=0;
int xpose=0;			/* transpose score for harp */
int bar_count=0;		/* print bar numbers ? */
int barCount=0;		        /* print bar numbers at start of line ? */
int pagenum=0;			/* page number if automaticly calculated */
int n_system=0;	/* what system are we in? */
FILE *op;  /* postScript output file */
FILE *tp;  /* postScript temp file */
int pk_checksum[MAXFONTS];

int max_b_w=0;			/* max bitmap width for ps */
int max_b_h=0;			/* max bitmap height for ps */
int max_off_w=0;		/* max bitap width offset for ps */
int max_off_h=0;		/* max bitmap height offset for ps */

#define POSTSCRIPT
#ifdef POSTSCRIPT
unsigned char ps_used[256];
#endif /* POSTSCRIPT */

int first_1=0, first_2=0;
char highlight;

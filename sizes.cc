/*
 *
 * default sizes for various formatting satuff
 *
 */
#include "win.h"
#include "tab.h"
#include <string.h>

double st_text = .32; /* in */		      /* basic staff to music space */
double m_space = .09; /* in */  /* was .1 */  /* interline spacing in music */
char flag_to_staff[] = "4 pt";            /* flags above imaginary top line */
int st_italian=0;		          /* one if high italian flags used */
char italian_offset[] = "4.5 pt";          /* shift staff for italain style */
char em[] = "7.0 pt";		            /* basic character design width */
char interspace[]="10.00 pt";	  /* distance between staff line was .10 pt */
double staff_len = 6.5;                      /* length of line horizontally */
double o_staff_len = 6.5;	// wbc feb 2005 changed from 0
char staff_height[120];

/* char staff_height[] = "00000.0057 in"; */ /* 37 */ /* thickness of staff line */
/* 0033 is light, 007 works on NeXT previewer */

double text_sp = /* 0.17 */ 0.17;/* in */  /* padding for text below music  */
char mus_space[] = "7 pt";  /* interline space for tab transcribed to music */
int n_measures=50;
char thick_bar[] = "0.024 in";
char baselinespace[] = "-0.02 in";          /* was .020 char above the line */
char grid_indent[] = "0.033 in";	       /* hor. offset of grid staff */
char flag_indent[] = "0.02 in";                /* hor. offset of flag staff */
char i_flag_indent[] = "-0.009 in";    /* hor. offset of italian flag staff */



struct list *listh=0;




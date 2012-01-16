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
 * drawing primitaves
 * interface between lute program and dvi/ps driver
 *
 */

#include <stdio.h>
#include <string.h>
#include "dvi.h"
#include "tab.h"
#include <time.h>

#ifdef POSTSCRIPT
extern FILE *tp;
void ps_command();
#endif /* POSTSCRIPT */

int dvi_h=0, dvi_v=0;		/* current primitave location */
int ps =0;
int x = 0;
static int curfont;		/* current font number */

#ifdef DVI
void dvi_command();
#endif /* DVI */

void p_movev();

reset_dvi_vh()			/* reset for a new page */
{
    dvi_h=dvi_v=0;
}

void
  p_moveh(hor)                  /* move horizontally (right is positive) */
int hor;			/* OUTPUT */
{
    if (!hor) return;

    if (ps) {
#ifdef POSTSCRIPT
	ps_moveh(hor);
#endif /* POSTSCRIPT */
    }
    else if (x) {
#ifdef X_WIN
	x_moveh(hor);
#endif /* X_WIN */
    }
    else {
#ifdef DVI
	dvi_h += hor;	
	if (hor & 0xff800000) {
	    dvi_command(RIGHT4, hor, 0);
	}
	else if (hor & 0x007f8000) {
	    dvi_command(RIGHT3, hor, 0);
	}
	else if (hor & 0x00007f80) {
	    dvi_command(RIGHT2, hor, 0);
	}
	else if (hor & 0x0000007f) {
	    dvi_command(RIGHT1, hor, 0); /* was RIGHT4 */
	}
#endif /* DVI */
    }
}
void
moveh(h)			
     char *h;
{
    p_moveh(str_to_dvi(h));
}
void
  move_n_h(h)			
     char *h;
{
    p_moveh(- str_to_dvi(h));
}
void
  fmoveh(h)
     double h;
{
    p_moveh(inch_to_dvi(h));
}

void p_movev(ver)
int ver;			/* OUTPUT */
{
    if (ver == 0) {
	return;
    }

    if (ps) {
#ifdef POSTSCRIPT
	ps_movev(ver);
#endif /* POSTSCRIPT */
    }
    else if (x) {
#ifdef X_WIN
	x_movev(ver);
#endif /* X_WIN */
    }
    else {
#ifdef DVI
	dvi_v += ver;
	if (ver > 0) {
	    if (ver & 0xff800000) {
		dvi_command(DOWN4, ver, 0);
	    }
	    else if (ver & 0x007f8000) {
		dvi_command(DOWN3, ver, 0);
	    }
	    else if (ver & 0x00007f80) {
		dvi_command(DOWN2, ver, 0);
	    }
	    else if (ver & 0x0000007f) {
		dvi_command(DOWN1, ver, 0);
	    }
	}
	else {
	    if (ver > -32767) {
		dvi_command(DOWN2, ver, 0);
	    }
	    else if (ver & 0xff000000) {
		dvi_command(DOWN4, ver, 0);
	    }
	}
#endif /* DVI */
    }
}
movev(v)			/* move vertically (down is positive) */
     char *v;
{
    p_movev(str_to_dvi(v));
}

move_n_v(v)			/* move opposite the value (up is positive) */
char * v;
{
    p_movev(-1 * str_to_dvi(v));
}

double
  fmovev(v)
double v;
{				/* this badly needs buffering! */
    p_movev(inch_to_dvi(v));
}

put_rule(w, h)
     char *w, *h;
{
    p_put_rule(str_to_dvi(w), str_to_dvi(h));
}

f_put_rule(w, h)
     double w,h;
{
    p_put_rule(inch_to_dvi(w), inch_to_dvi(h));
}

p_put_rule(w, h)		/* OUTPUT */
int w, h;			/* in dvi units */
{
    if (ps) {
#ifdef POSTSCRIPT
	if (highlight) 
	  ps_set_gray();
	
	ps_put_rule(w, h);
#endif /* POSTSCRIPT */
	if (highlight) {
	    highlight = 0;
	    ps_unset_gray();
	}
    }
    else if (x) {
#ifdef X_WIN
	x_put_rule(w, h);
#endif /* X_WIN */
    }
    else {
#ifdef DVI
	dvi_command(PUT_RULE, h, w);
#endif /* DVI */   
    }
}
put_a_char (c)
     unsigned char c;			/* OUTPUT */
{
    if (ps) {
#ifdef POSTSCRIPT
	if (highlight) 
	  ps_put_gray((unsigned char)c);
	else
	  ps_put_char((unsigned char)c);
#endif /* POSTSCRIPT */
    }
    else if (x) {
#ifdef X_WIN
	x_put_char((unsigned char)c);
#endif /* X_WIN */
    }
    else {
#ifdef DVI
	dvi_command(PUT1, (int)c, 0);
#endif /* DVI */
    }
}



set_a_char (c)
     unsigned char c;			/* OUTPUT */
{
    if (ps) {
#ifdef POSTSCRIPT
	dvi_h += p_get_w(curfont, c);
	ps_set_char(c);
#endif /* POSTSCRIPT */
    }
    else if (x) {
#ifdef X_WIN
/*	dvi_h set in x_set_char */
	x_set_char(c);
#endif /* X_WIN */
    }
    else {
#ifdef DVI
	dvi_h += p_get_w(curfont, c);
	dvi_command(SET1, (int)c, 0);
#endif /* DVI */
    }
}

use_font(fontnum)
int fontnum;			/* OUTPUT */
{
    curfont = fontnum;

    if (ps) {
#ifdef POSTSCRIPT
	if (fontnum == 0) ps_setLute();
	else ps_setfont(fontnum);
#endif /* POSTSCRIPT */
    }
    else if (x) {
#ifdef X_WIN
	if (fontnum == 0) x_setLute();
	else x_setfont();
#endif /* X_WIN */
    }
    else {
#ifdef DVI
	dvi_command(FNT1, fontnum, 0);
#endif /*DVI */
    }
}

get_font() { return (curfont); }

do_tie(length) double length; {
    if (ps) {
#ifdef POSTSCRIPT
	ps_tie(inch_to_dvi(length));
#endif /* POSTSCRIPT */
    }
    else {
    }
}

#ifdef POSTSCRIPT
ps_clipped_i(c, font)
     char c;
     int font;
{
    if (ps) {
	ps_command(PS_CLIP, (int)c, p_get_h(font, c), dvi_h, dvi_v);
    }
    else set_a_char(c);
}
#endif /* POSTSCRIPT */
double
str_to_inch(string)
     char *string;
{
    char *s, str[85];
    extern double atof();
    extern char * strstr();
    double val;

    mystrncpy(str, string, sizeof(str));
    s = strstr(str, "in");
    if (s) {			/* inches */
	*s = 0;
	return (atof(str));
    }
    else if (s = strstr(str, "pt")) {          /* points */
	*s = 0;
	val = (atof(str) / 72.27 );
	return (val);
    }
    else if (s = strstr(str, "mm")) {          /* points */
	*s = 0;
	return (atof(str) / 2.54);
    }
    printf("tab: str_to_inch: unsupported dimen %s  [ %s ]\n",
	   string, str);
    return(0);
}

pushloc_prim()
{
    if (ps || x) push();
    else {
#ifdef DVI
	dvi_command(PUSH, 0, 0);
#endif /* DVI */
    }
}

poploc_prim()
{
    if (ps || x) pop();
    else {
#ifdef DVI
	dvi_command(POP, 0, 0);
#endif /* DVI */
    }
}

static int save_h[REGS],  save_v[REGS];
static int stack_h[REGS], stack_v[REGS];
static int sp=0;

saveloc(reg)			/* save current location */
     int reg;			/* register */
{	
    slp(reg, save_h, save_v);
}
push()
{	
    slp(sp, stack_h, stack_v);
    sp++;
}
slp(reg, h, v)
     int reg;	
     int h[], v[];
{
    if (reg >= REGS) 
      printf ("tab: saveloc: illegal register %d\n", reg);
    h[reg] = dvi_h;
    v[reg] = dvi_v;
}

getloc(reg)			/* go to saved location */
     int reg;			/* register */
{	
    glp(reg, save_h, save_v);
}
pop()
{	
    sp--;
    glp(sp, stack_h, stack_v);
}
look()
{
    sp--;
    glp(sp, stack_h, stack_v);
    sp++;
}
glp(reg, h, v)
int reg;			/* OUTPUT */
int h[], v[];
{		      	
    int h_diff, v_diff;

    if (reg >= REGS) 
      printf ("tab: getloc: illegal register %d\n", reg);

    h_diff = h[reg] - dvi_h;
    v_diff = v[reg] - dvi_v;
    dvi_h = h[reg];
    dvi_v = v[reg];

    if (ps) {
#ifdef POSTSCRIPT
	ps_command(GLP, dvi_h, dvi_v, 0, 0);
#endif /* POSTSCRIPT */
    }
    else if (x) {
#ifdef X_WIN
/* x doesn't have to move back, as long as dvi_h and dvi_v are reset */
#endif /* X_WIN */
    }
#ifdef DVI
    else {
 	if (v_diff) dvi_command(DOWN4, v_diff, 0); 
	if (h_diff) dvi_command(RIGHT4, h_diff, 0);
    }
#endif
}

put_slash(bloc, eloc, count, f) /* draw a slash between two saved locations*/
int bloc, eloc, count;		/* the line is always horizontal */
struct file_info *f;
{				/* OUTPUT */
    double nflags = (double)count;
    int thickness = str_to_dvi("0.005 in");

    if (f->flags & LSA_FORM) thickness = str_to_dvi("0.023 in");

    saveloc(REGS-1);
/*     move_n_v("0.19 in"); */
    while (count) {
	if (ps) {
#ifdef POSTSCRIPT
	    ps_put_rule(save_h[eloc] - save_h[bloc], thickness);
#endif /* POSTSCRIPT */
	}
	else if (x) {
#ifdef X_WIN
	    x_put_rule(save_h[eloc] - save_h[bloc], thickness);
#endif /* X_WIN */
	}
#ifdef DVI
	else {
	    dvi_command(PUT_RULE,
			thickness, 
			save_h[eloc] - save_h[bloc]);
	}
#endif
	fmovev(str_to_inch("0.03 in") + 0.06/ nflags);
	count --;
    }
    getloc(REGS-1);
}

put_uline(bloc, eloc) /* draw a slash between two saved locations*/
int bloc, eloc;			/* OUTPUT */
{
    saveloc(REGS-1);
    if (ps) {
#ifdef POSTSCRIPT
	ps_put_rule(save_h[eloc] - save_h[bloc], str_to_dvi("0.005 in"));
#endif /* POSTSCRIPT */
    }
    else if (x) {
#ifdef X_WIN
	x_put_rule(save_h[eloc] - save_h[bloc], str_to_dvi("0.005 in"));
#endif /* X_WIN */
    }
    else {
#ifdef DVI
	dvi_command(PUT_RULE, 
		    str_to_dvi("0.005 in"),
		    save_h[eloc] - save_h[bloc]);
#endif /* DVI */
    }
    getloc(REGS-1);
}

put_thick_slant(bloc, eloc)
int bloc,eloc;
{
    if (ps ) {
#ifdef POSTSCRIPT
	ps_put_thick_line(save_h[bloc], save_v[bloc], save_h[eloc], save_v[eloc]);
#endif /* POSTSCRIPT */
    }
    else if (x) {
#ifdef X_WIN
	x_put_thick_line(save_h[bloc], save_v[bloc], save_h[eloc], save_v[eloc]);
#endif /* X_WIN */
    }

}

put_slant(bloc, eloc)
int bloc, eloc;			/* OUTPUT */
{
    int length, height, inter;
    unsigned char cc;
    int c_h, c_v;
    extern char interspace[];
    int i_space = str_to_dvi(interspace);
    int i_ratio;

    saveloc(REGS-1);

    length = (save_h[eloc] - save_h[bloc]);
    height = (save_v[eloc] - save_v[bloc]); 

    if (!length) {
	printf ("tab: put_slant: null length!\n");
	put_a_char(0);
	put_a_char('Q');
	return(-1);
    }


    i_ratio = height * 256 / length;
/* 
    printf ("  i_ratio %d length %d height %d\n", 
	    i_ratio, length, height);  */

    if (ps ) {
#ifdef POSTSCRIPT
	ps_put_line(save_h[bloc], save_v[bloc], save_h[eloc], save_v[eloc]);
#endif /* POSTSCRIPT */
    }
    else if (x) {
#ifdef X_WIN
	x_put_line(save_h[bloc], save_v[bloc], save_h[eloc], save_v[eloc]);
#endif /* X_WIN */
    }
#ifdef DVI
    else {
	if (i_ratio > 10){
	    cc = 3;
	    p_movev(i_space);
	}
	else if ( i_ratio > -40 ) { /* no slope */
	    dvi_command(PUT_RULE, str_to_dvi("0.005 in"),
			(save_h[eloc] - save_h[bloc]));
	    return(0);
	}
	else if (i_ratio > -100 ) cc = 4; /* shallow upwards */
	else cc = 1;		/* steep upwards */
	
	c_h = p_get_w(0, cc);
	c_v = p_get_h(0, cc); 

/*	printf ("tab: first put_slant: c_h %d c_v %d %X\n",
		c_h, c_v, c_v);
 */
	
	if (c_h == 0 || c_v == 0) {
	    printf("tab: put_slant: bad font char width or height %d\n", cc);
	    return (-1);
	}
	if (height < 0) c_v = - c_v;


	while (length > c_h) {
	    put_a_char((char)cc);
/*	    printf ("tab: put_slant: c_h %d c_v %d c_v %X\n",
		    c_h, c_v, c_v); */
	    p_moveh(c_h);
	    p_movev(c_v); 
	    length -= c_h;
	}
	inter = length - c_h;
	p_moveh(inter);
	p_movev(inter * (((c_v*64 )/ c_h)) / 64 );

	put_a_char((char)cc); 
    }

#endif /* DVI */
    getloc(REGS-1);
    return(0);
}
showloc()			/* showcurrent location */
{   printf ("tab: showloc: hor %d vert %d\n", dvi_h, dvi_v);}

void
get_current_loc(h, v)
     int *h, *v;
{    *h = dvi_h; *v = dvi_v; }

more()				/* return 1 if room for */
{				/* another system on page */
/*   extern double staff_space[]; */

/*    printf ("tab: more: dvi_v %d\n", dvi_v);   */

    if (ps) {
	if (inch_to_dvi(10.50 - 8.8) > dvi_v) /* length of page */
	  return(0);
	else 
	  return(1);
    }
    else {
#ifdef DVI
	if (dvi_v  > inch_to_dvi(8.8 / red))
	  return(0);
	else 
	  return(1);
#endif /* DVI */
    }
}

showsave(reg)			/* show a saved "register" */
     int reg;
{
    if (reg >= REGS) 
      printf ("tab: showsave: illegal register %d\n", reg);
    printf ("tab: showsave: %d %d\n", save_h[reg], save_v[reg]);
}

pushloc()			/* push and pop positions */
{				/* probably better to use */
    if (ps | x) {
	push();			/* PostScript must have a push */
    }
    else {
#ifdef DVI
	pushloc_prim();
#endif /* DVI */
    }
}

poploc()
{
    if (ps | x) pop(); 
    else {
#ifdef DVI
	poploc_prim();
#endif /* DVI */
    }
}

/* called from primative - if called from a move buffers move
   so only one move occurs between characters - if called from
   something else, write out move and return 
 */

#define N_S 5
p_num(n, f)			/* format and print a decimal integer */
     int n;			/* only useful for bar numbers */
     struct file_info *f;
{
    char string[N_S];
    int i;
    char c;

    sprintf(string, "%d", n);
    push();
    movev("0.18 in");
	if (f->flags & PS) use_font(1);
    else use_font(2);
    for (i=0; i < N_S && string[i]; i++){
	c = /* 140 + */ string[i] /* - '0' */;
#ifdef DVI
	if (ps | x) set_a_char(c);
	else 
	  {
	      put_a_char(c);
	      p_moveh(p_get_w(2, c));
	  }
#else
	set_a_char(c);
#endif /* DVI */
    }
    use_font(0);
    pop();
}

ps_draft() {ps_command(PDRAFT, 0, 0, 0, 0);}

ps_copyright() {ps_command(PCOPYRIGHT, 0, 0, 0, 0);}

#ifdef POSTSCRIPT


double d_to_p(n)     int n;
{    return ((double)n * 0.00000762);}

#ifdef POSTSCRIPT

/* 10 inches = 1000 ~ inches with 7 zeros */
int ps_top_of_page = 9867 * 10 * 1000;	

ps_init_hv()
{
    dvi_h = 0;
    dvi_v = ps_top_of_page;
    ps_command(INIT, dvi_h, dvi_v, 0, 0);
}

#endif /* POSTSCRIPT */

ps_put_rule(h, v)
     int h, v;
{
    ps_command(RULE, h, v, 0, 0);
}
ps_put_line(bx, by, ex, ey)
     int bx, by, ex, ey;
{
    ps_command(LINE, bx, by, ex, ey);
}

ps_put_thick_line(bx, by, ex, ey)
     int bx, by, ex, ey;
{
    ps_command(TH_LINE, bx, by, ex, ey);
}

ps_moveh(h)
    int h;
{
    ps_command(MOVEH, h, 0, 0, 0);
    dvi_h += h;
}

ps_movev(v)
    int v;	
{
    ps_command(MOVEV, 0, v, 0, 0);
    dvi_v -= v;
}

ps_movehv(h, v)
     int h, v;
{
    ps_command(MOVEVH, h, v,0 ,0);
    dvi_h += h;
    dvi_v -= v;
}

ps_put_char(c)			/* only works for fonts with no x width */
unsigned char c;
{
    ps_used[c]++;
    ps_command(PCHAR, (int)c, 0, 0,0);
}

ps_set_char(c) 
     unsigned char c; 
{ 
    ps_used[(int)c]++;
    ps_command(CHAR, (int)c, 0, 0,0);
}

ps_put_gray(c)			/* only works for fonts with no x width */
unsigned char c;
{
    ps_used[c]++;
    ps_command(PGRAY, (int)c, 0,0 ,0);
}

ps_set_gray()
{
    ps_command(P_S_GRAY, 0, 0, 0, 0);
}

ps_unset_gray()
{
    ps_command(P_U_GRAY, 0, 0, 0, 0);
}

ps_setfont(fontnum) 
     int fontnum;
{ 
    curfont = fontnum;
    switch (fontnum) {
      case 5:			/* font 5 10 pt italic */
	printf ("setting ital\n");
	ps_command( ITAL, 0, 0, 0, 0);
	break;
      case 4:			/* font 4 med */
	ps_command( MED, 0, 0, 0, 0);
	break;
      case 3:			/* font 3 big */
	ps_command( BIG, 0, 0, 0, 0);
	break;
      case 2:			/* font 2 */
	ps_command( TIMES, 0, 0, 0, 0);
	break;
      case 1:			/* font 1 */
	ps_command( SMALL, 0, 0, 0, 0);
	break;
      case 0:
	ps_command( LUTE, 0, 0, 0, 0);
	break;
      default:
	printf ("tab: ps_setfont: undefined font numbewr $d, using 2\n");
	ps_command( TIMES, 0, 0, 0, 0);
	break;
    }
}
ps_setLute() {
    ps_command( LUTE, 0, 0, 0, 0);
}

ps_tie(length) int length;{
    ps_command ( PTIE, length, 0, 0, 0);
}

static int last_com = OTHER;
static int last_font= OTHER;

void 
ps_command(com, h_n, v_n, hh_n, vv_n)
     int com, h_n, v_n, hh_n, vv_n;
{
    static int h, v, hh, vv;
    static int dvi_oh, dvi_ov;
    extern char *ps_font[80];
    time_t timeval;

    if (v_n == 189451468)
      printf ("ps_command %d \n", v_n);

    if (com == last_com) {
	switch (com) {
	  case MOVEH:
	    h += h_n;
	    return;
	  case MOVEV:
	    v += v_n;
	    return;
	  case GLP:
	    h = h_n;
	    v = v_n;
	    return;
	}
    }

    if ( last_com == GLP ) {
	switch (com) {
	  case MOVEH:
	    h += h_n;
	    return;
	  case MOVEV:
	    v -= v_n;
	    return;
	  default:
	    break;
	}
    }

    if (com == GLP) {
	switch (last_com) {
	  case RULE:
	    fprintf (tp, "%.2f %.2f Rule %%GLP\n", 
		     d_to_p(v), d_to_p(h));
	    h = h_n;
	    v = v_n;
	    last_com = GLP;
	    return;
	  case MOVEV:		/* throw the relative move away */
	  case MOVEH:
	    h = h_n;
	    v = v_n;
	    last_com = GLP;	    
	    return;
	  default:
	    break;
	}
    }

    switch (last_com) {
      case INIT:
	fprintf (tp, "%.2f %.2f MT %% INIT\n", d_to_p(h), d_to_p(v));    
	break;
      case RULE:
	fprintf (tp, "%.2f %.2f Rule%%reg\n", d_to_p(v), d_to_p(h));
	fprintf (tp, "%.2f %.2f MT\n", d_to_p(dvi_oh), d_to_p(dvi_ov));
	break;
      case MOVEH:
	fprintf (tp, "%.2f 0 RM\n", d_to_p(h));
	break;
      case MOVEV:
	fprintf (tp, "0 %.2f RM\n", d_to_p(-v));
	break;
      case MOVEVH:
	fprintf (tp, "%.2f %.2f RM\n", d_to_p(h), d_to_p(-v));
	break;
      case GLP:
	fprintf (tp, "%.2f %.2f MT\n", d_to_p(h), d_to_p(v)); 
	break;
      case LINE:
	fprintf (tp, "0.2 setlinewidth %.2f %.2f MT %.2f %.2f lineto stroke\n",
		 d_to_p(h), d_to_p(v), d_to_p(hh), d_to_p(vv));
	break;
      case TH_LINE:
	fprintf (tp, "%.2f  %.2f 0.9 sub moveto 0.0 1.8\n", 
		 d_to_p(h), d_to_p(v));
	fprintf (tp, "rlineto %.2f %.2f 0.9 add lineto 0.0 -1.8 rlineto closepath fill\n",
		 d_to_p(hh), d_to_p(vv ));
	break;
      case CHAR:
#ifndef VAX
	if ( h == 050 || h == 051 || h == '\\') 
	  fprintf(tp, "(\\%c)S\n", h);
	else if (my_isprint(h)) fprintf(tp, "(%c)S\n", h);
	else fprintf(tp, "<%02X>S\n", h);
#else
	fprintf(tp, "<%02X> S\n", h);
#endif				/* VAX */
	break;
      case PCHAR:
#ifndef VAX
	if ( h == 050 || h == 051 || h == '\\') 
	  fprintf(tp, "(\\%c)X\n", h);
	else if (my_isprint(h)) fprintf(tp, "(%c)X\n", h);
	else fprintf(tp, "<%02X>X\n", h);
#else
	fprintf(tp, "<%02X>X\n", h);
#endif				/* VAX */
	break;
#ifndef VAX
      case PGRAY:
	fprintf(tp, "gsave  0.6 setgray\n");
	if ( h == 050 || h == 051 || h == '\\') 
	  fprintf(tp, "(\\%c)X\n", h);
	else 
	  if (my_isprint(h)) fprintf(tp, "(%c)X\n", h);
	else 
	  fprintf(tp, "<%02X>X\n", h);
	fprintf(tp, "grestore\n");
		break;
#endif /* VAX */
      case LUTE:
	if ( last_font != LUTE ) {
	   fprintf (tp, "/LuteFont FF setfont\n"); 
	   last_font = LUTE;
	}
	break;
      case SMALL:
	fprintf (tp, "/%s FF %.2f SF\n", 
		ps_font[1], PS_SCALE_1 * 10.0 * red);
	last_font = SMALL;
	break;
      case TIMES:
	fprintf (tp, "/%s FF %.2f SF\n", 
		ps_font[2], PS_SCALE_2 * 10.0 * red);
	last_font = TIMES;
	break;
      case BIG:
	fprintf (tp, "/%s FF %.2f SF\n", 
		ps_font[3], 24.0 * red); /* was 28 */
	last_font = BIG;
	break;
      case MED:
	fprintf (tp, "/%s FF %.2f SF\n", 
		ps_font[4], 1.25 * PS_SCALE * red);
	last_font = MED;
	break;
      case ITAL:
	printf("prim.c: setting italic\n");
	fprintf (tp, "/%s FF %.2f SF\n", 
		ps_font[5], 10.0 * red);
	last_font = BIG;
	break;
      case PS_CLIP:
	fprintf (tp, "gsave\n");
	fprintf (tp, "currentpoint  /yval exch def /xval exch def pstack\n");
	fprintf (tp, "(%c) false charpath clip\n", h); 
#ifdef OLD_PRINTER
	fprintf (tp, "newpath\n");
	fprintf (tp, "xval %.2f moveto ", d_to_p(vv));      /* x, y */
	fprintf (tp, "(%c) stringwidth pop 0 rlineto 0 %f rlineto\n",
		 h, 0.73 * d_to_p(v));
	fprintf (tp, "(%c) stringwidth pop neg 0 rlineto\n", h);
	fprintf (tp, "closepath fill\n");
#else /* OLD_PRINTER */
	fprintf (tp, "xval %.2f ", d_to_p(vv));      /* x, y */
	fprintf (tp, "(%c) stringwidth pop %f rectfill\n", 
		 h, 0.73 * d_to_p(v));
#endif /* OLD_PRINTER */
	fprintf (tp, "grestore\n");
	fprintf (tp, " (%c) stringwidth pop 0 rmoveto\n", h);
	break;
      case PTIE:
	fprintf (tp, "%.2f doslur\n", d_to_p(h));
	break;
      case PDRAFT :
	fprintf (tp, "gsave\n");
	fprintf (tp, "/Times-Roman findfont dup 150 scalefont setfont\n");
 	fprintf (tp, "0.90 setgray \n"); 
	fprintf (tp, "25 650 moveto \n");
	fprintf (tp, "[.7 -.7 .7 .7 0 0] concat\n");
	fprintf (tp, "(DRAFT) show\n");
	timeval = time((time_t *)0);
	fprintf (tp, "25 scalefont setfont -330 425 moveto (%s) show\n",
		 ctime(&timeval));
	fprintf (tp, "grestore \n");
	break;
      case PCOPYRIGHT:
	fprintf (tp, "gsave\n");
 	fprintf (tp, "/Times-Roman findfont \n");
	fprintf (tp, "dup length dict begin\n");
	fprintf (tp, "  {1 index /FID ne {def} {pop pop} ifelse} forall\n");
	fprintf (tp, "  /Encoding ISOLatin1Encoding def\n");
	fprintf (tp, "currentdict end\n");
	fprintf (tp, "/Times-Roman-ISO exch definefont pop\n");
	fprintf (tp, "/Times-Roman-ISO findfont\n");
	fprintf (tp, "12 scalefont setfont\n");
	fprintf (tp, "0 -520 rmoveto\n");
	fprintf (tp, "( \251 ) show\n");
	fprintf (tp, "(Copyright 1994 Frank Wallace)show\n");
	fprintf (tp, "grestore \n");
      case P_S_GRAY:
	fprintf(tp, "0.6 setgray\n");
	break;
      case P_U_GRAY:
	fprintf(tp, "0.0 setgray\n");
	break;
      case OTHER:
	break;
      default:
	printf ("tab: ps_command: unknown command %d\n", last_com);
	break;
    }
    
    last_com = com;
    h = h_n; v = v_n; 
    hh = hh_n; vv = vv_n;
    dvi_oh = dvi_h; dvi_ov = dvi_v;
    return;
}
my_isprint(c) int c;
{
    if ( c < 0 )
      printf ( "here\n");

  if (c < 33 ) return (0);
  if (c > 126 ) return (0);
  return (1);
}
#endif /* POSTSCRIPT */

#ifdef DVI
static int o_com=NO_OP;

void 
  dvi_command(com, val1, val2) /* com, val */
int com, val1, val2;
{
    static int o_val1, o_val2;

    if (com == DOWN4) {
	switch (o_com) {
	  case DOWN3:
	  case DOWN4:
	    o_val1 += val1;
	    o_com = com;
	    return;
	  default:
	    break;
	}
    }
    else if (com == DOWN3 ) {
	switch (o_com) {
	  case DOWN3:
	  case DOWN4:
	    o_val1 += val1;
	    o_com = DOWN4;
	    return;
	  default:
	    break;
	}
    }
    if (com == RIGHT4) {
	switch (o_com) {
	  case RIGHT3:
	  case RIGHT4:
	    o_val1 += val1;
	    o_com = com;
	    return;
	  default:
	    break;
	}
    }

    switch (o_com) {
      case RIGHT4:
	put_byte((unsigned char)RIGHT4);
	signed_quad(o_val1);
	break;
      case RIGHT3:
	put_byte((unsigned char)RIGHT3);
	signed_trio(o_val1);
	break;
      case RIGHT2:
	put_byte((unsigned char)RIGHT2);
	signed_pair(o_val1);
	break;
      case RIGHT1:
	put_byte((unsigned char)RIGHT1);
	signed_byte(o_val1);
	break;
      case DOWN4:
	put_byte((unsigned char)DOWN4);
	signed_quad(o_val1);
	break;
      case DOWN3:
	put_byte((unsigned char)DOWN3);
	signed_trio(o_val1);
	break;
      case DOWN2:
	put_byte((unsigned char)DOWN2);
	signed_pair(o_val1);
	break;
      case DOWN1:
	put_byte((unsigned char)DOWN1);
	signed_byte(o_val1);
	break;
      case PUT_RULE:
	put_byte((unsigned char)PUT_RULE); signed_quad(o_val1); signed_quad(o_val2);
	break;
      case PUT1:
	put_byte((unsigned char)PUT1); put_byte(o_val1);
	break;
      case SET1:
	put_byte((unsigned char)SET1);
	put_byte(o_val1);
	break;
      case FNT1:
	put_byte((unsigned char)FNT1);
	put_byte(o_val1);
	break;
      case PUSH:
	put_byte((unsigned char)PUSH);
	break;
      case POP:
	put_byte((unsigned char)POP);
	break;
      case EOP:			/* this should flush */
	put_byte((unsigned char)EOP);
	break;
      case NO_OP:
	put_byte((unsigned char)NO_OP);
	break;
      default:
	printf("tab: dvi_command: undefined command %d\n", com);
	break;
    }

    o_val1 = val1;
    o_val2 = val2;
    o_com = com;

}
#endif/*  DVI */

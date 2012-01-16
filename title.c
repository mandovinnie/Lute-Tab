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
#include "tab.h" 

extern int line;
extern char staff_len[];
extern char flag_to_staff[];
static int font = 2;


#define EN 'I'

char
  format_title(f)			/* get text between curly braces */
struct file_info *f;
{
    char c, cc;
    double get_width();
    double cur = 0.0;		/* current position */
    double en = get_width(font, EN);
    pushloc();
  
    while ((c = geti()) != (char)EOF) { /* get the text */
	switch (c) {
	  case ' ':
	  case '\n':
	    fmoveh(en);
	    cur += en;
/*	    printf ("title: get width font %d EN %d %f\n",
		    font, EN, en);
 */
	    break;
	  case '/':		/* lots of hfill */
	    if ((c = geti()) != '/' && c != (char)EOF) {
		struct words lbuf[120];
		int lbp=0, i;
		double total=0.0;

		while ( c != (char)EOF && lbp < sizeof(lbuf)) {
		    if (c == '}') break;
		    lbuf[lbp].let = c;
		    switch (c) {
		      case ' ':
		      case '\t':
			lbuf[lbp].width = en;
			lbuf[lbp].let = ' ';
			break;
		      case '<':
		      case '%':
		      case '>':
		      case '[':
		      case ']':
		      case '|':
			  case '\\':
			lbuf[lbp].width = 0;
			break;
		      case '(':	/* upside down ! */
			lbuf[lbp].width = get_width(font, 161);
			break;
		      case ')':	/* upside down ? */
			lbuf[lbp].width = get_width(font, 191);
			break;
		      default:
			lbuf[lbp].width = get_width(font, c);
			break;
		    }
		    total += lbuf[lbp].width;
		    lbp++;
		    c = geti();
		}

		fmoveh ( str_to_inch (staff_len) - total );
		fmoveh ( -1.0 * cur );
		for (i=0; i< lbp; i++) {
		    switch (lbuf[i].let) {
		      case ' ':
			fmoveh(en);
			break;
		      case '<':
		      case '%':
		      case '>':
		      case '[':
		      case '\\':
			do_special(lbuf[i].let, lbuf[i+1].let, font, f);
			i++;
			break;		       
		      case ')':
		      case '(':
			do_sp(c, font, f);
			break;
		      default:
			set_a_char(lbuf[i].let);
			break;
		    }
		}
		goto done;
	    }
	    else set_a_char ('/');
	    break;
	  case '}':
	    goto done;
	  case '<':
	  case '%':
	  case '>':
	  case '[':
	  case ']':
	  case '\\':
	    do_special(c, cc=geti(), font, f);
	    cur += get_width(font, cc);
/*	    ungeti(cc);*/
	    break;
	  case '(':
	    do_sp(c, font, f);
	    cur += get_width(font, 161);
	    break;
	  case ')':
	    do_sp(c, font, f);
	    cur += get_width(font, 191);
	    break;
	  default:
	    set_a_char(c);
	    cur += get_width(font, c);
	    break;
	}
/*	printf ("title: c %c cur %f\n", c, cur); */
    }
  done:
    if (c != (char)EOF) {
	while ((c = geti()) != '\n') /* swallow line */
	  ;
    }
    poploc(); 
    return (c);
}

dotitle(f)
     struct file_info *f;
{
    extern int n_measures;	/* in dvi_f, reset measure number */
    geti();			/* throw away first { */

    n_measures = 0;

    if (f->flags & PS) {
#ifdef POSTSCRIPT
	ps_setfont(font);
#endif 
    }
    else {
	use_font(font); 
    }
    format_title(f); 
    if (f->flags & MARKS)
      put_rule("0.25 in", "0.02 in");
}

get_misc(flag, f)			/* get text from input file */
     int flag;
     struct file_info *f;
{
    char c;
    int i;
    switch (flag) {
      case TITLE:
	while ((c = get()) != '}') { /* get the text */
	    puti(c);
	}
	puti('}'); puti('\n');
	break;
      case ARGS:
	i=0;
	while ((c = get()) != '\n') {
	    puti(c);
	}
	puti('\n');
	break;
      default:
	printf("tab: bad flag in get_misc\n");
	break;
    }
    while ((c=get()) != '\n');	/* flush the line */
    line++;
}

do_special(c, next, font, f)	/* sets an accent and the character following */
     unsigned char c, next;
     int font;
     struct file_info *f;
{
    double res;

    if ( ! next && strchr("<>[]:", c) ) {
	printf ("tab: do_special: special char with nothing after it %c\n",
		c);
    }

    switch (c) {
      case '<':			/* slash */
	if (f->flags & PS ) {
	    res = (get_width(font, next)
		   - get_width(font, 0302)) / 3.0;
	    fmoveh(res);
	    put_a_char(0302);
	    fmoveh( -1 * res);
	}
	else {
	    push();
	    fmoveh((get_width(font, next)
		    - get_width(font, 0023)) / 3.0 );
	    set_a_char(0023);
	    pop();
	}
	break;
      case '%':			/* back slash */
	if (f->flags & PS ) {
	    res = (get_width(font, next)
		   - get_width(font, 0301)) / 3.0;
	    fmoveh(res);
	    put_a_char(0301);
	    fmoveh( -1 * res);
	}
	else {
	    push();
	    fmoveh((get_width(font, next)
		    - get_width(font, 0022)) / 3.0 );
	    set_a_char(0022);
	    pop();
	}
	break;
      case '>':			/* tilde above */
	if (f->flags & PS ) {
	    res =(get_width(font, next)
		  - get_width(font, 0304)) / 2.0; 
	    fmoveh(res); 
	    put_a_char(0304);
	    fmoveh(- res); 
	}
	else {
	    push();
	    fmoveh((get_width(font, next)
		    - get_width(font, 0176)) / 2.0 );
	    set_a_char(0176);
	    pop();
	}
	break;
      case '[':			/* squiggle below */
	if (f->flags & PS ) {
	    res = (get_width(font, next)
		   - get_width(font, 0313)) / 2.0;
	    fmoveh(res);
	    put_a_char(0313);
	    fmoveh( - res);
	}
	else {
	    push();
	    /* not in dvi
	       fmoveh((get_width(font, next)
	       - get_width(font, 0313)) / 2.0 );
	       put_a_char(0313);
	       */
	    pop();
	}
	break;
      case ':':			/* two dots above */
	/*printf ("tab: two dots\n"); */
	if (f->flags & PS ) {
	    res = (get_width(font, next)
		   - get_width(font, 0310)) / 2.0;
	    fmoveh(res);
	    put_a_char(0310);
	    fmoveh( - res);
	}
	else {
	    push();
	       fmoveh((get_width(font, next)
	       - get_width(font, 127)) / 2.0 );
	       put_a_char(127);
	    pop();
	}
	break;
      case ']':			/* word tie */
	use_font(0);
	if (f->flags & PS ) {
	    set_a_char(0201);
	}
	else {
	    set_a_char(0201);
	}
	use_font(font);
	break;
      case '\\':			/* squiggle below */
	break;
      default:
	break;
    }
    if (next == 'i') {
	if (f->flags & PS) {
#ifdef POSTSCRIPT
	    ps_clipped_i(next, font);
#endif /* POSTSCRIPT */
	}
	else set_a_char(next);
    }
    else set_a_char(next);
}

do_sp(c, font, f)
     unsigned char c;
     int font;
     struct file_info *f;
{
    switch (c) {
      case '(':			/* upside down ! */
	if (f->flags & PS) {
	    set_a_char(161);	/* 0241 */
	}
	else {
	    set_a_char(60);	/* 074 */
	}
	break;
      case ')':			/* upside down ? */
	if (f->flags & PS) {
	    set_a_char(191);	/* 0277 */
	}
	else {
	    set_a_char(62);	/* 076 */
	}
	break;
      case '@':			/*  ./. */
	use_font(0);
	set_a_char(57);
	use_font(font);
	break;
      default:
	break;
    }
}

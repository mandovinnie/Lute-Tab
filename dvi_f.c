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
 * drawing and typesetting stuff
 *
 */
#include <stdio.h>
#include <setjmp.h>
#include "tab.h"

/* these are the basic design sizes for the text */
/* note that a few parameters for grid and underscore are in prim.c */

char interspace[]="10 pt";	  /* distance between staff line was .14 in */
char staff_len[] = "6.5 in            ";     /* length of line horizontally */
char staff_height[] = "0.0037 in";/* 55 */       /* thickness of staff line */
/* 0033 is light, 007 works on NeXT previewer */
char baselinespace[] = "-0.02 in";          /* was .020 char above the line */
char grid_indent[] = "0.033 in";	       /* hor. offset of grid staff */
char italian_offset[] = "4.5 pt";          /* shift staff for italain style */
char em[] = "7.0 pt";		            /* basic character design width */
char flag_to_staff[] = "4 pt";            /* flags above imaginary top line */
char thick_bar[] = "0.024 in";	                           /* for thick bar */
double text_sp = 0.18;/* in */	           /* padding for text below music  */
double st_text = .32; /* in */		      /* basic staff to music space */
int st_italian=0;		          /* one if high italian flags used */
double m_space = .1;  /* in */                /* interline spacing in music */
extern int n_system;	                          /* what system are we in? */
extern void underline();
extern int min_d_w[];
extern int min_O_w[];
static int grid_flag=0;

int n_measures=0;

dvi_format(l_p, j, f, l)
int *l_p;			/* characters in line */
int j;				/* number of this character */
struct file_info *f;		/* general flags, etc */
struct list *l;			/* data */
{
    extern double str_to_inch(), atof();
    unsigned int c;
    int i, cc, ccc;
    int skip_spaces;		/* spaces to skip */
    static double last_move;	/* last interchar space, for post orn. */
    double staff_h = str_to_inch(staff_height);
    double d_i_space = str_to_inch(interspace);
    int i_space = inch_to_dvi(d_i_space);
    double EM = str_to_inch(em);
    int last_line_in_grid = 0;
    char *ch;			/* in this case, the chord to print */
    char *nxt;			/* the next chord */
    struct list *p;
    static int double_grid=0;
    extern char flag_indent[];
    double val;
    int font;

    if (l) ch = l->dat;
    else return;

    if (l->next) nxt = l->next->dat;

    saveloc(1);
    
    use_font(0);

    if (f->flags & MARKS ) f_put_rule(.1, .01);

/*    printf ("dvi_f: ch %s\n", ch);  */

    switch (c = (unsigned int)*ch) {
      case 'b':			/* barline */
	push();
	val=5.0;
	if (f->flags & FIVE) {
	    p_movev(-i_space); 
	    val = 4.0;
	}
	if (f->flags & CON_SEV ) 
	  p_movev (i_space);
	if (f->line_flag == ON_LINE)
	  p_movev((int)(5.5 * i_space));
	else p_movev(6 * i_space);
	if (ch[1] != '!') {
	    if (ch[1] == 'Q' ) 
	      highlight++;
	    if (j == 0 || j == (*l_p - 1)) {
		f_put_rule (1.3 * staff_h,
			    val * d_i_space 
			    + staff_h);

		if (f->flags & NOTES && f->flags & LSA_FORM) {
		    double v_bar;
		    v_bar = 6 * d_i_space
		      + st_text + text_sp * f->n_text 
			+ f->c_space + 5 * m_space;
		    f_put_rule(staff_h, v_bar);
		}
	    }
	    else {
		f_put_rule (staff_h,
			    val * d_i_space 
			    + staff_h);
	    }
	}
	if (bar_count || barCount) {
	    static int firstb;
	    if (j == 0) firstb = 1;
	    if (j+1 != *l_p && *ch == 'b'
		&& *nxt != 'b'
		&& *nxt != '.'
		&& *nxt != 'Q'
		&& *nxt != 'q'
		) {	
		if (ch[1] != '!') {
		    n_measures++;	                
		    if ((bar_count && ! (n_measures % 5)) || 
			(firstb && barCount)) {
			p_num(n_measures, f);
			firstb = 0;
		    }
/*		    push();
		    fmovev(.2);
		    p_num(n_measures, f);
		    pop();
 */
		}
	    }
	}
	if (ch[1] == 'T') {
	    push();
	    fmoveh(-0.5 * get_width(0, 128));
	    fmovev(-6.7 * d_i_space);
	    put_a_char(128);
	    pop();
	}


	pop();
	if (f->flags & NOTES )	/* print a barline in the music */
	  if (! (f->flags & NO_MUSIC_B)
	      || j == 0 
	      || ( j== 1 && l->prev->dat[0] == '8')
	      || j == *l_p - 1
	      || l->next->dat[0] == 'b'
	      || l->next->dat[0] == 'B'
	      || l->next->dat[0] == '.'
	      || l->prev->dat[0] == 'b'
	      || l->prev->dat[0] == '.') {
	      push();		/* at bottom of tab staff */
	      
	      fmovev ( -st_text -text_sp * f->n_text -m_space
		      - f->c_space);
	      
	      if (j == 0 || j == *l_p -1) {
		  f_put_rule (1.3 * staff_h,
			  4 * m_space 
			  + staff_h);
	      }
	      else {
		  f_put_rule (staff_h,
			  4 * m_space 
			  + staff_h);
	      }
	      pop();
	  }
	break;
      case 'B':			/* thick barline */
	push();
	val=5.0;
	if (f->flags & FIVE) {
	    p_movev(-i_space); 
	    val = 4.0;
	}
	if (f->flags & CON_SEV ) 
	  p_movev (i_space);
	if (f->line_flag == ON_LINE) 
	  p_movev((int)(5.5 * i_space));
	else p_movev(6 * i_space);
	
	if ((j == 0 || j == *l_p - 1)
	    && f->flags & NOTES && f->flags & LSA_FORM) {
	    double v_bar;
	    v_bar = 6 * d_i_space
	      + st_text + text_sp * f->n_text 
		+ f->c_space + 5 * m_space;
	    f_put_rule(str_to_inch(thick_bar), v_bar);
	}
	else {
	    f_put_rule (str_to_inch(thick_bar),
			val * d_i_space + staff_h);
	}
	pop();
	if (f->flags & NOTES) {
	    push();
	    
	    fmovev ( -st_text -text_sp * f->n_text -m_space 
		    - f->c_space);
	      
	    f_put_rule (str_to_inch(thick_bar),
			4 * m_space 
			+ staff_h);
	    pop();
	}
    
      case 'i':			/* indent as for barline - no line*/
	break;
      case '.':
	movev(baselinespace);
	if (f->flags & CON_SEV ) 
	  p_movev (i_space);
	if (f->line_flag == ON_LINE) 
	  p_movev((int)(1.5 * i_space));
	else p_movev(2 * i_space);

	for (i=0; i <5;i++) {
	    mapchar('Z', f);
	    p_movev(i_space);
	}
	if (f->flags & NOTES ) {

	    if (f->line_flag == ON_LINE) fmovev( -5.5 * d_i_space);
	    else fmovev ( -6.0 * d_i_space);     /* to top of staff */

	    fmovev ( -st_text);
	    fmovev ( -text_sp * f->n_text - f->c_space);
	    fmovev ( -3 * m_space - .01 /* fudge */);
	    mapchar('Z', f);
	    fmovev( -m_space);
	    mapchar('Z', f);
	}
	break;
      case 'd':
	fmovev ( -1.0 * str_to_inch("4.0 pt"));
	put_a_char(254);
	if (ch[1] == 'b') {	/* make a bar line */
	    fmoveh(get_width(0, 254) / 2.0);
	    fmovev(6 * d_i_space + str_to_inch("4.0 pt"));
	    f_put_rule (staff_h,
			5.0 * d_i_space 
			+ staff_h);
	}
	break;
      case 'U':
      case 'u':
      case 'R':
      case 'S':
      case 'T':
      case 'V':
	if (f->flags & NOTES) {
		push();

		fmovev ( -st_text -text_sp * f->n_text -m_space );
		fmovev ( - f->c_space);
		fmovev (-2.0 * m_space);
		fmovev (get_height(0, c) / 2.0);
		put_a_char(c);
		pop();
	}
	push();
	if (f->flags & CON_SEV ) 
	  p_movev (i_space);
	if (f->line_flag == ON_LINE) 
	  fmovev(3. * d_i_space);
	else 
	  fmovev(3.5 * d_i_space);

	if ( c == 'U') c = 19;
	else if (c == 'u') c = 20;

	fmovev(get_height(0, c) / 2.0 );
	put_a_char(c);
	
	pop();
	break;
      case 'Q':			/* squiggle */
      case 'q':
	fmovev( 5.9 * d_i_space);
	if (f->flags & CON_SEV ) 
	  p_movev (i_space);
	if (f->line_flag == ON_LINE)
		p_movev (i_space/-2);
	put_a_char(c);
	break;
      case '=':			/* W. = W */
	if (f->flags & NOTES) {
	    int curfont = get_font();
	    push();
	    if (f->line_flag == ON_LINE) fmovev (- 5.5 * d_i_space);
	    else fmovev (- 6.0 * d_i_space);
	    fmovev ( -5 * m_space);
	    fmovev ( st_text);
	    fmovev ( -text_sp * f->n_text);
	    fmovev ( - f->c_space);
	    use_font(0);
	    for (i=1; i<STAFF; i++) {
		if (strchr ("=()", ch[i])) {
		    use_font(1);
		    set_a_char(ch[i]);
		    use_font(0);
		}
		else if (ch[i] == '.'){
		    fmovev(.021);
		    set_a_char(ch[i]);
		    if (! (f->flags & PS ))
		      fmoveh(-.5 * get_width(curfont, '.'));
		    fmovev(-0.021);
		}
		else if (ch[i] == 'W'){
		    if (f->num_flag == ITAL_NUM) set_a_char(216);
		    else set_a_char(ch[i]);
		}
		else { 
		    set_a_char(ch[i]);
		}
	    }
	    use_font(curfont);
	    pop();
	}
	break;
      case 'f':
	push();
	if (f->flags & PS ) use_font(1);
	else use_font(2);
	set_a_char('F'); set_a_char('i'); set_a_char('n');
	if (f->flags & NOTES ){
	    pop();
	    push();
	    if (f->line_flag == ON_LINE) fmovev (- 5.5 * d_i_space);
	    else fmovev (- 6.0 * d_i_space);
	    fmovev ( -5 * m_space);
	    fmovev ( st_text);
	    fmovev ( -text_sp * f->n_text);
	    fmovev ( - f->c_space);
	    set_a_char('F'); set_a_char('i'); set_a_char('n');
	}
	pop();
	break;
      case 'F':			/* change fonts */
	if (f->flags & PS) change_ps_font(l->special);
	break;
      case 'D':			/* D.C. da-capo */
	push();
	if (f->flags & PS ) use_font(1);
	else use_font(2);
	set_a_char('D'); set_a_char('.'); set_a_char('C'); set_a_char('.');
	if (f->flags & NOTES ){
	    pop();
	    push();
	    if (f->line_flag == ON_LINE) fmovev (- 5.5 * d_i_space);
	    else fmovev (- 6.0 * d_i_space);
	    fmovev ( -5 * m_space);
	    fmovev ( st_text);
	    fmovev ( - f->c_space);
	    fmovev ( -text_sp * f->n_text);
	    set_a_char('D'); set_a_char('.'); 
	    set_a_char('C'); set_a_char('.');
	}
	pop();
	break;
      case '8':			/* Octave - *va */
	push();
	if (f->flags & PS ) use_font(1);
	else use_font(2);
	if (f->flags & NOTES ){
	    printf ("tab: dvi_f: octave\n");
	    pop();
	    push();
	    if (f->line_flag == ON_LINE) fmovev (- 5.5 * d_i_space);
	    else fmovev (- 6.0 * d_i_space);
	    fmovev (d_i_space);
	    fmovev ( st_text);
	    fmovev ( - f->c_space);
	    fmovev ( -text_sp * f->n_text); 
	    fmovev ( - 5.5 * m_space );
	    set_a_char('8'); set_a_char('v'); 
	    set_a_char('b');
	}
	pop();
	break;
      case 'G':			/* time signature */
	push();			/* better be at the beginning of line! */
	if (ch[1] == 'M') {	/* g cleff and notes*/
	}
	else if (isdigit(ch[1])) {
	    if ( j == 0 )	  /* before staff, Karen Meyers style */
	      fmoveh( -1 * str_to_inch("0.28 in"));

	    if (f->flags & PS) font = 4;
	    else font = 2;
	    use_font(font);

	    if (strchr (ch, '-') && (f->flags & NOTES)) {
		double len=0.0;	                    /* eg 3-4 or 12-4 */

		push();	                   /* in tablature too */
		if (f->flags & CON_SEV ) 
		  p_movev (i_space);
		if (f->line_flag == ON_LINE) fmovev(3.0 * d_i_space);
		else fmovev(3.5 * d_i_space);
		
		push();
		i=1;
		while (ch[i] != '-')  len += 1.5 * get_width(font, ch[i++]);
		/* 1.5 = 1.2 * 1.25 */
		/* ten to twelve correction, 12 to 15 corr. */
		fmoveh (len / -2.0);
		i=1;
		while (ch[i] != '-')  set_a_char(ch[i++]);
		pop();
		fmovev(1.5 * get_height(font, '1') + .3 * m_space);
		j= ++i;	len = 0.0;
		while (ch[j] != ' ')  len += 1.5 * get_width(font, ch[j++]);
		fmoveh (len / -2.0);
		while (ch[i] != ' ')  set_a_char(ch[i++]);
		pop();

		if (f->flags & NOTES) {
		    fmovev ( -st_text);                  /* place in music */
		    fmovev ( - f->c_space);
		    fmovev ( -text_sp * f->n_text);
		    fmovev (-3.2 * m_space);
		    push();
		    len = 0;
		    i=1;
		    while (ch[i] != '-') 
			len += 1.5 * get_width(font, ch[i++]);
		    /* 1.5 = 1.2 * 1.25 */
		    /* ten to twelve correction, 12 to 15 corr. */
		    fmoveh (len / -2.0);
		    i=1;
		    while (ch[i] != '-')  set_a_char(ch[i++]);
		    pop();
		    fmovev(1.5 * get_height(font, '1') + .3 * m_space);
		    j= ++i;	len = 0.0;
		    while (ch[j] != ' ')  len += 1.5 * get_width(font, ch[j++]);
		    fmoveh (len / -2.0);
		    while (ch[i] != ' ')  set_a_char(ch[i++]);
		}
	    }
	    else {		/* eg. 3 */
		font = 0;
		use_font(font);
		switch (ch[1]) {
		  case '2':
		    cc = 'R';
		    break;
		  case '3':
		    cc = 'T';
		    break;
		  case '4':
		    cc = 'S';
		    break;
		  case '5':
		    cc = 'V';
		    break;
		  case '6':
		    cc = 'P';
		    break;
		  default:
		    cc = '\0';
		    printf (
		       "tab: dvi_format: no character for large number %c\n",
			    ch[1]);
		    break;
		}
		push();
		if (f->line_flag == ON_LINE) fmovev(3.0 * d_i_space);
		else fmovev(3.5 * d_i_space);

		fmovev( 0.5 * get_height(font,cc));

		if (ch[2] && ch[2] == 'Q' ) {
		    highlight++;
		    put_a_char(cc);
		    highlight = 0;
		}
		else put_a_char(cc);
		
		pop();
		if (f->flags & NOTES) {
		    push();
		    fmovev ( -st_text -text_sp * f->n_text -m_space );
		    fmovev ( - f->c_space);
		    fmovev (-2.0 * m_space);
		    fmovev (get_height(font, cc) / 2.0);
		    put_a_char(cc);
		    pop();
		}
	    }
	    use_font(0);
	}
	else {			/* a character, not a digit */
	    if ( j == 0 ) 
	      fmoveh( -1 * str_to_inch("0.28 in"));
	    fmovev((8.25 * d_i_space + get_height(0, c)) / 2.0 );
	    put_a_char(ch[1]);
	}
	pop();
	break;
      case 'k':
	push();

	fmovev ( -text_sp * f->n_text);
	fmovev ( -st_text);
	fmovev ( - f->c_space);
	fmovev ( -5 * m_space);

/*	f_put_rule (0.1, 0.01); */

	switch (ch[1]){
	  case 'g':
	  case 'd':
	  case 'a':
	    if (f->flags & LSA_FORM) {
		fmovev(0.5 * get_height(0, 22));
		put_a_char(22);	
	    } 
	    else if (f->note_flag == ITAL_NOTES) {
		fmovev(0.5 * get_height(0, 23));
		put_a_char(23);	
	    } 
	    else {
		fmovev(.57 * m_space);
		put_a_char('#');
	    }
	    if (ch[1] == 'g') goto fin;

	    fmoveh(get_width(0, '#'));
	    fmovev(1.5 * m_space);
	    
	    if (f->flags & LSA_FORM) 
	      put_a_char(22);
	    else if (f->note_flag == ITAL_NOTES)
	      put_a_char(23);	
	    else 
	      put_a_char('#');

	    if (ch[1] == 'd') goto fin;

	    fmoveh(get_width(0, '#'));
	    fmovev(-2 * m_space);
	    if (f->flags & LSA_FORM)
	      put_a_char(22);	
	    else if (f->note_flag == ITAL_NOTES)
	      put_a_char(23);	
	    else 
	      put_a_char('#');

	    break;
	  case 'f':
	  case 'b':
	  case 'e':
	    fmovev ( 2.4 * m_space);
	    put_a_char('?');
	    if (ch[1] == 'f') goto fin;

	    fmovev ( -1.5 * m_space);
	    fmoveh(0.05 + get_width(0, '?'));
	    put_a_char('?');
	    if (ch[1] == 'b') goto fin;

	    fmovev ( 2 * m_space);
	    fmoveh(0.05 + get_width(0, '?'));
	    put_a_char('?');
	    break;
	  default:
	    printf ("tab: dvi_format: unsupported key signature %c\n",
		    ch[1]);
	    break;
	}
      fin:
	pop();
	break;
      case 'A':			/* again! - a modern repeat */
	push();
	use_font(2);
	fmovev ( -1.0 * str_to_inch("0,24 in"));
	fmoveh ( str_to_inch(min_d_w));
	put_a_char(ch[1]);
	use_font(0);
	pop();
	{
	    double val=5.0;
	    if (f->flags & FIVE) {
		p_movev(-i_space); 
		val = 4.0;
	    }
	    p_movev(6 * i_space);
	    f_put_rule (staff_h,
			(val + 4) * d_i_space + staff_h);
	    fmovev  (-1 * (val + 4) * d_i_space + staff_h);
	    f_put_rule(str_to_inch("0.60 in"), staff_h);
	}
	break;
      case '&':			/* for after ornaments */
	fmoveh(-1.0 * last_move);
	fmoveh(0.75 * EM); 
/*	if (marks) put_rule("0.01 in", "0.3 in");  */
	/* fall through */
      default:                  /* GRIDS and FLAGS */
	push();			/* this is for whole chord */
	push();                 /* draw the grids */

	if (ch[1] == 'R') { /* rest in tablature */
	    push();
	    if (ch[2] == '.') {
		push();
		fmoveh (0.1);
		fmovev(-.05);
		put_a_char('.');
		pop();
		ch[2] = ' ';
	    }
	    else if (ch[2] == '!') { /* don't draw flag */
		ch[1] = '!';
		ch[2] = ' ';
	    }
	    if (f->line_flag & ON_LINE)
	      fmovev( 2.5 * d_i_space);
	    else
	      fmovev( 3.0* d_i_space);
	    if ( ch[0] == 'W') {
		fmovev (d_i_space * .30);
		f_put_rule(0.09, d_i_space * .30);
	    }
	    else if ( ch[0] == 'w') 
	      f_put_rule(0.09, d_i_space * .30);
	    else {
		fmovev(( /* d_i_space + */ get_height(0,0375))/2.0);
		put_a_char(0375);
	    }
	    pop();
	}

	fmovev(-1.0 * str_to_inch(flag_to_staff));

	if ((cc = ch[1]) == '#' || grid_flag 
		|| cc == '*' || cc == '|' ) { 
	    int nn;
	    int grids;

	    if (! grid_flag ) {	/* first one */
		grid_flag = atoi((char *)&*ch);
		grids = grid_flag;
/* 		printf ("dvi_format: set grid flag  %d\n", grid_flag);  */
		saveloc(3);	/* this depends on no dots in grid! */
	    }
	    else {
		p = l->next;
		for (nn=1; nn < 4; nn++) {
		    if ( p == NULL ) 
				;
			else if ( p->dat[0] == 'x' ) {
			goto end;
		    }
		    else if ( strchr("+^&", p->dat[0])) {
			p = p->next;
			continue;
		    }
/* fall through for end of grid */
		    saveloc(4);
		    getloc(3);
		    moveh(grid_indent); /* indent of hor. beams */
		    move_n_v("0.19 in");

		    put_slash(3, 4, grid_flag, f); 
		    getloc(4);
		    grid_flag = 0;
		    last_line_in_grid++;
/*		    printf ("dvi_format: grid done\n"); */
		    goto end;
		}
	    }
	  end:
	    if ( ! strchr("+^&", c)) {
		push();
		moveh(flag_indent);
		put_a_char(194);                          /* flag staff */

		if (cc == '*') {                            /* print dot */
			if (f->flag_flag == CONTEMP_FLAGS) {
			    fmoveh(str_to_inch ("0.04 in")  
				   + l->space / 10.0 );
			    movev ("0.025 in");
			}
			put_a_char('.');
		}
		else if ( cc == '|') {              /* print extra bar  */
		    move_n_v("0.19 in");
		    fmovev  ( grids * str_to_inch ("0.05 in"));

		    if (l->next->dat[1] != '|') { 
			if (!double_grid ) {    /* must be following dot */
			    fmoveh  ( -1.0 * 
				     (l->space + l->padding)/ 2.2
				     + str_to_inch(".01 in"));
			    f_put_rule((l->space + l->padding)/ 2.2, 
				       staff_h);
			}
			else {
			    /* finish double grid */
			    double_grid=0;
			}
		    }
		    else {
			double_grid++;
			fmoveh(0.006); /* fudge factor, unknown reason */
			f_put_rule((l->space + l->padding), 
				   staff_h);
		    }
		}
		pop();
	    }
	}			/* end GRIDS */
	else if (!strchr("+^&.q", c)) {	
	    if (strchr("Yy", c)) { /* draw FERMATA */
		push();
		movev(flag_to_staff); 
		put_a_char(c);
 		pop();
		fmoveh(0.5 * (get_width(0, c) /* - get_width(0, 0) */));
		if (ch[1] == '0') {
		    put_a_char(194);
		}
		else if (ch[1] == 'b') {             /* draw BARLINE */
		    if (f->flags & CON_SEV ) 
		      p_movev (i_space/2);
		    fmovev(6 * d_i_space + str_to_inch("4.0 pt"));
		    f_put_rule (staff_h,
				5.0 * d_i_space 
				+ staff_h);
		    ch[1] = ' ';
		} 
		else if (ch[1] == '.')               /* don't advance */
		  ;
		else		                           /* advance */
		  fmoveh(0.5 * get_width(0, 0));
	    }
	    else if (ch[1] == 't') { /* special TRIPLET */
		push();
		fmoveh((l->space + l->padding) );
		if (strchr("^+&", *nxt)) /* another staff of */
		  fmoveh(l->next->padding);                /*  ornaments */ 
		put_a_char(205); /* modern flag, three lines */
		pop();
	    }
	    else if (ch[1] == '!' || /* don't draw a flag */
		     ch[1] == 'M')
	      ;
	    else {		/* this should cover everything! */
		push();
		if (ch[1] == 'Q' || ch[1] == '@' ) {
		    highlight++;
		}
		mapflag(c, f);
		pop();

		if (ch[1] == '.' || ch[1] == '@'
		    || ch[1] == 'W') { /* draw the flag dot */
		    push();
		    if (f->flag_flag == ITAL_FLAGS ||
			f->flag_flag == S_ITAL_FLAGS) {
			fmoveh (0.092);
			fmovev (0.01);
		    }
		    else if ( c == 'w' || c == 'W' || c == 'J' ||
			( c == '0' && f->flag_flag == THIN_FLAGS)) {
			fmoveh(get_width(0, c) + str_to_inch("0.018 in"));
			movev ("0.025 in");
		    }
		    else if (f->flag_flag == CONTEMP_FLAGS) {
			fmoveh(str_to_inch ("0.12 in") + l->space / 4.0 );
			movev ("0.025 in");
		    }
		    else {
			fmoveh(get_width(0, c + 146) - str_to_inch("0.018 in"));
			movev ("0.060 in"); 
		    }
		    put_a_char('.'); 
		    pop();
		}
	    }
	}
	pop();			/* end flags */

	if (ch[1] == 'W' || ch[1] == 'w') { /* a tie or slur */
	    double t_width, t_note;
	    struct list *tt = l->next;

	    t_width = l->space + l->padding;

	    while (tt->dat[0] == 'b' || tt->dat[0] == '&') {
		t_width +=( tt->space + tt->padding); 
		if (tt->next) 
		  tt = tt->next;
		else break;
	    }
	    push();
	    fmoveh(t_note = get_width(0, 'w'));
	    fmovev(-1 * get_height(0, 0302));
	    do_tie(t_width - t_note);	/* length of tie in inches */
	    if (f->flags & NOTES ) {
		if ( ! (f->flags & TXT )) fmovev(get_height(0, 0302) ); 

		fmovev ( -st_text -text_sp * f->n_text -m_space );
		fmovev ( - f->c_space);
		fmovev (-2.0 * m_space);

		do_tie (t_width - t_note);	
	    }
	    pop();
	}
	
/* LETTERS HERE ! */

	if (f->line_flag == ON_LINE)
		p_movev (i_space/2);
	else p_movev(i_space);

	movev (baselinespace);	/* get moved up to draw letters */

	skip_spaces = 0;
	if (strchr ("Yy", c)) {
	    if (ch[1] == '0') 
	      fmoveh ((get_width(0, c) - get_width(0, 0))/ 2.0);
	    else if (ch[1] == '.')
	      fmoveh(0.5 * get_width(0, c));
	    else if (ch[1] == 'b');
	    else 
	      fmoveh ((get_width(0, c) - get_width(0, 0))/ 2.0);
	}

	for (i=2; i < STAFF; i++) { 
	    /*	    printf ("ch[%d] is %d %c\n", i, ch[i], ch[i]); */
	    if (l->prev && l->prev->dat[0] != 'G' 
		&& l->prev->dat[i] == 'Q') {
/*		printf ("Highlight here\n");*/
		highlight++;
	    }
	    else highlight = 0;
/* #ifdef SIG
	    switch (cc = ch[i]) {
   #else  */
	    switch (cc = (unsigned char)ch[i]) {
/*
#endif
 */
	      case ' ': 
		skip_spaces++; 
		break;		/* dont print the blanks */
	      case 'Q':
		break;
	      case 'r':
		if (skip_spaces) {
		    p_movev(skip_spaces * i_space);
		    skip_spaces = 0;
		}
		break;
	      case 'U':		/* blank lines yet */
		underline(&skip_spaces, 5, 6, 0);
		break;
	      case 'X':
		do_uline(&skip_spaces, 5, 6, 0);
		break;
	      case '{':
		underline(&skip_spaces, 11, 12, 0);
		break;
	      case '}':
		do_uline(&skip_spaces, 11, 12, 0);
		break;
	      case '(':
		underline(&skip_spaces, 9, 10, 0);
		break;
	      case ')':
		do_uline(&skip_spaces, 9, 10, 0);
		break;
	      case '[':
		push();
		if (skip_spaces) {
		    p_movev(skip_spaces * i_space);
		    skip_spaces = 0;
		}
		put_a_char(cc);
		fmoveh(get_width(0,cc));
		fmovev(-0.25 * get_height(0, cc));
		underline(&skip_spaces, 7, 8, 1);
		pop();
		break;
	      case ']':
		if (skip_spaces) {
		    p_movev(skip_spaces * i_space);
		    skip_spaces = 0;
		}
		put_a_char(cc);
		do_uline(&skip_spaces, 7, 8, 1);
		break;
	      default:
		if (skip_spaces) {
		    p_movev(skip_spaces * i_space);
		    skip_spaces = 0;
		}
		if (cc == '-') { /* draw vertical bar*/
		    double w = str_to_inch("0.006 in");
		    double h = d_i_space; 
		    double p = 0.0;

		    push();
		    p_movev( i_space / 2);
		    fmoveh(0.3 * EM); /* fudge factor! */

		    if ( strchr(" -G", ccc = ch[i + 1])) {
			p = 0.0;
			h = d_i_space;
		    }
		    else if (strchr("dl", ccc)) {
			p = str_to_inch("-7.0 pt");
			h = str_to_inch("3.0 pt");			
		    }
		    else if (strchr("ib", ccc)) {
			p = str_to_inch("-4.0 pt");
			h = str_to_inch("6.0 pt");			
		    }
		    else if (strchr("jmnop", ccc)) {
			p = str_to_inch("-3.0 pt");
			h = str_to_inch("7.0 pt");			
		    }
		    else {
			p = str_to_inch("-2.5 pt");
			h = str_to_inch("7.5 pt");			
		    }
		    fmovev(p);
		    /*  
		       I should use the real height of letter here!
		       fmovev(-1.0 * get_depth(0, ccc));
		       h = d_i_space - get_depth(0, ccc);
		       */
		    if (ch[i - 1] == 'G') 
		      h -= str_to_inch("4.5 pt");
		    if (h > 0.0 )f_put_rule(w, h);
		    pop();
		} /* big numbers for bass strings */


		else if (cc >= '0' && cc <= '9' ) {
		    push();
		    mapchar((unsigned char)(cc - '0' + 150), f);
		    pop();
		}		    
		else if (cc >=140 && cc <= 149) {
		    push();
		    if ( f->line_flag != ON_LINE )
				movev("-2.75 pt"); /* fingerings */
		    put_a_char(cc);    /* draw number */
		    pop();
		}
		else if (cc == '`') { /* comma */
		    push();
		    movev("4.0 pt");
		    put_a_char('\'');
		    pop();
		}
		else if (c == '+' && cc == '_' ) { /* special underline */
		    push();
		    fmoveh(0.8 * EM);
		    put_a_char(cc);
		    pop();
		}
		else if (c == '&' && cc == ':') { /* infix .. */
		    push();
		    fmoveh(-0.7 * EM);
		    fmovev( -0.3 * d_i_space);
		    put_a_char(cc);
		    pop();
		}
		else if (i != 8 && cc == '/' ) {
		    if (c == '+' ) {
			push();
			fmoveh( str_to_inch(min_O_w) 
			       - str_to_inch("5.0 pt"));
			put_a_char(2);
			pop();
		    }
		    else {
			moveh("-5.0 pt");
			put_a_char(2);
			moveh("5.0 pt");
		    }
		}
		else if (strchr("uw" ,cc )) {
		    push();
		    if (f->line_flag == ON_LINE)
		      movev (italian_offset);
		    if (cc == 'u') put_a_char(248);
		    else put_a_char (249);
		    pop();
		}
		else if (cc == '.') {
		    push();
		    if (f->line_flag == ON_LINE) {
			movev (italian_offset);
			fmovev (-.2 * d_i_space);
		    }
		    put_a_char(cc);  /* draw dot */
		    pop();
		}
		else if (cc == '*') {
		    put_a_char(64); /* star */
		}
		else if (cc == '\\') {
		    put_a_char(3); /*  backslash character */
		}
		else if ( i == 2 && cc == 'd' && 
			 ( *ch != 'x' || grid_flag || last_line_in_grid)) {
		    push();
		    if (f->line_flag == ON_LINE)
		      movev (italian_offset);
		    put_a_char('D'); /* should be in getsystem */
		    last_line_in_grid = 0;
		    pop();
		}
		else if (strchr(".+:#", cc) &&
			 strchr( "dD", nxt[i])) {
		    push();
		    movev("0.02 in");
		    put_a_char(cc);
		    pop();
		}
		else {
		    push();
		    mapchar(cc, f);
		    pop();
		}
		p_movev(i_space);
		break;
 	    }
	}
	pop();
	break;
    }
    getloc(1);

    if (l->notes) do_notes(f, l);
    
    if (l->text) do_text(f, l);
    
    last_move = l->space + l->padding;
    fmoveh(l->space + l->padding);
    if (f->flags & MARKS) put_rule("0.01 in", "0.25 in");


}
    
#define NUM_INDENT

format_pagenum(num, f)
     char num[];
     struct file_info *f;
{
    int i;
    int h, v;
#ifdef POSTSCRIPT
    extern int ps_top_of_page;
#endif /* POSTSCRIPT */

    push();
    use_font(2);

    get_current_loc(&h, &v);

    if (f->flags & PS) {
#ifdef POSTSCRIPT
	p_movev( v - ps_top_of_page );	/* cf ps_init_hv in prim.c */
#endif /* POSTSCRIPT */
    }
    else 
      p_movev( -v );

    fmovev (-.45);
    for (i=0; i<5; i++) {
	if (num[i] != ' ') {
	}
	else num[i] = '\0';
    }

    if ( atoi(num) % 2 ) { 
	moveh(staff_len);
	for (i=0; i<5 && num[i]; i++) {
	    fmoveh(-1.0 * get_width(2, num[i]) * PS_SCALE * red / 10);
	}
#ifdef NUM_INDENT
	moveh(".28 in");
#endif /* NUM_INDENT */
    }
    else {
#ifdef NUM_INDENT
	fmoveh(-1 * str_to_inch(".28 in"));
#endif /* NUM_INDENT */
    }
    for (i=0; i<5 && num[i]; i++) {
	set_a_char(num[i]);
    }
    pop();
    return(0);
}

#ifdef VAX
isupper(c)
char c;
{
    if ((c >= 'A') && (c <= 'Z')) return (1);
	else return (0);
}
islower(c)
char c;
{
    if ((c >= 'a') && (c <= 'z')) return (1);
	else return (0);
}
isdigit(c)
char c;
{
    if ((c >= '0') && (c <= '9')) return (1);
	else return (0);
}
#endif /* VAX */

online(c)
char c;
{    switch (c) {
      case 'A': case 'C': case 'E': case 'G': case 'b': case 'd': case 'f':
	return(1);
      default:	return(0); }}


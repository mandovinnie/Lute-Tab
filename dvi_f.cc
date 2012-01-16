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
#include "tfm.h"
#include <setjmp.h>
#include "tab.h"
#include "print.h"
#include "sizes.h"
#include "system.h"

/* EXTERNAL */
void mapflag(print *p, font_list *f_a[], char c, struct file_info *f);
void mapchar(print *p, font_list *f_a[], unsigned char c, struct file_info *f);
void change_ps_font(char *line);

// void put_slant(int bloc, int eloc);

void get_current_loc(int *h, int *v);
void p_num(int n,struct file_info *f);
int  get_font();
int online(char c);
int dot_ch(struct list * l);
void check_bar(print *p, int j, int *l_p, struct list* l);
void do_key_s( char ch[], print *p, font_list *f_a[], struct file_info *f);
void do_time_sig( char ch[], int j, int font,
		  print *p, font_list *f_a[], struct file_info *f);

/* these are the basic design sizes for the text */
/* note that a few parameters for grid and underscore are in prim.c */


extern int n_system;	        
extern char min_d_w[];
extern char min_O_w[];
static int grid_flag=0;

extern int text_font;
extern int title_font;
extern double b_d_pad;		// in pass1 for D and dot

#if defined WIN32
#pragma optimize ("g", off)
#endif

void dvi_format(print *p, i_buf *i_b, font_list *f_a[], 
int *l_p,			/* characters in line */
int j,				/* number of this character */
struct file_info *f,		/* general flags, etc */
struct list *l)			/* data */
{
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
    struct list *pp;
    static int double_grid=0;
    extern char flag_indent[];
    double val;
    int font=0;
    double cap_dot_offset;

    if (l) ch = l->dat;
    else return;

    if (l->next) nxt = l->next->dat;

//    p->saveloc(1);
    p->push();    
    p->use_font(0);

    if (f->flags & MARKS ) p->put_rule(.1, .01);

    dbg1 (Inter, "dvi_f: ch %s\n", (void *)ch); 

    switch (c = (unsigned int)*ch) {
    case 'b':			/* barline */
	p->push();
	val=5.0;
	if (f->flags & FIVE) {
	    p->p_movev(-i_space); 
	    val = 4.0;
	}
	else if (f->flags & FOUR) {
	    p->p_movev(2 * -i_space); 
	    val = 3.0;
	}
	else if (f->m_flags & SEVEN) {
	    p->p_movev(i_space); 
	    val = 6.0;
	}
	if (f->flags & CON_SEV ) 
	    p->p_movev (i_space);
	if (f->line_flag == ON_LINE)
	    p->p_movev((int)(5.5 * i_space));
	else p->p_movev(6 * i_space);
	if (ch[1] != '!') {
	    if (ch[1] == 'Q' ) 
		p->set_highlight();
	    if (j == 0 || j == (*l_p - 1)) {
		p->put_rule (1.3 * staff_h,
			     val * d_i_space 
			     + staff_h);
		/* this draws the bar from top staff to tab system */
		if (f->flags & NOTES && f->note_flag == MOD_NOTES) {
		    double v_bar;
		    v_bar = 6 * d_i_space
			+ st_text + text_sp * f->n_text 
			+ f->c_space + 5 * m_space;
		    if (f->m_flags & TWOSTAFF ) {
		      v_bar += text_sp * f->n_text
			+ str_to_inch(interspace)
			+ 5 * m_space
			+ f->c_space;
		    }
		    p->put_rule(1.3 * staff_h, v_bar);
		}
	    }
	    else {
		p->put_rule (staff_h,
			     val * d_i_space 
			     + staff_h);
	    }
	}
	check_bar(p, j, l_p, l); 

	if (ch[1] == 'T') {
	    p->push();
	    p->moveh(-0.5 * f_a[0]->fnt->get_width(128));
	    p->movev(-6.7 * d_i_space);
	    p->put_a_char(128);
	    p->pop();
	}
	else if (ch[1] == 'v' || ch[1] == 'V') {
	    p->push();
	    p->moveh(-0.5 * f_a[0]->fnt->get_width(9));
	    p->movev(f_a[0]->fnt->get_height(9));
	    p->put_a_char (9);
	    p->movev(-5.5 * d_i_space);
	    p->put_a_char (8);
	    if (ch[1] == 'V') {
	      p->movev(-0.4* d_i_space);
	      p->put_a_char (8);
	    }
	    p->pop();	  
	}

	p->pop();

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
		p->push();		/* at bottom of tab staff */
	      
		p->movev ( -st_text -text_sp * f->n_text - m_space
				  - f->c_space );
	      
		if ( j == 0 || j == *l_p -1) { // thicken first, last barline

		    p->put_rule (1.3 * staff_h, 4 * m_space + staff_h);

		    if (f->m_flags & TWOSTAFF ) {
			p->movev( -5 * m_space );
			p->movev( -text_sp * f->n_text );
			p->movev( -str_to_inch(interspace) );
			p->movev ( -f->c_space );
			p->put_rule (1.3 * staff_h,
				     4 * m_space + staff_h);
		    }
		}
		else {
		    p->put_rule (staff_h, 4 * m_space + staff_h);
		    if (f->m_flags & LONGBAR ) {
		      p->push();
		      p->put_rule (staff_h, 
				  -( st_text + text_sp * f->n_text + m_space
				  + f->c_space + str_to_inch(flag_to_staff)));
		      p->pop();
		    }
		    if (f->m_flags & TWOSTAFF ) {
		      p->movev( -5 * m_space );
		      p->movev( -text_sp * f->n_text );
		      p->movev( -str_to_inch(interspace));
		      p->movev ( -f->c_space );
		      p->put_rule (1.3 * staff_h,
				   4 * m_space + staff_h );
		    }
		}
		p->pop();
	    }
	if (ch[1] > '0' && ch[1] <= '9' ) {
	    dbg1(Warning, 
		 "tab: dvi_f -number after barline %c\n", (void *)ch[1]);
	    p->push();
	    p->movev(-.1);
/*	    p->put_a_char(140 + ch[1] - '0');*/
	    p->use_font(3);	// was 2, 3 at rainer's request
	    p->put_a_char(ch[1]);
	    p->use_font(0);
	    p->pop();
	}
	break;
    case 'B':			/* thick barline */
	p->push();

	val=5.0;
	if (f->flags & FIVE) {
	    p->p_movev(-i_space); 
	    val = 4.0;
	}
	else if (f->flags & FOUR) {
	    p->p_movev(2 * -i_space); 
	    val = 3.0;
	}
	else if (f->m_flags & SEVEN) {
	    p->p_movev(i_space); 
	    val = 6.0;
	}
	if (f->flags & CON_SEV ) 
	    p->p_movev (i_space);
	if (f->line_flag == ON_LINE) 
	    p->p_movev((int)(5.5 * i_space));
	else p->p_movev(6 * i_space);
	
	if ((j == 0 || j == *l_p - 1)
	    && f->flags & NOTES && f->note_flag == MOD_NOTES) {
	    double v_bar;
	    v_bar = 6 * d_i_space
		+ st_text + text_sp * f->n_text 
		+ f->c_space + 5 * m_space;
	    p->put_rule(str_to_inch(thick_bar), v_bar);
	}
	else {
	    p->put_rule (str_to_inch(thick_bar),
			 val * d_i_space + staff_h);
	}
	check_bar(p, j, l_p, l);     
	p->pop();
	if (f->flags & NOTES) {
	    p->push();
	    
	    p->movev ( -st_text -text_sp * f->n_text -m_space 
		       - f->c_space);
	      
	    p->put_rule (str_to_inch(thick_bar),
			 4 * m_space 
			 + staff_h);
	    if (f->m_flags & TWOSTAFF ) {
		p->movev( -5 * m_space /* - staff_h */);
		p->movev( /* -st_text */ -text_sp * f->n_text );
		p->movev( -str_to_inch(interspace));
		p->movev ( -f->c_space );
		p->put_rule (str_to_inch(thick_bar),
			     4 * m_space + staff_h);
	    }
	    p->pop();
	}
    case 'i':			/* indent as for barline - no line*/
    case 'j':			/* indent as for barline - no line*/
	break;
    case '.':
	p->movev(baselinespace);
	if (f->flags & CON_SEV ) 
	    p->p_movev (i_space);
	if (f->line_flag == ON_LINE) 
	    p->p_movev((int)(1.5 * i_space));
	else p->p_movev(2 * i_space);

	for (i=0; i < (f->flags & FOUR ? 3:f->flags & FIVE ? 4:5);i++) {
	    mapchar(p, f_a, /* ch[i+3] */'Z', f);
	    p->p_movev(i_space);
	}
	if (f->flags & NOTES ) {

	    if (f->line_flag == ON_LINE) p->movev( -5.5 * d_i_space);
	    else p->movev ( -6.0 * d_i_space);     /* to top of staff */

	    p->movev ( -st_text);
	    p->movev ( -text_sp * f->n_text - f->c_space);
	    p->movev ( -3 * m_space - .01 /* fudge */);
	    mapchar(p, f_a, 'Z', f);
	    p->movev( -m_space);
	    mapchar(p, f_a, 'Z', f);
	    if (f->m_flags & TWOSTAFF ) {
	      p->movev( -5 * m_space );
	      p->movev( -text_sp * f->n_text );
	      p->movev( -str_to_inch(interspace) );
	      p->movev ( -f->c_space );
	      mapchar(p, f_a, 'Z', f);
	      p->movev( m_space);
	      mapchar(p, f_a, 'Z', f);
	    }
	}
	break;
    case 'd':
	if (baroque) 
	    p->movev (1.1);
	else
	    p->movev ( -1.0 * str_to_inch("4.0 pt"));
	p->put_a_char(254);
	if (ch[1] == 'b') {	/* make a bar line */
	    p->moveh(f_a[0]->fnt->get_width(254) / 2.0);
	    p->movev(6 * d_i_space + str_to_inch("4.0 pt"));
	    p->put_rule (staff_h,
			 5.0 * d_i_space 
			 + staff_h);
	}
	break;
    case 'U':
    case 'u':
    case 'R':
    case 'S':
	/*      case 'T':  this is now for text */
    case 'V':
	if (f->flags & NOTES) {
	    p->push();

	    p->movev ( -st_text -text_sp * f->n_text -m_space );
	    p->movev ( - f->c_space);
	    p->movev (-2.0 * m_space);
	    p->movev (f_a[0]->fnt->get_height(c) / 2.0);
	    p->put_a_char(c);
	    if (f->m_flags & TWOSTAFF ) {
		p->movev( -5 * m_space - staff_h);
		p->movev( -str_to_inch(interspace) );
		p->movev( /* -st_text  */-text_sp * f->n_text );
		p->put_a_char(c);
	    }
	    p->pop();
	}
	p->push();
	if (f->flags & CON_SEV ) 
	    p->p_movev (i_space);
	if (f->line_flag == ON_LINE) 
	    p->movev(3. * d_i_space);
	else 
	    p->movev(3.5 * d_i_space);

	if ( c == 'U') c = 19;
	else if (c == 'u') c = 20;

	p->movev(f_a[0]->fnt->get_height(c) / 2.0 );
	p->put_a_char(c);
	
	p->pop();
	break;
    case '$':			// swallow it
	break;
    case 'm':
	c = 191;
    case 'Q':			/* squiggle */
    case 'q':
	p->movev( 5.9 * d_i_space);
	if (f->flags & CON_SEV ) 
	    p->p_movev (i_space);
	if (f->line_flag == ON_LINE)
	    p->p_movev (i_space/-2);
	p->put_a_char(c);
	break;
    case '=':			/* W. = W */
	if (f->flags & NOTES) {
	    int curfont = p->get_font();
	    p->push();
	    if (f->line_flag == ON_LINE) p->movev (- 5.5 * d_i_space);
	    else p->movev (- 6.0 * d_i_space);
	    p->movev ( -5 * m_space);
	    p->movev ( st_text);
	    p->movev ( -text_sp * f->n_text);
	    p->movev ( - f->c_space);
	    p->use_font(0);
	    for (i=1; i<STAFF; i++) {
		if (strchr ("=()", (int)ch[i])) {
		    p->use_font(1);
		    p->set_a_char(ch[i]);
		    p->use_font(0);
		}
		else if (ch[i] == '.'){
		    p->movev(.021);
		    p->set_a_char(ch[i]);
		    if (! (f->flags & PS ))
			p->moveh(-.5 * f_a[curfont]->fnt->get_width('.'));
		    p->movev(-0.021);
		}
		else if (ch[i] == 'W'){
		    if (f->num_flag == ITAL_NUM) p->set_a_char(216);
		    else p->set_a_char(ch[i]);
		}
		else { 
		    p->set_a_char(ch[i]);
		}
	    }
	    p->use_font(curfont);
	    p->pop();
	}
	break;
    case 'f':
	p->push();
	if (f->flags & PS ) p->use_font(1);
	else p->use_font(2);
	p->push();
	p->movev(0.07);
	p->set_a_char('F'); p->set_a_char('i'); p->set_a_char('n');
	p->pop();
	if (f->flags & NOTES ){
	    p->pop();
	    p->push();
	    if (f->line_flag == ON_LINE) p->movev (- 5.5 * d_i_space);
	    //	    else if (f->flags & WALLACE) p->movev (- 5.5 * d_i_space);
	    else p->movev (- 6.0 * d_i_space);
	    p->movev ( -5 * m_space);
	    p->movev ( st_text);
	    p->movev ( -text_sp * f->n_text);
	    p->movev ( - f->c_space);
	    p->push();
	    p->set_a_char('F'); p->set_a_char('i'); p->set_a_char('n');
	    p->pop();
	    if (f->m_flags & TWOSTAFF ) {
	      p->movev( -5 * m_space /* - staff_h */);
	      p->movev( /* -st_text */ -text_sp * f->n_text );
	      p->movev( -str_to_inch(interspace));
	      p->movev ( -f->c_space );
	      p->set_a_char('F'); p->set_a_char('i'); p->set_a_char('n');
	    }
	}
	p->pop();
	break;
    case 'T':
	if (l->text) {

	    p->push();
	    do_text(p, i_b, f_a, l, l->text, f, 1, 1);
	    p->pop();

	    free (l->text);
	    l->text = NULL;
	}
	break;
    case 'F':			/* change fonts */
	if (ch[1] == 'M') {	// an F clef, instead
				// this is printed as a note
	}
	else {
	    char *p;
	    char cc[2];
	    int fontnum;
	    
	    p=&(l->special[2]);
	    
	    cc[0] = *p;
	    cc[1] = '\0';
	    fontnum = atoi(cc);
//	    printf("dvi_f: fontnum %d\n", fontnum);
	    switch (fontnum) {
	    case 1:
	    case 5:
		text_font = fontnum;
		font = text_font;
		break;
	    case 2:
	    case 3:
		title_font = fontnum;
		break;
	    default:
		dbg0(Warning, "Format error in text font specification\n");
	    }
	}
	break;
    case 'D':			/* D.C. da-capo */
	p->push();
	p->movev(0.07);
	if (f->flags & PS ) p->use_font(1);
	else p->use_font(2);
	p->set_a_char('D'); p->set_a_char('.'); p->set_a_char('C'); p->set_a_char('.');
	if (f->flags & NOTES ){
	    p->pop();
	    p->push();
	    if (f->line_flag == ON_LINE) p->movev (- 5.5 * d_i_space);
	    //	    else if (f->flags & WALLACE) p->movev (- 5.5 * d_i_space);
	    else p->movev (- 6.0 * d_i_space);
	    p->movev ( -5 * m_space);
	    p->movev ( st_text);
	    p->movev ( - f->c_space);
	    p->movev ( -text_sp * f->n_text);
	    p->set_a_char('D'); p->set_a_char('.'); 
	    p->set_a_char('C'); p->set_a_char('.');
	}
	p->pop();
	break;
    case '8':			/* Octave - *va */
	p->push();
	if (f->flags & PS ) p->use_font(1);
	else p->use_font(2);
	if (f->flags & NOTES ){
/*	    printf ("tab: dvi_f: octave\n"); */
	    p->pop();
	    p->push();
	    if (f->line_flag == ON_LINE) p->movev (- 5.5 * d_i_space);
	    else p->movev (- 6.0 * d_i_space);
	    p->movev (d_i_space);
	    p->movev ( st_text);
	    p->movev ( - f->c_space);
	    p->movev ( -text_sp * f->n_text); 
	    p->movev ( - 5.5 * m_space );
	    p->set_a_char('8'); p->set_a_char('v'); 
	    p->set_a_char('b');
	}
	p->pop();
	break;
    case 'G':			/* time signature */
	p->push();		/* better be at the beginning of line! */
	do_time_sig(ch, j, font, p, f_a, f);
	
	p->pop();
	break;
    case 'k':
	p->push();

	p->movev ( -text_sp * f->n_text);
	p->movev ( -st_text);
	p->movev ( - f->c_space);
	p->movev ( -5 * m_space);
	do_key_s(ch, p, f_a, f);
	if (f->m_flags & TWOSTAFF ) {
	  p->movev( -5 * m_space - staff_h);
	  p->movev( -str_to_inch(interspace) );
	  p->movev( /* -st_text  */-text_sp * f->n_text );
	  p->movev ( -f->c_space );
	  do_key_s(ch, p, f_a, f);
	}
	  p->pop();
	break;
    case 'A':			/* again! - a modern repeat */
	p->push();
	p->use_font(2);
	p->movev ( -.24 );
	p->moveh ( str_to_inch(min_d_w));
	if (f->flags & NOTES){
	  p->push();
	  p->movev(-0.940);
	  p->set_a_char(ch[1]);
	  p->set_a_char('.');
	  p->pop();
	}
	else 
	  p->put_a_char(ch[1]);
	p->use_font(0);
	p->pop();
	{
	    double val=5.0;
	    if (f->flags & FIVE) {
		p->movev(-i_space); 
		val = 4.0;
	    }
	    else if (f->flags & FOUR) {
		p->movev(2 * -i_space); 
		val = 3.0;
	    }
	    p->p_movev(6 * i_space);
	    if (f->flags & NOTES){
	      //	      p->movev(-0.940);
	      p->put_rule (staff_h,
			 (val + 4) * d_i_space + staff_h + 0.940);
	      //	      p->put_rule(0.05, 0.05);
	    }
	    else {
	      p->put_rule (staff_h,
			 (val + 4) * d_i_space + staff_h);
	    }
	    if (ch[1] == '1') 
	      check_bar(p, j, l_p, l);         
	    if (f->flags & NOTES){
	      	      p->movev(-0.940);
	    }
	    p->movev  (-1 * (val + 4) * d_i_space + staff_h);
	    p->put_rule(str_to_inch("0.60 in"), staff_h);
	}
	break;
    case '*':			/* a mark, as in AA BB */
	p->push();
	p->use_font(2);
	p->movev(-f_a[0]->fnt->get_height(
	    f->flag_flag == ITAL_FLAGS ? 217 : 195)
		 /* -f_a[2]->fnt->get_height(ch[1]) */);
	p->put_a_char(ch[1]);
	p->use_font(0);
	p->pop();
	break;
    case '&':			/* for after ornaments */
	p->moveh(-1.0 * last_move);
	p->moveh(0.75 * EM);
	if (baroque && l->prev && l->prev->dat[0] != '2' ) p->moveh(.05);
/*	if (f->line_flag == ON_LINE) p->movev (-0.5 * d_i_space);  */
	if (f->flags & MARKS) p->put_rule(0.01, 0.17);  
	/* fall through */
    default:                  /* GRIDS and FLAGS HERE*/
	p->push();			/* this is for whole chord */
	p->push();                 /* draw the grids */

	if (ch[1] == 'R') { /* rest in tablature */
	    p->push();
	    if (ch[2] == '.') {
		p->push();
		p->moveh (0.1);
		p->movev(-.05);
		p->put_a_char('.');
		p->pop();
		ch[2] = ' ';
	    }
	    else if (ch[2] == '!') { /* don't draw flag */
		ch[1] = '!';
		ch[2] = ' ';
	    }
	    if (f->line_flag == ON_LINE)
		p->movev( 2.5 * d_i_space);
	    else
		p->movev( 3.0 * d_i_space);
	    if ( ch[0] == 'W') {
		p->movev (d_i_space * .30);
		p->put_rule(0.09, d_i_space * .30);
	    }
	    else if ( ch[0] == 'w') 
		p->put_rule(0.09, d_i_space * .30);
	    else if ( ch[0] == '1') {
		p->movev(( f_a[0]->fnt->get_height(16))/2.0);
		p->put_a_char(16);
	    }
	    else {
		p->movev(( f_a[0]->fnt->get_height(0375))/2.0);
		p->put_a_char(0375);
	    }
	    p->pop();
	}
	p->movev(-1.0 * str_to_inch(flag_to_staff));
	if (f->flags & MARKS) p->put_rule(.3, 0.01);

	if ((cc = ch[1]) == '#' || grid_flag /* GRIDS here */
	    || cc == '*' || cc == '|' ) { 
	    int nn;
	    int grids;

	    if (! grid_flag ) {	/* first one */
		grid_flag = atoi((char *)&*ch);
		grids = grid_flag;
/* 		printf ("dvi_format: set grid flag  %d\n", grid_flag);   */
		p->saveloc(3);	/* this depends on no dots in grid! */
	    }
	    else {		/* not first */
		pp = l->next;
		for (nn=1; nn < 4; nn++) {
		    if ( pp == NULL ) ;	/* no more chords - end grid */
		    else if ( pp->dat[0] == 'b' ) ; /* end grid at barline */
		    else if ( pp->dat[0] == 'x' ) {
			goto end;
		    }
		    else if ( strchr("+^&:", (int)pp->dat[0])) {
			pp = pp->next;
			continue;
		    }
/* fall through for end of grid */
		    p->saveloc(4);
		    p->getloc(3);
		    p->moveh(grid_indent); /* indent of hor. beams */
		    p->move_n_v("0.19 in");

		    p->put_slash(3, 4, grid_flag, f); 
		    p->getloc(4);
		    grid_flag = 0;
		    last_line_in_grid++;
/*		    printf ("dvi_format: grid done\n");  */
		    goto end;
		}
	    }
	    end:
	    if ( ! strchr("+^&:", (int)c)) {
		p->push();
		p->moveh(flag_indent);
		p->put_a_char(194);                          /* flag staff */

		if (cc == '*') {                            /* print dot */
		    if (f->flag_flag == CONTEMP_FLAGS) {
			p->moveh(0.04  
				 + l->space / 10.0 );
			p->movev ("0.025 in");
		    }
		    p->put_a_char('.');
		}
		else if ( cc == '|') {              /* print extra bar  */
		    p->move_n_v("0.19 in");
		    p->movev  ( grids * str_to_inch ("0.05 in"));

		    if (l->next->dat[1] != '|') { 
			if (!double_grid ) {    /* must be following dot */
			    p->moveh  ( -1.0 * 
					(l->space + l->padding)/ 2.2
					+ str_to_inch(".01 in"));
			    p->put_rule((l->space + l->padding)/ 2.2, 
					staff_h);
			}
			else {
			    /* finish double grid */
			    double_grid=0;
			}
		    }
		    else {
			double_grid++;
			p->moveh(0.006); /* fudge factor, unknown reason */
			p->put_rule((l->space + l->padding), 
				    staff_h);
		    }
		}
		p->pop();
	    }
	}			/* end GRIDS */
	else if (!strchr("+^&.q:", (int)c)) {	
	    if (strchr("Yy", (int)c)) { /* draw FERMATA */
		p->push();
		p->movev(flag_to_staff); 
		p->put_a_char(c);
		// now we must draw the dot... feb 2001
		if (ch[1] == '-')
		  p->put_a_char(92);
 		p->pop();
		p->moveh(0.5 * (f_a[0]->fnt->get_width(c)));
		if (ch[1] == '0') {
		    p->put_a_char(194);
		}
		else if (ch[1] == 'b') {             /* draw BARLINE */
		    int spaces = 
			(f->m_flags & SEVEN ? 6 : f->flags & FOUR ? 
			 3 : f->flags & FIVE ? 4 : 5);
		    if (f->flags & CON_SEV ) 
			p->p_movev (i_space/2);
		    p->movev((spaces + 1) * d_i_space + str_to_inch("4.0 pt"));
		    p->put_rule (staff_h,
				 spaces * d_i_space + staff_h);
		    ch[1] = ' ';
		} 
		else if (ch[1] == 'B' || ch[1] == 'W'
			 || ch[1] == 'w' || ch[1] == 'L' ) {
		    double ww;

		    if (ch[1] == 'B') ch[1] = 'J';
		    ww = f_a[0]->fnt->get_width(ch[1]);
		    p->moveh(-.5 * ww);
		    if (f->flag_flag == ITAL_FLAGS ||
			f->flag_flag == S_ITAL_FLAGS ||
			f->flag_flag == THIN_FLAGS) {
		      if (ch[1] == 'w' )
			p->put_a_char(0331);
		      else if (ch[1] == 'W' )
			p->put_a_char(0330);
		      else 
			p->put_a_char(ch[1]);
		    }
		    else {
		      if (ch[1] == 'w' ) 
			p->movev(-.027);
		      p->put_a_char(ch[1]);
		    }
		}
		else if (ch[1] == '.')               /* don't advance */
				      ;
		else		                           /* advance */
		    p->moveh(0.5 * f_a[0]->fnt->get_width(0));
	    }
	    else if (ch[1] == 't') { /* special TRIPLET */
		p->push();
		p->saveloc(5);
		mapflag(p, f_a, c, f);

		p->moveh((l->space + l->padding) );
		if (strchr("^+&:", (int)*nxt)) /* another staff of */
		    p->moveh(l->next->padding);        /*  ornaments */ 
		mapflag(p, f_a, c, f);

		p->push();
		p->use_font(2);
		p->movev(-.34);
		p->put_a_char('3');
		p->use_font(0);  
		p->pop();

		p->moveh((l->space + l->padding) );
		if (strchr("^+&:", (int)*nxt)) /* another staff of */
		    p->moveh(l->next->padding);        /*  ornaments */ 
		mapflag(p, f_a, c, f);
		p->saveloc(6);
		p->movev(-.24);
//		p->put_uline(6, 5);
                p->do_rtie(6, 5);
		p->pop();
	    }
	    else if (ch[1] == '!' || /* don't draw a flag */
		     ch[1] == 'M')
					  ;
	    else {		/* this should cover everything! */
		p->push();
		if (ch[1] == 'Q' || ch[1] == '@' ) {
		    p->set_highlight();
		    if (f->m_flags & PAREN ) 
			p->paren_highlight();
		    else 
			p->gray_highlight();
		}
		mapflag(p, f_a, c, f);
		p->pop();

		if (ch[1] == 'B') {	// dot before a flag
		    p->push();
		    p->moveh (-.055);
		    if (ch[2] == 'd')
		      p->movev (-.059);
		    p->movev(0.055);
		    p->put_a_char('.');
		    p->pop();
		}
		if (ch[1] == '.' || ch[1] == '@'
		    || ch[1] == 'W') { /* draw the flag dot */
		    p->push();
		    if (ch[1] == '@') p->set_highlight();
			if (f->m_flags & PAREN ) 
			    p->paren_highlight();
			else
			    p->gray_highlight();
		    if (f->flag_flag == ITAL_FLAGS ||
			f->flag_flag == S_ITAL_FLAGS) {
			p->moveh (0.092);
			p->movev (0.01);
		    }
		    else if ( c == 'w' || c == 'W' || c == 'J' ||
			      ( c == '0' && f->flag_flag == THIN_FLAGS)) {
			if ( baroque ) p->movev (-0.06);
			p->moveh(f_a[0]->fnt->get_width(c) 
				 + str_to_inch("0.018 in"));
			p->movev ("0.025 in");
		    }
		    else if (f->flag_flag == CONTEMP_FLAGS) {
			p->moveh(str_to_inch ("0.12 in") + l->space / 4.0 );
			if (baroque && ch[2] == 'b') {
			  p->movev(-0.066);
			}
			p->movev ("0.025 in");
		    }
		    else if ( baroque ) {
			p->movev (-0.06);
			if (ch[2] == 'b') {
			  p->movev (-0.06);
			}
			else if (ch[2] == 'f') p->movev (-0.0);
			switch (c) {
			case '1':
			case '2':
			case '3':
			    p->moveh(f_a[0]->fnt->get_width(c + 146)/2.0);
			    break;
			default:                    /* first flag is */
			    p->moveh(f_a[0]->fnt->get_width(c + 146) 
				     /* 194 - 48*/
				     - str_to_inch("0.008 in"));
			}
			p->movev ("0.060 in"); 
		    }
		    else {
			p->moveh(f_a[0]->fnt->get_width(c + 192 /* was 146 */) 
				 - str_to_inch("0.018 in"));
			p->movev ("0.060 in"); 
		    }
		    if (baroque && ch[2] == 'f') {
		      //		      printf ("here\n");
		      p->movev(-.09);
		    }
		    p->put_a_char('.'); /* the flag dot ! */
		    if (ch[1] == '@') p->clear_highlight();
		    p->pop();
		}
	    }
	}
	p->pop();			/* end flags */

	if (ch[1] == 'W' 
	    && ch[0] != 'Y'  /* || ch[1] == 'w' */) { /* a tie or slur */
	    double t_width, t_note;
	    struct list *tt = l->next;

	    t_width = l->space + l->padding;

	    while (tt->dat[0] == 'b' || tt->dat[0] == '&') {
		t_width +=( tt->space + tt->padding); 
		if (tt->next) 
		    tt = tt->next;
		else break;
	    }
	    p->push();
	    p->moveh(t_note = f_a[0]->fnt->get_width('w'));
	    p->movev(-1 * f_a[0]->fnt->get_height(0302));
	    p->do_tie(t_width - t_note);	/* length of tie in inches */
	    if (f->flags & NOTES ) {
		if ( ! (f->flags & TXT )) p->movev(f_a[0]->fnt->get_height(0302) ); 

		p->movev ( -st_text -text_sp * f->n_text -m_space );
		p->movev ( - f->c_space);
		p->movev (-2.0 * m_space);

		p->do_tie (t_width - t_note);	
	    }
	    p->pop();
	}
	
/* LETTERS HERE ! */

	if (f->line_flag == ON_LINE)
	    p->p_movev (i_space/2);
	else p->p_movev(i_space);

	p->movev (baselinespace);	/* get moved up to draw letters */

	if (ch[0] =='O') {
	    struct list *pp = l->prev;
/*	    printf ("here %f %f\n", pp->space, pp->padding); */
	    cap_dot_offset = pp->space + pp->padding 
		- f_a[0]->fnt->get_width('a') - .02;
	    p->moveh (-cap_dot_offset );
	}

	skip_spaces = 0;
	if (strchr ("Yy", (int)c)) {
	    if (ch[1] == '0') 
		p->moveh ((f_a[0]->fnt->get_width(c) 
			   - f_a[0]->fnt->get_width(0))/ 2.0);
	    else if (ch[1] == '.')
		p->moveh(0.5 * f_a[0]->fnt->get_width(c));
	    else if (ch[1] == 'b');
	    else 
		p->moveh ((f_a[0]->fnt->get_width(c) 
			   - f_a[0]->fnt->get_width(0))/ 2.0);
	}
	for (i=2; i < STAFF; i++) { 
/*	    printf ("ch[%d] is %d %c\n", i, ch[i], ch[i]);  */
	    if (ch[0] == 'O' || l->prev && l->prev->dat[0] != 'G' 
		&& l->prev->dat[i] == 'Q') {
		p->set_highlight();
		if (f->m_flags & PAREN ) 
		    p->paren_highlight();
		else
		    p->gray_highlight();
	    }
	    else p->clear_highlight();
	    if (c == ':' && ch[i] != ' ') {
		p->movev(-.045);
		if (baroque) {
		    if (l->next->dat[i] == 'b') {
			p->movev(-.02);
			p->moveh(-.04);
		    }
		    else if (l->next->dat[i] == 'D') {
			p->movev(-.02);
			p->moveh(.04);
		    }
		    else if (l->next->dat[i] == 'z') {
			p->movev(-.04);
			p->moveh(.01);
		    }
		}
	    }
	    switch (cc = (unsigned char)ch[i]) {

	    case ' ': 
		skip_spaces++; 
		break;		/* dont print the blanks */
	    case 'Q':
		break;
		/* this has been removed - it is for debugging
		  case 'r':
		  if (skip_spaces) {
		  p->p_movev(skip_spaces * i_space);
		  skip_spaces = 0;
		  }
		  break;
		  */
	    case 'U':		/* straight lines */
		if (skip_spaces) {
		    p->p_movev(skip_spaces * i_space);
		    skip_spaces = 0;
		}
		my_underline(p, f_a, &skip_spaces, 5, i);
		break;
	    case 'X':
		if (skip_spaces) {
		    p->p_movev(skip_spaces * i_space);
		    skip_spaces = 0;
		}
		do_uline(p, &skip_spaces, 5, 6, i);
		break;
	    case '{':		/* slanting lines */
		my_underline(p, f_a, &skip_spaces, 11, i);
		break;
	    case '}':
		if (baroque) 
		    do_thick_uline(p, &skip_spaces, 11, 12, i);
		else
		    do_uline(p, &skip_spaces, 11, 12, i);
		break;
	    case '(':		/* more slanting lines */
		my_underline(p, f_a, &skip_spaces, 9, i);
		break;
	    case ')':
		do_uline(p, &skip_spaces, 9, 10, i);
		break;
	    case '[':		/* rags wants this to stack */
		if (skip_spaces) {
		    p->p_movev(skip_spaces * i_space);
		    skip_spaces = 0;
		}
		if (f->flags & PS) {
		    my_underline(p, f_a, &skip_spaces,  7 , i);
		}
		else {
		    p->put_a_char(cc);
		    p->push();
		    p->moveh(f_a[0]->fnt->get_width(cc));
		    p->movev(-0.25 * f_a[0]->fnt->get_height(cc));
		    my_underline(p, f_a, &skip_spaces, 7, i);
		    p->pop();
		    p->p_movev(i_space);
		}
		break;
	    case ']':
		if (skip_spaces) {
		    p->p_movev(skip_spaces * i_space);
		    skip_spaces = 0;
		}
		if (f->flags & PS) {
		}
		else {
		    p->put_a_char(cc);
		}
		do_uline(p, &skip_spaces, 7, 8, i);
		break;
	    default:
		if (skip_spaces) {
		    p->p_movev(skip_spaces * i_space);
		    skip_spaces = 0;
		}
		/* case "-" */
		if (cc == '-') { /* draw vertical bar*/
		    double w = str_to_inch("0.006 in");
		    double h = d_i_space; 
		    double pp = 0.0;

		    if (baroque) {
			w = 0.011;
		    }

		    if (baroque && f->flags & PS && c == '+' 
			&& ch[i+1] == '-' ) {
			int s_h, s_v, e_h, e_v;
			p->get_current_loc(&s_h, &s_v);
			p->movev(i_space);
			i++;
			while (ch[i+1] == '-') {
			    i++;
			    p->movev(i_space);
			}
			// fudge - extend a bit
			p->movev(0.03);
			p->get_current_loc(&e_h, &e_v);
			p->vert_curve(s_v - e_v);
		    }
		    else {
			p->push();
			p->p_movev( i_space / 2);
			p->moveh(0.3 * EM); /* fudge factor! */
			if ( strchr(" -G", (int)(ccc = ch[i + 1]))) {
			    pp = 0.0;
			    h = d_i_space;
			}
			else if (strchr("dl", (int)ccc)) {
			    pp = str_to_inch("-7.0 pt");
			    h = str_to_inch("3.0 pt");			
			}
			else if (strchr("ib", (int)ccc)) {
			    pp = str_to_inch("-4.0 pt");
			    h = str_to_inch("6.0 pt");			
			}
			else if (strchr("jmnop", (int)ccc)) {
			    pp = str_to_inch("-3.0 pt");
			    h = str_to_inch("7.0 pt");			
			}
			else {
			    pp = str_to_inch("-2.5 pt");
			    h = str_to_inch("7.5 pt");			
			}
			p->movev(pp);
			
			//
			//   I should use the real height of letter here!
			//   p->movev(-1.0 * f_a[0]->fnt->get_depth(0, ccc));
			//   h = d_i_space - f_a[0]->fnt->get_depth(0, ccc);

			if (ch[i - 1] == 'G') 
			    h -= str_to_inch("4.5 pt");
			if (h > 0.0 )p->put_rule(w, h);
			p->pop();
		    }
		} 

		/* big numbers for bass strings */

		else if (cc >= '0' && cc <= '9' ) { // numbers
		    p->push();
		    if (baroque && i == 8 && (cc == '4' 
				|| cc == '5' || cc == '6')) {
			p->movev(0.09);
			if (cc== '4')
			    p->put_a_char(14);
			if (cc== '5')
			    p->put_a_char(15);
			if (cc== '6')
			    p->put_a_char(16);
		    }
		    else
			mapchar(p, f_a, (unsigned char)(cc - '0' + 150), f);
		    p->pop();
		}		    
		else if (cc >=140 && cc <= 149) {
		    p->push();
		    if ( f->line_flag != ON_LINE )
			p->movev("-2.75 pt"); /* fingerings */
		    p->put_a_char(cc);    /* draw number */
		    p->pop();
		}
		else if (cc == '`' || cc == ',') { /* (back) comma on line*/
		    p->push();
		    p->moveh("0.7 pt");
		    p->movev("4.0 pt");
		    p->put_a_char('\'');
		    p->pop();
		}
		else if (c == '+' && cc == '_' ) { /* special my_underline */
		    p->push();
		    p->moveh(0.76 * EM);
		    p->movev(-.025);
		    if (baroque) {
		      p->moveh(-.02);
		      if (l->next->dat[i] == 'D') p->moveh(.096);
		    }
		    if (f->line_flag == ON_LINE)
			p->movev(0.5 * d_i_space);
		    p->put_a_char(cc);
		    p->pop();
		}
		else if (c == '&' && cc == ':') { /* infix .. */
		    p->push();
		    p->moveh(-0.7 * EM);
		    p->movev( -0.3 * d_i_space);
		    p->put_a_char(cc);
		    p->pop();
		}
		else if (i != 8 && cc == '/' && 
			(i != 2 && f->flags & CONVERT )) {// a slant
		    p->push();
		    if ( f->line_flag == ON_LINE)
			p->movev(0.5 * d_i_space);
		    if (c == '+' ) {
			p->moveh( str_to_inch(min_O_w) 
				  - str_to_inch("5.0 pt"));
			if (baroque) p->movev(-0.03);
			p->put_a_char(/* wbc 2*/ 5);
		    }
		    else {
			p->moveh("-5.0 pt");
			p->put_a_char(/* 2 */ 5);
		    }
		    p->pop();
		}
		else if (strchr("uw" ,(int)cc )  && i > 7) {
		    p->push();
		    if (f->line_flag == ON_LINE)
			p->movev (italian_offset);
		    if (cc == 'u') p->put_a_char(248);
		    else p->put_a_char (249);
		    p->pop();
		}
		else if (cc == '.') {
		    p->push();
		    if (f->line_flag == ON_LINE) {
			if (c == '+' && baroque) {
			    p->movev (italian_offset);
			    p->movev (.35 * d_i_space);
			}
			else {
			    p->movev (italian_offset);
			    p->movev (-.2 * d_i_space);
			}
		    }
		    p->put_a_char(cc);  /* draw dot */
		    p->pop();
		}
		else if (cc == '*') {
		    p->put_a_char(64); /* star */
		}
		else if (cc == '\\') {
		    p->put_a_char(3); /*  backslash character */
		}
		else if ( i == 2 && cc == 'd' && 
			  ( *ch != 'x' || grid_flag || last_line_in_grid)) {
		    p->push();
		    if (f->line_flag == ON_LINE)
			p->movev (italian_offset);
		    if ((f->m_flags & DSUP ))
		      p->put_a_char('d'); 
		    else
		      p->put_a_char('D'); /* should be in getsystem */
		    last_line_in_grid = 0;
		    p->pop();
		}
		else if (baroque && cc == 'd' 
			 && l->prev && l->prev->prev 
			 && i > 2 
			 && l->prev->dat[i - 1] == ','
			 && l->prev->prev->space < 0.065 ) {
		    p->put_a_char('D');
		}
		else if (strchr(".+:#", (int)cc) &&
			 strchr( "dD", (int)nxt[i])) {
		    p->push();
		    p->movev("0.02 in");
		    p->put_a_char(cc);
		    p->pop();
		}
		else if ( cc == ':' && baroque) {
		    p->push();
		    if (f->line_flag == ON_LINE) /* wbc Dec 15 */
			p->movev (str_to_inch(italian_offset)/2.0);
		    p->moveh(0.015);
		    p->put_a_char(cc);
		    p->pop();
		}
		else if (i == 8 && c == '+' && dot_ch(l)) {
		    p->push();	// just for bourdons with an ornament and D
		    p->moveh(b_d_pad);
		    mapchar(p, f_a, cc, f);
		    p->pop();
		}
		else if ( cc > 'p' && cc < 'x' && c != '+' ) {
		  //		  printf ("here c is %d %c\n", cc, cc);
		  p->push();
		  p->use_font(2);  
		  p->put_a_char(cc);
		  p->use_font(0);
		  p->pop();
		}
		else {
		    p->push();
		    mapchar(p, f_a, cc, f);
		    p->pop();
		}
		p->p_movev(i_space);
		break;
 	    }
		p->clear_highlight();
	}
	p->pop();
	break;
    }
    p->pop();
    //p->getloc(1);

    if (l->notes) do_notes(p, f_a, l->notes, f);
    if (l->notes2) {
	p->push();
//	p->movev ( -st_text );
	p->movev ( -str_to_inch(interspace) );
	p->movev ( -text_sp * f->n_text );
	p->movev ( -f->c_space );
	p->movev ( -5 * m_space );
	do_notes(p, f_a, l->notes2, f);
	p->pop();
    }
    
    if (l->text) {
      do_text(p, i_b, f_a, l, l->text, f, 0, 1);
      free (l->text->words->words); /* leak */
      free (l->text->words); /* leak */
      free (l->text);
      l->text = NULL;
    }
    if (l->text2) {
      do_text(p, i_b, f_a, l, l->text2, f, 0, 2);
            free (l->text2->words->words);
            free (l->text2->words);
            free (l->text2);
	    l->text2 = NULL;
    }
//    printf("dvi_format: space %f padding %f\n", l->space, l->padding);
    last_move = l->space + l->padding;
    p->moveh(l->space + l->padding);
    /*    printf("space %f padding %f\n", l->space, l->padding); */
    if (f->flags & MARKS) p->put_rule("0.01 in", "0.25 in");


}

#if defined WIN32
#pragma optimize ("g", off)
#endif
   
#define NUM_INDENT

void format_pagenum(print *p, font_list *f_a[], 
		    char *num, struct file_info *f)
{
    int i;
    int h, v;
    //    extern int ps_top_of_page;
    //    printf("format_pagenum: char is %s\n", num);

    p->push();
    if (f->flags & PS ) 
      p->use_font(7);
    else 
      p->use_font(2);

    p->get_current_loc(&h, &v);

    if (f->flags & PS) {
	p->p_movev( v - p->ps_top() );	/* cf ps_init_hv in prim.c */
    }
    else 
      p->p_movev( -v );

    p->movev (-.45/red);

    for (i=0; i<5; i++) {
	if (num[i] != ' ') {
	}
	else num[i] = '\0';
    }
#define PS_SCALE_2 12.0
    if ( atoi(num) % 2 ) { 
	p->moveh(staff_len > o_staff_len ? staff_len : o_staff_len);
	for (i=0; i<5 && num[i]; i++) {
	    p->moveh(-1.0 * f_a[2]->fnt->get_width(num[i]) 
		     * PS_SCALE_2 * red / 10);
	}
#ifdef NUM_INDENT
	p->moveh(".28 in");
#endif /* NUM_INDENT */
    }
    else {
#ifdef NUM_INDENT
	p->moveh(-1 * str_to_inch(".28 in"));
#endif /* NUM_INDENT */
    }
    for (i=0; i<5 && num[i]; i++) {
	p->set_a_char(num[i]);
    }
    p->pop();
    return;
}

#ifdef VAXX

int isupper(char c)
{
    if ((c >= 'A') && (c <= 'Z')) return (1);
	else return (0);
}
int islower(char c)
{
    if ((c >= 'a') && (c <= 'z')) return (1);
	else return (0);
}
int isdigit(char c)
{
    if ((c >= '0') && (c <= '9')) return (1);
	else return (0);
}
#endif /* VAX */

int online(char c)
{    
    switch (c) {
      case 'A': case 'C': case 'E': case 'G': case 'b': case 'd': case 'f':
    case '1': case '3': case '5':
	return(1);
      default:	return(0); 
    }
}

int dot_ch(struct list * l) 
{
    int i;

    for (i=2; i < 8; i++) {
	if (l->dat[i] != ' ') {
	    if (l->next->dat[i] == 'D' ) {
		return (1);
	    }
	}
    }
    return(0);
}

// this next proceeduer swallows dots in a b...b situation
// returns 1 if there is another b after some dots
// returns 0 if there is something other than b after 0 or more dots
//
int bdot(struct list* l)
{
  struct list *tt=l;
  while (tt->next){
    tt = tt->next;
    if (tt->dat[0] == 'b' || tt->dat[0] == 'B' )
      return(0);
    else if (tt->dat[0] == '.')
      continue;
    return(1);
  }
  return(0);
}
int rev_bdot(struct list* l)
{
  struct list *tt=l;
  while (tt->prev){
    tt = tt->prev;
    if (tt->dat[0] == 'b' || tt->dat[0] == 'B' )
      return(0);
    else if (tt->dat[0] == '.')
      continue;
    return(1);
  }
  return(1);
}

// -C bar_count means number every 5th bar, 
// -c barCount means number every first bar on every line
//

void check_bar(print * p, int j, int *l_p, struct list* l)
{
    if (bar_count || barCount || barCCount) {
	char *ch, *nxt=0;

	if (l) ch = l->dat;
	if (l->next) nxt = l->next->dat;

	if (( *ch == 'b' || *ch == 'B' || *ch == 'A')
	    && j+1 != *l_p 
	    && j != *l_p
	    && bdot(l)              // check for b..b
	    && *nxt != 'Q'
	    && *nxt != 'q') {	
	    n_measures++;
//	    printf ("measures %d\n", n_measures);
	    if (bar_count && ! (n_measures % 5))  
	      p->p_num(n_measures);
	    else if (barCCount) {
	      int i;
	      if (j > 4 )
		p->p_num(n_measures);
	      else if ( i=rev_bdot(l) )
		p->p_num(n_measures);
	      else 
		;
	    }
	}
	if ((barCount || barCCount) && j==0) {
	  if (bdot(l)) p->p_num(n_measures);
	      else p->p_num(n_measures+1);
	}
    }
}




void do_key_s( char ch[], print *p, font_list *f_a[], struct file_info *f)
{
    char flat='?';    
    if (f->note_flag == MOD_NOTES) flat = 24;

    p->push();
    
    switch (ch[1]){
    case 'g':
    case 'd':
    case 'a':
      if ( f->m_flags & FCLEF ) p->movev ( 1. * m_space);
      if (f->note_flag == MOD_NOTES) {
	p->movev(0.5 * f_a[0]->fnt->get_height(22));
	p->put_a_char(22);	
      } 
      else if (f->note_flag == ITAL_NOTES) {
	p->movev(0.5 * f_a[0]->fnt->get_height(23));
	p->put_a_char(23);	
      } 
      else {
	p->movev(.57 * m_space);
	p->put_a_char('#');
      }
      if (ch[1] == 'g') goto fin;
      
      p->moveh(f_a[0]->fnt->get_width('#'));
      p->movev(1.5 * m_space);
      
      if (f->note_flag == MOD_NOTES) 
	p->put_a_char(22);
      else if (f->note_flag == ITAL_NOTES)
	p->put_a_char(23);	
      else 
	p->put_a_char('#');
      
      if (ch[1] == 'd') goto fin;
      
      p->moveh(f_a[0]->fnt->get_width('#'));
      p->movev(-2 * m_space);
      if (f->note_flag == MOD_NOTES)
	p->put_a_char(22);	
      else if (f->note_flag == ITAL_NOTES)
	p->put_a_char(23);	
      else 
	p->put_a_char('#');
      
      break;
    case 'f':
    case 'b':
    case 'e':
      if ( f->m_flags & FCLEF ) p->movev ( 1. * m_space);
      p->movev ( 2.43 * m_space);
      p->put_a_char(flat);
      if (ch[1] == 'f') goto fin;
      
      p->movev ( -1.5 * m_space);
      p->moveh(0.015 + f_a[0]->fnt->get_width(flat));
      p->put_a_char(flat);
      if (ch[1] == 'b') goto fin;
      
      p->movev ( 2 * m_space);
      p->moveh(0.015 + f_a[0]->fnt->get_width(flat));
      p->put_a_char(flat);
      break;
    case 'c':			// do nothing
      break;
    default:
	dbg1 (Warning, "tab: dvi_format: unsupported key signature %c\n",
	      (void *)ch[1]);
	break;
    }
fin:
    p->pop();
    return;
}

void do_time_sig( char ch[], int j, int font,
		  print *p, font_list *f_a[], struct file_info *f)

{
    int i;
    double d_i_space = str_to_inch(interspace);
    int i_space = inch_to_dvi(d_i_space);
    char cc = '\0';
    double staff_h = str_to_inch(staff_height);

    if (ch[1] == 'M') {	/* g cleff and notes*/
	// this is handled as notes
    }
    else if (isdigit(ch[1])) {

	if ( j == 0 ) 	  /* before staff, Karen Meyers style */
	    p->moveh( -0.28 );
	else {
	    p->moveh(0.11);	// this should match line in pass1 !
	}

	if (f->flags & PS) font = 4;
	else font = 2;
	p->use_font(font);

	if (strchr (ch, (int)'-') ) {
	    double len=0.0;	                    /* eg 3-4 or 12-4 */

	    p->push();	                   /* in tablature too */

	    if (f->flags & CON_SEV ) 
		p->p_movev (i_space);
	    if (f->line_flag == ON_LINE) 
		p->movev(3.0 * d_i_space);
	    else 
		p->movev(3.5 * d_i_space);
	    if (f->flags & FOUR)  p->movev(-1 *  d_i_space);
	    else if (f->flags & FIVE)  p->movev(d_i_space/ -2);
		  
	    p->push();		/* upper characters */
	    i=1;
	    while (ch[i] != '-')  /* was 1.5 */
		len += 1.0 * f_a[font]->fnt->get_width(ch[i++]);

	    p->moveh (len / -2.0);
 
	    p->movev(-.42 * m_space);
	    i=1;
	    while (ch[i] != '-')  p->set_a_char(ch[i++]);
	    p->pop();

	    p->movev(f_a[font]->fnt->get_height('1') + .3 * m_space);
	    j= ++i;	len = 0.0;
	    while (ch[j] != ' ') 
	      len += 1.0 * f_a[font]->fnt->get_width(ch[j++]); /* was 1.5 */

	    p->moveh (len / -2.0);
 
	    while (ch[i] != ' ')  p->set_a_char(ch[i++]);
	    p->pop();
	    /* PS font is 4, 24 pt */
	    if (f->flags & NOTES) {
	        if (f->flags & PS ) {
		  font = 6;
		  p->use_font(font);
		}
		p->movev ( -st_text);                  /* place in music */
		p->movev ( - f->c_space);
		p->movev ( -text_sp * f->n_text);
		p->movev (-3.2 * m_space);
		p->push();

		len = 0;
		i=1;
		while (ch[i] != '-') 
		    len += 1.5 * f_a[font]->fnt->get_width(ch[i++]);
		/* 1.5 = 1.2 * 1.25 */
		/* ten to twelve correction, 12 to 15 corr. */
		p->moveh (len / -2.0);
		//		p->put_rule(0.04, 0.04);
		i=1;
		while (ch[i] != '-')  p->set_a_char(ch[i++]);
		p->pop();
		p->movev(1.8 * f_a[font]->fnt->get_height('1') + .3 * m_space);
		j= ++i;	len = 0.0;
		while (ch[j] != ' ')  
		  len += 1.5 * f_a[font]->fnt->get_width(ch[j++]);
		p->moveh (len / -2.0);
		while (ch[i] != ' ')  p->set_a_char(ch[i++]);
	    }
	}
	else {		/* eg. 3 , no music */
	    font = 0;
	    p->use_font(font);
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
		dbg1 (Warning,
		      "tab: dvi_format: no character for large number %c\n",
		      (void *)ch[1]);
		break;
	    }
	    p->push();
	    if (f->line_flag == ON_LINE) p->movev(3.0 * d_i_space);
	    else p->movev(3.5 * d_i_space);
	    if (f->flags & FOUR)  p->movev(-1 *  d_i_space);
	    else if (f->flags & FIVE)  p->movev(d_i_space/ -2);

	    p->movev( 0.5 * f_a[font]->fnt->get_height(cc));

	    if (ch[2] && ch[2] == 'Q' ) {
		p->set_highlight();
		if (f->m_flags & PAREN ) p->gray_highlight();
		p->put_a_char(cc);
		p->clear_highlight();
	    }
	    else {
		if (ch[2] && ch[2] == '|' ) {
		    p->push();
		    p->moveh( 0.47 * f_a[font]->fnt->get_width(cc));
		    p->movev( 0.4 * f_a[font]->fnt->get_height(cc));
		    p->put_rule(0.008, 1.86 * f_a[font]->fnt->get_height(cc));
		    p->pop();
		}
		p->put_a_char(cc);
	    }
		
	    p->pop();
	    if (f->flags & NOTES) {
		p->push();
		p->movev ( -st_text -text_sp * f->n_text -m_space );
		p->movev ( - f->c_space);
		p->movev (-2.0 * m_space);
		p->movev (f_a[font]->fnt->get_height(cc) / 2.0);
		p->put_a_char(cc);
		if (f->m_flags & TWOSTAFF ) {
		    p->push();
		    p->movev( -5 * m_space - staff_h);
		    p->movev( -str_to_inch(interspace) );
		    p->movev( -text_sp * f->n_text );
		    p->movev ( -f->c_space );
		    p->put_a_char(cc);
		    p->pop();
		}
		p->pop();
	    }
	}
	p->use_font(0);
    }
    else {			/* a character, not a digit */
	if (f->flags & FOUR)  p->movev(-1 *  d_i_space);
	else if (f->flags & FIVE)  p->movev(d_i_space/ -2);

	if ( j == 0 ) 
	    p->moveh( -1 * str_to_inch("0.28 in"));
	if (baroque)
	    p->movev(-.012 + d_i_space/-2.0 );
	p->movev((8.25 * d_i_space 
		   + f_a[0]->fnt->get_height('G')) / 2.0 );
	if (ch[1] == 'O' ) { 
	    p->movev(-3.12 * d_i_space);
	    p->put_a_char(18); // special O time signature
	}
	else if  (ch[1] == 'o' ) { 
	    p->movev(-1.85 * d_i_space);
	    p->put_a_char(17); // special O time signature
	}
	else {
	  p->put_a_char(ch[1]);
	  if (f->flags & NOTES) {
		p->push();
		p->movev((-7.25 * d_i_space - 
			  f_a[0]->fnt->get_height('G')) / 2.0 );

		p->movev ( -st_text -text_sp * f->n_text -m_space );
		p->movev ( - f->c_space);
		p->movev (-2.0 * m_space);
		p->movev (f_a[font]->fnt->get_height(cc) / 2.0);
		p->put_a_char(ch[1]);
		p->pop();
	  }
	}
    }
    return;
}





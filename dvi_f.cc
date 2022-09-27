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
#include "win.h"
#include "tfm.h"
#include "tab.h"
#include "print.h"
#include "sizes.h"
#include "system.h"

/* EXTERNAL */
void mapflag(print *p, font_list *f_a[], char c, struct file_info *f);
void mapchar(print *p, font_list *f_a[], unsigned char c, struct file_info *f);
void change_ps_font(char *line);
int setflag(file_info *f, char * string, pass pass);
void format_title(print *p, i_buf *i_b, font_list *f_a[], struct file_info *f);

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
  int i=0, cc, ccc;
  int skip_spaces;		/* spaces to skip */
  static double last_move;	/* last interchar space, for post orn. */
  double staff_h = str_to_inch(staff_height);
  double d_i_space = str_to_inch(interspace);
  int i_space = inch_to_dvi(d_i_space);
  double EM = str_to_inch(em);
  int last_line_in_grid = 0;
  char *ch;			/* in this case, the chord to print */
  char *nxt=0;		/* the next chord */
  struct list *pp;
  static int double_grid=0;
  extern char flag_indent[];
  double val;
  int font=0;
  double cap_dot_offset;
  int wide_note = 0;

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
      if (ch[1] == 'Q'  || ch[1] == 'L') {
	p->set_highlight();
	if (f->m_flags & RED )
	  p->red_highlight();
	else
	  p->gray_highlight();
      }
      if (j == 0 || j == (*l_p - 1)) {
//	printf ("dvi_f.cc: 133: %f %f\n", 1.3 * staff_h, val * d_i_space + staff_h);
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
	  if (f->flags & FIVE) {
	    p->put_rule(1.3 * staff_h, v_bar - d_i_space);
	  }
	  else if (f->flags & FOUR) {
	    p->put_rule(1.3 * staff_h, v_bar - 2 * d_i_space);
	  }
	  else
	    p->put_rule(1.3 * staff_h, v_bar);
	}
      }
      else {
	p->put_rule (staff_h,
		     val * d_i_space
		     + staff_h);
      }
    }
    if (ch[1] == 'X' || ch[1] == 'L')  // L is no count, light bar
      ;
    else check_bar(p, j, l_p, l);

    //	printf ("dvi_f: measures %d ch %s\n", n_measures, ch);

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
	    		 -( st_text + text_sp * f->n_text + m_space + m_space
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
      if (!(f->m_flags & QUIET ))
	dbg1(Warning,
	   "tab: dvi_f: number after barline %c\n", (void *)((int)ch[1]));
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
      if (f->flags & FIVE) {
	p->put_rule(str_to_inch(thick_bar), v_bar - d_i_space);
      }
      else if (f->flags & FOUR) {
	p->put_rule(str_to_inch(thick_bar), v_bar - (2 * d_i_space));
      }
      else
	p->put_rule(str_to_inch(thick_bar), v_bar);
    }
    else {
      if (f->flags & FIVE) {
        p->put_rule(str_to_inch(thick_bar), val * d_i_space + staff_h);
      }
      else if (f->flags & FOUR) {
        p->put_rule(str_to_inch(thick_bar), val * d_i_space + staff_h);
      }
      else
	p->put_rule (str_to_inch(thick_bar),
		     val * d_i_space + staff_h);
    }
    if (ch[1] == 'X' )
      ;
    else check_bar(p, j, l_p, l);
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
    /* v draws the v and upside doown v to mark the page turn
     * in the original.  But the baroque font is different
     * and has numbers, so it doesn't work for baroque
     */
  case 'v':
    //      printf("v here\n");
    if (! baroque ) {
      p->push();
      p->moveh(-0.5 * f_a[0]->fnt->get_width(9));
      p->movev(f_a[0]->fnt->get_height(9));
      if (f->line_flag != ON_LINE)
	p->movev(0.5 * d_i_space);	// wbc Sept 06
      p->put_a_char (8);
      p->movev(5.5 * d_i_space);
      p->put_a_char (9);
      //      p->movev(-5.5 * d_i_space);
      p->pop();
    }
    break;
  case '.':
    p->push();
    p->movev(baselinespace);
    if (f->flags & CON_SEV )
      p->p_movev (i_space);
    if (f->line_flag == ON_LINE)
      p->p_movev((int)(1.5 * i_space));
    else p->p_movev(2 * i_space);

    for (i=0; i < (f->flags & FOUR ? 3:f->flags & FIVE ? 4:f->m_flags & SEVEN ? 6:5);i++) {
      mapchar(p, f_a, /* ch[i+3] */'Z', f);
      p->p_movev(i_space);
    }
    p->pop();
    p->push();
// the following stolen from the code for b2
    if (ch[1] > '0' && ch[1] <= '9' ) {
      if (!(f->m_flags & QUIET ))
        dbg1(Warning,
           "tab: dvi_f -number after a dot %c\n", (void *)((int)ch[1]));
      p->push();
      p->movev(-.1);
      /*            p->put_a_char(140 + ch[1] - '0');*/
      p->use_font(3);   // was 2, 3 at rainer's request
      p->put_a_char(ch[1]);
      p->use_font(0);
      p->pop();
    }


    if (f->m_flags & COUNT_DOTS) {
      p->movev(6.0 * d_i_space);
      // printf ("check a dot %s\n", ch);
      if (ch[1] == 'X' )
         ;
      else check_bar(p, j, l_p, l);
    }
    p->pop();

    if (f->flags & NOTES ) {

//      if (f->line_flag == ON_LINE) p->movev( -5.5 * d_i_space);
	if (f->line_flag == ON_LINE) p->movev( -.5 * d_i_space);
// 2019      else p->movev ( -6.0 * d_i_space);     /* to top of staff */

      if (f->flags & FIVE) {
	p->movev(i_space);
      }
      else if (f->flags & FOUR) {
	p->movev(2 * i_space);
      }

// 	printf("dvi_f.cc: st_text %f text_sp %f  f->n_text %d f->c_space %f\n",st_text, text_sp, f->n_text, f->c_space);
      p->movev ( -st_text);
      p->movev ( -text_sp * f->n_text - f->c_space);
      p->movev ( -2 * m_space + .004 /* fudge */);  // was -2 July 2019 wbc  was -0.01
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
    //            fprintf(stderr, "in dvi_f: got $\n");
    {
      int i, j=0;
      char buffer[80];
      char *bp;
      bp = &buffer[0];
      for (i=1; (c = l->dat[i]) != NEWLINE && c ; i++) {
	*bp=c;
	bp++;
      }

      buffer[--i] = 0;
      if (setflag(f, buffer, second))
	break;		// return if we set the flag
    }

    break;
  case 'm':
    c = 191;
  case 'Q':			/* squiggle */
  case 'q':
    p->movev( 5.9 * d_i_space);
    if (f->flags & CON_SEV )
      p->p_movev (i_space);
    if (f->m_flags & SEVEN)
      p->movev(0.5 * d_i_space);
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
      //	  printf("text ps %f n_text %d\n", text_sp, f->n_text);
      p->movev ( -text_sp * f->n_text );
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
		     (val + 4) * d_i_space + staff_h + 0.940 + (text_sp * f->n_text));
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
	p->movev(-text_sp * f->n_text);
      }
      p->movev  (-1 * (val + 4) * d_i_space + staff_h );
      p->put_rule(str_to_inch("0.60 in"), staff_h);
    }
    break;
  case '*':			/* a mark, as in AA BB */
    p->push();
    p->use_font(2);
    p->movev(-f_a[0]->fnt->get_height(f->flag_flag == ITAL_FLAGS ? 217 : 195));
    p->put_a_char(ch[1]);
    p->use_font(0);
    p->pop();
    break;
  case '[':			// a short bit of text
    //    dbg0(Warning, "dvi_format: not implemented\n");
    p->push();
    {
      int i;
      p->use_font(2);
      p->movev(-d_i_space/2
	       -f_a[0]->fnt->get_height(
		   f->flag_flag == ITAL_FLAGS ? 217 : 195));
      for (i=1; i< 15 && ch[i] != '\0' && ch[i] != '}'; i++ )
	p->set_a_char(ch[i]);
      // printf("FIX ME line 786\n");
      p->use_font(0);
    }
    p->pop();
    break;
  case '&':			/* for after ornaments */
    // printf ("HERE dvi_f.cc line 710  i is %d\n", i);
    p->moveh(-1.0 * last_move);
    p->moveh(0.75 * EM);
    if (baroque && l->prev && l->prev->dat[0] != '2' ) p->moveh(.05);
    if (f->flags & MARKS) p->put_rule(0.01, -0.17);
    if (ch[1] != '-') {
      //printf("dvi_f.cc: HERE flag ornament on first line\n"); // wbc sept 2015 adjust ornament
      p->moveh(2 * f_a[0]->fnt->get_width( l->prev->dat[0]));
      // printf ("HERE dvi_f.cc: line 737  %c\n", ch[1]);
      switch (ch[1]) {
      case '{':
	// printf ("HERE dvi_f.cc line 721  i is %d\n", i);
	skip_spaces = 0;
	p->push();
	my_underline(p, f_a,  &skip_spaces, 11, i);
	p->pop();
	break;
      case '[':
	//printf ("HERE dvi_f.cc line 728 \n");
	skip_spaces = 0;
	i = 1;
	p->push();
	my_underline(p, f_a,  &skip_spaces, 7, i);
	p->pop();
	break;
      case ']':
	//printf ("HERE dvi_f.cc line 736\n");
	skip_spaces = 0;
	i = 1;
	p->push();
	//printf("dvi_f.cc: 739: end of [] \n");
	do_r_uline(p, &skip_spaces, 7, 8, i);
	p->pop();
	break;
      default:
	p->push();
	if (f->line_flag == ON_LINE)
	  p->p_movev((int)(-0.5 * i_space));
	p->put_a_char(ch[1]);
	p->pop();
      }
    }  // end ch[1] != '-'
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
      else if ( ch[0] == '0') {
	if (baroque) {
	  p->movev(( f_a[0]->fnt->get_height(15))/2.0);
	  p->put_a_char(15);
	}
	else {
	  p->movev(( f_a[0]->fnt->get_height(0375))/2.0);
	  p->put_a_char(0375);
	}
      }
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
      static int grids=0;

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
	if(ch[1] != 'M')             /* wbc feb 07  for music no tablature*/
	  p->put_a_char(194);                          /* flag staff */

	if (cc == '*') {                            /* print dot */
	  if (f->flag_flag == CONTEMP_FLAGS) {
	    p->moveh(0.04
		     + l->space / 10.0 );
	    p->movev ("0.025 in");
	  }
	  p->put_a_char('.');
	}
	else if ( cc == '.' ) {
	  p->moveh(0.04
		   + l->space / 10.0 );
	  if (f->flag_flag == CONTEMP_FLAGS) {
	    p->movev ("0.025 in");
	  } 
	  p->put_a_char('.');
	}
	else if ( cc == '|') {              /* print extra bar  */
	  p->move_n_v("0.19 in");
	  p->movev  ( grids * str_to_inch ("0.05 in"));

	  if (l->next && l->next->dat[1] != '|') {
	    // printf("dvi_f: | and next ne | \n");
	    if (!double_grid ) {    /* must be following dot */
	      if (l->prev && l->prev->prev && l->prev->prev->dat[1] == '.') {
		p->moveh  ( -1.0 *
			    (l->space + l->padding)/ 2.2
			    + str_to_inch(".01 in"));
		p->put_rule((l->space + l->padding)/ 2.2,
			    /* staff_h */ str_to_inch("0.005 in"));
		// printf("dvi_f: not double_grid printing half rule\n");
              }
	      else {
		p->moveh  ( -1.0 *
			    (l->space + l->padding)
			    + str_to_inch(".01 in"));
		p->put_rule((l->space + l->padding),
			    /* staff_h */ str_to_inch("0.005 in"));
		// printf("dvi_f: not double_grid printing half rule\n");
	      }
	    }
	    else { // not following a dot, double_grid is true
	      /* finish double grid */
	      double_grid=0;
 	      // printf("double grid cleared, not printing anything\n");
	    }
	  }
	  else {   // cc not equal either * nor | 
	    double_grid++;
	    p->moveh(0.006); /* fudge factor, unknown reason */
	    p->put_rule((l->space + l->padding),
			/* staff_h */ str_to_inch("0.005 in"));
	    // printf("dvi_f: double_grid  just set printing rule\n");
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
	// note the the 3/4 circle y has its own dot, the half circle doesn't
	if (ch[1] == '-' || ch[3] == 'Z' || ch[1] == 'b' /* || ch[1] == 'B' */ ) {
	  // printf("DOT\n");
	  p->put_a_char(92);
	}
	if (f->flags & NOTES ) {
	  p->movev(-(/*        6 * d_i_space */
		     + st_text
		     + text_sp * f->n_text
		     + f->c_space
		     + 5 * m_space));
	  p->put_a_char(c);
	  p->put_a_char(92);
	}
	p->pop();
	p->moveh(0.5 * (f_a[0]->fnt->get_width(c)));
	if (ch[1] == '0') { // the 0 flag
	  p->put_a_char(194);
	}
	else if (ch[1] == 'b') {             /* draw BARLINE */
	  int spaces =
	    (f->m_flags & SEVEN ? 6 : f->flags & FOUR ?
	     3 : f->flags & FIVE ? 4 : 5);
	  if (f->flags & CON_SEV )
	    p->p_movev (i_space/2);
	  if (f->line_flag & ON_LINE )
            p->p_movev (-i_space/2);
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
	if (!baroque)
	  mapflag(p, f_a, c, f);

	p->moveh((l->space + l->padding) );
	if (strchr("^+&:", (int)*nxt)) /* another staff of */
	  p->moveh(l->next->padding);        /*  ornaments */
	//	mapflag(p, f_a, c, f);

	p->push();
	if (baroque) {
	  p->movev(-.125);
	  p->put_a_char(0205);
	}
	else {
	  p->use_font(2);
	  p->movev(-.34);
	  p->put_a_char('3');
	  p->use_font(0);
	}
	p->pop();

	p->moveh((l->space + l->padding) );
	if (strchr("^+&:", (int)*nxt)) /* another staff of */
	  p->moveh(l->next->padding);        /*  ornaments */
	//	mapflag(p, f_a, c, f);
	p->moveh(f_a[0]->fnt->get_width('d'));
	p->saveloc(6);
	if ( baroque )
	  p->movev(0.02);
	else
	  p->movev(-.24);
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
	  else if (f->m_flags & RED )
	    p->red_highlight();
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
	  if (f->m_flags & PAREN ) {
	    p->clear_highlight(); /* p->paren_highlight()*/;
	  }
	  else if (f->m_flags & RED )
	    p->red_highlight();
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
    if (c == '+') {
      int ii;
      for (ii=2; ii< 10; ii++) {
	if ((baroque && l->next && l->next->dat[ii] == 'E')
	    || (baroque && l->next && l->next->dat[ii] == 'D')){
	  wide_note = ii;
	  // printf("HERE we got a plus with an E aug 2019 %d\n", wide_note);
	}
      }
    }
    p->pop();			/* end flags */

    if (( ch[1] == 'W' ||  ch[1] == 'w')          /* wbc sept 2015 */
	&& ch[0] != 'Y'  /* || ch[1] == 'w' */) { /* a tie or slur */
      double t_width, t_note;
      struct list *tt = l->next;

      t_width = l->space + l->padding;

      while (tt->dat[0] == 'b' || tt->dat[0] == '&'
	    || tt->dat[0] == 'B' || tt->dat[0] == '.' ) {
	t_width +=( tt->space + tt->padding);
	if (tt->next)
	  tt = tt->next;
	else break;
      }
      p->push();
      p->moveh(t_note = f_a[0]->fnt->get_width('w'));
      p->movev(-1 * f_a[0]->fnt->get_height(0302));
      p->do_tie(t_width - (1.46 * t_note));	/* length of tie in inches */
      if (f->flags & NOTES ) {
	if ( ! (f->flags & TXT )) p->movev(f_a[0]->fnt->get_height(0302) );

	p->movev ( -st_text -text_sp * f->n_text -m_space );
	p->movev ( - f->c_space);
	p->movev (-2.0 * m_space);

	p->do_tie (t_width - t_note);
      }
      p->pop();
    }

    /* LETTERS HERE ! letters here */

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
    for (i=2; i < STAFF; i++) { // the letter notes loop right here

      /*	    printf ("ch[%d] is %d %c\n", i, ch[i], ch[i]);  */
      if (ch[0] == 'O' || (l->prev && l->prev->dat[0] != 'G'
			   && l->prev->dat[i] == 'Q')) {
	p->set_highlight();
	if (f->m_flags & PAREN )
	  p->paren_highlight();
	else if (f->m_flags & RED )
	  p->red_highlight();
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
	skip_spaces++;
	break;
      case 'r':         /* return to earlier part sign repeat */
	// mapchar(p, f_a, (unsigned char)0xff, f);
	if (i > 4) {
	  p->push();
	  // printf ( "case r: i is %d skip %d ch %s\n", i, skip_spaces, ch);
	  /* was 9.3 to allow space for 7th line */
	  p->p_movev((7.7 - (i - skip_spaces)) * i_space);
	  if (ch[8] && ch[8] != ' ' && ch[8] != 'r') {
	    p->p_movev(i_space);
	  }
	  mapchar(p, f_a, (unsigned char)0xff, f);
	  p->pop();
	}
	/* else  p->put_a_char(0xff);  */
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
	if (baroque ) {
	  /* wbc aug 2019 */ /* p->put_a_char (240); */
	  // printf ("mordent HERE\n");
	}
	break;
      case '{':		/* slanting lines */
	my_underline(p, f_a, &skip_spaces, 11, i);
	break;
      case '}':
	if (baroque) {
	  do_thick_uline(p, &skip_spaces, 11, 12, i);
	}
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
	if (f->flags & DVI_O) {
	  p->put_a_char(cc);
	}
	do_uline(p, &skip_spaces, 7, 8, i);
	break;
      case 240: /* wbc aug 2019 new <! special characters  here */
	/* 2mordent twomordent */
	// printf("dvi_f: case 240  %d  %s\n", i, ch);
	if (skip_spaces) {
	  p->p_movev(skip_spaces * i_space);
	  skip_spaces = 0;
	}
	p->push();
	if (ch[0] != '+' && ch[0] != '&')
	  p->moveh(-0.097);
	p->put_a_char (cc);
	p->pop();
	break;
      case 241: /* wbc aug 2019 new <! special characters  here */
	/* 3mordent threemordent */
	// printf("dvi_f: case 241  %d  %s\n", i, ch);
	if (skip_spaces) {
	  p->p_movev(skip_spaces * i_space);
	  skip_spaces = 0;
	}
	p->push();
	if (ch[0] != '+' && ch[0] != '&')
	  p->moveh(-0.1);
	p->put_a_char (cc);
	p->pop();
	break;
      case 229:
	// baroque, two parallel vertical strokes under
	p->push();
	p->movev(0.04);
	p->moveh(-0.04);
	p->put_a_char(124);
	p->moveh(0.06);
	p->put_a_char(124);
	p->pop();
	break;
      case 133: // upside down underline
	if (skip_spaces) {
	  p->p_movev(skip_spaces * i_space);
	  skip_spaces = 0;
	}
	if (f->flags & DVI_O) {
	  p->put_a_char(cc);
	}
	do_r_uline(p, &skip_spaces, 7, 8, i);
	break;
      case 134: // double wiggle underline
	if (skip_spaces) {
	  p->p_movev(skip_spaces * i_space);
	  skip_spaces = 0;
	}
	if (f->flags & DVI_O) {
	  p->put_a_char(cc);
	}
	do_w_uline(p, &skip_spaces, 7, 8, i);
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

	else if (cc >= '0' && cc <= '9' ) { // numbers  bourdons
	  p->push();
	  if (baroque && i == 8 && (cc >= '1' && cc <= '9')) {

	    if (f->m_flags & SEVEN )
	      p->movev(d_i_space);
	    p->movev(0.09);
	    switch (cc) { 
	    case '1':
	      p->put_a_char(8);
		break;
	    case '2':
	      p->put_a_char(9);
		break;
	    case '3':
		p->put_a_char(10);
		break;
	    case '4':
		p->put_a_char(11);
		break;
	    case '5':
		p->put_a_char(12);
		break;
	    case '6':
		p->put_a_char(13);
		break;
	    case '7':
		p->put_a_char(14);
		break;
	    case '8':
		p->put_a_char(6);
		break;
	    case '9':
		p->put_a_char(7);
		break;
	    }
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
	else if (/* cc == '`' || */ cc == ',') { /* (back) comma on line*/
	  /* wbc aug 2019 removed the ` backquote for a comma */
	  p->push();
	  p->moveh("0.7 pt");
	  p->movev("4.0 pt");
	  p->put_a_char('\'');
	  p->pop();
	}
	else if (c == '+' && cc == '_' ) { /* special my_underline ornament */
	  p->push();                       // the underbar is always a pre ornament
	  p->moveh(0.76 * EM);
	  p->movev(-.025);
	  if (baroque) {
	    p->moveh(-.06);
	    //  printf ("under smile %c - \n", l->next->dat[i]);
	    if (l->next &&
		     (l->next->dat[i] == 'D'
		      || l->next->dat[i-1] == 'D'
		      || l->next->dat[i-2] == 'D' )) {
	      p->moveh(0.05);
	      // printf ("dvi_f.cc: smile: case 1\n");
	    }
	    else if (l->next && l->next->dat[i] == 'E') {
	      p->moveh(.04); /* was .21 */
	      // printf ("dvi_f.cc: smile: case 2\n");
	      if  (l->next && l->next->dat[i+1] == ' ') {
		p->movev(0.06);
		// printf ("dvi_f.cc: smile: case 3\n");
	      }
	    }
	    else if (l->padding == 0.0900) {
	      p->moveh(0.025);
	    }
	    else if (l->next && l->next->dat[i] == ' ') {
	      // printf("dvi_f: 1 padding %f\n", l->padding);
	      p->movev(-.02); /* wbc July july 2019 fix up the underbar */
	      p->moveh(-.097);
	      // printf ("dvi_f.cc: smile: case 4\n");
	    }
	    // if (i > 2 && l->next && l->next->dat[i-1] == 'D') p->movev(-0.094);
	    else {
	      // printf("dvi_f: 2 padding %f\n", l->padding);
	      p->moveh(-.097);
	      // printf ("dvi_f.cc: smile: case 5\n");
	    }
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
	  if (baroque && l->dat[i-1] == 'E' ) {
	    p->moveh ( "0.02 in");
	  }
	  if (f->line_flag == ON_LINE) {
	    if (c == '+' && baroque) {
	      p->movev (italian_offset);
	      p->movev (.35 * d_i_space);
	      p->movev(-0.08);  // wbc July 2019 fudge factor
	    }
	    else {
	      p->movev (italian_offset);
	      p->movev (-.2 * d_i_space);
	    }
	  }
	  else { // not ON LINE
	    /* if (baroque) p->movev(0.017); */ /* wbc sept 2022 - try to center the dot in the line */
		  if (baroque) p->movev(0.008);	
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
	  // printf ("HERE\n");
	  if ((f->m_flags & DSUP )) {
	    mapchar(p, f_a, 'd', f);
	    //	    p->put_a_char('d');
	  }
	  else {
	    mapchar(p, f_a, 'D', f);
	    // p->put_a_char('D'); /* should be in getsystem */
	  }
	  last_line_in_grid = 0;
	  p->pop();
	}
	else if (baroque && cc == 'd'
		 && l->prev && l->prev->prev
		 && i > 2
		 && l->prev->dat[i - 1] == ','
		 && l->prev->prev->space < 0.065 ) {
	  //   p->put_a_char('D');
	  p->push();
	  mapchar(p, f_a, 'D', f);
	  p->pop();
	}
	// baroque f madness  July 2019 wbc
        else if (baroque && cc == 'f' && i > 1 ) {
	  //above
	  if  (ch[i-1] == 'a' // squat
	       || ch[i-1] == 'G'
	       // below
	       || ( ch[i+1] == 'g' && ch[i-1] != 'f' )
	       // || ( ch[i+1] == 'h' && ch[i-1] != 'f' )
	       ) {
	    p->push();
	    // squat f
	    // printf("Here found f to be squat correctedi - %c + %c\n",  ch[i-1],  ch[i+1]);
	    mapchar(p, f_a, 213, f);
	    p->pop();
	  }
	  else if (ch[i-1] == 'f'  // medium`
		   || ch[i+1] == 'b'
		   || ch[i+1] == 'c'
		   || ch[i-1] == 'c'
	       || ch[i-1] == 'h'
	       || ch[i+1] == 'h'
		   || ( ch[i+1] == 'g' && ch[i-1] == 'f' )
		   || ch[i+1] == 'f') {
	    p->push();
	    // medium
	    // printf("Here found f to be med   correctedi - %c + %c\n",  ch[i-1],  ch[i+1]);
	    mapchar(p, f_a, 214, f);
	    p->pop();
	  }
	  else {
	    p->push();
	    // full sized
	    mapchar(p, f_a, cc, f);
	    p->pop();
	  }
	}
	else if (baroque && cc == 'h'
		 && i > 3
		 && (ch[i-1] == 'f' ||  ch[i-1] == 'h')) {
	  p->push();
	  p->moveh(".04 in");
	  mapchar(p, f_a, 'h', f);
	  p->pop();
	}
	else if (strchr(".+:#", (int)cc) && nxt &&
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
	  p->moveh(b_d_pad);  // because the / is at a different place
	  if (baroque)
	    mapchar(p, f_a, 114, f);
	  else
	    mapchar(p, f_a, cc, f);
	  p->pop();
	}
	else if ( cc > 'p' && cc < 'x' && c != '+' ) {
	  //		  printf ("here c is %d %c\n", cc, cc);
	  if ( f->extended_character_set ) {
	  //  printf ("HERE %c\n", cc);
	    p->push();
            p->use_font(2);
            p->put_a_char(cc);
            p->use_font(0);
            p->pop();
	  }
	  else if ( cc == 'q' )
            p->put_a_char(070); // wbc sep 2013 a Kapsberger ornament //
          else {		// use extended char set if char not defined
	    p->push();
	    p->use_font(2);
	    p->put_a_char(cc);
	    p->use_font(0);
	    p->pop();
          }
	}
	else if ( (c == '+' || c == '&') && cc == 'x' ) {
	  // this is for ornaments as opposed to ital ten (X)
	  p->push();

	  if (baroque && wide_note && wide_note != i) {
	    p->moveh (0.18);  // this should match line 256 in pass1.cc
	  }


	  if (f->line_flag == ON_LINE) /* wbc Dec 16 2002 */
	    p->movev (str_to_inch(italian_offset));
	  p->moveh("0.03 in");
	  p->put_a_char(cc);
	  //		  fprintf(stderr, "print x ornament here\n");
	  p->pop();
	}
	else if ( c == '&' && cc == '^') {
	  p->push();
	  p->moveh("0.05 in");
	  mapchar(p, f_a, cc, f);
	  p->pop();
	}
	else if ( i+skip_spaces == 8 && (cc == 's' || cc == 't') && wide_note) {
	  p->push();
	   p->moveh (0.1);
	   mapchar(p, f_a, cc, f); 
	   p->pop();
	}
	else if ( cc == '/') {  /* wbc July 2019 */
	  p->push();
	  if (baroque) {
	    // printf("dvi_f.cc: 1722 bourdon test  ch %s %d\n", ch, (i+skip_spaces));
	    if ( (i + skip_spaces) == 8||(i + skip_spaces) == 9) {  //bourdon
	      if (wide_note && wide_note != i) {
		 p->moveh (0.1);  // was .2  this should match line 256 in pass1.cc
		//   printf("dvi_f.cc: 1722 bourdon wide note \n");
	      }
	      //  printf("dvi_f.cc: 1729 bourdon not wide  \n");
	      //  p->moveh(0.1);  // wbc sept 8 19 test case
	      mapchar(p, f_a, 114, f);  // wbc for bourdons  114 = r
	    }
	    else if (c != '+' && c != '&') {  // the slash in note position
	      p->moveh("-0.086 in");
	      if (  l->dat[i-1] == '.' || l->dat[i-1] == ' ') {
		// printf("dvi_f.cc: 1736: shift slash up %c %d %s\n",
		// c, i, l->dat);
		p->movev(-0.07);
	      }
	      mapchar(p, f_a, cc, f);
	    }
	    else if (c == '&') {  // the slash after the note
	      p->moveh("0.036 in");
	      mapchar(p, f_a, 114, f);
	    }

	    else {  // a slash before the note is really between notes
	      p->moveh("-0.002 in");
	      p->movev(-0.061);
	      if ( l->padding > 0.10) { // this is a hack for the extra padding
		// when the down curved D is used
		// ouch - a kerning table
		if (l->next && l->next->dat[i] == 'D') {
		  p->moveh(0.06);
		}
		else if (l->next && l->next->dat[i] == 'E') { 
		  p->moveh(0.15);
		}
		else  {
		  p->moveh(0.06);
		}
	      }
	      if (l->next && l->next->dat[i-1] == 'd') {
		p->movev(-0.012);
	      }
	      if (l->next && l->next->dat[i] == 'D') {
		// printf ("HERE D padding %f\n", l->padding);
		p->movev(-0.012);
	      }
	      if (l->next && l->next->dat[i] == 'c') {
		p->movev(-0.012);
	      }
	      // the following is not an ornament thing
	      //if (c != '+' 
	      //  && l->dat[i-1] == '.' || l->dat[i-1] == ' ') {
	      //  printf("dvi_f.cc: 1769: HERE %c %d %s\n", c, i, l->dat);
	      //  p->movev(-0.1);
	      //}
	      
	      // printf ("dvi_f.cc: 1768 - -  char %cc padding %f   %d\n",
		//c, l->padding, i+skip_spaces);
	      // mapchar(p, f_a, 114, f);
	      mapchar(p, f_a, cc, f);
	    }
	  }
	  else {  // the renaissance way
	    p->moveh("-0.007 in");
	    mapchar(p, f_a, cc, f);
	  }
	  p->pop();
	}
	else if ( cc == '_' && baroque && c != '&') { // wbc july 2019 for underbar, not pre ornament
	  p->push();
	  //if ( i > 2 && l->dat[i-1] == 'D') {
	  p->movev(-0.051);
	  p->moveh(-0.064);
	  //}
	  mapchar(p, f_a, cc, f);
	  p->pop();
	}

	else {
	  p->push();
	  
	  if (baroque && wide_note && wide_note != i ) {
	    // printf ("padding %f  %s\n", l->padding, ch);
	    if (l->padding > 0.090) p->moveh (0.09);  // this should match line 256 in pass1.cc
	  }
	  
	  mapchar(p, f_a, cc, f);
	  p->pop();
	}
	p->p_movev(i_space);
	// p->put_a_char (0); // wbc July 2019 debug - print a square a box
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

// print the bar count if appropriate and increment n_measures
// -C bar_count means number every 5th bar,
// -c barCount means number every first bar on every line
// -CC means number every bar
//

void check_bar(print * p, int j, int *l_p, struct list* l)
{
  char *ch=0, *nxt=0, *prev=0;

  if (bar_count || barCount || barCCount) {

    if (l) {
      ch = l->dat;
      //      printf ("entering check_bar: ch %c\n", *ch);
    }
    //    else printf ("in check_bar, l not defined\n");

    if (l->next) nxt = l->next->dat;
    //    else printf ("in check_bar, ch %c nxt not defined\n", *ch);
    if (l->prev) prev = l->prev->dat;
    //    else printf ("in check_bar, ch %c prev not defined\n", *ch);

    if (( *ch == 'b' || *ch == 'B' || *ch == 'A')
	&& j+1 != *l_p
	&& j != *l_p
	&& bdot(l)              // check for b..b
	&& *nxt != 'Q'
	&& *nxt != 'q') {
      n_measures++;
      //  printf ("check bar b measures %d\n", n_measures);
    }
    
    else if ( *ch == '.' ) {
      //      printf ("dvi_f: check_bar: dot: ch %c\n", *ch);

      if ( prev ) {
	//	printf ("dvi_f: check_bar: dot: ch %c prev %c\n", *ch, *prev);
	if ( *prev == 'b'
	     || *prev == 'B'
	     || *prev == '.'){
	  //	  printf ("dot - prev is a b of some sort - exiting\n");
	  return;
	}
      }

      if ( nxt ) {
	//	printf ("dvi_f: check_bar: dot: ch %c nxt %c\n", *ch, *nxt);
	if ( *nxt == 'b'
	     || *nxt == 'B'
	     || *nxt == '.'
	     ) {
	  //  printf ("dot - nxt is a b of some sort - exiting\n");
	  return;
	}
	n_measures++;
      }
      //printf ("dot measures %d\n", n_measures);
    }
  }

/*  printf ("  j %d  l_p %d measures %d ", 
	  j, *l_p, n_measures);
  if (prev) printf (" prev %c ", *prev);
  if (nxt) printf (" nxt %c ", *nxt);
  printf ("\n");
 */

  if (j+1 == *l_p || j+2 == *l_p ) return;

  /* this is where we print the number */
  /* choose one of these below to put number under first or second bar */
  /* the first one gives wht wrong number */
  if (!j && ! barCount && prev && (*prev == 'b' || *prev == 'B')) return;
  if (nxt && ( *nxt == 'b' || *nxt == 'B')) {
    if  (bar_count && ! (n_measures % 5))
      p->p_num(n_measures);
    else if (j == 0 && barCount) 
      p->p_num(n_measures+1);
    else if (j == 0 && barCCount) {
      p->p_num(n_measures+1);
    }
    return;
  }

  if (bar_count && ! (n_measures % 5))
    p->p_num(n_measures);
  else if (barCCount) {
    if (j > 4 )
      p->p_num(n_measures);
    else if ( rev_bdot(l) )
      p->p_num(n_measures);
  }
  else if ((barCount) && j==0) {
    if (n_measures != 1 ) {
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
	      (void *)((int)ch[1]));
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
	else {			// why do we do this?
	  //    p->moveh(0.11);	// this should match line in pass1 !
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
	    if (f->flags & FOUR)
	      p->movev(-1 *  d_i_space);
	    else if (f->flags & FIVE)
	      p->movev(d_i_space/ -2);
	    else if (f->m_flags & SEVEN)  /* wbc - watch this */
	      p->movev(0.5 * d_i_space);

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
		p->pop();

		if (f->m_flags & TWOSTAFF ) {
		  len = 0;
		  i=1;

		  p->push();
		  p->movev( -5 * m_space - staff_h);
		  p->movev( -1 * str_to_inch(interspace) );
		  p->movev( -text_sp * f->n_text );
		  p->movev ( -f->c_space );
		  p->push();

		  while (ch[i] != '-')
		    len += 1.5 * f_a[font]->fnt->get_width(ch[i++]);
		  p->moveh (len / -2.0);
		  i=1;
		  while (ch[i] != '-')  p->set_a_char(ch[i++]);
		  p->pop();

		  p->movev(1.8 * f_a[font]->fnt->get_height('1') + .3 * m_space);
		  j= ++i;	len = 0.0;
		  while (ch[j] != ' ')
		    len += 1.5 * f_a[font]->fnt->get_width(ch[j++]);
		  p->moveh (len / -2.0);
		  while (ch[i] != ' ')  p->set_a_char(ch[i++]);
		  p->pop();
		}
	    }
	}
	else {		/* eg. 3 , no music */
	    font = 0;
	    p->use_font(font);
	    switch (ch[1]) {
	    case '0':
	      cc = 033;
	      break;
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
		    (void *)((int)ch[1]));
	      break;
	    }
	    p->push();
	    if (f->line_flag == ON_LINE) p->movev(3.0 * d_i_space);
	    else p->movev(3.5 * d_i_space);
	    if (f->flags & FOUR)  p->movev(-1 *  d_i_space);
	    else if (f->flags & FIVE)  p->movev(d_i_space/ -2);
	    else if (f->m_flags & SEVEN)
	      p->movev(0.5 * d_i_space);

	    p->movev( 0.5 * f_a[font]->fnt->get_height(cc));

	    if (ch[2] && ch[2] == 'Q' ) {
	      p->set_highlight();
	      if (f->m_flags & PAREN )
		p->paren_highlight();
	      else if (f->m_flags & RED )
		p->red_highlight();
	      else
		p->gray_highlight();
	      p->put_a_char(cc);
	      p->clear_highlight();
	    }
	    else {   		// no highlight
	      if (ch[2] && ch[2] == '|' ) {
		p->push();
		p->moveh( 0.47 * f_a[font]->fnt->get_width(cc));
		p->movev( 0.4 * f_a[font]->fnt->get_height(cc));
		p->put_rule(0.008, 1.86 * f_a[font]->fnt->get_height(cc));
		p->pop();
	      }
	      //	      if (cc == 27 ) fprintf(stderr, "HGERE\n");
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
	if (f->flags & FOUR)
	  p->movev(-1 *  d_i_space);
	else if (f->flags & FIVE)
	  p->movev(d_i_space/ -2);
	else if (f->m_flags & SEVEN)
	      p->movev(0.5 * d_i_space);

	if ( j == 0 )
	    p->moveh( -1 * str_to_inch("0.28 in"));
	if (baroque) {
	  if (f->line_flag == ON_LINE)
	    p->movev(-.012 + 0.0  * d_i_space );
	  else
	    p->movev(-.012 + .5 * d_i_space );
	}
	//
	// why are we getting the height of a 'G' when the
	// character is something else?
	//
	p->movev((8.25 * d_i_space
		   + f_a[0]->fnt->get_height('G')) / 2.0 );

	if (ch[1] == 'O' || ch[1] == 'o' ) {
	  p->push();
	  if (f->flags & FOUR )	     p->movev(1.0 * d_i_space);
	  else if ( f->flags & FIVE) p->movev(0.5 * d_i_space);

	  // was -1.85 * d_i_space
	  p->movev(-3.120 * d_i_space);

	  if (f->line_flag == BETWEEN_LINE)
	    p->movev(.5 * d_i_space);
	  if (ch[1] == 'O' ) p->put_a_char(18); // special O time signature
	  else if (ch[1] == 'o' ) p->put_a_char(17); // special o cut time signature
	  p->pop();
	}
	else {
	  p->push();
	  if (f->line_flag == ON_LINE)
	    p->movev( -.012  - 0.50  * d_i_space );
	  p->put_a_char(ch[1]);
	  p->pop();
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





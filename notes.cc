/*
 * notes.c
 * wbc Aug 93
 *
 */

#include "tab.h"
#include "print.h"
#include "tfm.h"
#include "system.h"
#include "sizes.h"


#define LEDGER 	p->moveh ( -.043 );p->put_rule(0.16, str_to_inch(staff_height));

/* EXTERNAL */
int online(char c);
double stem; 
double stem_w;
void 
 do_beam(print *p, font_list *f_a[], struct notes *nn, struct file_info *f);

/* LOCAL */
void do_a_stem(print *p, font_list *f_a[], struct notes *nn);
int text_pt(print *p, font_list *f_a[], struct text *text, int level);
void space_rest(print *p, notes *nn);
double d_i_space = str_to_inch(interspace);
double rest = d_i_space * .3;

int breaksys=0;		// does this tie go to next system?
int breaksys2=0;		// does this tie go to next system?
double tlength=1.0;		// the length of a tie

void
do_notes(print *p, font_list *f_a[], struct notes *nn, struct file_info *f)
{
  int do_rest=0;
  //    struct notes *nn = l->notes;
  double center = 0.00;
  double i_center = 0.01;
  double w_center = -0.01;
  double l_center = 0.0;
  double n_center = m_space / 18.0;
  double mod_off = m_space / -6;	/* -3.77 move up for modern flags */
  static int up;
  rest = m_space / 2 * .8;

  if (f->note_flag != ITAL_NOTES) {
    stem = 3.39 * m_space;                   /* was 3.87 */
    stem_w = str_to_inch(".72 pt");
  }

  if (red == 1.0) {
    center = 0.0 + m_space/2.0;             /* was 017 */
    w_center = 0.0 + m_space/1.76;
    l_center = 0.003 + m_space/2.0;
  }
  else if (red == 0.94440) {	/* 95 */
    center = 0.01 + m_space/2.0;
    w_center = 0.0 + m_space/2.0;
    l_center = -0.003 + m_space/2.0;
  }
  else if (red == 0.88880) {	/* 9 */
    center = 0.0 + m_space/2.0;
    w_center = 0.0 + m_space/2.0;
    l_center = 0.0015 + m_space/2.0;
  }
  else if (red == 0.777770) {	/* 8 */
    center = -0.005 + m_space/2.0;
    w_center = 0.0 + m_space/2.0;
    l_center = 0.003 + m_space/2.0;
  }
  else {
    center = -0.005 + m_space/2.0;
    w_center = 0.00 + m_space/2.0;
    l_center = 0.003 + m_space/2.0;
  }
  i_center += center;
  p->push();

  p->movev ( -st_text );
  p->movev ( -text_sp * f->n_text );
  p->movev ( - f->c_space );
  p->movev ( m_space ); /* to low A line */
  if (f->flags & MARKS) p->put_rule(02., 0.01);
  /*    p->movev ( center ); */ /* center note on line */ 
  if (f->flags & MARKS) p->put_rule(1., 0.01);
  //    printf("do note - note %c time %c\n", 
  //	   nn->note, nn->time);

  switch (nn->note) {
  case 'R':
    /* this centers the rest */
    if (nn->time == 'W' || nn->time == 'w') {
      p->movev (-4.5 * m_space);
    }
    else p->movev (-3.5 * m_space);
    do_rest++;
    break;
  case '?':			/* do the A ledger line */
    p->push();
    LEDGER;
    p->pop();
  case '@':                        /* this is low G */
  case 'A':
    p->push();
    p->movev (-1.0 * ( 'C' - 'A' ) /* the C ledger line */
	      * (m_space / 2.0));
    LEDGER;
    p->pop();
  case 'B':
  case 'C':
    p->movev (-1.0 * ( nn->note - 'A' ) 
	      * (m_space / 2.0));
    p->push();
    if (nn->note %2 -1) p->movev (m_space / -2.0);
    LEDGER;
    p->pop();
    break;
  case 'D':
  case 'E':
  case 'F':
  case 'G':
    p->movev (-1.0 * ( nn->note - 'A' ) 
	      * (m_space / 2.0));
    break;
  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
  case 'g':
    p->movev (-1.0 * ((double)( nn->note - 'a') + 7. ) 
	      * (m_space / 2.0));
    break;
  case '4':			/* high d */
  case '3':			/* high c */
    p->push();
    p->movev (-16.0 * m_space / 2.0);
    //p->moveh ( -.075 );
    // p->put_rule(0.2, str_to_inch(staff_height));
    LEDGER
      p->pop();
  case '2':			/* high b */
  case '1':			/* high a */
	
    p->movev (-14.0 * m_space / 2.0);
    p->push();
    p->moveh ( -.043 );p->put_rule(0.16, str_to_inch(staff_height));
    p->pop();
    p->movev (((double)( '1' - nn->note))
	      * (m_space / 2.0));
    break;
  case 'h':			/* high a */
    p->movev (-1.0 * ((double)( nn->note - 'a') + 7. )
	      * (m_space / 2.0));
    p->push();
    p->moveh ( -.043 );
    p->put_rule(0.16, str_to_inch(staff_height));
    p->pop();
    break;
  case 'i':			/* high b */
    p->movev (-1.0 * ((double)( nn->note - 'a') + 7. )
	      * (m_space / 2.0));
    p->push();
    p->moveh ( -.043 );
    p->movev(m_space / 2.0);
    p->put_rule(0.16, str_to_inch(staff_height));
    p->pop();
    break;
  case '0':			// no note here
    p->pop();
    return;
  default:
    break;
  }

  if (f->flags & MARKS) p->put_rule(0.5, 0.01);

  p->moveh (i_flag_indent);

  if (nn->note < 'b' &&
      nn->note >  '3' )  up = 1;
  else up = 0;
		
  if (breaksys2 &&
      nn->time != 'G' ) {		// carry over a tie
    //    dbg0(Warning, "Breaksys2 set\n");
    breaksys2 = 0;
    p->push();
    p->moveh(-tlength);
    if (up )
      p->do_tie_reversed(tlength);
    else
      p->do_tie(tlength);
    p->pop();
  }

  switch (nn->time){
  case 'Z':			/* a squiggle */
    // the famous Zap or zap character!
    p->movev(0.02);
    p->put_a_char( 251 );	/* a squiggle */
    break;
  case 'R':
    if (f->note_flag ==  MOD_NOTES) {
      p->put_a_char( 26 );
    }
    else p->put_a_char( 253 );	/* a rest */
    break;
  case 'G':
    p->movev( -2.5 * m_space);
    if (f->note_flag == ITAL_NOTES)
      p->put_a_char(252);
    else {
      p->movev (0.47 * m_space);
      p->put_a_char(21); /* 21 */
    }
    if (nn->note == '8') {
      if (nn->sharp == 'a') {
	p->moveh("0.089 in");
	p->movev (-5*m_space);
      }
      if (nn->sharp == 'b') {
	p->moveh("0.071 in");
	p->movev (2.4*m_space);
      }
      p->put_a_char(26); /* a little 8 */
    }
    // unset FCLEF flag
    break;
  case 'F':
    p->movev( -3.5 * m_space);
    p->movev(0.27 * m_space);
    p->put_a_char('z');
    f->m_flags |= FCLEF;
    break;
  case 'W':
    p->movev (w_center);
    if (f->note_flag == ITAL_NOTES) {
      if ( do_rest){
	p->movev ( -1.0 * m_space + rest);
	space_rest(p, nn);
	p->put_rule(0.09, rest);
      }
      else if ( nn->note > 'b') p->put_a_char (224);
      else  p->put_a_char(216);
    }
    else /* MOD NOTES */ {
      if ( do_rest){
	//	      p->put_rule(0.02,0.02);
	space_rest(p, nn);
	p->movev (-0.0025);
	p->movev ( -1 * m_space + rest);
	p->put_rule(0.09, rest);
      }
      else {
	p->movev(mod_off);
	if ( nn->note > 'b') p->put_a_char (0127);
	else  p->put_a_char(0127);
      }
    }
    break;
  case 'w':
    p->movev(w_center);
    if (f->note_flag == ITAL_NOTES){
      if ( nn->note >= 'b'){
	if (nn->special == 'r') 
	  p->put_a_char (217);
	else
	  p->put_a_char (225);
      }
      else if (do_rest) {
	p->movev(-0.01);
	space_rest(p, nn);
	p->put_rule (0.09, rest);
      }
      else {
	if (nn->special == 'r') 
	  p->put_a_char(225);
	else
	  p->put_a_char(217);
      }
    }
    else {
      if (do_rest ) {
	p->movev(-0.01);
	space_rest(p, nn);
	p->put_rule (0.09, rest);
      }
      else {
	p->movev(mod_off);
	p->put_a_char(211); // 212 is older notes
	do_a_stem(p, f_a, nn);
      }
    }
    break;
  case 'J':
    p->movev(l_center);
    p->movev(0.012);
    if (do_rest) p->put_a_char (0375);
    else p->put_a_char('J');
    break;
  case 'L':
    p->movev(l_center);
    if (do_rest) p->put_a_char (0375);
    else p->put_a_char('L');
    break;
  default:			/* 0, 1, 2, 3 */
    p->movev(n_center);
    if (f->note_flag == ITAL_NOTES) {

      if (do_rest) {
	switch (nn->time) {
	case '1':
	  p->put_a_char (16);
	  break;
	case '0':
	  p->put_a_char (15);
	  break;
	default:
	  p->put_a_char (0375);
	  break;
	}
      }
	    
      else if (nn->note == 'b' // a b between two a's
	       && (nn->staff == 1 ? 
		   ( nn->parent->prev
		     && nn->parent->prev->notes
		     && ! nn->parent->prev->notes->reversed
		     && nn->parent->prev->notes->note <= 'a' 
		     && nn->parent->next
		     && nn->parent->next->notes
		     && nn->parent->next->notes->note <= 'a' ) :
		   ( nn->parent->prev 
		     && nn->parent->prev->notes2 
		     && nn->parent->prev->notes2->note <= 'a'
		     && nn->parent->next
		     && nn->parent->next->notes2
		     && nn->parent->next->notes2->note <= 'a' ))) {
	nn->reversed=1;
	p->movev(i_center);
	p->put_a_char (218 + nn->time - '0');
      }
	    
      else if ( nn->note >= 'b' || (
				    nn->note >= '1' && nn->note <= '4' )) {
	if (nn->special == 'r') {
	  p->movev(i_center);
	  p->put_a_char (218 + nn->time - '0');
	}
	else {
	  p->movev(i_center);
	  p->put_a_char (226 + nn->time - '0');
	}
      }
      else if (nn->note == 'a' // an a between two b's
	       && nn->staff == 1 ? 
	       ( nn->parent->prev
		 && nn->parent->prev->notes
		 && nn->parent->prev->notes->note >= 'b' 
		 && nn->parent->next
		 && nn->parent->next->notes
		 && nn->parent->next->notes->note >= 'b' 
		 && ! nn->parent->prev->notes->reversed ) :
	       ( nn->parent->prev 
		 && nn->parent->prev->notes2 
		 && nn->parent->prev->notes2->note >= 'b' 
		 && nn->parent->next
		 && nn->parent->next->notes2
		 && nn->parent->next->notes2->note >= 'b' 
		 && ! nn->parent->prev->notes2->reversed )) {
	nn->reversed=1;
	p->movev(i_center);
	p->put_a_char (226 + nn->time - '0');
      }
      else {
	if (nn->special == 'r') {
	  p->movev(i_center);
	  p->put_a_char (226 + nn->time - '0');
	}
	else {
	  p->movev(i_center);
	  p->put_a_char (218 + nn->time - '0');
	}
      }
    }
    else {			        /* modern flags  */
      p->movev(center);
      p->movev(mod_off);
      if (nn->special == '#' ||           /* beams */
	  nn->special == 'x' ||
	  nn->special == '=' ) {
	

	p->push();
	p->put_a_char(215);     /* solid note body */
	do_beam(p, f_a, nn, f);
	p->pop();
      }
      else if (do_rest) {
	//	    printf ("rest ");
	if (do_rest) {
	  switch (nn->time) {
	  case '1':
	    p->put_a_char (16);
	    break;
	  case '0':
	    if (f->note_flag ==  MOD_NOTES) {
	      p->push();
	      p->movev(0.02);
	      p->put_a_char (15);
	      p->pop();
	    }
	    else
	      p->put_a_char (0375);
	    break;
	  default:
	    p->put_a_char (0375);
	    break;
	  }
	}
      }
      else if (nn->time == '0' ||          // not part of grid
	       nn->time == '1' ) {
	p->put_a_char(215);              // solid note body
	do_a_stem(p, f_a, nn);
      }
      else if (nn->time == '2') {        // two flags
	p->put_a_char(215);              // solid note body
	do_a_stem(p, f_a, nn);
      }
      else p->put_a_char(202 + nn->time - '0');
    }
  }
  if (nn->special == '-'
      || nn->special == '=') {                          // a tie
    struct list *nl= nn->parent->next;
    double tspace = nn->parent->space + nn->parent->padding;
    double t_indent=0.014;
    double head_w = f_a[0]->fnt->get_width(215);
	
    breaksys = 0;

    if (nn->staff == 1 ) {
      if (nl->dat[0] == 'b') {
	if ( ! nl->next) {
	  //	  printf("set breaksys\n");
	  breaksys=1;
	}
      }
      while (nl->next 
	     && ((! nl->notes )  || nl->notes->note == '0' )) {
	tspace += ( nl->space + nl->padding);
	nl = nl->next;
      }
    }
    else if (nn->staff == 2 ) {
      while (nl->next 
	     && ! nl->notes2 ) {
	tspace += ( nl->space + nl->padding);
	nl = nl->next;
      }
    }
    //	printf("notes: tie length %f\n", tspace);
    if (tspace < 0.4) t_indent = 0.0;
    tlength=tspace - (2 * t_indent) - head_w;
    p->push();
    if (tspace < 0.3) {
      tlength += head_w;
      head_w /= 2.0;
    }
    p->moveh(head_w);
    if (up) {
      p->moveh (t_indent);  p->movev(.3 * m_space);
      if (breaksys)
	p->do_tie_reversed(tlength * 1.5);
      else
	p->do_tie_reversed(tlength);
    }
    else {
      p->moveh (t_indent);  p->movev(-1.3 * m_space);
      if (breaksys)
	p->do_tie(tlength * 1.5);
      else
	p->do_tie(tlength);
    }
    p->pop();
  }


  switch (nn->sharp) {
  case '^':
    p->push();
    if (nn->time == 'J') p->moveh (.07);
    p->moveh(.1);
    p->movev(-.006);	      /* center dot to note, above line */
    if (online(nn->note)) p->movev(-.03);
    p->put_a_char('.');	/* the dot */
    p->pop();
  case '+':			/* 23 = old sharp, 22 = new one */
    if (!(f->flags & SHARP_UP)) {

      if (f->note_flag == MOD_NOTES) {
	p->moveh (-.1);
	p->movev(0.5 *( f_a[0]->fnt->get_height(22) - m_space));
	p->put_a_char(22);
      }
      else if (f->note_flag == ITAL_NOTES){
	p->moveh (-.1);
	p->movev (-.013);
	p->put_a_char(23);
      }
      else {
	p->moveh (-.1);
	p->put_a_char('#');
      }
    }
    else {			/* sharp up */
      p->pop();
      p->push();
      p->movev ( -st_text );
      p->movev ( -text_sp * f->n_text );
      p->movev ( - f->c_space);
      p->movev ( -6 * m_space ); /* to low A line */
      p->movev(0.05); /* center note on line */
      if (f->note_flag == ITAL_NOTES ) {
	p->put_a_char(23);
      }
      else if (f->note_flag == MOD_NOTES) {
	p->moveh(-0.02);
	p->put_a_char(22);
      }
      else p->put_a_char('#');
    }
    break;
  case 'v':			/* dot and flat */
    p->push();
    if (nn->time == 'J') p->moveh (.07);
    p->moveh(.1);
    p->movev(-.009);
    if (online(nn->note)) p->movev(-.03);
    p->put_a_char('.');	/* dot */
    p->pop();
  case '-':			/* flat */
    if (!(f->flags & SHARP_UP)) {
      p->moveh (-.076);
      p->movev (-0.022);
      p->put_a_char('?');	/* flat */
    }
    else {
      p->pop();
      p->push();
      p->movev ( -st_text );
      p->movev ( -text_sp * f->n_text );
      p->movev ( - f->c_space);
      p->movev ( -6 * m_space ); /* to low A line */
      p->movev(0.05); /* center note on line */
      if (nn->special == 'x'
	  && nn->note >= 'F') p->movev(-.130); // above beam
      p->put_a_char('?');
    }
    break;
  case '.':
    if (nn->time == 'J') p->moveh (.07);
    p->moveh(.1);
    if (f->note_flag != ITAL_NOTES) p->moveh(.024);
    p->movev(.015);
    if (online(nn->note)) p->movev(-.03);
    p->put_a_char('.');
    break;
  case 'N':			// dot and natural
   p->push();
    if (nn->time == 'J') p->moveh (.07);
    p->moveh(.1);
    p->movev(-.009);
    if (online(nn->note)) p->movev(-.03);
    p->put_a_char('.');	/* dot */
    p->pop();
  case 'n':			// natural
    if (!(f->flags & SHARP_UP)) {
      p->moveh (-.076);
      /*	    p->movev (-0.022); wbc oct 2000  */
      p->movev (3 * 0.022);
      p->put_a_char(25);	/* natural */
    }
    else {
      p->pop();
      p->push();
      p->movev ( -st_text );
      p->movev ( -text_sp * f->n_text );
      p->movev ( - f->c_space);
      p->movev ( -6 * m_space ); /* to low A line */
      p->movev(0.05); /* center note on line */
      if (nn->special == 'x' &&
	  nn->note >= 'F') p->movev(-.130); // above beam
      p->put_a_char(25);
    }
    break;
  }
  p->pop();
}

extern int text_font;

void
do_text(print *p, i_buf *i_b, font_list *f_a[], 
	struct list *l, struct text *text, struct file_info *f, 
	int in_sys, int staff)
{
  int i;
  int level = 0;			/* which line of text we are on */
  int font;
  char *pp;
  double text_used = 0.0; /* text width used so far */
  struct list *ll;
  struct t_words *tp;
  double d_i_space = str_to_inch(interspace);
  
  p->push();
  if (in_sys) {
    p->movev(3 * d_i_space - .02);
  }
  else {
    if (f->flags & NOTES ) {
      
      p->movev ( -1 * str_to_inch(flag_to_staff) ); 
      if (f->flags & MARKS) p->put_rule (.4, 0.01);
      p->movev ( -st_text ); /* to the top of the flags plus a little */
      if (f->flags & MARKS) p->put_rule (.5, 0.01);
      p->movev ( -text_sp * (f->n_text - 1));
      if (f->flags & MARKS) p->put_rule (.6, 0.01);
      if (staff == 2 ) {
	//		p->movev ( -st_text ); 
	p->movev ( -str_to_inch(interspace) );
	p->movev ( -text_sp * (f->n_text));
	p->movev ( -f->c_space);
	p->movev ( -5 * m_space);
      }
    }
    else {			/* No NOTES */
      if (f->line_flag == ON_LINE) {
	p->movev ( 6. * d_i_space );
      } else {
	p->movev ( 7. * d_i_space );
      }
      p->movev ( text_sp );
    }
  }
  if (in_sys)
    font =5;		// italic text font
  else
    font = text_font;
  
  p->use_font(font);
  
  tp = text->words;
  
  while (tp) {
    p->push();
    for (i=0;i < tp->length; i++) {
      pp = &tp->words[i];
      
      if (f->flags & MARKS) p->put_rule(.002, .25);

      switch (*pp) {
      case  '\t':
	dbg0 (Warning, "tab: do_text: tab character in text!\n");
	goto next;
      case ' ':
	p->moveh(f_a[font]->fnt->get_width('-'));
	text_used += f_a[font]->fnt->get_width('-');
	break;
      case ']':	/* word tie*/
	text_used += f_a[0]->fnt->get_width(0201);
	p->use_font(0);
	p->set_a_char(0201);
	p->use_font(font);
	break;
      case '\\':	/* escape the next character */
	text_used += 
	  print_special_char(&pp, p, i_b, f_a[font], f, 0);
	i++;
	i++;
	break;
      case '@':
	do_sp(*pp, font, p /* , f */);
	break;
      case '-':
	p->moveh (l->space /2.0);
	p->moveh (l->padding/2.0);
	p->moveh (text_used / -2.0);
	p->moveh (f_a[font]->fnt->get_width ('-')/ -2.0);
	/* here we use ll which is specific to l, not text line */
	ll = l;
	if (staff == 1) {
	  while ((ll = ll->next) 
		 && (int)text_pt(p, f_a, ll->text, level)) 
	    p->moveh ((ll->space + ll->padding)/2.0);
	}
	else if (staff == 2) {
	  while ((ll = ll->next) 
		 && (int)text_pt(p, f_a, ll->text2, level)) 
	    p->moveh ((ll->space + ll->padding)/2.0);
	}
	p->set_a_char(*pp);
	break;
      case '"':
	// Feb 98 - I can't remember why we insert a space
	// before a quote
	//	p->moveh((tp->size - 
	// f_a[font]->fnt->get_width ('"'))/2.0);
	p->set_a_char(*pp);
	break;
      case '|':	/* means concatenate with next text */
	break;	/* text_used += 0.0 */
      default:
	p->set_a_char (*pp);
	if (*pp == 014 && f->flags & PS) *pp = (char)0256; /* fi */
	if (*pp == 015 && f->flags & PS) *pp = (char)0257; /* fl */
	if (*pp == 031 && f->flags & PS) *pp = (char)0373; /* fs */
	text_used += f_a[font]->fnt->get_width(*pp);
	if (f->flags & MARKS) p->put_rule(.002, .15);
	break;
      }
    }
  next:
    p->pop();
    if (in_sys) 
      p->movev(d_i_space);
    else
      p->movev(text_sp);
    text_used = 0.0;
    tp = tp->next;
    level++;
  }
  if (f->flags & MARKS) p->put_rule(.002, .25);
  p->pop();
  p->use_font(0);
}

     /* true if text esists in this chord at level */
int
text_pt(print *p, font_list *f_a[], struct text  *text,	int level)
{
    struct t_words *tp;

    if (!text) return (1);

    tp = text->words;

    while (level && tp) {
	tp = tp->next;
	level--;
    }

    if (!tp) return(1);		/* no text at this level */
    if (tp && tp->length == 0) return(1); /* text length 0 */
    else return (0);
}

void
do_a_stem(print *p, font_list *f_a[], struct notes *nn)
{
    static int up;
    //    int extra=0;
    double w_stem = stem;

    p->push();
    if (nn->note < 'b' &&
	nn->note > '5')  up = 1;
    else up = 0;
    
    if (nn->note > '0' && nn->note < '@') 
      w_stem += m_space * (nn->note -'0' -1)/3.5;
    
    if (up) {
	p->moveh(f_a[0]->fnt->get_width(215) - str_to_inch("0.59 pt"));
	p->movev(-.043);

	p->put_rule(stem_w, w_stem);
	if (nn->time == '1' || nn->time == '2' ) {
	    p->movev ( f_a[0]->fnt->get_height (214) - w_stem);
	    p->movev(m_space/4);
	    p->moveh(stem_w);
	    p->put_a_char(214);
	}
	if (nn->time == '2') {
	    p->movev ( .08 );
	    p->put_a_char(214);
	}
    }
    else {
	p->movev(w_stem - 0.043);
	p->moveh(stem_w / 3.0);
	p->put_rule(stem_w, w_stem);
	if (nn->time == '1' || nn->time == '2') {
	    p->moveh(stem_w);
	    p->movev(-m_space/4);
	    p->put_a_char(213);
	}
	if (nn->time == '2') {
	    p->movev ( .08 );
	    p->put_a_char(213);
	    p->moveh(-stem_w);
	    p->put_rule( stem_w, 0.08);
	}
    }
    p->pop();
}

/*  center a rest mark in a measure */
void space_rest(print *p, notes *nn)
{
    struct list *tl = nn->parent;
    double space=0.0;
    int i= -8;
    
    if (tl->prev) {
	p->moveh(-0.5 * (tl->prev->space + tl->prev->padding) );
	if (nn->staff == 1 && tl->prev->notes ) // fudge for rest preceeded
	    p->moveh(tl->prev->space / 1.5);    // by a note rather than a 
	if (nn->staff == 2 && tl->prev->notes2 )// barline
	    p->moveh(tl->prev->space / 1.5);
    }

    while ( tl && tl->dat[0] != 'b' && i++ ) {
	space += tl->space + tl->padding;
	if (nn->staff==0) 
		dbg0 (Warning, "space_rest: null staff\n");
	if ( nn->staff == 1 && tl->next 
		&& tl->next->notes && tl->next->notes->note != '0' ) break;
	if ( nn->staff == 2 && tl->next 
		&& tl->next->notes2 && tl->next->notes2->note != '0' ) break;
	tl = tl->next;
    }
    p->moveh (( space - (2 * rest ))/2.0 );
}

/* is the rest of this line a music specification? 
   if it is, return 1, if not return 0 
   */

int is_music(char *text)
{
    char *t = text;
    if ( ! strchr("BLWw0123", *t))
	return (0);
    t++;
    if ( *t < '?' || *t > 'k')
      if ( *t < '1' || *t > '4')
	return (0);
    t++;
    if ( ! strchr ( "0+-^v.n", *t))
	return (0);
    return (1);
    
}

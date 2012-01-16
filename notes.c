/*
 * notes.c
 * wbc Aug 93
 *
 */

#include "tab.h"

#define BASE 10			/* base for saves note registers */
extern char interspace[];
extern double text_sp;
extern double m_space;
extern double st_text;
extern char staff_height[];
extern char i_flag_indent[];

double stem; 
double stem_w;

do_notes(f, l)
     struct list *l;
     struct file_info *f;
{
    int do_rest=0;
    double d_i_space = str_to_inch(interspace);
    double rest = d_i_space * .3;
    double center = 0.05;
    double mod_off = -0.016;	/* move up for modern flags */

    if (f->note_flag != ITAL_NOTES) {
	stem = 3.87 * m_space;
	stem_w = str_to_inch(".72 pt");
    }

    push();

    fmovev ( -st_text );
    fmovev ( -text_sp * f->n_text );
    fmovev ( - f->c_space );
    fmovev ( m_space ); /* to low A line */

    fmovev ( center ); /* center note on line */

    switch (l->notes->note) {
      case 'R':
	/* this centers the rest */
	fmovev (-4.5 * m_space);
	do_rest++;
	break;
      case 'A':
      case 'B':
      case 'C':
	push();
	fmovev (-1 * (( 'C' - 'A' ) + 1) 
		* (m_space / 2.0));
	fmoveh(-0.05); 
	f_put_rule(0.2 ,str_to_inch(staff_height));
	pop();
	fmovev (-1 * ( l->notes->note - 'A' ) 
		* (m_space / 2.0));
	break;
      case 'D':
      case 'E':
      case 'F':
      case 'G':
	fmovev (-1 * ( l->notes->note - 'A' ) 
		* (m_space / 2.0));
	break;
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
      case 'g':
	fmovev (-1 * ((double)( l->notes->note - 'a') + 7. ) 
		* (m_space / 2.0));
	break;
      case '4':			/* high d */
      case '3':			/* high c */
	push();
	fmovev (-17 * m_space / 2.0);
	fmoveh ( -.075 );
	f_put_rule(0.2, str_to_inch(staff_height));
	pop();
      case '2':			/* high b */
      case '1':			/* high a */
	
	fmovev (-15 * m_space / 2.0);
	push();
	fmoveh ( -.075 );
	f_put_rule(0.2, str_to_inch(staff_height));
	pop();
	fmovev (-1 * ((double)( l->notes->note - '1') - 1) * (m_space / 2.0));
	break;
      default:
	break;
    }

    moveh (i_flag_indent);

    switch (l->notes->time){
      case 'Z':			/* a squiggle */
	fmovev(-.035);
	put_a_char( 251 );	/* a squiggle */
	break;
      case 'R':
	put_a_char( 253 );	/* a rest */
	break;
      case 'G':
	fmovev( -3 * m_space);
	if (f->note_flag == ITAL_NOTES)
	  put_a_char(252);
	else {
	    fmovev (0.47 * m_space);
	    put_a_char(21); /* 21 */
	}
	break;
      case 'W':
	if (f->note_flag == ITAL_NOTES) {
	    if ( do_rest){
		fmovev ( -1 * m_space + rest);
		f_put_rule(0.09, rest);
	    }
	    else if ( l->notes->note > 'b') put_a_char (224);
	    else  put_a_char(216);
	}
	else /* MOD NOTES */ {
	    if ( do_rest){
		fmovev ( -1 * m_space + rest);
		f_put_rule(0.09, rest);
	    }
	    else {
		fmovev(mod_off);
		if ( l->notes->note > 'b') put_a_char (0127);
		else  put_a_char(0127);
	    }
	}
	break;
      case 'w':
	if (f->note_flag == ITAL_NOTES){
	    if ( l->notes->note >= 'b'){
		if (l->notes->special == 'r') 
		  put_a_char (217);
		else
		  put_a_char (225);
	    }
	    else if (do_rest) {
		f_put_rule (0.09, rest);
	    }
	    else {
		if (l->notes->special == 'r') 
		  put_a_char(225);
		else
		  put_a_char(217);
	    }
	}
	else {
	    fmovev(mod_off);
	    put_a_char(212);
	    do_a_stem(l);
	}
	break;
      case 'J':
	if (do_rest) put_a_char (0375);
	else put_a_char('J');
	break;
      case 'L':
	if (do_rest) put_a_char (0375);
	else put_a_char('L');
	break;
      default:
	if (f->note_flag == ITAL_NOTES) {
	    if (do_rest) put_a_char (0375);
	    else if ( l->notes->note >= 'b') {
		if (l->notes->special == 'r') 
		  put_a_char(218 + l->notes->time - '0');
		else 
		  put_a_char (226 + l->notes->time - '0');
	    }
	    else {
		if (l->notes->special == 'r') 
		  put_a_char (226 + l->notes->time - '0');
		else 
		  put_a_char(218 + l->notes->time - '0');
	    }
	}
	else {			/* modern flags */
	    fmovev(mod_off);
	    if (l->notes->special == '#' ||
		l->notes->special == 'x' ||
		l->notes->special == '-' ) {
		static int i;
		static int up;
		int j;
		int hor, ver;	/* current */
		int h, v, hh, vv; /* end points */
		int top;	/* desired height */
		double slope;

		if (l->notes->special == '#') {
		    if (l->notes->note < 'b' &&
			l->notes->note >  '3' )  up = 1;
		    else up = 0;
		}
		push();
		put_a_char(215);
		if (up)
		  fmoveh(get_width(0, 215) - str_to_inch("0.59 pt"));
		fmovev(-.043);
		if (l->notes->special == '#') {	/* first */
		    i = BASE;
		}
		saveloc (i++);
		if (! l->next->notes ||
		    l->next->notes->special != 'x') { /* last one */
		    
		    j = i;
		    fmovev(up ? - stem: stem);
		    fmoveh( stem_w );
		    saveloc(j);           	/* for cross bar */
		    get_current_loc(&h, &v);
		    getloc(BASE);
		    fmovev(up ? - stem: stem);
		    saveloc(j+1);
		    get_current_loc(&hh, &vv);
		    slope = (double)(v - vv) / (double)(h - hh);
		    if (f->flags & PS)
		      put_thick_slant(j, j+1);
				/* now draw vert lines */
		    while (i - BASE) {
			getloc(--i);
			if (i == BASE || i == j-1) {
			    if (!up) fmovev(stem); 
			    f_put_rule(stem_w, stem);
			}
			else {	/* in the middle */
			    get_current_loc(&hor, &ver);
			    top = vv + (int)((double)(hor - hh) * slope);
			    p_put_rule(inch_to_dvi(stem_w),
				       top - ver);
			}
		    }
		}
		pop();
	    }
	    else if (l->notes->time == '0' ||
		     l->notes->time == '1' ) {
		put_a_char(215);
		do_a_stem(l);
	    }
	    else put_a_char(202 + l->notes->time - '0');
	}
    }
    switch (l->notes->sharp) {
      case '^':
	push();
	if (l->notes->time == 'J') fmoveh (.07);
	fmoveh(.1);
	fmovev(.015);
	if (online(l->notes->note)) fmovev(-.03);
	put_a_char('.');
	pop();
      case '+':
	if (!(f->flags & SHARP_UP)) {
	    if (f->flags & LSA_FORM) {
		fmoveh (-.13);
		fmovev(0.5 *( get_height(0, 22) - m_space));
		put_a_char(22);
	    }
	    else if (f->note_flag == ITAL_NUM){
		fmoveh (-.1);
		fmovev(0.5 *( get_height(0, 23) - m_space));
		put_a_char(23);
	    }
	    else {
		fmoveh (-.1);
		put_a_char('#');
	    }
	}
	else {			/* sharp up */
	    pop();
	    push();
	    fmovev ( -st_text );
	    fmovev ( -text_sp * f->n_text );
	    fmovev ( - f->c_space);
	    fmovev ( -6 * m_space ); /* to low A line */
	    fmovev(0.05); /* center note on line */
	    if (f->note_flag == ITAL_NOTES ) {
/*		fmoveh(0 * get_width(0, 23));*/
		put_a_char(23);
	    }
	    else put_a_char('#');
	}
	break;
      case 'v':			/* dot and flat */
	push();
	if (l->notes->time == 'J') fmoveh (.07);
	fmoveh(.1);
	fmovev(.015);
	if (online(l->notes->note)) fmovev(-.03);
	put_a_char('.');
	pop();
      case '-':
	if (!(f->flags & SHARP_UP)) {
	    fmoveh (-.1);
	    put_a_char('?');
	}
	else {
	    pop();
	    push();
	    fmovev ( -st_text );
	    fmovev ( -text_sp * f->n_text );
	    fmovev ( - f->c_space);
	    fmovev ( -6 * m_space ); /* to low A line */
	    fmovev(0.05); /* center note on line */
	    put_a_char('?');
	}
	break;
      case '.':
	if (l->notes->time == 'J') fmoveh (.07);
	fmoveh(.1);
	fmovev(.015);
	if (online(l->notes->note)) fmovev(-.03);
	put_a_char('.');
	break;
    }
    pop();
}

do_text(f, l)
     struct file_info *f;
     struct list *l;
{
    int i;
    int level = 0;			/* which line of text we are on */
    int font;
    char p;
    double text_used = 0.0; /* text width used so far */
    struct list *ll;
    struct t_words *tp;
    double d_i_space = str_to_inch(interspace);

    if (l->text) {
	
	push();
	if (f->flags & NOTES ) {
	    fmovev ( -st_text );
	    fmovev ( -text_sp * f->n_text);
	    fmovev (m_space );
	    fmovev (0.04);
	}
	else {
	    fmovev (6. * d_i_space);
	    fmovev ( text_sp * f->n_text);
	}

	if (f->flags & PS ) {
	    if (f->flags & ITAL_TXT){
		font = 5;
		printf ("notes.c setting ital font\n");
	    }
	    else font = 1;
	}
	else font = 2;

	use_font(font);

	tp = l->text->words;

	while (tp) {
	    push();
	    for (i=0;i < tp->length; i++) {
		p = tp->words[i];
		switch (p) {
		  case  '\t':
		    printf ("tab: do_text: tab character in text!\n");
		    goto next;
		  case ' ':
		    fmoveh(get_width(font, '-'));
		    text_used += get_width(font, '-');
		    break;
		  case ']':	/* word tie*/
		    text_used += get_width(0, 0201);
		    use_font(0);
		    if (i > 2 && tp->words[i-2] == '<') {
			text_used += 0.2 * get_width(0, 0210);
			fmoveh (0.2 * get_width(0, 0201));
		    }
		    set_a_char(0201);
		    use_font(font);
		    break;
		  case '<':	/* slash */
		  case '%':	/* back slash */
		  case '>':	/* tilde above */
		  case '[':	/* squiggle below */
		  case ':':	/* two dots above */
		  case '\\':	/* escape the next character */
		    do_special(p, tp->words[i+1], font, f);
		    text_used += get_width(font, tp->words[i+1]);
		    i++;
		    break;
		  case ')':
		  case '(':
		  case '@':
		    do_sp(p, font, f);
		    break;
		  case '-':
		    fmoveh((l->space + l->padding - text_used 
			    - get_width (font,'-'))/2.0);
		    ll = l;
		    while ((ll = ll->next) 
			   && (int)text_pt(ll, level)) 
		      fmoveh ((ll->space + ll->padding)/2.0);
		    set_a_char(p);
		    break;
		  case '"':
		    fmoveh((tp->size - get_width (font,'"'))/2.0);
		    set_a_char(p);
		    break;
		  case '|':	/* means concatenate with next text */
		    break;	/* text_used += 0.0 */
		  default:
		    set_a_char(p);
		    text_used += get_width(font, p);
		    break;
		}
	    }
	  next:
	    pop();
	    fmovev(text_sp);
	    text_used = 0.0;
	    tp = tp->next;
	    level++;
	}
	if (f->flags & MARKS) f_put_rule(.002, .25);
	pop();
	use_font(0);
    }
}

text_pt(l, level)	 /* true if text esists in this chord at level */
     struct list *l;		/* the chord */
     int level;
{
    struct t_words *tp;

    if (! l->text) return (1);

    tp = l->text->words;

    while (level && tp) {
	tp = tp->next;
	level--;
    }

    if (!tp) return(1);		/* no text at this level */
    if (tp && tp->length == 0) return(1); /* text length 0 */
    else return (0);
}

do_a_stem(l)
struct list *l;
{
    static int up;

    push();
    if (l->notes->note < 'b' &&
	l->notes->note > '5')  up = 1;
    else up = 0;

    if (up) {
	fmoveh(get_width(0, 215) - str_to_inch("0.59 pt"));
	fmovev(-.043);
	f_put_rule(stem_w, stem);
	if (l->notes->time == '1') {
	    fmovev ( get_height (0, 214) - stem);
	    fmoveh(stem_w);
	    put_a_char(214);
	}
    }
    else {
	fmovev(stem - 0.043);
	fmoveh(stem_w / 3.0);
	f_put_rule(stem_w, stem);
	if (l->notes->time == '1') {
	    fmoveh(stem_w);
	    put_a_char(213);
	}
    }
    pop();
}

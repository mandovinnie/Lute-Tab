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
 * 
 */
#include "tab.h"
// #include "dviprint.h"
#include "print.h"
#include "sizes.h"
#include "system.h"
#include "sound.h"
#include "raw_snd.h"
/* #include "midi_snd.h" */

extern char interspace[];
extern char staff_height[];

void put_note(print *p, int string, unsigned char c, int timeval, struct file_info *f, char *ch);
void score(print *p, double space, double width,  struct file_info *f, char *ch, char *prev);
int find_note(int string, char c, struct file_info *f);
int setflag(file_info *f, char * string, pass pass);
#define OCTAVE 12

/* low b-flat is 0 */
/*
 a  b- b c c# d e- e f f# g a- a  b- b  c  c+ d  e- e  f  f+ g
 0  1  2 3 4  5 6  7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22

 g+ a  b- b  c  c+ d  e- e  f  f+ g  g+ a  b-
 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37

 */
int strings[] = {
    0,		/* null string */
    34,		/* g 1 */		    
    29,		/* d 2 */
    24,		/* A 3 */
    20,		/* F 4 */
    15,		/* C 5 */
    10,		/* G 6 */
    8,          /* F 7 */
    7,		/* Eb 8 */
    5,		/* D 9 */
    3           /* C 10 */ /* just to make life ok for 8,9 string */
  };            /* 0 is low A */
int b_strings[] = {
    0,		/* null string */
    32,		/* f */		    
    29,		/* d */
    24,		/* a */
    20,		/* F */
    17,		/* d */
    12,		/* A */
    10,         /* g */
    8,		/* F */
    7,		/* E */
    5,		/* D */
    3,          /* C */
    1           /* B- */
  };

/* LOCAL */
int getpos(int note,int *adj);
void ledger(print *p, double dist);

char *arg_str=0;

int *tuning_str(char *str)
{
    static int s[16];
    int j=strlen(arg_str);
    int k, count;
    int sharp, flat;
    int val=0;

    s[0]=sharp=flat=0;
    for (k=1; k<16; k++) s[k]=0;

    count=0;
    //    fprintf(stderr, "find note: arg %s %d\n", arg_str, j);
    while (--j > -1) {
      //	printf ("j is %d %c\n", j, str[j]);
	
	switch (k=tolower(str[j])) {
	case '+':
	    sharp++;
	    break;
	case '-':
	    flat++;
	    break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	    val = (4 - (k-'0')) * 12; // octave * 12 starting at b flat
//	    printf("octave val %d\n", val);
	    break;
	case 'a':
	    val -=2;
	case 'b':
	    val -=1;
	case 'c':
	    val -=2;
	case 'd':
	    val -=2;
	case 'e':
	    val -=1;
	case 'f':
	    val -=2;
	case 'g':
	    val +=10;
	    if (sharp) val++;
	    if (flat) val--;
	    sharp=flat=0;
	    count++;
//	    printf("count %d val %d\n", count, val);
	    s[count]=val;
	    break;
	default:
	    break;
	}
    }
    return(s);
}

int find_note(
     int string,
     unsigned char c,
     struct file_info *f)
{
    int fret;
    int note=0;
    int *str;

//    printf("find note: string %d char %c\n", string, c);

    if (arg_str) 
	str = tuning_str(arg_str);
    else if (baroque) 
	str = b_strings;
    else 
	str = strings;

    if ((f->num_flag == ITAL_NUM) && ((
	    (c >= '0' && c <= '9') 
	    || ( c >= 230 && c <= 240 ) 
	    || c == 'x') || c == 'y' || c == 'z')) {
	if ( c == 'x' || c == 230 )
	    fret = 10;
	else if ( c == 'y' || c == 231 )
	    fret = 11;
	else if ( c == 'z' || c == 232 )
	    fret = 12;
	else if ( c >= 232 && c <= 240 )
	    fret = c - 220;
	else
	    fret = c  - '0';
	if (f->m_flags & MILAN) 
	  note = str[string] + fret;
	else
	  note = str[7 - string] + fret;
    }
    else if (string==7 && c >= '4' && c <= '6') {
	if (c == '4') note = str[11];
	if (c == '5') note = str[12];
	if (c == '6') note = str[13];
    }
    else {
	fret = tolower(c) - 'a';
	if (fret > 9) fret--;
	note = str[string] + fret;
    }
    note += f->transpose;
//    printf("find note: note %d\n", note);
    return (note);
}

int dot=0;
int o_dot=0;
int o_timeval=0;
extern sound *sp;
double conv=2;

void
score(print *p, struct list *l, struct file_info *f,
      i_buf *i_b, font_list *f_a[])
{
    double space=l->space;
    double width=l->padding;
    char *ch=l->dat;
    char *prev=l->prev ? l->prev->dat: 0;
    char *next=l->next ? l->next->dat: 0;
    int i;
    unsigned char c;
    int timeval=0;
    char cc = *ch;
    int rest_note=1;			// clear this when there is a note to play

    //    printf("conv %f\n", conv);
    p->clear_highlight();
    
    p->push();

    p->use_font(0);

    if (ch[1] == '.') 
      dot = 1;

    switch (cc) {
    case 'B':
    case 'b':
	p->movev(mus_space);
	p->movev(10.0 * str_to_inch(mus_space));
	if (f->m_flags & GUIT ) {
	  p->movev(-6.0 * str_to_inch(mus_space));
	  p->put_rule (str_to_inch(staff_height),
		       4.0 * str_to_inch(mus_space) 
		       + str_to_inch (staff_height));
	}
	else
	  p->put_rule (str_to_inch(staff_height),
		       10.0 * str_to_inch(mus_space) 
		       + str_to_inch (staff_height));
	if (f->m_flags & SOUND) {
	  sp->add_bar();
	}
	break;
    case '.':
	p->movev(3.0 * str_to_inch(mus_space));
	p->put_a_char('Z');
	p->movev(mus_space);
	p->put_a_char('Z');
	p->movev(5.0 * str_to_inch(mus_space));
	p->put_a_char('Z');
	p->movev(mus_space);
	p->put_a_char('Z');
	break;
    case 'G':
      break;
    case 'R':
    case 'S':
      //    case 'T':
    case 't':
	p->movev (3.0 * str_to_inch(mus_space));
	p->put_a_char(cc);
	break;
    case 'Q':
    case 'q':
    case 'm':
	p->movev (9.0 * str_to_inch(mus_space));
	p->put_a_char(cc);
	break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
      if (ch[1] == 'M')
	break;
      else if (baroque || f->flag_flag == CONTEMP_FLAGS) timeval = cc - '0' + 1;
      else timeval = cc - '0';
      goto rest;
      //    case 'B':
      //	timeval = -2;
      //	goto rest;
    case 'W':
	if (baroque || f->flag_flag == CONTEMP_FLAGS) timeval = -1;
	else timeval = -2;
	goto rest;
    case 'w':
	if (baroque || f->flag_flag == CONTEMP_FLAGS) timeval = 0;
	else timeval = -1;
	goto rest;
    case 'J':			// breve
      timeval = -2;
      goto rest;
    case 'L':
      timeval = -3;
      goto rest;
    case '+':
    case '^':
    case '&':
    case '*':
    case ':':
    case 'v':
    case 'u':			// cut time sign
    case 'k':			// key sign
    case '8':
	break;
    case 'x':
	timeval = o_timeval;
	dot = o_dot;
	goto rest;
    case 'Y':
    case 'y':
	timeval = -2;
	timeval = 0;  /* wbc sept 2015 */
	goto rest;
    case 'T':
	if (l->text) {
	    p->push();
	    do_text(p, i_b, f_a, l, l->text, f, 1, 1);
	    p->pop();
	    
	    free (l->text);
	    l->text = NULL;
	}
	break;
    case '$':
      //      fprintf(stderr, "in score: got $\n");
      {
	int i, j=0;
	char buffer[80];
	char *bp;
	bp = &buffer[0];
	for (i=1; (c = l->dat[i]) != NEWLINE; i++) {
	  *bp=c; 
	  bp++;
	}
	
	buffer[--i] = 0;
	if (setflag(f, buffer, second))
	  break;		// return if we set the flag
      }  
      


    case '[':
    case 'U':			// do nothing
    case 'O':			// do nothing
    case 'A':			// do nothing
    case 'i':			// do nothing
    case 'j':			// do nothing
    case 'Z':			// do nothing
    case 'd':
	break;
    case 'f':
	p->push();
	if (f->flags & PS ) p->use_font(1);
	else p->use_font(2);
	p->set_a_char('F'); p->set_a_char('i'); p->set_a_char('n');
	if (f->flags & NOTES ){
	    double d_i_space=str_to_inch(interspace);
	    p->pop();
	    p->push();
	    if (f->line_flag == ON_LINE) p->movev (- 5.5 * d_i_space);
	    //	    else if (f->flags & WALLACE) p->movev (- 5.5 * d_i_space);
	    else p->movev (- 6.0 * d_i_space);
	    p->movev ( -5 * m_space);
	    p->movev ( st_text);
	    p->movev ( -text_sp * f->n_text);
	    p->movev ( - f->c_space);
	    p->set_a_char('F'); p->set_a_char('i'); p->set_a_char('n');
	}
	p->pop();
	break;
     default:
	dbg1(Warning, "score: time value %c not defined\n", 
	     (void *)((int)cc));
    rest:
	o_timeval = timeval;
	o_dot = dot;
	if (!strchr ((const char *)"+^i", (int)cc)) {
	  for (i=2; i< STAFF; i++ ) {
	    if ((c = tolower(ch[i])) != ' ') {
	      rest_note=0;
	      if (c > 'p' && c != 'x' && c < 220 ) // june 05 wbc ignored the N10..N20
		continue;                   // feb 04 wbc raised this.
	      if ( prev && prev[0] == '+' && prev[i] == 'Q') {
		p->set_highlight();
	      }
	      if ( i < 8 ) {	
		// flip italian
		if (f->num_flag == ITAL_NUM &&
		    (((c >= '0' && c <= '9') 
		      || c == 'x' || (c >= 230 && c <= 240 )) && i < 8)) {
		  // 
		  // Peter Nightingale uses !x, \.!x and \:!x for 10, 11, 12
		  // 
		  if ( c == 'x') {
		    if (prev[i] == ':') {
		      put_note(p, i-1, 'z', timeval, f, ch);
		    }
		    else if (prev[i] == '.') {
		      put_note(p, i-1, 'y', timeval, f, ch);
		    }
		    else
		      put_note(p, i-1, 'x', timeval, f, ch);
		  }

		  else if (c >= 230 && c <= 240) // wbc June 2005
		    put_note(p, i-1, c, timeval, f, ch);
		  else
		    put_note(p, i-1, c, timeval, f, ch);
		}
		else if (c == 'z')
		  put_note(p, i-1, 'd', timeval, f, ch); 
		else if (c >= 'a' && c <= 'p')
		  put_note(p, i-1, c, timeval, f, ch); /* i-1 is string number */
	      } // i >= 8 - bourdon
	      else if (prev && isalnum(c)) {
		if (prev[i] == '/' )
		  put_note(p, 8, 'a', timeval, f, ch);
		else if (prev[i] == 's' )
		  put_note(p, 9, 'a', timeval, f, ch);
		else if (prev[i] == 't' )
		  put_note(p, 10, 'a', timeval, f, ch);
		else if (prev[0] != '+' && c == '4') 
		  put_note(p, 11, 'a', timeval, f, ch);
		else if (prev[0] != '+' && c == '5') 
		  put_note(p, 12, 'a', timeval, f, ch);
		else if (!(f->num_flag == ITAL_NUM)) {
		  if ( c != 'u' && c != 'x')
		    put_note(p, i-1, c, timeval, f, ch);
		}
		else
		  put_note(p, i-1, c, timeval, f, ch);
	      }
	      else if (isalnum(c)) { // no / here
		put_note(p, 7, c, timeval, f, ch);
	      }
	      p->clear_highlight();
	    }
	    // else space here
	    //  else {
	    //  put_note(p, i-1, 255, timeval, f, ch);
	    //}
	  }
	  if (f->m_flags & SOUND) {
	    // timeval is from -2 to 5 - 128 to 
	    double t_val=0.0;
	    switch (timeval) {
	    case -2:
	      t_val = 512;
	      break;
	    case -1:
	      t_val = 256;
	      break;
	    case 0:
	      t_val = 128;
	      break;
	    case 1:
	      t_val = 64;
	      break;
	    case 2:
	      t_val = 32;
	      break;
	    case 3:
	      t_val = 16;
	      break;
	    case 4:
	      t_val = 8;
	      break;
	    case 5:
	      t_val = 4;
	      break;
	    }
	    if (dot) t_val = 1.5 * t_val;
	    // 
	    // handle rests here
	    // 
	    if (ch[1] == 'M')
	      ;
	    else if (rest_note) {
	      sp->rest(t_val/conv);
	    }
	    else
	      sp->play(t_val/conv);
	  }
	}

	dot = 0;
	break;
    }
    
    /*    getloc(1); */
    p->pop();
    p->moveh(space + width);
}

void put_note(print *p, int string, unsigned char c, int timeval, struct file_info *f, char *ch)
{
    int note, adj=0;
    int pos=0;

    note = find_note(string, c, f);
    pos = getpos(note, &adj);
    //    if (f->m_flags & GUIT) {
    //      fprintf(stderr, "HERE\n");
    //      pos -= 12;
    //    }
    p->push();
    p->movev("0.083 in");
    p->movev((double)(pos) * str_to_inch(mus_space) *  0.5);
    // fprintf (stderr, "put_note: pos is %d %d %c\n", pos, adj, note);

    if (timeval == -3 ) p->put_a_char('W');
    else if (timeval == -2 ) p->put_a_char('J');         /* whole note*/
    else if (timeval ==-1 ) p->put_a_char(0127);         /* whole note*/
    else if (timeval == 0 ) p->put_a_char(0167);       /* half note */
    else if (timeval == 1 ) p->put_a_char(0312);     /* quarter note */
    else if (timeval == 2 ) p->put_a_char(0313);     /* eight note */
    else if (timeval == 3 ) p->put_a_char(0314);     /* sixteenth note */
    else if (timeval == 4 ) p->put_a_char(0315);     /* thirty-2 note */
    else if (timeval == 5 ) p->put_a_char(0316);     /* 64 note */
    else p->put_a_char(0202 + timeval);              /* a number */
    if (dot) {
	p->moveh(1.0 * str_to_inch(".1 in"));
	p->put_a_char('.');
	p->moveh(-1.0 * str_to_inch(".1 in"));
    }
   	/* ledger line */
    if (f->m_flags & GUIT ) {
      // on line
      if (pos == 11 || pos == 13 || pos == 15 || pos == 17 || pos == 19 || pos == 21 || pos == 23) ledger (p, 0.5);
      if (pos == 13 || pos == 15 || pos == 17 || pos == 19 || pos == 21 || pos == 23) ledger (p, 1.5);
      if (pos == 15 || pos == 17 || pos == 19 || pos == 21 || pos == 23) ledger (p, 2.5);
      if (pos == 17 || pos == 19 || pos == 21 || pos == 23) ledger (p, 3.5);
      if (pos == 19 || pos == 21 || pos == 23) ledger (p, 4.5);
      if (pos == 21 || pos == 23) ledger (p, 5.5);
      if (pos == 23) ledger (p, 6.5);
      // between lines
      if (pos == 10 || pos == 12 || pos == 14 || pos == 16 || pos == 18 || pos == 20 || pos == 22) ledger (p, 1.0);
      if (pos == 12 || pos == 14 || pos == 16 || pos == 18 || pos == 20 || pos == 22) ledger (p, 2.0);
      if (pos == 14 || pos == 16 || pos == 18 || pos == 20 || pos == 22) ledger (p, 3.0);
      if (pos == 16 || pos == 18 || pos == 20 || pos == 22) ledger (p, 4.0);
      if (pos == 18 || pos == 20 || pos == 22) ledger (p, 5.0);
      if (pos == 20 || pos == 22) ledger (p, 6.0);
    }
    if (pos == 11 || pos == 23 || pos == 25) ledger (p, 0.5);
    else if (pos == 24 || pos == 26) ledger (p, 1.0);
    if (pos == 25) ledger (p, 1.5);
    if (pos == 26) ledger (p, 2.0);
    if (pos == 27) {ledger (p, 1.5);ledger (p, 2.5);}
    if (adj) {
	p->moveh ("-.07 in");
	if (adj == 1) p->put_a_char('#');
	else p->put_a_char('?');
    }
    p->pop();

    if ((f->m_flags & SOUND) && (ch[1] != 'M')) 
      sp->add(note);
      
}

void ledger(print *p, double dist)
{
    p->movev (dist * -1.0 * str_to_inch(mus_space) + 0.017);
    p->moveh("-0.05 in");
    p->put_rule("0.2 in", "0.003 in");
    p->movev (dist * str_to_inch(mus_space) - 0.017);
    p->moveh("0.05 in");
}
/*          a  -b   b  c  c# d -e  e  f  f# g -a  */
/* renaissance */
int r_steps[] = {0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 7};
int r_flat[] =  {0,-1, 0, 0, 1, 0,-1, 0, 0, 1, 0,-1};

/* baroque */
int b_steps[] = {0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 7};
int b_flat[] =  {0,-1, 0, 0, 1, 0,-1, 0, 0, 1, 0,-1};
// sharp tuning
//int b_steps[] = { 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 7};
//int b_flat[] =  { 0,-1, 0, 0, 1, 0,-1, 0, 0, 1, 0,-1};

int 
getpos(               /* pos as int from top g = m string 1 = 45 */
     int note,              /* string 1 open = 33 = g = 9 steps */
     int *adj)			/* -1 flat 0 1 sharp */
{
    int oct, step;

    oct = note/OCTAVE;
    step = note % OCTAVE;
    if (baroque) {
	*adj = b_flat[step];
	return(27 - (oct * 7 + b_steps[step]));
    }
    else {
	*adj = r_flat[step];
	return(27 - (oct * 7 + r_steps[step]));
    }
}


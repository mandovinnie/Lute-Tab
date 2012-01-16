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
#include <stdio.h>
#include <ctype.h>
#include "tab.h"

extern char interspace[];
extern char staff_height[];

double str_to_inch();


#define OCTAVE 12

/* low b-flat is 0 */
/*
 b- b c c# d e- e f f# g a- a  b- b  c
 0  1 2 3  4 5  6 7 8  9 10 11 12 13 14
 */
int strings[] = {
    0,		/* null string */
    33,		/* g 1 */		    
    28,		/* d 2 */
    23,		/* A 3 */
    19,		/* F 4 */
    14,		/* C 5 */
    9,		/* G 6 */
    7,          /* F 7 */
    4,		/* D 8 */
    2,		/* C 9 */
    0           /* b- 10 */ /* just to make life ok for 8,9 string */
  };
int b_strings[] = {
    0,		/* null string */
    31,		/* f */		    
    28,		/* d */
    23,		/* A */
    19,		/* F */
    16,		/* D */
    11,		/* A */
    8,		/* F# */
    6,		/* E */
    4		/* D */
  };

find_note(string, c, f)
     int string;
     char c;
     struct file_info *f;
{
    int fret;
    int note;

    if (f->num_flag == ITAL_NUM) {
	fret = c - '0';
	note = strings[7 - string] + fret;
	if (f->flags & X_POSE) note +=2;
	return (note);
    }
    else {
	fret = tolower(c) - 'a';
	if (baroque ) note = b_strings[string] + fret;
	else note = strings[string] + fret;
	if (f->flags & X_POSE) note +=2;
	return (note);
    }
}

int dot=0;
int o_timeval=0;

score( n, space, width, f, ch, prev) 
     int n;			/* char we are on */
     double space;
     double width;
     struct file_info *f;
     char *ch, *prev;
{
    int i;
    char c;
    int timeval;
    char cc = *ch;

    push();

    use_font(0);

    switch (cc) {
      case 'b':
	movev(interspace);
	fmovev(10.0 * str_to_inch(interspace));
	f_put_rule (str_to_inch(staff_height),
		    10.0 * str_to_inch(interspace) 
		    + str_to_inch (staff_height));
	break;
      case '.':
	fmovev(3.0 * str_to_inch(interspace));
	put_a_char('Z');
	movev(interspace);
	put_a_char('Z');
	fmovev(5.0 * str_to_inch(interspace));
	put_a_char('Z');
	movev(interspace);
	put_a_char('Z');
	break;
      case 'R':
      case 'S':
      case 'T':
      case 't':
	fmovev (3.0 * str_to_inch(interspace));
	put_a_char(cc);
	break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
	if (baroque || f->flag_flag == CONTEMP_FLAGS) timeval = cc - '0' + 1;
	else timeval = cc - '0';
	goto rest;
      case 'w':
	if (baroque || f->flag_flag == CONTEMP_FLAGS) timeval = 0;
        else timeval = -1;
	goto rest;
      case 'x':
        timeval = o_timeval;
rest:
	o_timeval = timeval;
	if (ch[1] == '.') dot++;
	if (!strchr ("+^i", cc)) {
	    for (i=2; i< STAFF; i++ ) {
		if ((c = tolower(ch[i])) != ' ') {
		    if ( i < 8 ) {
			if (f->num_flag == ITAL_NUM) {
			    if (c >= '0' && c <= '9' && i < 8)
			      put_note(i-1, c, timeval, f);
			}
			else if (c >= 'a' && c <= 'p')
			  put_note(i-1, c, timeval, f); /* i-1 is string number */
		    }
		    else if (prev) {
			if (prev[i] == '/' )
			  put_note(8, 'a', timeval, f);
			else if (prev[i] == 's' )
			  put_note(9, 'a', timeval, f);
			else if (prev[i] == 't' )
			  put_note(10, 'a', timeval, f);
			else if (!(f->num_flag == ITAL_NUM))
			  put_note(i-1, c, timeval, f);
		    }
		}
	    }
	}
	dot = 0;
	break;
    }
    
/*    getloc(1); */
    pop();
    fmoveh(space + width);
}

put_note(string, c, timeval, f)
     int string;
     char c;
     int timeval;
     struct file_info *f;
{
    int note, adj;
    int pos=0;

    note = find_note(string, c, f);
    pos = getpos(note, &adj);
    push();
    movev("0.12 in");
    fmovev((double)(pos) * str_to_inch(interspace) *  0.5);

    if (timeval == -1 ) put_a_char(0127);         /* whole note*/
    else if (timeval == 0) put_a_char(0167);       /* half note */
    else if (timeval == 1 ) put_a_char(0312);     /* quarter note */
    else if (timeval == 2 ) put_a_char(0313);     /* eight note */
    else if (timeval == 3 ) put_a_char(0314);     /* sixteenth note */
    else if (timeval == 4 ) put_a_char(0315);     /* thirty-2 note */
    else if (timeval == 5 ) put_a_char(0316);     /* 64 note */
    else put_a_char(0202 + timeval);              /* a number */
    if (dot) {
	fmoveh(1.0 * str_to_inch(".1 in"));
	put_a_char('.');
	fmoveh(-1.0 * str_to_inch(".1 in"));
    }
   	/* ledger line */
    if (pos == 11 || pos == 23 || pos == 25) ledger (0.5);
    else if (pos == 24) ledger (1.0);
    if (pos == 25) ledger (1.5);
    if (adj) {
	moveh ("-.07 in");
	if (adj == 1) put_a_char('#');
	else put_a_char('?');
    }
    pop();
}

ledger(dist)
double dist;
{
    fmovev (dist * -1.0 * str_to_inch(interspace) + 0.017);
    moveh("-0.05 in");
    put_rule("0.2 in", "0.001 in");
    fmovev (dist * str_to_inch(interspace) - 0.017);
    moveh("0.05 in");
}
/*            -b   b  c  c# d -e  e  f  f# g -a  a  */
/* renaissance */
int r_steps[] = {0,  0, 1, 1, 2, 3, 3, 4, 4, 5, 6, 6};
char r_flat[] = {-1, 0, 0, 1, 0,-1, 0, 0, 1, 0,-1, 0};

/* baroque */
int b_steps[] = {0,  0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6};
char b_flat[] = {-1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0};
 
getpos(note, adj)     /* pos as int from top g = m string 1 = 45 */
     int note;              /* string 1 open = 33 = g = 9 steps */
     int *adj;			/* -1 flat 0 1 sharp */
{
    int oct, step;

    oct = note/OCTAVE;
    step = note % OCTAVE;
    if (baroque) {
	*adj = b_flat[step];
	return(26 - (oct * 7 + b_steps[step]));
    }
    else {
	*adj = r_flat[step];
	return(26 - (oct * 7 + r_steps[step]));
    }
}

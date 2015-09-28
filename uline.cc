
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
 * this draws an underline
 * my_underline sets the starting point.  When the program
 * gets to the endpoint do_uline is called, and the line is 
 * drawn from the saved beginning to the current point
 */

#include "tab.h"
#include "print.h"
#include "system.h"

extern char interspace[];

/* EXTERNAL */

/* start an my_underline -
 *
 * skip spaces - flag
 * s1 starting point 5 or 7 for horizontal line
 *                   9 or 11  for slanted line
 * line  - staff line we are on
 */

void
my_underline(print *p, font_list *f_a[],
	     int *skip_spaces,
	     int s1,
	     int line)
 	/* should we adjust beginning and end? */
{
    int i_space = str_to_dvi(interspace);
    int s1a;

    if (s1 < 9)
      s1a = 15 + (((s1-3) / 2) * line);
    else
      s1a = s1;

    //   printf ("my_underline: skip %d line %d s1 %d s1a %d\n", 
    //	    *skip_spaces, line, s1, s1a);
 
    if (*skip_spaces) {
	p->p_movev(*skip_spaces * i_space);
	*skip_spaces = 0;
    }
    p->push();

    if (s1 != 7){
	p->p_movev( (-4 * i_space) / 10);
	p->moveh(inch_to_dvi(f_a[0]->fnt->get_width(0)/ 2 ));
    }

    p->saveloc(s1a);
    p->pop();
    p->p_movev(i_space); 
}

void 
do_uline(print *p, int *skip_spaces, int s1, int s2, int line)
{
    int i_space = str_to_dvi(interspace);
    int s1a;
 
    if (s1 < 9) s1a = 15 + (((s1-3) / 2) * line);
    else (s1a = s1);

    //    printf ("do_uline: line %d s1 %d sa-current-location 2 %d s1a-start %d\n",
    //	    line, s1, s2, s1a);

    if (*skip_spaces) {
	p->p_movev(*skip_spaces * i_space);
	*skip_spaces = 0;
    }

    p->push();
    if (s1 != 7) {
	p->p_movev( (-4 * i_space) / 10);
    }

    p->saveloc(s2);		/* save current location */
    p->getloc(s1a);		/* go to start */

    if (s1 > 7 ) p->put_slant(s1a, s2);
    else {
      p->put_uline( s1a, s2);
      // p->put_r_uline( s1a, s2);
    }
    p->pop();
    p->p_movev(i_space); 
}

void 
do_r_uline(print *p, int *skip_spaces, int s1, int s2, int line)
{
    int i_space = str_to_dvi(interspace);
    int s1a;
 
    if (s1 < 9) s1a = 15 + (((s1-3) / 2) * line);
    else (s1a = s1);

    // printf ("do_r_uline: line %d s1 %d s2 %d s1a %d\n", line, s1, s2, s1a);

    if (*skip_spaces) {
	p->p_movev(*skip_spaces * i_space);
	*skip_spaces = 0;
    }

    p->push();
    if (s1 != 7) {
	p->p_movev( (-4 * i_space) / 10);
    }

    p->saveloc(s2);		/* save current location */
    p->getloc(s1a);		/* go to start */

    if (s1 > 7 ) p->put_slant(s1a, s2);
    else  p->put_r_uline( s1a, s2);
    p->pop();
    p->p_movev(i_space); 
}

void 
do_w_uline(print *p, int *skip_spaces, int s1, int s2, int line)
{
    int i_space = str_to_dvi(interspace);
    int s1a;
 
    if (s1 < 9) s1a = 15 + (((s1-3) / 2) * line);
    else (s1a = s1);

    // printf ("do_w_uline: line %d s1 %d s2 %d s1a %d\n", line, s1, s2, s1a);

    if (*skip_spaces) {
	p->p_movev(*skip_spaces * i_space);
	*skip_spaces = 0;
    }

    p->push();
    if (s1 != 7) {
	p->p_movev( (-4 * i_space) / 10);
    }

    p->saveloc(s2);		/* save current location */
    p->getloc(s1a);		/* go to start */

    if (s1 > 7 ) p->put_slant(s1a, s2);
    else  p->put_w_uline( s1a, s2);
    p->pop();
    p->p_movev(i_space); 
}

void 
do_thick_uline(print *p, int *skip_spaces, int s1, int s2, int line)
{
    int i_space = str_to_dvi(interspace);
    int s1a;
 
    // printf ("do_thick_uline: line %d s1 %d s2 %d s1a %d\n", line, s1, s2, s1a);

    if (s1 < 9)
      s1a = 15 + (((s1-3) / 2) * line);
    else 
      (s1a = s1);
    if (*skip_spaces) {
	p->p_movev(*skip_spaces * i_space);
	*skip_spaces = 0;
    }

    p->push();
    if (s1 != 7) {
	p->p_movev( (-4 * i_space) / 10);
    }

    p->saveloc(s2);		/* save current location */
    p->getloc(s1a);		/* go to start */

    if (s1 > 7 ) p->put_med_slant(s1a, s2);
    else  p->put_uline(s1a, s2);
    p->pop();
    p->p_movev(i_space); 
}

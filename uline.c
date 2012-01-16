
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
extern char interspace[];

underline(skip_spaces, s1, s2, no_adj )
     int *skip_spaces;
     int s1, s2;
     int no_adj;		/* should we adjust beginning and end? */
{
    double str_to_inch();
    int i_space = str_to_dvi(interspace);

    if (*skip_spaces) {
	p_movev(*skip_spaces * i_space);
	*skip_spaces = 0;
    }
    push();
    if (! no_adj){
	p_movev( (-4 * i_space) / 10);
	p_moveh(p_get_w(0, 0)/ 2 );
    }
    saveloc(s1); 
    pop();
    p_movev(i_space); 
}

do_uline(skip_spaces, s1, s2, no_adj)
int *skip_spaces;
int s1, s2, no_adj;
{
    int i_space = str_to_dvi(interspace);

    if (*skip_spaces) {
	p_movev(*skip_spaces * i_space);
	*skip_spaces = 0;
    }
    push();
    if (! no_adj) {
	p_movev( (-4 * i_space) / 10);
    }
    saveloc(s2);
    getloc(s1);
    if (s1 > 7 ) put_slant(s1, s2);
    else put_uline( s1, s2);
    pop();
    p_movev(i_space); 
}

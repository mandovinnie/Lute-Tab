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
#include <setjmp.h>
#include "tab.h"
#include "print.h"
#include "tfm.h"
#include "system.h"

#define WEIGHTS 15		/* number of different wieghts */

void p_err(int sys, int line, char *arg);

void
printsystem(print *p, i_buf *i_b, font_list *f_a[], int *l_p, struct file_info *f)
{
    double extra/* , total */;	/* space taken by notes */
    /* 0=0f 1=1f 2=2f 3=3f 4=4,5f 5=i or b  6=i or b with following
       7=ornament  8=big C  
       9=squiggle  10=fermata with dot   11= ~ 12 = after orn (0) 
       13=fingering */

    n_system++;

    /*   total = */ extra = 0.0;

    pass1(f_a, l_p, f, &extra);		/* total the widths */

    pass2(p, i_b, f_a, l_p, f, &extra);

    return;
}

void p_err(int sys, int line, char *arg)
{
    dbg3(Warning, "tab: %s in system %d at line %d\n", 
	    (void *)arg, (void *)sys, (void *)line);
}


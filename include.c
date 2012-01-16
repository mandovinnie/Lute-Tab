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

#include "tab.h"

include(f, file)	      /* we should read as main input, then return */
struct file_info *f;
char file[];
{
    struct file_info *new;
    struct b old_input;
    extern struct b input;
    FILE *fp;
    int nread;

    new = (struct file_info *)malloc( sizeof (struct file_info));
    /* printf ("include, file is %s\n", file); */

    new->file = (char *)malloc( BL );
    new->i_file = (char *)malloc( BL );
    new->out_file = (char *)malloc( BL );
    strcpy (new->file, file);
    strcpy (new->i_file, f->i_file);
    new->i_p = f->i_p;
    strcpy (new->out_file, f->out_file);
    new->flags     = f->flags;
    new->line_flag = f->line_flag;
    new->flag_flag = f->flag_flag;
    new->char_flag = f->char_flag;
    new->num_flag  = f->num_flag;
    new->note_flag = f->note_flag;
    new->n_text    = f->n_text;
    new->note_conv = f->note_conv;
    new->c_space   = f->c_space;
    new->page      = f->page;
    
    fp = fopen(new->file, "r");
    if (fp == NULL) {
	printf("tab: can't open include file %s, continuing anyways\n",
		 new->file);
	return;
    }

    bcopy (&input, &old_input, sizeof (struct b));

    fseek(fp, 0L, 2);

    input.ib_s = ftell(fp);
    fseek(fp, 0L, 0);
    input.ib = (char *) malloc(input.ib_s);
    input.ib_p = 0;

    if ((nread = fread ( input.ib, 1, input.ib_s, fp)) != input.ib_s) {
	printf(
		 "tab: rdfile: error reading file %s, %d read %d wanted\n", 
		 f->file, nread, (int)input.ib_s);
	if (nread <= 0) exit (-1); 
    }
    fclose (fp);

    new->flags |= INCLUDE;

    get_tab_file(new);

    f->page = new->page;
    free(input.ib);
    free(new);

    bcopy (&old_input, &input, sizeof (struct b));
/*    new->flags &= ~INCLUDE; */
    return;
    
}

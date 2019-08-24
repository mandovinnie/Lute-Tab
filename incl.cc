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
#ifdef sgi
#include <bstring.h>
#endif
#include "win.h"
#include "tab.h"
#include "i_buf.h"
#include "file_in.h"

void get_tab_file(file_in *fi, i_buf *ib, struct file_info *f);
void include(i_buf *ib, struct file_info *f, char file []);

extern double staff_len, o_staff_len;

/* we should read as main input, then return */

void include(i_buf *ib, struct file_info *f, char file [])
{
    struct file_info *newf;
    file_in *ffi;

    newf = (struct file_info *)malloc( sizeof (struct file_info));

    newf->file = (char *)malloc( BUFSIZ );
    strcpy (newf->file, file);
    newf->out_file  = (char *)malloc( BUFSIZ );
    newf->flags     = f->flags;
    newf->m_flags   = f->m_flags;
    newf->line_flag = f->line_flag;
    newf->flag_flag = f->flag_flag;
    newf->char_flag = f->char_flag;
    newf->num_flag  = f->num_flag;
    newf->note_flag = f->note_flag;
    newf->n_text    = f->n_text;
    newf->note_conv = f->note_conv;
    newf->c_space   = f->c_space;
    newf->page      = f->page;
    newf->sys_skip  = f->sys_skip;
    /* font names and font sizes? */
    newf->include   = 1;

    if ( ! (f->m_flags & QUIET ))
      dbg1(Warning, "tab: including file %s\n", (void *)newf->file);

    newf->flags |= INCLUDE;

    staff_len = o_staff_len;

#if defined WIN32
    ffi = new file_in(newf->file, "r");
#else
    ffi = new file_in(newf->file, "rb");
#endif  /* WIN32 */

    get_tab_file(ffi, ib, newf);

    delete (ffi);

    f->page = newf->page;
    free(newf->file);
    free(newf->out_file);
    free(newf);

    return;

}

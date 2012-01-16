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
#ifdef X_WIN
#undef POSTSCRIPT
#undef DVI
#endif

/*
#ifdef lintX
#define DVI
#define POSTSCRIPT
#endif
*/


main (argc, argv)
     int argc;
     char **argv;
{
    SIGNED_C c, get();
    struct file_info ff;
    struct file_info *f = &ff;

    printf ("\n\ntab: copyright 1993 Wayne Cripps\n  %s\n\n", version);

    f = (struct file_info *)malloc(sizeof (struct file_info));
    f->file = (char *)malloc( BL );
    f->i_file = (char *)malloc( BL );
    f->out_file = (char *)malloc( BL );
    f->line_flag = BETWEEN_LINE;
    f->flag_flag = STAND_FLAGS;
    f->char_flag = STAND_CHAR;
    f->num_flag = STAND_NUM;
    f->note_conv = 3;
    f->c_space = 0.0;
    f->file[0] = '\0';
    f->flags = 0;
    f->page = 0;

    init_ps_font_names();

    argc--;
    *argv++;

    args (argc, argv, f);

    if (!f->file[0]) usage();

    strcpy (f->i_file, "inter.itb");

    if (!strncmp (f->file, f->i_file, strlen(f->i_file)) ) {
	printf("tab: using intermediate file\n");
    }
    else {
	rdfile(f, 0 /* input file */);

/* pre read first line for size commands */

	if ((c = get()) == '-') {
	    char buf[BL];
	    int i=0;
	    buf[i++] = c;
	    while ((c = get ()) != '\n')
	      buf[i++] = c;
	    args_from_string(buf, f);
	}
	else unget(c);

	f->i_p = fopen (f->i_file, "w+");
	if (f->i_p == NULL ) {
	    printf ("cant open intermediate file for writing\n");
	    exit (-3);
	}

/* main input routine here */

	get_tab_file(f);
	fflush(f->i_p);
    }

    rdfile(f, 1);  /* inter file */

    if (f->flags & PS) { 
#ifdef POSTSCRIPT
#ifdef vax
	if (!strlen(f->out_file)) 
#endif /* vax */
	  strcpy (f->out_file, "out.ps");
	make_ps(f->out_file, f);
#else
	printf ("tab was not compiled with POSTSCRIPT option!\n");
	printf ("define POSTSCRIPT in tab.h and recompile\n");
#endif /* POSTSCRIPT */
    }
    else if (f->flags & X) {
#ifdef X_WIN
	make_x(f);
#endif /* X_WIN */
    }
    else {
#ifdef DVI    
	if (!strlen(f->out_file)) strcpy (f->out_file, "out.dvi");
 	make_dvi(f); 
#endif
    }
    malloc_stat();
    exit (0);
}

static struct file_info *ff;

get_tab_file(f)
struct file_info *f;
{
    char *p, *pp;
    int r;
    int ret;
    char buf[BL];

    ff = f;

    while (1) {
	getl(buf, sizeof(buf));
/* 	printf ("tab: get tab file: %s", buf);    */

	p = buf;
	if (setjmp(env) != 0) {
	    printf ("tab: read error not in systems\n");
	    return;
	}
	switch (*p) {
	  case '-':
	    switch (buf[1]) {
	      case 'h':		/* flakey behavior - h cant be turned off */
		{
		    printf ("h flag here in get tab file\n");
		    args_from_string(buf, f);
		}
		p = buf;
		do {
		    puti(*p);
		    p++;
		} while (*p != '\n');
		puti('\n');

		break;
	      default:
		do {
		    puti(*p);
		    p++;
		} while (*p != '\n');
		puti('\n');
		break;
	    }
	    break;
	  case '{':
	  case '[':
	    do {
		puti(*p);
		p++;
	    } while (*p != '\n');
	    puti('\n');
	    break;
	  case '\0':
	  case 'e':
	    if (!(f->flags & INCLUDE)) 
	      puti('e'); puti('\n');
	    return;
	  case 'p':
/* 	    printf ("main.c: got a p\n");*/
	    puti('p'); puti('\n');
	    break;
	  case '\n':
	  case '\t':
	  case ' ':
	    break;
	  case '%':
	    break;
	  case 'I':	                      	        /* include file */
	    {
		p++;		/* remove I */
		while ( *p == ' ' ) 
		  p++;
		pp = p;
		while ( *pp != '\n' )
		  pp ++;
		*pp = '\0';
		include(f, p);
	    }
	    break;
	  default:		/* assume that it must be lines of chords */
	    if ((ret = setjmp(env)) == 0) {
		r = getsystem(f, buf);	/* get line tab */
		if (!r) {
		    return;
		}
		puti('\n');
	    }
	    else if (ret == 1){ /* longjmp - read error */
		printf ("tab:read error, unexpected eof\n");
		return;	        /* no more to do */
	    } else {		/* normal eof upon an e character*/
		printf ("tab: normal eof\n");
		return;
	    }
	}
    }
}

puti(c)				/* put a char to intermediate buffer */
     char c;                    /* (or file) */
{
    putc(c, ff->i_p);
}



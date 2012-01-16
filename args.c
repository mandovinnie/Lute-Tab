

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
#define N_ARGS 12
extern char staff_len[];

args(argc, argv, f)
int argc;
char ** argv;
struct file_info *f;
{
    double atof();
    char *ptr, *strstr();
    static char o_staff_len[BL];

    if (strlen (o_staff_len) == 0)
      strcpy (o_staff_len, staff_len);

#ifndef DVI
/*    ps++; */
#endif /* DVI */

    while (argc) {
	if (**argv == '-') {
	    (*argv)++;
/*	    printf ("\ntab: args.c: #%s#\n", *argv);*/

	    switch (**argv) {
	      case 'a':
		(*argv)++;
		switch (**argv) {
		  case 'f':
		    if (f->flags & PS ){
			int num;
			char name[80];
			argv++;
			argc--;
			num = atoi(*argv);
			argv++;
			argc--;
			strcpy (name, *argv);
			if (f->flags & PS )
			  set_ps_font (num, name);
			ps_text_tfm(f, num);
		    }
		    break;
		  default:
		    printf("tab: args.c: bad argument to -a flag\n");
		    break;
		}
		break;
	      case 'C':		/* number all measures */
		bar_count++;
		break;
	      case 'c':
		barCount++;	/* number first measures */
		break;
	      case 'b':
		baroque++;
		break;
	      case 'd':
		f->flags |= DEBUG;
		break;
	      case 'B':
		f->line_flag = BETWEEN_LINE;
		f->flag_flag = BOARD_FLAGS;	
		f->char_flag = BOARD_CHAR;
		f->num_flag = STAND_NUM;
		f->note_conv = 0;
		f->note_flag = ITAL_NOTES;
		break;
	      case 'D':
		f->line_flag = BETWEEN_LINE;
		f->flag_flag = THIN_FLAGS;	
		f->char_flag = STAND_CHAR;
		f->num_flag = STAND_NUM;
		f->note_conv = 0;
		f->note_flag = ITAL_NOTES;
		break;
	      case 'e':		/* rotate page 90 degrees */
		f->flags |= ROTATE;
		break;
	      case 'E':
		f->flags |= DRAFT;
		break;
	      case 'f':		/* french - standard */
		f->line_flag = BETWEEN_LINE;
		f->flag_flag = STAND_FLAGS;
		f->char_flag = STAND_CHAR;
		f->num_flag = STAND_NUM;
		f->note_flag = ITAL_NOTES;
		break;
	      case 'F':		/* modern */
		f->line_flag = BETWEEN_LINE;	
		f->flag_flag = CONTEMP_FLAGS;
		f->char_flag = STAND_CHAR;
		f->num_flag = STAND_NUM;
		f->note_flag = ITAL_NOTES;
		break;
	      case 'G':
		f->flags |= COPYRIGHT;
		break;
	      case 'H':		/* convert to Italian */
		f->flags |= CON_SEV;
	      case 'h':
		f->flags |= CONVERT;
		f->line_flag = ON_LINE;	
		f->num_flag = ITAL_NUM;
		f->note_flag = ITAL_NOTES;
		f->flag_flag = THIN_FLAGS;
		break;
	      case 'i':		/* plain italian with line offset */
		f->line_flag = ON_LINE;
		f->flag_flag = S_ITAL_FLAGS;
		f->char_flag = STAND_CHAR;
		f->num_flag = ITAL_NUM;
		f->note_flag = ITAL_NOTES;
		break;
	      case 'I':
		f->line_flag = BETWEEN_LINE;
		f->flag_flag = S_ITAL_FLAGS;
		f->char_flag = STAND_CHAR;
		f->num_flag = ITAL_NUM;
		f->note_flag = ITAL_NOTES;
		break;
	      case 'x':
		f->line_flag = ON_LINE;
		f->flag_flag = THIN_FLAGS;
		f->char_flag = STAND_CHAR;
		f->num_flag = ITAL_NUM;
		f->note_flag = ITAL_NOTES;
		break;
	      case 'O':
		f->line_flag = ON_LINE;
		f->flag_flag = CAP_FLAGS;
		f->char_flag = STAND_CHAR;
		f->num_flag = ITAL_NUM;
		f->note_flag = ITAL_NOTES;
		break;
	      case 'T':
		f->line_flag = BETWEEN_LINE;
		f->flag_flag = THIN_FLAGS;
		f->char_flag = STAND_CHAR;
		f->num_flag = STAND_NUM;
		f->note_flag = ITAL_NOTES;
		break;
	      case 'w':
		f->line_flag = ON_LINE;
		f->flag_flag = S_ITAL_FLAGS; 
		f->char_flag = STAND_CHAR; 
		f->num_flag = ITAL_NUM; 
		f->note_flag = ITAL_NOTES; 
		f->flags |= WALLACE; 
		f->flags |= NO_MUSIC_B; 
		f->flags |= BIGNOTES; 
		f->flags |= ROTATE;
		f->flags |= SHARP_UP;
		f->note_conv = 0;
		break;
	      case 'Z':
		f->line_flag = ON_LINE;
		f->flag_flag = S_ITAL_FLAGS; 
		f->char_flag = STAND_CHAR;
		f->note_flag = ITAL_NOTES;
		f->num_flag = ITAL_NUM; 
/*		f->flags |= WALLACE; */
		f->flags |= NO_MUSIC_B; 
		f->flags |= BIGNOTES; 
/*		f->flags |= SHARP_UP; */
		f->note_conv = 0;
		break;
	      case 'l':
		(*argv)++;
		if (!**argv) *argv++, argc--;
		if (argc < 1 ) usage();
		if (**argv == '-') usage();
		if ( ! strncmp ("reset", *argv, 5))
		  strcpy (staff_len, o_staff_len);
		else {
		    strcpy (o_staff_len, staff_len);
		    strcpy(staff_len, *argv);
		}
		break;
	      case 'L':
		f->flags |= LSA_FORM;
		f->line_flag = BETWEEN_LINE;
		f->flag_flag = CONTEMP_FLAGS; 
		f->char_flag = STAND_CHAR; 
		f->num_flag = STAND_NUM; 
		f->note_flag = MOD_NOTES; 
		f->note_conv = 0;
		break;
	      case 'm':
		f->flags |= MANUSCRIPT;
		break;
	      case 'M':
		f->flags |= MARKS;
		break;
	      case 'n':
		f->char_flag = MACE_CHAR;
		break;
	      case 'N':
		f->char_flag = ROB_CHAR;
		break;
	      case 'o':
		(*argv)++;
		if (!**argv) *argv++, argc--;
		if (argc < 1 ) usage();
		if (**argv == '-') usage();
		strcpy (f->out_file, *argv);
		if (ptr = strstr (f->out_file, ".tab")) {
		    *ptr = '\0';
		}
		if (f->flags & PS) strcat (f->out_file, ".ps");
		else strcat (f->out_file, ".dvi");
		printf ("tab: sending output to %s\n", f->out_file);
		break;
	      case 'p':
		f->flags |= PAGENUM;
		break;
#ifdef POSTSCRIPT
	      case 'P':
		f->flags |= PS;
		f->flags &= ~DVI_O;
		break;
	      case '6':
		f->flags |= DPI600;
		red = 1.0;
		break;
#endif /* POSTSCRIPT */
	      case 'Q':
		f->flags |= BIGNOTES;
		break;
	      case 'r':
		baroque=0;
		break;
	      case 'R':
		if ((*argv)[1] == '9')      red = 0.88880;
		else if ((*argv)[1] == '8') red = 0.777770;
		else                        red = 0.6666666;
		break;
	      case 'S':
		f->flags |= SAMELINE;
		break;
	      case 't':
		f->line_flag = ON_LINE;
		break;
	      case 'v':
		f->flags |= VERBOSE;
		break;
	      case 'V':
		printf ("%s\n", version);
		exit(0);
	      case 'W':
		f->flags |= X_POSE;
		break;
#ifdef X_WIN
	      case 'X':
		f->flags |= X;
		f->flags &= ~DVI_O;
		f->flags &= ~PS;
		break;
#else
	      case 'X':
		f->flags |= NO_EXPAND;
		break;
#endif /* X_WIN */
	      case 'Y':
		f->flags |= NO_WORD;
		break;
	      case 'y':
		f->flags &= ~ITAL_TXT;
		break;
	      case 'z':
		f->flags |= ITAL_TXT;
		break;
	      case '5':
		f->flags |= FIVE;
		break;
	      default:
		printf ("tab: unknown flag %c\n", (**argv));
		usage();
		break;
	    }
	    *argv++;
	    argc--;
	}
	/* assume what is left is filename */
	else {			/* not - */
	    strcpy (f->file, *argv);
	    *argv++;
	    argc--;
	}
/*	 	printf ("args: flags %X\n", f->flags);  */
    }
    if (f->line_flag != ON_LINE && 
	f->line_flag != BETWEEN_LINE)
      printf ("tab: line flag error! %X\n", (unsigned int)f->line_flag);


#ifdef POSTSCRIPT
#ifndef DVI
/*     printf ("args: setting ps mode\n"); */
    f->flags |= PS;
    f->flags &= ~DVI_O;
#endif /* DVI */
#endif /* POSTSCRIPT */    
}

args_from_string(buf, f)
char *buf;
struct file_info *f;
{
    int argc, i, j;
    char argv[120];
    char *argp[N_ARGS];
    int quote=0;

    for( i=0; i < N_ARGS; i++) argp[i] = 0;
    argp[0]=argv;
    argc=1;

    for (i=0,j=0; i<sizeof (argv) && buf[i] != 0; i++, j++) {
	argv[j] = buf[i];

	if (argv[j] == '"' || argv[j] == '\'') {
	    if (quote) {
		quote=0;
		argv[j] = 0;
	    }
	    else {
		i++;
		argv[j] = buf[i];
		quote++;
	    }
	}
	if (!quote && buf[i] == ' ') {
	    argv[j] = '\0';
	    argp[argc] = &argv[j+1];
	    argc++;
	}
    }
    argv[i]='\0';

/*    for (i=0; i<N_ARGS; i++) {
	printf("--> **%s**\n", argp[i]);
    }
 */
    args(argc, argp, f);
    
}


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
 *  wbc June 7 1991
 *
 */

#include <stdio.h>
#include <setjmp.h>
#include "tab.h"


int line = 0;
char music[4];
char i_text[64];		/* buffer for text */
int  text_f = 0;		/* flag - do we have text? */


getsystem(f, buf)
struct file_info *f;
char buf[];			/* first line */
{ 
    int *l_p;
	char N[2];
    int barline=0,BARline=0;
    char staff[STAFF], ornament[STAFF], finger[STAFF], a_ornament[STAFF];
    SIGNED_C c;
    char cc, *p;
    int i, j;
    SIGNED_C get();
    int gridflag=0;
    int hushbar=0, tie=0, dimline=0;	/* for non print bars, and ties */
    int skip;

    l_p = (int *)my_malloc(sizeof (int));
    *l_p = 0;


    music[0] = 0;

    do {
      start:			/* a return point for multible b */
	c = buf[0];
/*  	printf(" ->%sXXX", buf);      */


	staff[0] = 0; skip = 0;

	switch (c) {
	  case '%':
	    break;
	  case '-':
	    p = buf;
	    do {
		puti(*p);
		p++;
	    } while (*p != '\n');
	    puti('\n');
	    printf ("getsystem: setting switches %s", buf);
	    args_from_string(buf, f);
	    break;
	  case ' ':
	    incr(buf);
	    goto start;
	    /* new line */
	  case '\n':
	    return(1);
	  case 'R':
	    staff[0] = buf[1];
	    staff[1] = c;
	    for (i=2;i<STAFF;i++) staff[i] = ' ';
	    j = 2;
	    if (buf[j] == '.') {
		staff[2] = '.';
		j++;
	    }
	    if (buf[j] == '\t') {
		skip = j;
		(void)do_music(staff, buf, l_p, &skip, 0, f);
	    }
	    else
	      store(l_p, staff, f);
	    break;
	  case 'd':
	    staff[0] = c;
	    staff[1] = buf[1];
	    if (staff[1] != 'b'){
		for (i=1;i<STAFF;i++) staff[i] = ' ';
	    }
	    else
	      for (i=2;i<STAFF;i++) staff[i] = ' ';
	    store(l_p, staff, f);
	    break;
	  case 'S':		/* a time signature */
	    staff[0] = 'G';
	    if (strchr(buf, '-')) { /* 10/8, etc */
		for (i=1; i<STAFF; i++) {
		    if (buf[i] == '\n') break;
		    staff[i] = buf[i];
		}
		for (;i<STAFF; i++) {
		    staff[i] = ' ';
		}
	    }
	    else {

		staff[1] = buf[1];
		if (staff[1] == 'C') staff[1] = 'U'; /* a C signature */
		if (staff[1] == 'c') staff[1] = 'u';
		staff[2] = buf[2];
		if (staff[2] == '\n' || staff[2] == '\0') {
		    staff[2] = ' ';
		}
		for (i=3;i<STAFF;i++) staff[i] = ' ';
	    }
	    store(l_p, staff, f);
	    break;
	  case 'A':		/* again - repeat */
	    staff[0] = c;
	    staff[1] = buf[1];	/* one or two */
	    for (i=2;i<STAFF;i++) staff[i] = ' ';
	    store(l_p, staff, f);
	    break;
	  case 'P':
	    staff[0] = c;
	    staff[1] = buf[1];
	    staff[2] = buf[2];
	    if (staff[2] == '\n') {
		staff[2] = ' ';
	    }
	    for (i=3;i<STAFF;i++) staff[i] = ' ';
	    store(l_p, staff, f);
	    return(2);
	    break;
	  case 'b':
	    /* barline */
	    if ((c = buf[1]) == '!') {
		hushbar++;
		i_error(1, "hushbar, barline not printed", c);
	    }
	    else if (c == 'T') tie++;
	    else if (c == 'Q') dimline++;
	    else if (c == 'b' || c == '.') {
		store(l_p, "b-         ", f);
		incr(buf);
		goto start;
	    }
	    else if ( c != '\n')
	      i_error(1, "Text  after a barline (b)", c );
	    barline++;
	    break;
	  case 'B':
	    if (buf[1] == '\n' || buf[1] == '!') {
		BARline++;
		if ((c = buf[1]) == '!') hushbar++;
		break;
	    }
	    else {
		staff[0] = 'J';
		staff[1] = '-';
		if (buf[1] == '-')i++;
		else skip--;
		goto rest;
	    }
	  case 't':
	  case '#':
	    staff[0] = buf[1];
	    staff[1] = c;
	    goto rest;
	  case 'x':
	    if (gridflag) break;
	  case 'Y':
	  case 'y':
	  case 'L':
	  case 'W':
	  case 'w':
	  case '0':
	  case '1':
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '~':
	    /* new staff */
	    staff[0] = c;
	    staff[1] = buf[1];
	    if (c == 'Y' || c == 'y') {
		switch (staff[1]) { 
		  case '.':
		    staff[1] = '.';
		    staff[2] = staff[8] = ' ';
		    for (i=3;i<8;i++)  staff[i] = 'Z';
		    store (l_p, staff, f);
		    goto done;
		    break;
		  case 'b':
		    staff[1] = 'b';
		    for (i=3;i<9;i++)  staff[i] = ' ';
		    store (l_p, staff, f);
		    goto done;
		  case '0':
		  case '-':
		    break;
		  default:
		    staff[1] = '-';
		    skip--;
		    break;
		}
	    }
	    else if (staff[1] == '!') {
		staff[1] = '!';
	    }
	    else if (staff[1] == '-') {
		staff[1] = '-';
	    }
	    else if (staff[1] == '#') {
		staff[1] = '#';
	    }
	    else if (staff[1] == '*') {	/* dot in grid */
		staff[1] = '*';
	    }
	    else if (staff[1] == '|') {	/* extra line in grid */
		staff[1] = '|';
	    }
	    else if (staff[1] == '.') {
		staff[1] = '.';
	    }
	    else if (staff[1] == 't') {
		staff[1] = 'w';
	    }
	    else if (staff[1] == 'W') {
		staff[1] = 'W';
	    }
	    else if (staff[1] == 'Q') {	/* highlight */
		staff[1] = 'Q';
	    }
	    else if (staff[1] == '@') {	/* dotted highlight */
		staff[1] = '@';
	    }
	    else {
		skip--;
		staff[1] = '-';
	    }

	  rest:
	    for (i = 0; i < STAFF ; i++) {
		ornament[i] = ' ';
		a_ornament[i] = ' ';
		finger[i] = ' ';
	    }
	    ornament[1] = '-';
	    a_ornament[1] = '-';
	    finger[1] = '-';
	    for (i = 2; i < STAFF; i++) {
		staff[i] = buf[i+skip];
		switch (staff[i]) {
		  case 'd':
		    if ( i == 2 ) {
			if (staff[0] != 'x')
			  staff[i] = 'D';
		    }
		    else  
		      if (staff[i-1] != ' ' && staff[i-1] != '-' )
			staff[i] = 'D';
		    break;
		  case '0':	/* reversed 0 and space for internal lang */
		    break;
		  case 'N':	/* number 10 -> 19 */
		    skip++;
		    N[0] = buf[i+skip];
		    skip++;
		    N[1] = buf[i+skip];
		    staff[i] = atoi(N);
		    if ( ! isdigit (N[0])) i_error(1, "bad fret number", N[0]);
		    if ( ! isdigit (N[1])) i_error(1, "bad fret number", N[1]);
		    if (staff[i] > 34) {
			i_error(1, "fret number higher than max 34", 
				staff[i]);
			staff[i]=0;
		    }
		    staff[i] += 220;
		    break;
		  case 'z':
		    staff[i] = '0';
		    break;
		  case '\n':
		    line++;
		    for ( ; i < STAFF; i++)
		      staff[i] = ' ';
		    break;
		  case ',': 	/* suggested by john@minster.york.ac.uk */
		    break;
		  case '/':
		    if ((cc = buf[i + (++skip)]) == '/') { 
			ornament[i] = 's';
			if ((cc = buf[i + (++skip)]) == '/') { 
			    ornament[i] = 't';
			    skip++;
			}
			else {	/* only two slashes */
			}
			i--; 
		    }
		    else if ( cc == ' ' ) {
			/* leave staff[i] as '/' */
			skip--;
		    }
		    else {
			ornament[i] = '/';
			i--;
		    }
		    break;
		  case '!':	/* infix */
		    staff[i] = buf[i + (++skip)];
		    break;
		  case '"':	/* prefix a non prefix char */
		    ornament[i] = buf[i + (++skip)];
		    skip++;
		    i--;
		    break;
		  case 'Q':
		  case '#':
		  case '+':
		  case '$':
		  case 's':	/* two slashes */
		  case 't':	/* three slashes */
		  case '`':	/* comma on line */
		  case '<':
		  case '=':
		  case 'x':	/* small x */
		  case '\'':	/* comma above line */
		  case '^':	/* hat */
		  case '@':	/* asterix - star */
		  case '_':	/* semicircle underbar - smile */
		    ornament[i] = buf[i+skip];
		    skip++;
		    i--;
		    break;
		  case '*':
		    ornament[i] = '.'; /* was 'Z' */
		    skip++;
		    i--;
		    break;
		  case '%':
		    ornament[i] = ':';
		    skip++;
		    i--;
		    break;
		  case 'M':	/* music - time, note, sharp */
		  case '\t':
		    do_music(staff, buf, l_p, &skip, i, f); 
		    i=STAFF;
		    break;
		  case 'T':	/* text */
		    if (i != STAFF) j = i;
		    text_f ++;
		    while ( i < STAFF ) staff[i++] = ' ';
		    strncpy(i_text, &buf[++j + skip], sizeof(i_text));
		    break;
		  case 'R':	/* rest */
		    /*		    printf ("here i %d skip %d buf %s\n", i, skip, buf);   */
		    staff[1] = 'R';
		    if (buf[i + skip + 1] == '!') {
			staff[2] = '!';
			i=3;
		    }
		    else i=2;
		    if (buf[i+skip + 1] == '\t')
		      do_music(staff, buf, l_p, &skip, i+1, f); 
		    for(; i < STAFF; i++ ) staff[i] = ' ';
		    break;
		  case '\\':	/* finger number */
		    cc = buf[i + (++skip)]; /* we expect a number 1-4 */
		    if (cc >='1' && cc <='4' ) {
		        finger[i] = 140 + cc - '0';
			skip++;
			i--;
		    }
		    else {
			switch (cc) {
			  case  ':': /* infix .. */
			    a_ornament[i] = cc;
			    skip++;
			    i--;
			    break;
			  case '*':
			    finger[i] = '.';
			    skip++;
			    i--;
			    break;
			  case '?':
			    finger[i] = 250;
			    skip++;
			    i--;
			    break;
			  case '+':
			  case 'x':
			    finger[i] = cc;
			    skip++;
			    i--;
			    break;
			  case '\\':
			    staff[i] = cc;

			    break;
			  default:
			    i_error(1, "bad finger or \\ character", cc);
			    break;
			}
		    }
		    break;
		  default:
		    break;
		}
		/*  after ornaments */
		if ((cc = buf[i + skip + 1]) == '&' ) {
		    cc = buf[i + (skip += 2)];
		    if (cc == '*' ) cc = '.'; /* was Z */
		    if (cc == '%' ) cc = ':'; /* was Z */
		    a_ornament[i] = cc;
		}
	    }
	    for ( i=2; i< STAFF; i++) 
	      if (ornament[i] != ' ') {
		  ornament[0] = '+';
		  store (l_p, ornament, f);
		  break;
	      }
	    for ( i=2; i< STAFF; i++) 
	      if (finger[i] != ' ') {
		  finger[0] = '^';
		  store (l_p, finger, f);
		  break;
	      }
	    store(l_p, staff, f);

	    for ( i=2; i< STAFF; i++) 
	      if (a_ornament[i] != ' ') {
		  a_ornament[0] = '&';
		  store (l_p, a_ornament, f);
		  break;
	      }
 	    break;
	    /* repeat */
	    /* the next case is a duplicate of above, more of less */
	  case 'M':		/* music - time, note, sharp */
	    (void)do_music(staff, buf, l_p, &skip, 0, f);
	    break;

	  case '.':
	    staff[0] = '.';
	    staff[1] = '-'; staff[2] = staff[8] = ' ';
	    for (i=3; i < (	/* f->flags & FIVE ? 7 : */ 8); i++)
	      staff[i] = 'Z';
	    for (;i < STAFF; i++) 
	      staff[i] = ' ';
	    store(l_p, staff, f);
	    if ((c = buf[1]) == 'b' || c == '.') {
		incr(buf);
		goto start;
	    }
	    break;
	  case 'e':		/* don't do this on included files! */
	    /*	    printf("tab: getsystem: got an e\n"); */
	    if (!(f->flags & INCLUDE)) 
	      puti('e'); puti('\n');
	    return(END_OK);
	  case 'p':
	    printf("tab: getsystem: got a p\n");
	    puti('p'); puti('\n');
	    return(END_MORE);
	  case 'C':		/* a big C */
	    staff[0] = 'U';
	    staff[1] = '-';
	    for (i = 2 ;i < STAFF; i++) staff[i] = ' ';
	    store(l_p, staff, f);
	    break;
	  case 'c':		/* a big C with a line */
	    staff[0] = 'u';
	    for (i = 1 ;i < STAFF; i++) staff[i] = ' ';
	    store(l_p, staff, f);
	    break;
	  case '8':		/* 8va - octave */
	  case 'D':		/* da capo (D.C.)  */
	    staff[0] = c;
	    for (i = 1 ;i < STAFF; i++) staff[i] = ' ';
	    store(l_p, staff, f);	    
	    break;
	  case 'F':		/* a big 4 */
	    /*	    printf ("\n Please use S4 rather than F\n");
		    staff[0] = 'S';
		    for (i = 1 ;i < STAFF; i++) staff[i] = ' ';
		    store(l_p, staff, f);
		    */
	    i=0;
	    while (buf[i] != '\0') {
		puti(buf[i++]);
	    }
	    break;
	  case 'T':		/* a big 3 */
	    printf ("\n Please use S3 rather than T\n");
	  case 'Q':		/* a tail */
	  case 'q':		/* a tail */
	  case 'V':		/* a hold */
	  case 'i':		/* indent as for barline, no barline */
	  case 'f':		/* fine */
	    staff[0] = c;
	    for (i = 1 ;i < STAFF; i++) staff[i] = ' ';
	    store(l_p, staff, f);
	    break;
	  case '=':		/* W. = W */
	    staff[0] = c;
	    for (i = 1 ; buf[i] != '\n' && i < STAFF; i++) {
		staff[i] = buf[i];
	    }
	    for (;i< STAFF; i++) staff[i] = ' ';
	    store(l_p, staff, f);
	    break;
	  case 'k':		/* key signature */
	    staff[0] = c;
	    staff[1] = buf[1];	/* a, d, g, b, e (flat)  */
	    for (i = 2 ;i < STAFF; i++) staff[i] = ' ';
	    store(l_p, staff, f);
	    break;
	  case EOF:
	    return(0);
	  case ',':
	    printf ("Comma seperated lines not supported\n");
	    break;
	  default:
	    i_error(1, "getsystem: bad control character", c);
	    break;
	}
done:
	if (barline) {
	    barline = 0;
	    if (hushbar)  store (l_p, "b!         ", f);
	    else if (tie) {
		store (l_p, "bT         ", f);
		tie = 0;
	    }
	    else if (dimline) {
		store (l_p, "bQ         ", f);
		dimline = 0;
	    }
	    else store(l_p, "b-         ", f);
	    hushbar =0;
	}
	if (BARline) {
	    BARline = 0;
	    if (hushbar) store(l_p, "B!         ", f);
	    else store(l_p, "B-         ", f);
	    hushbar =0;

	}
	getl(buf, 100);
    } while (buf[0]);

/*    printf(" ->>>%s", buf); */

    return(4);
}

store(l_p, staff, f)
     int *l_p;
     char staff[];
     struct file_info *f;
{
    int i;

    if ( f->flags & CONVERT ) {
	int n_lines = 8;
    if ( f->flags & CON_SEV ) n_lines = 9;
	puti (staff[0]);
	puti (staff[1]);
	for (i = 2 ; i < n_lines; i++) {
		int j = n_lines + 1 - i;
		char c = staff[j];

		if ( c >= 'a' && c <= 'i') c = '0' + c - 'a';
		else if ( c == 'k' ) c = 'x';
		else if ( c == 'D' ) c = '3';
		else if ( c == 'F' ) c = '5';
	  	puti(c);
	}
	for ( ; i < STAFF; i++ ) 
	  puti (staff[i]);
	}
    else   /* not CONVERT */
      for  (i = 0 ; i < STAFF ; i++)
	puti(staff[i]);

    if (music[0]) {
	puti('M');
	for (i = 0; i < 3; i++) {
	    if (i == 0 && music[i] == 'B' ) puti('J');
	    else puti(music[i]);
	}
	if (music[3]) puti(music[3]);
	music[0] = music[3] = '\0';
    }
    if (text_f) {
	i = 0;
	text_f = 0;
/*	printf ("getsystem: i_text |%s|\n", i_text); */
	puti('T');
	while (i_text[i] != '\n') 
	  puti(i_text[i++]);
    }
    puti('\n');
    *l_p++; 
}

incr(buf)	/* strip off first byte of buf */
     char  buf[];
{
    int i;
    for ( i=0;; i++) {
	buf[i] = buf[i+1];
	if (buf[i] == '\0') return;
	if (buf[i] == '\n') {
	    buf[i+1] = '\0';
	    return;
	}
    }
}


i_error(stat, message, control)
     int stat;		/* 0 for exit, 1 for continue */
     char *message, control;
{
    extern int line_count;
	extern struct b input;

    printf ("tab: %s %c %3u at line %3d character position %d\n",
	     message, control, control, line_count, input.ib_p);
    if (stat) return;
    exit (-1);
}

usage()
{
    printf ("            -5 five lines rather than 6\n");
    printf ("            -6 600 dpi output\n");
    printf ("            -B Board style\n");
    printf ("            -C number every fifth measure\n");
    printf ("            -D dowland style\n");
    printf ("            -E prints \"Draft\" on each page\n");
    printf ("            -F modern style flags\n");
    printf ("            -G prints \"Copyright\" on each page\n");
    printf ("            -H convert 7 course french tab to italian)\n");
    printf ("            -I italian style flags, between lines\n");
    printf ("            -L LSA/Meyers style\n");
    printf ("            -M prints marks at every chord\n");
    printf ("            -N font patterned after Robinson\n");
    printf ("            -O Capirola flags, notes on lines\n");
    printf ("            -P PostScript output, if it is not the default\n");
    printf ("            -Q adjust spacing for Longa and breves\n");
    printf ("            -R[789] reduce size 70%% 80%% 90%%\n");
    printf ("            -S two systems on one line\n");
    printf ("            -T thin flags, between lines, italian notes\n");
    printf ("            -V shows version number\n");
    printf ("            -W transpose mensural output one whole tone\n");
    printf ("            -X don't expand for debugging\n");
    printf ("            -Y no word expansion for debugging\n");
    printf ("            -Z italian styleof some sort\n");
    printf ("            -a extended args as in -af for font specification\n");
    printf ("            -b uses baroque tuning in translation\n");
    printf ("            -c number first measure on each line\n");
    printf ("            -d debug\n");
    printf ("            -e rotate page to landscape\n");
    printf ("            -f french tab (default)\n");
    printf ("            -h convert french tab to italian)\n");
    printf ("            -i italian style flags, notes on lines\n");
    printf ("            -l \"xx in\" length of following lines\n");
    printf ("            -m translates to score\n");
    printf ("            -n font patterned after Mace\n");
    printf ("            -o name out output files - otherwise out.ps\n");
    printf ("            -p numbers pages\n");
    printf ("            -r uses renaissance tuning (default) \n");
    printf ("            -t print letters on the line\n");
    printf ("            -v prints intermediate format\n");
    printf ("            -w like Wallace book\n");
    printf ("            -x thin flags, on lines\n");
    printf ("            -y don't use italic font for text\n");
    printf ("            -z use italic font for text\n");
    printf ("tab: usage: tab [-o outfile] [option...] filename\n");
#ifdef POSTSCRIPT
    printf ("            -P PostScript Output\n");
#endif /* POSTSCRIPT */
    exit (-1);
}


do_music(staff, buf, l_p, skip, j, f)
     char staff[];		/* tab written already */
     char buf[];
     int *l_p;
     int *skip;
     int j;		/* is zero if there is no tablature */
     struct file_info *f;
{
    int dontstore=0;		/* if not 0 someone else does store  */
    int i = j;
    char buff[STAFF+1];

/*    printf ("tab: do_music: staff %sXX j %d skip %d\n", staff, j, *skip);
  */
    if ( j == 0 && buf[1] == '-')j = 1;
    if (j != 0 ) dontstore++;

    music[0] = buf[++j + *skip];
    music[1] = buf[++j + *skip];
    music[2] = buf[++j + *skip];
    if (music[0] == 'G') {
	music[1] = music[2] = ' ';
    }

    if (music[0] == '\n' ||
	music[1] == '\n' ||
	music[2] == '\n') {
	i_error(1, "tab with incomplete music specification ",
		music[0]);
    }
    if (staff[0] == 0)
      sprintf(buff, "%cM         ", music[0]);
    else if (staff[1] == 'R' )
      sprintf(buff, "%cR%c        ", staff[0], staff[2]); /* get the dot */
    else {
	for ( ; i< STAFF; i++) {
	    staff[i] = ' ';
	}
	strncpy ( buff, staff, STAFF);
	
    }
    switch ( buf[++j + *skip] ) {
      case 'T':
      case 't':
	j--;
	break;
      case 'r':
	music[3] = 'r';
	break;
      case '#':
	music[3] = '#';
	break;
      case 'x':
	music[3] = 'x';
	break;
      default:
	j--;
	break;
    }
    if ( buf[++j + *skip] != 'T' &&
	buf[j + *skip] != '\t' ) {
	if (!dontstore) store (l_p, buff, f);
	return;
    }
		
    text_f ++;			/* text */ 
    strncpy(i_text, &buf[++j + *skip], sizeof(i_text));
    if (!dontstore) store (l_p, buff, f);
    return;
}

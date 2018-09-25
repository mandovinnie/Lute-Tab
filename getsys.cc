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

#include "tab.h"
#include "file_in.h"
#include "i_buf.h"

int line = 0;
char music[9];
char i_text[256];		/* buffer for text */
int  text_f = 0;		/* flag - do we have text above */
int  text_s = 0;		/* flag - do we have text in system */
int  setflag(file_info *f, char * string, pass pass);
int is_music(char *text);

/* LOCAL */
static char cur_key = '\0';

void incr(char buf[]);
void do_music(i_buf *ib, unsigned char staff[], char buf[], int *l_p, int *skip,
     int j, struct file_info *f);
void Mstore(i_buf *ib, int *l_p, unsigned char staff[], struct file_info *f);
void args_from_string(char *buf, struct file_info *f);
int getsystem(file_in *fi, i_buf *ib, struct file_info *f,char buf[]);

int getsystem(file_in *fi, i_buf *ib, struct file_info *f,char buf[])
{ 
    int *l_p;
    char N[3];
    int barline=0,BARline=0,barnum=0;
    unsigned char staff[STAFF], ornament[STAFF]; 
    unsigned char finger[STAFF], a_ornament[STAFF], t_ornament[STAFF];
    signed char c;
    char cc, *p, *pp; 
    int i, j=0;
/*    signed char get(); */
    int gridflag=0;
    int hushbar=0, tie=0, dimline=0;/* for non print bars, and ties */
    int nocountbar=0;		// for non ocunting bar
    int Key=0;                       //for auto key signature
    int orig=0,Orig=0;
    int skip;
    int cur_chord=0;
    int cur_line=0;
    
    l_p = (int *)malloc(sizeof (int));
    *l_p = 0;
    
    
    music[0] = music[4] = 0;
    memset(music, 0, sizeof(music));
    
    do {
    start:			/* a return point for multible b */
	c = buf[0];
	//  	printf(" ->%sXXX", buf);  
	//      printf("cur_chord %d\n", cur_chord);
	cur_chord++;

	staff[0] = 0; skip = 0;
	for (i = 0; i < STAFF ; i++) a_ornament[i] = ' ';
	a_ornament[1] = '-';
	    
	switch (c) {
	case '?':
	    staff[0] = c;
	    for (i=1;i<STAFF;i++) staff[i] = ' ';
	    Mstore( ib ,l_p, staff, f);
	    break;
	case '%':
	    break;
	case '-':		// flags set in middle of system
	    p = buf;
	    pp = p+1;
	    //	    dbg1(Warning, "getsystem: flag %c\n", (void *)*pp );
	    switch (*pp) {
/*
	    case 'a':
	      if (!strncmp(&buf[1], "autoKey", 7)) {
		goto end;
	      }
	      break;
 */
	    case 't':
	      if (!strncmp(&buf[1], "twostaff", 8)) {
		goto end;
	      }
	      dbg1(Warning, "getsystem: flag -%c in middle of a system\n",
		   (void *) *pp );
	      break;
	    case 'e':
	    case 'b':
	    case 'E':
	    case 'h':
	    case 'H':
	    case 'j':
	    case '6':
	    case 'R':
	    case 'p':
	    case 'w':
	    case 'W':
	    case 'D':
	    end:
		args_from_string(buf, f);
		break;
	    default:
		do {		/* write args to  */
		    ib->PutByte(*p);	/* intermediate file */
		    p++;
		} while (*p != NEWLINE);
		ib->PutByte(NEWLINE);
		break;
	    }
	    break;
	case ' ':
	    incr(buf);
	    goto start;
	    /* new line */
	case NEWLINE:
	  /* memory leak here */
	    free (l_p);
	    return(1);
	case 'R':
	    staff[0] = buf[1];
	    if (staff[0] == '\n') {
	      staff[0] = '1'; 	// do something
	      if ( ! (f->m_flags & QUIET) )
		dbg2(Warning, "Getsystem: R with no number after it sys %d chord %d\n",
		 (void*) f->cur_system, 
		 (void *)cur_chord);

	    }
	    staff[1] = c;
	    for (i=2;i<STAFF;i++) staff[i] = ' ';
	    j = 2;
	    if (buf[j] == '.') {
		staff[2] = '.';
		j++;
	    }
	    if (buf[j] == '\t') {
		skip = j;
		do_music(ib, staff, buf, l_p, &skip, 0, f);
	    }
	    else
		Mstore( ib ,l_p, staff, f);
	    break;
	case 'd':
	    staff[0] = c;
	    staff[1] = buf[1];
	    if (staff[1] != 'b'){
		for (i=1;i<STAFF;i++) staff[i] = ' ';
	    }
	    else
		for (i=2;i<STAFF;i++) staff[i] = ' ';
	    Mstore( ib,l_p, staff, f);
	    break;
	case 'S':		/* a time signature */
	    staff[0] = 'G';
	    if (strchr(buf, '-')) { /* 10/8, etc */
		for (i=1; i<STAFF; i++) {
		    if (buf[i] == NEWLINE) break;
		    staff[i] = buf[i];
		}
		for (;i<STAFF; i++) {
		    staff[i] = ' ';
		}
	    }
	    else {
	  
		staff[1] = buf[1];
		if (staff[1] == 'C')
		  if (baroque) 
		    staff[1] = 19; /* a C signature */
		  else
		    staff[1] = 'U'; /* a C signature */
		else if (staff[1] == 'c') {
		  if (baroque)
		    staff[1] = 20;
		  else
		    staff[1] = 'u';
		}
		staff[2] = buf[2];
		if (staff[2] == NEWLINE || staff[2] == '\0') {
		    staff[2] = ' ';
		}
		for (i=3;i<STAFF;i++) staff[i] = ' ';
	    }
	    Mstore( ib,l_p, staff, f);
	    break;
	case 'A':		/* again - repeat */
	    staff[0] = c;
	    staff[1] = buf[1];	/* one or two */

	    //	    if ((cur_chord < 2) && cur_key && (f->m_flags & AUTOKEY)) {
	    //	      Key=1;
	    //	    } 

	    for (i=2;i<STAFF;i++) staff[i] = ' ';
	    Mstore( ib,l_p, staff, f);
	    break;
	case 'T':
	    if (buf[1] != '-') {
		if ( ! (f->m_flags & QUIET) )
			dbg0(Warning, "T in system - use S3 \n");
	    }
	    else {
	      text_s++;
	      strncpy(i_text, &buf[2], sizeof(i_text));
	    }
	    staff[0] = c;
	    staff[1] = '-';
	    for (i=2;i<STAFF;i++) staff[i] = ' ';
	    Mstore( ib,l_p, staff, f);
	    break;
	case 'P':
	    staff[0] = c;
	    {
		int i;
		for (i=1; i< STAFF; i++) {
		    staff[i] = buf[i];
		    if (staff[i] == NEWLINE ) {
			staff[i] = ' ';
			break;
		    }
		}
		for (;i<STAFF;i++) staff[i] = ' ';
	    }
	    Mstore( ib,l_p, staff, f);
	    return(2);
	case 'b':
	    /* barline */
	    if ((c = buf[1]) == '!') {
		hushbar++;
//		dbg1(Warning, "hushbar, barline not printed\n", (void *)c);
	    }
	    else if (c == 'T') tie++;
	    else if (c == 'Q') dimline++;
	    else if (c == 'v') orig=1;
	    else if (c == 'V') Orig=1;
	    else if (c == 'X') nocountbar=1;
	    else if (c == 'b' || c == '.') {
		Mstore( ib,l_p, (unsigned char *)"b-         ", f);
		incr(buf);
		goto start;
	    }
	    else if (c > '0' && c <= '9') {
		barnum = c;
	    }
	    else if ( c != NEWLINE)
		if (! (f->m_flags & QUIET))dbg3(Warning, 
		     "Text %c after a barline (b), system %d chord %d\n", 
		     (void *)((int)c) , (void *)f->cur_system, (void *)cur_chord);
	    barline++;

	    if ((cur_chord < 2) && cur_key && (f->m_flags & AUTOKEY)) {
	      Key=1;
	    } 

	    break;
	case 'B':
	    if (buf[1] == NEWLINE || buf[1] == '!') {
		BARline++;
		if ((c = buf[1]) == '!') hushbar++;
		break;
	    }
	    else if ( buf[1] == 'X' ) {
	      BARline++;
	      nocountbar=1;
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
	case 'O':
	  /*
	case '$': this is only used in Straloch MS
	*/
	case '@':  // a dot after a bar line after a flag from Cosens
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
		    Mstore( ib, l_p, staff, f);
		    goto done;
		case 'b':
		    for (i=2;i < STAFF;i++)  staff[i] = ' ';
		    Mstore( ib, l_p, staff, f);
		    goto done;
		case '0':
		case '-':
		case 'L':
		case 'B':
		case 'W':
		case 'w':
		case '1':
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
	    else if (staff[1] == 'B') {	/* dot before flag */
		staff[1] = 'B';
	    }
	    else if (staff[1] == '&') { /* flag ornament new Sept 2015 wbc*/
	      a_ornament[0] = '&';
              a_ornament[1] = buf[2];

	      //printf("Flag Ornament: %c\n", buf[2] ); // wbc Sept 2015
              //printf("buf before: %s", buf);
	      buf[1] = '-';
              for (i=3;i<STAFF;i++) buf[i-1] = buf[i];
	      //  printf("buf after: %s", buf);
	      //printf ("a_ornament at 393: %s\n", a_ornament);
              staff[1] = buf[1];
	    }
	    else {
		skip--;
		staff[1] = '-';
	    }

	rest:

	    //	    printf("a_ornament at 403: %s\n", a_ornament);
	    
	    for (i = 0; i < STAFF ; i++) {
		ornament[i] = ' ';
		//	a_ornament[i] = ' ';
		t_ornament[i] = ' ';
		finger[i] = ' ';
	    }
	    ornament[1] = '-';
	    // a_ornament[1] = '-';
	    t_ornament[1] = '-';
	    finger[1] = '-';
	    for (i = 2; i < STAFF; i++) {
		staff[i] = buf[i+skip];
		switch (staff[i]) {
		case 'd':
		  if ( f->m_flags & DSDOWN ) {
		    staff[i] = 'D';
		  }
		  else if ( ! (f->m_flags & DSUP )) {
		    if ( i == 2 ) { /* top line */
		      if (staff[0] != 'x') {
			staff[i] = 'D';
		      }
		    }
		    else if (staff[i-1] != ' ' && staff[i-1] != '-' ) {
		      staff[i] = 'D';
		    }
		  }
		  
		  if (baroque && buf[i+1+skip] == 'E') { 
		    staff[i] = 'z';
		    if (baroque && staff[i-1] == 'D') 
		      staff[i-1] = 'z';
		  }
		  if (baroque && staff[i-1] == 'E' && staff[i-2] == 'z') { 
		    staff[i] = 'z';
		  }
		  break;
		case 'g':
		  if (baroque 
		      && buf[i+1+skip] != '\n'
		      && buf[i+1+skip] != ' '
		      && buf[i+1+skip] != '.') {
		    staff[i] = 'G';
		  } 
		  break;
		case '0':	/* reversed 0 and space for internal lang */
		    break;
		case 'N':	/* number 10 -> 19 */
		  // seems to work from 00 to 30 plus 32 33
		  // but should only work from 10 to 30
		    skip++;
		    N[0] = buf[i+skip];
		    skip++;
		    N[1] = buf[i+skip];
		    N[2] = 0;
		    {
		      unsigned int fff, rrr;
		      fff = atoi(N);
		      rrr = fff;
		    }
		    staff[i] = atoi(N);
		    if ( ! isdigit (N[0])) 
			dbg1(Warning, "bad fret number", (void *)((int)N[0]));
		    if ( ! isdigit (N[1])) 
			dbg1(Warning, "bad fret number", (void *)((int)N[1]));
		    if (staff[i] > 30) {
			dbg1(Warning, "fret number %d higher than max 34\n", 
			     (void *)((int)staff[i]));
			staff[i]=0;
		    }
		    if (staff[i] < 10) {
			dbg1(Warning, "fret number %d lower than min 10\n", 
			     (void *)((int)staff[i]));
			staff[i]=0;
		    }
		    staff[i] += 220;
		    break;
		case 'z':
		    staff[i] = '0';
		    break;
		case ']':
		  staff[i] = ']';
		  if (buf[i+skip+1] == 'v') {
		    staff[i] = 133;
		    skip++;
		  }
		  if (buf[i+skip+1] == 'w') {
		    staff[i] = 134;
		    skip++;
		  }
		  break;
		case NEWLINE:
		    line++;
		    for ( ; i < STAFF; i++)
			staff[i] = ' ';
		    break;
		case '/':
		    if ((cc = buf[i + (++skip)]) == '/') { 
			ornament[i] = 's';     /* two slashes */
			if ((cc = buf[i + (++skip)]) == '/') { 
			    ornament[i] = 't'; /* three slashes */
			    skip++;
			}
			else {	/* only one slash */
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
		    if (staff[i] == '0') { // a octal number after !0
		      // which must also be fixed in map.cc
		      staff[i] = buf[i + skip + 3] - '0';
		      staff[i] += (buf[i + skip + 2] - '0') * 8;
		      staff[i] += (buf[i + skip + 1] - '0') * 64;
		      skip += 3;
		    }
		    break;
		case 0xe2:	// this is a three character apple
		  if (buf[i+1+skip] == (char)0x80) {              // double quote
		    //	    printf ("HERE %x %x %x \n", 
		    //	    staff[i], (buf[i+1+skip]& 0xff), (buf[i+2+skip]&0xff));
		    skip++;skip++;
		  }
		  else {
		    dbg3(Warning, "uncaught special character %x %x %x \n", 
			 (void*)(buf[i+skip]&0xff), (void*)(buf[i+1+skip]&0xff), 
			 (void*)(buf[i+2+skip]&0xff));
		    staff[i] = '"';
		    skip++;skip++;
		  }
		case 0xd2:
		case 0xd3:
//		  DON'T BREAK break; Continue on as if we had a real quote
		case '"':	/* prefix a non prefix char */
		    ornament[i] = buf[i + (++skip)];
		    if (ornament[i] == ']' ) {
		      if ( buf[i+skip+1] == 'v') {
			ornament[i] = 133;
			skip++;
		      }
		      if ( buf[i+skip+1] == 'w') { //wavy
			ornament[i] = 134;
			skip++;
		      }
		    }
		    else if (ornament[i] == '\n') {
		      dbg0 (Warning, "tab: getsys: double quote with no following character at end of line\n");
		      skip--;
//		      i = STAFF;
		    }
		    skip++;
		    i--;
		    break;
		case 'x':	/* small x */
		  /*
		  // printf ("small x here\n");
		  if ( f->num_flag == ITAL_NUM) {
		    // printf ("ital num and x here\n");
		    break;
		  }
		  */
		case 'Q':
		case '#':
		case '+':
		case '$':
		case '`':	/* comma on line */
		case ',':	/* comma on line */
		case '<':
		case '=':
		case '\'':	/* comma above line */
		case '^':	/* hat */
		case '@':	/* asterix - star */
		case '_':	/* semicircle underbar - smile */
		    ornament[i] = buf[i+skip];
		    skip++;
		    i--;
		    break;
		case 's':	/* two slashes */
		case 't':	/* three slashes */
		    if (i > 7 ) {
			ornament[i] = buf[i+skip];
			skip++;
			i--;
		    }
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
		    do_music(ib, staff, buf, l_p, &skip, i, f); 
		    i=STAFF;
		    break;
		case 'T':	/* text */
		    if (i != STAFF) j = i;
		    text_f ++;
		    while ( i < STAFF ) staff[i++] = ' ';
		    strncpy(i_text, &buf[++j + skip], sizeof(i_text));
		    break;
		case 'R':	/* rest */
		    staff[1] = 'R';
		    if (buf[i+skip + 1] == '\t')
			do_music(ib, staff, buf, l_p, &skip, i+1, f); 
		    if (buf[i + skip + 1] == '!') {
			staff[2] = '!';
			i=3;
		    }
		    else i=2;
		    for(; i < STAFF; i++ ) staff[i] = ' ';
		    break;
		case '|':
		  staff[i] = '|';
		  //		  if (baroque) {
		    if (buf[i+skip + 1] == '|') {
		      skip++;
		      staff[i] = 229;
		    }
		    //		  }
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
			case '.':
			case ':': /* infix .. */
			    t_ornament[i] = cc;
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
			    dbg3(Warning,
  "tab: getsystem: bad finger or \\ character %c at line %d of system %d\n", 
				 (void *)((int)cc), 
				 (void*)cur_line,
				 (void*)f->cur_system);
			    break;
			}
		    }
		    break;
		default:
		    break;
		}
		/*  after ornaments */
		if ((cc = buf[i + skip + 1]) == '&' ) {
		  // printf("getsys.cc:line 690 got &\n");
		  char ccc;
		  cc = buf[i + (skip += 2)];
		  if (cc == '*' ) cc = '.'; /* was Z */
		  if (cc == '%' ) cc = ':'; /* was Z */

		  if (cc == ']') {
		    ccc = buf[i + (skip += 1)];
		    
		    if ( ccc == 'v' ) 
		      cc = 133;
		    else if ( ccc == 'w' ) 
		      cc = 134;
		    else
		      skip -= 1;
		  }

		  if (cc == '\n') {
		    dbg2(Warning, 
       	 "tab: getsystem: & with no character after it line %d sys %d\n",
			 (void*)cur_line,
			 (void*)f->cur_system);
		    buf[i + (skip += 2)]= ' ';
		    buf[i + (skip += 3)]= '\n';
		    cc = ' ';
		  }
		  a_ornament[i] = cc;
		}
	    }
	    for ( i=2; i< STAFF; i++) 
		if (finger[i] != ' ') {
		    finger[0] = '^';
		    Mstore( ib, l_p, finger, f);
		    break;
		}
	    for ( i=2; i< STAFF; i++) 
		if (ornament[i] != ' ') {
		    ornament[0] = '+';
		    Mstore( ib, l_p, ornament, f);
		    break;
		}
	    for ( i=2; i< STAFF; i++) 
		if (t_ornament[i] != ' ') {
		    t_ornament[0] = ':';
		    Mstore( ib, l_p, t_ornament, f);
		    break;
		}
	    Mstore( ib,l_p, staff, f);

	    for ( i=1; i< STAFF; i++) // was i=2 wbc sept 2015
	      //printf ("a_ornament at 740: %2d %c %s\n", i, a_ornament[i], a_ornament);
	      if ((i == 1 && a_ornament[i] != '-' )
		  || a_ornament[i] != ' ') {
		  
		    a_ornament[0] = '&';
		    //printf ("a_ornament at 744: %d %c %s\n", i, a_ornament[i], a_ornament);
		    Mstore( ib, l_p, a_ornament, f);
		    break;
		}
 	    break;
	    /* repeat */
	    /* the next case is a duplicate of above, more of less */
	case 'M':		/* music - time, note, sharp */
	    /* here is where we skip a tab */
	    if (buf[1] == '\t') {
		skip++;
		
	    }
	    (void)do_music(ib, staff, buf, l_p, &skip, 0, f);
	    /* store here ? */
	    break;

	case '.':		/* this does not acutally determine */
	    staff[0] = '.';	/* the number of dots printed */
	    staff[1] = '-'; staff[2] = staff[8] = ' ';
	    for (i=3; i < 8; i++)
	      staff[i] = 'Z';
	    for (;i < STAFF; i++) 
		staff[i] = ' ';
	    Mstore( ib,l_p, staff, f);
	    if ((c = buf[1]) == 'b' || c == '.') {
		incr(buf);
		goto start;
	    }
	    break;
	case 'e':		/* don't do this on included files! */
	    if (!(f->flags & INCLUDE)) 
		ib->PutByte('e'); ib->PutByte(NEWLINE);
	    free (l_p); /* memory leak */
	    return(END_OK);
	case 'p':
	    ib->PutByte('p'); ib->PutByte(NEWLINE);
	    return(END_MORE);
	case 'C':		/* a big C */
	    staff[0] = 'U';
	    staff[1] = '-';
	    for (i = 2 ;i < STAFF; i++) staff[i] = ' ';
	    Mstore( ib,l_p, staff, f);
	    break;
	case 'c':		/* a big C with a line */
	    staff[0] = 'u';
	    for (i = 1 ;i < STAFF; i++) staff[i] = ' ';
	    Mstore( ib,l_p, staff, f);
	    break;
	case '8':		/* 8va - octave */
	case 'D':		/* da capo (D.C.)  */
	    staff[0] = c;
	    for (i = 1 ;i < STAFF; i++) staff[i] = ' ';
	    Mstore( ib,l_p, staff, f);	    
	    break;
	case 'F':		/* a big 4 */
	    i=0;
	    while (buf[i] != '\0') {
		ib->PutByte(buf[i++]);
	    }
	    break;
/*	  case 'T':	*/	/* a big 3 */
	case 'm':		/* Mesangeau's tail */
	case 'Q':		/* a tail */
	case 'q':		/* a tail */
	case 'V':		/* a hold */
	case 'i':		/* indent as for barline, no barline */
	case 'v':
	case 'j':		// small indent
	case 'f':		/* fine */
	    staff[0] = c;
	    for (i = 1 ;i < STAFF; i++) staff[i] = ' ';
	    Mstore( ib,l_p, staff, f);
	    break;
	case '=':		/* W. = W */
	    staff[0] = c;
	    for (i = 1 ; buf[i] != NEWLINE && i < STAFF; i++) {
		staff[i] = buf[i];
	    }
	    for (;i< STAFF; i++) staff[i] = ' ';
	    Mstore( ib,l_p, staff, f);
	    break;
	case 'k':		/* key signature */
	    staff[0] = c;
	    staff[1] = buf[1];	/* a, d, g, f, b, e (flat)  */
	    cur_key = buf[1];
	    //	    printf("cur_key %c\n", cur_key);
	    for (i = 2 ;i < STAFF; i++) staff[i] = ' ';
	    Mstore( ib, l_p, staff, f);
	    break;
	case '*':  // insert a letter here
	    staff[0] = c;
	    staff[1] = buf[1];
	    for (i = 2 ;i < STAFF; i++) staff[i] = ' ';
	    Mstore( ib, l_p, staff, f);
	    break;
	case EOF:
	    return(0);
	case '$':
	  {
	    int i, j=0;
	    char buffer[80];
	    char *bp;
	    bp = &buffer[0];
	    for (i=1; (c = buf[i]) != NEWLINE; i++) {
	      *bp=c; 
	      bp++;
	    }
	    
	    buffer[--i] = 0;
	    if (setflag(f, buffer, first))
	      break;		// return if we set the flag
	    ib->PutByte('$');
	    while (j < i && buffer[j] != '\0') {
	      ib->PutByte(buffer[j++]);
	    }
	    ib->PutByte(NEWLINE);
	  }  
	  break;
	case '[':
	  staff[0] = c;
	  //	  if ( ! (f->m_flags & QUIET) ) 
	  //	    dbg0(Warning, "text in system not implemented\n");
	  for (i = 1 ;i < STAFF; i++) { 
	    if (buf[i] == '\n' ) {
	      staff[i] = ' ';
	    }
	    else if (buf[i] == '\0' ) {
	      staff[i] = ' ';
	    }
	    else staff[i] = buf[i];
	  }
	  //	  staff[STAFF-1] = '\n';
	  Mstore( ib, l_p, staff, f);
	  break;
	default:	
	  if ( ! (f->m_flags & QUIET) ) 
	    dbg3(Warning, 
		 "getsystem: bad flag character %c in system %d chord %d\n", 
		 (void *)((int)c), 
		 (void*) f->cur_system, 
		 (void *)cur_chord);
	  break;
	}
    done:
	if (barline) {
	    barline = 0;
	    if (hushbar)  
	        Mstore( ib, l_p, (unsigned char *)"b!         ", f);
	    else if (tie) {
		Mstore( ib, l_p, (unsigned char *)"bT         ", f);
		tie = 0;
	    }
	    else if (dimline) {
		Mstore( ib, l_p, (unsigned char *)"bQ         ", f);
		dimline = 0;
	    }
	    else if (nocountbar) {
		Mstore( ib, l_p, (unsigned char *)"bX         ", f);
		nocountbar = 0;
	    }
	    else if (orig) {
		Mstore( ib, l_p, (unsigned char *)"bvabc      ", f);
		orig = 0;
	    }
	    else if (Orig) {
		Mstore( ib, l_p, (unsigned char *)"bV         ", f);
		Orig = 0;
	    }
	      
	    else if (barnum) {
		char line[13];
		strcpy (line, "b");
		line[1] = barnum;
		line[2] = '\0';
		strcat (line, "         ");
		Mstore ( ib, l_p, (unsigned char *)line, f);
		barnum = 0;
	    }
	    else Mstore( ib,l_p, (unsigned char *)"b-         ", f);
	    hushbar =0;
	}
	if (BARline) {
	    BARline = 0;
	    if (hushbar) 
	        Mstore( ib, l_p, (unsigned char *)"B!         ", f);
	    else if (nocountbar)
	      Mstore( ib, l_p, (unsigned char *)"BX         ", f);
	    else 
	        Mstore( ib ,l_p, (unsigned char *)"B-         ", f);
	    hushbar =0;
	    nocountbar=0;

	}
	if (Key) {
	  //	  printf("getsys: cur_key is %c\n", cur_key);
	        char b[13];
		music[0] = 'G';
		music[1] = ' ';
		music[2] = ' ';
		if (f->m_flags & TWOSTAFF ) {
		  music[3] = '\0';
		  music[4] = 'G';
		  music[5] = ' ';
		  music[6] = ' ';
		}
	        Mstore( ib ,l_p, (unsigned char *)"GM         ", f);
		strcpy (b, "k");
		strcat (b, &cur_key);
		strcat (b, "         ");
	        Mstore( ib ,l_p, (unsigned char *)b, f);
		Key=0;
	}
	*fi->GetLine(buf, 100);
	cur_line++;
    } while (buf[0]);

    return(4);
}

void
Mstore(i_buf *ib, int *l_p, unsigned char *staff, struct file_info *f)
{
  int i;

  if ( f->flags & CONVERT 
       && staff[1] != 'R' 
       && staff[0] != '='
       && strchr( "LWw012345xYy", staff[0]) ) {
    int n_lines = 8;
    if ( f->flags & CON_SEV ) 
      n_lines = 9;
    if (f->flags & FIVE) 
      n_lines = 7;
    if (f->flags & FOUR) 
      n_lines = 6;
    ib->PutByte (staff[0]);
    ib->PutByte (staff[1]);
#ifndef __linux__
    //      staff[1] = '-';		// to not screw up conversions..
#endif /* linux */
    i = 2;
    if (f->flags & CONV_COR) i = 1;
    // if (f->m_flags & SPANISH) printf ("SPANISH\n");

    for (; i < n_lines; i++) {
      int j = n_lines + 1 - i;
      unsigned char c = staff[j];

      if ( tolower(c) >= 'a' && tolower(c) <= 'i') c = '0' + tolower(c) - 'a';
      else if ( c == '.' && staff[j-1] != ' ' && staff[j-1] != '-') {
	staff[j-2] = c;
	c = ' ';	/* don't print the dots */
      }
      else if ( c == ' ' && staff[j-1] != ' ' && staff[j-2] == '^') {
	c = '^';staff[j-2] = ' ';
      }
      else if ( c == 'k' ) c = '9';
      else if ( c == 'l' ) c = 'x';
      else if ( c == 'm' ) c = 231; /* wbc sept 07 fix for high notes */
                                    /* should print XI  N10 */
      else if ( c == 'n' ) c = 232;
      else if ( c == 'x' ) c = 'l';
      else if ( c == 230 ) c = 'l'; // N10
      else if ( c == 231 ) c = 'm';
      else if ( c == 232 ) c = 'n'; // N12
      else if ( c == 233 ) c = 'o'; // N13
      else if ( c == 234 ) c = 'p'; // N14
      // wbc sep 2018       else if ( c >= '0' && c <= '9' && (0 && (!(f->m_flags & SPANISH)))) {
      else if ( c >= '0' && c <= '9' ) {
	if (i == 8 && f->line_flag == ON_LINE 
	    && f->flags & CONV_COR ) {
	  if ( c < '7' ) c = 'a' + c - '0';
	  if ( c == '7' ) c = 'a';
	}
	else {
	  c = 'a' + c - '0';
	  if (c == 'd' && (  staff[j+1] != ' '))
	    c = 'D';
	  if ( c == 'j' ) c = 'k';
	}
      }
      else if ( i == 3 && staff[n_lines] == '.' ) {
	c = '.';
	staff[n_lines] = ' ';
      }
      ib->PutByte(c);
    }
    for ( ; i < STAFF; i++ ) 
      ib->PutByte (staff[i]);
  }
  else if ( f->flags & CONVERT && staff[0] == '+' ) {
    int n_lines = 8;
    if ( f->flags & CON_SEV ) n_lines = 9;
    ib->PutByte (staff[0]);
    ib->PutByte (staff[1]);
    i = 2;
    if (f->flags & CONV_COR) i = 1;
    for (; i < n_lines; i++) {
      int j = n_lines + 1 - i;
      char c = staff[j];

      ib->PutByte(c);
    }
    for ( ; i < STAFF; i++ ) 
      ib->PutByte (staff[i]);
  }

  else   /* not CONVERT */
    for  (i = 0 ; i < STAFF ; i++) {
      if ( staff[i] == 209 ) { // mac special double dash
	staff[i]  = '-'; 
	if ( i < (STAFF-1)) 
	  staff[i+1]  = '-'; 
      }
      ib->PutByte(staff[i]);
    }
    
  if (staff[0] != '^' && staff[0] != '+' && music[0]) {
    ib->PutByte('M');
    for (i = 0; i < 3; i++) {
      if (i == 0 && music[i] == 'B' ) ib->PutByte('J');
      else ib->PutByte(music[i]);
    }
    if (music[3]) ib->PutByte(music[3]);
    music[0] = music[3] = '\0';
  }
  if (music[4]) {
    ib->PutByte('M');
    for (i = 4; i < 7; i++) {
      if (i == 4 && music[i] == 'B' ) ib->PutByte('J');
      else ib->PutByte(music[i]);
    }
    if (music[7]) ib->PutByte(music[7]);
    music[4] = music[7] = '\0';
  }
  if ((staff[0] != '^' && staff[0] != '+') && (text_f || text_s)) {
    // if ((text_f || text_s)) {
    i = 0;
    text_f = text_s = 0;
    ib->PutByte('T');
    while (i_text[i] != NEWLINE) 
      ib->PutByte(i_text[i++]);
  }
  ib->PutByte(NEWLINE);
  *l_p++; 
}

void
incr(char buf[])	/* strip off first byte of buf */
{
    int i;
    for ( i=0;; i++) {
	buf[i] = buf[i+1];
	if (buf[i] == '\0') return;
	if (buf[i] == NEWLINE) {
	    buf[i+1] = '\0';
	    return;
	}
    }
}

void
do_music(i_buf *ib, unsigned char staff[], char buf[], int *l_p, int *skip,
     int j,		/* is zero if there is no tablature */
     struct file_info *f)
{
    int dontstore=0;		/* if not 0 someone else does store  */
    int i = j;
    int b;
    char buff[STAFF+5];
    //    printf ("do_music: buf %s\n", buf);

    if ( j == 0 && buf[1] == '-')j = 1;
    if (j != 0 ) dontstore++;

    if (music[0] == 0 ) b=0;
    else b=4;

    music[b] = buf[++j + *skip];
    music[b+1] = buf[++j + *skip];
    music[b+2] = buf[++j + *skip];
    /*
        printf ("do music  XXXXX %X\n", f->m_flags);
        printf ("do music twostaff %x\n", TWOSTAFF);
        printf ("do music autokey  %x\n", AUTOKEY);
    */

    if (music[b] == 'G' || music[b] == 'F') {
      strcpy (buff, "GM");
      if (music[b+1] == '8') {
	strcat (buff, "8");
	if (music[b+2] == 'a')
	  strcat (buff, "a");
	if (music[b+2] == 'b')
	  strcat (buff, "b");
      }
      else
	music[b+1] = music[b+2] = ' ';
      for (i=2 ; i< STAFF; i++) buff[i] = ' '; // print g cleff on second line
      if (f->m_flags & TWOSTAFF) {
	music[4] = music[b];
	music[5] = music[6] = ' ';
	strcat ( buff, "MG");
      }
    }
    
    if (music[b] == NEWLINE ||
	music[b+1] == NEWLINE ||
	music[b+2] == NEWLINE ) {
	dbg4(Warning, 
	     "tab with incomplete music specification %c%c%c line %d\n",
	     (void *)(int)music[b], 
	     (void *)(int)music[b+1], 
	     (void *)(int)music[b+2], 
	     (void *)*l_p);
    }

    if (staff[0] == 0)
#ifdef MAC
      {
	  buff[0] = music[b];
	  buff[1] = 'M';
	  for (i=2; i < STAFF; i++) buff[i] = ' ';
      }
#else
      sprintf(buff, "%cM         ", music[b]);
#endif
    else if (staff[1] == 'R' )
#ifdef MAC
      {
	  buff[0] = staff[0];
	  buff[1] = 'R';
	  buff[2] = staff[2];
	  for (i=3; i < STAFF; i++) buff[i] = ' ';
      }
#else
    sprintf(buff, "%cR%c        ", staff[0], staff[2]); /* get the dot */
#endif
    else {
	for ( ; i< STAFF; i++) staff[i] = ' ';
	
	strncpy ( buff, (char *)staff, STAFF);
	
    }
    switch ( buf[++j + *skip] ) {
      case 'T':
	j--;
	break;
      case 't':
	j--;  /* wbc March 2000 for T as a text sign */
	break;
      case 'r':
	music[b+3] = 'r';
	break;
      case '#':
	music[b+3] = '#';
	break;
      case 'x':
	music[b+3] = 'x';
	break;
      case '-':
	music[b+3] = '-';
	break;
      case '=':
	music[b+3] = '=';
	break;
      default:
	j--;
	break;
    }
    //    printf("do music - music is %c%c%c%c\n", 
    //    music[b], music[b+1], music[b+2], music[b+3]);
    if ( buf[++j + *skip] == '\t' ){ // test for more music first
	if (is_music(&buf[j + *skip + 1])) {
	    (void)do_music(ib, staff, buf, l_p, skip, j, f);
	    if (!dontstore) 
		Mstore( ib, l_p, (unsigned char *)buff, f);
	    return;
	}
	else {
	  text_f ++;			/* text */ 
	  strncpy(i_text, &buf[++j + *skip], sizeof(i_text));
	  if (!dontstore) 
	    Mstore( ib, l_p, (unsigned char *)buff, f);
	  return;
	}
    }
    else if ( buf[j + *skip] == 'T') {  /* test for text */
      text_f ++;			/* text */ 
      strncpy(i_text, &buf[++j + *skip], sizeof(i_text));
      if (!dontstore) 
	Mstore( ib, l_p, (unsigned char *)buff, f);
      return;
    }
    
    else {
	if (!dontstore) 
	    Mstore( ib, l_p, (unsigned char *)buff, f);
	return;
    }
}



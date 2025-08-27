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
#include "win.h"
#include "tab.h"
#include "print.h"
#include "tfm.h"

/* weights */

#define 	W_NONE  0.0
#define         W_TINY  0.000001
#define		W_QUART 0.15  
#define		W_HALF  0.3    
#define		W_ONE   1.0     
#define		W_TWO   2.0      
#define		W_THREE 3.0       
#define		W_FOUR  4.0        

/* weights of each flag when filling out line */

char min_0_w[] = "0.2000 in    ";
char min_1_w[] = "0.1400 in    ";	/* one flag */
char min_2_w[] = "0.1200 in    ";
char min_3_w[] = "0.1000 in    ";
char min_4_w[] = "0.0900 in    ";	/* four flags */
char min_b_w[] = "0.0300 in    ";	/* bar */
char min_d_w[] = "0.0600 in    ";	/* repeat dot */
char min_O_w[] = "0.0900 in    ";	/* ornament was 0.09 */
char min_F_w[] = "0.0700 in    ";	/* a fingering number */

/* struct words { char* width; double weight; } */

struct w chart[10] = {
    {(char*)"0.26   in", 6.0},		/* B (J) */
    {(char*)"0.23   in", 5.0},		/* W */
    {(char*)"0.23   in", 5.0},		/* w */
    {min_0_w, W_FOUR},		/* 0 */
    {min_1_w, W_THREE},		/* 1 */
    {min_2_w, W_TWO},		/* 2 */
    {min_3_w, W_ONE},		/* 3 */
    {min_4_w, W_ONE},		/* 4 */
    {min_4_w, W_ONE},		/* 5 */
    {min_4_w, W_ONE}		/* 6 */
};
struct w big_chart[10] = {
    {min_0_w, 6.0},		/* B (J) */
    {min_1_w, 4.0},		/* W */
    {min_2_w, W_TWO},		/* w */
    {min_3_w, W_ONE /* - 0.07 */ },	/* 0 */ /* wbc Sept 22 2022 */
    {min_4_w, W_HALF + 0.35 },		/* 1 */ /* these sizes should be bigger than the character width */
    {min_4_w, W_QUART },		/* 2 */
    {min_4_w, W_QUART },		/* 3 */
    {min_4_w, W_QUART },		/* 4 */
    {(char*)"0.05    in", W_QUART},	/* 5 */
    {(char*)"0.05    in", W_QUART}	/* 6 */
};

char vals[] = "JWw0123456";

static int first_1=0 /* , first_2=0 */;
double b_d_pad = 0.1;		// just for dots before wrapped D
int bdot(struct list* l);
extern int breaksys;		// in notes.cc, for ties across lines
extern int breaksys2;		// in notes.cc, for ties across lines
extern double tlength;		// the length of a tie

/* LOCAL */
double text_check(struct list *l, int line, double *total_weight,
		  int in_sys, int staff);
void find_val(char val, struct list *ll, int *found,
     double *total_width, double *total_weight, int offset,
     struct w *wp, char dot);
void pass1(font_list *f_a[], int *l_p, struct file_info *f, double *extra);


void pass1(font_list *f_a[], int *l_p, struct file_info *f, double *extra)
{
  int i, j;
  static int b_init=0;
  char c;
  static double old_width, old_weight;
  static char old_flag;
  static struct list *l;
  struct list *ll;		/* for second pass */
  char *d, cc;
  double weight=0.0;
  double total_width=0.0, total_weight=0.0;
  unsigned char dd;
  struct w *wp;
  extern int n_measures;
  int measures=n_measures;
  extern struct list *listh;
  extern char thick_bar[];
  extern double staff_len;

  if (1/* first_1 == 0 */ ) {
    l = listh;
    first_1++;
  }
  else if (l && l->next) l = l->next;
  else return;
  ll = l;

  if (f->flags & BIGNOTES) wp = big_chart;
  else wp = chart;

  if (baroque && ! b_init) {
    double base_sp = f_a[0]->fnt->get_width (/* 2 + */ 1+ 194); /* three flags */
    double rt;
    base_sp *= .83;
    base_sp = 0.02;
    rt = 1.28;
    b_init = 1;

    /* for Baroque tab */
    /* this overrides the defaults assigned in the global inititalization above */
    /* for one or more flages the width is based on the width of the flag character */
    /* for no flage, half note, or whole note it is a guess */
    {
      char * foo;
      int bf_num[8] = {int('B'),(int)'W',(int)'w',194,195,196,197,198};
      int i;

      //  foo = (char *) malloc(45);
      // chart[2].width = foo;
      //
      // wbc jan 2025 made corrections to half and quarter note
      //
      //      wp[1].width = (char *)malloc(45);    // wp1 is whole note
      // snprintf(wp[1].width, 1+sizeof(wp[1].width), "%.3f in", /*1.2*/ 5.1 * rt * rt * rt * base_sp);
      wp[2].width = (char *)malloc(45);    // wp2 is half note
      snprintf(wp[2].width, 1+sizeof(wp[2].width), "%.3f in", /*1.2*/ 7.9 * rt * rt * rt * base_sp);
      wp[3].width = (char *)malloc(45);    // wp3 is quarter note no flags
      snprintf(wp[3].width, 1+sizeof(wp[3].width), "%.3f in", 3 * (f_a[0]->fnt->get_width(bf_num[3])));/* 0 flags */
      wp[4].width = (char *)malloc(45);    // wp4 is one flag
      snprintf(wp[4].width, 1+sizeof(wp[4].width), "%.3f in", (f_a[0]->fnt->get_width(bf_num[4])));/* 1 flags */
      wp[5].width = (char *)malloc(45);    // wp5 is two flags
      snprintf(wp[5].width, 1+sizeof(wp[5].width), "%.3f in", f_a[0]->fnt->get_width(bf_num[5]));  /* 2 flags */
      wp[6].width = (char *)malloc(45);    // wp6 is three flags
      snprintf(wp[6].width, 1+sizeof(wp[6].width), "%.3f in", f_a[0]->fnt->get_width(bf_num[6]));  /* 3 flags */
    
      if (0) {
	printf("pass1: init font widths \n");
	for (i=0; i< 8; i++) {
	  printf("i %d width %9s weight %f  font 0 width %f\n",
		 i, chart[i].width,  chart[i].weight,
		 f_a[0]->fnt->get_width(bf_num[i]));
	}
	printf("\n");
      }
    }
  }  /* end baroque && ! b_init */
  
  /* figure out basic widths on the first pass through */

  for (j=0; j < (*l_p) && l; j++){	/* get widths */

    if ( l == 0 ){
      dbg0(Warning, "tab: pass1: nil pointer l\n");
    }

    d = l->dat;

    /*	printf ("pass1 dat %d ->%s", j, l->dat);   */
    /*	printf ("old weight %f width %f\n", old_width, old_weight); */

    if (f->flags & VERBOSE)  {
      dbg2( Inter, "tab: pass1: %d l:%3d: ",
	    (void *)n_system, (void *)(j+1));
      for (i=0; i<STAFF; i++) {
	dd = d[i];
	if (dd >= 'a' && dd <= 'p')
	  dbg1 ( Inter, "  %1c", (void *)(int)dd);
	else if (dd == 'D'|| dd == 'Q' || dd == 'E' || dd == 'R')
	  dbg1 ( Inter, "  %1c", (void *)((int)dd));
	else if (dd == 'G')
	  dbg0 ( Inter, "  G");
	else if (dd == 'U')
	  dbg0 ( Inter, "  U");
	else if (dd == 'L')
	  dbg0 ( Inter, "  L");
	else if (dd == 'J')
	  dbg0 ( Inter, "  J");
	else if (dd == 'W')
	  dbg0 ( Inter, "  W");
	else if (dd == 'w')
	  dbg0 ( Inter, "  w");
	else if (dd == 'x')
	  dbg0 ( Inter, "  x");
	else if (dd == 'X')
	  dbg0 ( Inter, "  X");
	else if (dd == 'Y')
	  dbg0 ( Inter, "  Y");
	else if (dd == 'Z')
	  dbg0 ( Inter, "  Z");
	else if (dd == '^')
	  dbg0 ( Inter, "  ^");
	else if (dd == 'r')
	  dbg0 ( Inter, "  r");
	else if (dd == 's')
	  dbg0 ( Inter, " //");
	else if (dd == 'T')
	  dbg0 ( Inter, "  T");
	else if (dd == 'V')
	  dbg0 ( Inter, "  V");
	else if (dd == 'v')
	  dbg0 ( Inter, "  v");
	else if (dd == 222)
	  dbg0 ( Inter, " +*");
	else if (dd == 238)
	  dbg0 ( Inter, "238");
	else if (dd == 239)
	  dbg0 ( Inter, "239");
	else if (dd == 254)
	  dbg0 ( Inter, "254");
	else if (dd == 255)
	  dbg0 ( Inter, "255");
	else if (dd == 't')
	  dbg0 ( Inter, "///");
	else if (dd == '|')
	  dbg0 ( Inter, "  |");
	else if (dd == ' ')
	  dbg1 ( Inter, "  %1c", (void *)'X');
	else if ( dd == '\n' )
	  goto eend;
	else if (dd >= 040 && dd <= 0100 )
	  dbg1 ( Inter, "  %1c", (void *)((int)dd));
	else if (dd >= 0202 && dd <= 0213)
	  dbg1 ( Inter, "  %1c", (void *)(dd - 0202));
	else if (dd >= 0214 && dd <= 0225)
	  dbg1 ( Inter, "  %1c", (void *)(dd - 0214));
	else if (dd >= 0226 && dd <= 0237)
	  dbg1 ( Inter, "  %1c", (void *)(dd - 0226));
	else if ((dd == 70 || dd == 120) && i == 0)
	  dbg1 ( Inter, "  %1c", (void *)((int)dd));
	else if (i == 1 && d[0] == '*')
	  dbg1 ( Inter, "  %1c", (void *)((int)dd));
	else
	  dbg1 ( Inter, "%03d",  (void *)((int)dd));
      }
    eend:
      dbg0 ( Inter, "\n");
    }

    switch (c=d[0]) {
//      int ii;
    case '?':
      l->padding = 3.0;
      weight = 1.0;
      break;
    case '+':		/* ornaments */
      l->padding = str_to_inch(min_O_w);  /* 0.09 */
      //printf("pass1.cc: +ornament padding %f  d %s\n", l->padding, d);
      //printf("pass1.cc: width +  %f \n", f_a[0]->fnt->get_width('+'));
      //printf("pass1.cc: width -  %f \n", f_a[0]->fnt->get_width('-'));
      //printf("pass1.cc: width _  %f \n", f_a[0]->fnt->get_width('_'));
      //printf("pass1.cc: width %f \n", f_a[0]->fnt->get_width('+'));
      if (strchr("Yy", l->next->dat[0]))
	l->padding = 0.01 /* str_to_inch(min_d_w) */;
      else if (strchr("{}[]()UX", c))
	l->padding = 0.01;
      else if ((unsigned char)d[2] == 239    /* stroke strokex */
           || (unsigned char)d[3] == 239
           || (unsigned char)d[4] == 239
           || (unsigned char)d[2] == 243 
           || (unsigned char)d[3] == 243 
           || (unsigned char)d[4] == 243 
           || (unsigned char)d[5] == 243) 
	l->padding = 0.05;
      else if (strchr("x", l->dat[2])||strchr("x", l->dat[3])
	       ||strchr("x", l->dat[4])||strchr("x", l->dat[5]))
	l->padding += 0.04;
      else {
	if ( l->prev && l->prev->padding > 0.04 ) {
	  l->prev->padding -= 0.04;
	  total_width -= 0.04;
	}
	else {
	  /* l->prev->padding = 0.01; */
	  /* total_width = 0.01; */
	  //			printf ("tab: pass1: negative padding\n");
	  //			printf ("tab: pass1: %s\n", l->prev->dat);
	  //			printf ("tab: pass1: %s\n", l->dat);
	}
      }

      // printf("pass1.cc: + inter padding   %f\n", l->padding);

      // this is about ornaments before the notes

      {
	int Q_found=0;
	int orn_found=0;
	int found_wide_letter = 0;
	int underscore_found = 0;
	for (i = 2; i < 8 /* STAFF*/; i++ ) {  // wbc jan 2025 was for i=2  STAFF = 11;
	  if (d[i] == 'Q')
	    Q_found=1;
	  if (d[i] != ' ')
	    orn_found++;
	  if ( 0 && l->next->dat[i] == 'D') {
	    found_wide_letter = l->next->dat[i];
	  }
	  if ( l->next->dat[i] == 'E') { 
	    if ( ! f->char_flag && ROB_CHAR ) {  // wbc jan 2025 
	      found_wide_letter = l->next->dat[i];
	    }
	  }
	  //found_wide_letter = 0;
	  if (d[i] != ' ' && d[i] != 'Q') {
	    if ( d[i] == 'x' ) {
	      if (found_wide_letter) {
		if ( f->char_flag & ROB_CHAR ) {  // wbc jan 2025
		  printf("pass1.c: adding to Robinson padding 1 \n");
		  l->padding += 0.01;;
		}
		else {
		  printf("pass1.c: adding to padding 2 \n");
		  l->padding += 0.019;
		}
		break;
	      }
	    }
	    else {
	      if (found_wide_letter) {
		l->padding += b_d_pad /* b_d_pad is 0.1 */; /* 0.18 see above */
		// printf("adding to padding 2 padding %f  b_d_pad %f\n", l->padding, b_d_pad);
	      }
	      break;
	    }
	    if (d[i] == '_') {
	      underscore_found = 1;
	      //printf("underscore %d\n", i);
	    }
	    if (d[i] == '/') {
	      //	printf("slash %d\n", i);
	    }
	    // printf("found_wide_letter %d  b_d_pad:  %f\n", found_wide_letter, b_d_pad);
	    // found_wide_letter = 1;
	    // break;
	  }
	}  // end for loop
	if (d[8] == '/' || d[8] == 's'|| d[8] == 't') {  // wbc jan 2025 bass bourdons closer together
	  //XSprintf ("pass1.cc: ornament loop %s num orn %d\n", d, orn_found);
	  if (orn_found == 0 && l->prev && ((l->prev->dat[0] != 'b')&&(l->prev->dat[0] != 'B')&&(l->prev->dat[0] != '.'))) {
	    if (l->prev) l->prev->padding -= 0.05;
	    total_width -= 0.05;
	  }
	}

	// printf("found_wide_letter %d  b_d_pad: %f padding %f underscore_found %d orn_found %d\n",
	//       found_wide_letter, b_d_pad, l->padding, underscore_found, orn_found );
	
	// if ( underscore_found )  l->padding = 0.05;
        if (!found_wide_letter && underscore_found && orn_found < 2 )  l->padding = 0.0;
	if (!orn_found && Q_found) l->padding = 0.0;
	//if (underscore_found && d[9] == '/'  && orn_found < 3)  l->padding = 0.0;
	// printf(" l->padding %f %c\n", l->padding, d[10]); 
      }
//    DONE:
      weight = W_NONE;
      if (f->m_flags & NOSPACEBEFORE ) {
	l->padding = 0;
	l->padding = 0.10;
	l->prev->padding -= 0.10;
      }
      //printf("pass1: + final padding %f\n", l->padding);
      break;
    case ':':		/* ornaments above the note */
      l->padding = 0;
      weight = W_NONE;
      break;
    case '&':		/* ornaments after the note*/
      l->padding = 0;
      {
	for (i = 2; i < STAFF; i++ ) {
	  if (d[i] != ' ' && d[i] != '{' && d[i] != '}'
	      && d[i] != '(' && d[i] != ')' 
              && (!(f->m_flags & NOSPACE )) ) {
	    l->padding = str_to_inch(min_O_w);

	    //	if ( 1 && l->prev && l->prev->dat[i] == 'e') {
	    //  printf ("HERE  ornament is %c\n", d[i]);
	    //  l->prev->padding += 0.5;
	    //   total_width += 0.5;
	    //  }
	  }
	}
      }
      weight = W_NONE;
      break;
    case '^':		/* fingerings */
      l->padding = str_to_inch(min_F_w);
      weight = W_NONE;
      {
	for (i = 2; i < STAFF; i++ ) {
	  if (d[i] != ' ' ) {
	    if (l->next->dat[i] == 'D') {
	      if (baroque) l->padding += 0.107;
	      else l->padding += 0.08;
	      break;
	    }
	    else if ( l->dat[0] == '^'
		      && f->char_flag == ROB_CHAR
		      && l->next->dat[i] == 'd' ) {
	      l->padding += 0.05;
	      break;
	    }
	    else if  (baroque && l->next->dat[0] == '+' &&
		      l->next->next->dat[i] == 'd') {
	      l->padding += 0.08;
	    }
	    else if (l->next->dat[i] == 'E') {
	      /*   l->padding -= 0.0204;*/
	      l->padding += 0.1;
	      break;
	    }
	    if (l->next->dat[0] == '+' && l->next->next ) {
	      if (l->next->next->dat[i] == 'E' ||
		  l->next->next->dat[i] == 'D') {
		l->padding += 0.05;
	      }
	    }
	    else
	      if (i > 2 &&
		  ((l->next->dat[i-1] == 'g') ||
		  (l->next->dat[i-1] == 'G'))) {
		l->padding += 0.02;
	      }
	  }
	}
      }

      break;
    case 'i':     /* insert a space */
      weight = W_NONE;
      if (l->next && strchr("G", l->next->dat[0]) && strchr("0", l->next->dat[1]))
	l->padding = 0;
      else
	l->padding = str_to_inch(min_d_w);
      goto rest;
    case 'j':     /* insert a space */
      weight = W_NONE;
      l->padding = 0.2 * str_to_inch(min_d_w);
      goto rest;
    case 'B':  // I think this gets overridden below
      weight = W_NONE;
      l->padding = str_to_inch(thick_bar);
      goto rest;
    case '.':
      l->padding = 0.0;
      if (l->next &&
	  (l->next->dat[0] == '.' || l->next->dat[0] == 'b'
	   || l->next->dat[1] == 'B' )) { 
	l->padding = str_to_inch(min_d_w);
	goto rest;
      }
      else if (l->next && l->next->dat[0] && strchr("Yy", l->next->dat[0])) {
	l->padding = f_a[0]->fnt->get_width (l->next->dat[0])/-2.0;
	goto rest;
      }
      //	    if ( j == *l_p - 1)
      //			printf("dot at end\n");
    case 'b':  // I think this gets overridden below
      l->padding = str_to_inch(min_d_w);  // wbc jan 2025
      // for measure numbers conflicting with bourdons
      // only works for measure numbers at start of line
      if (((bar_count && j == 0) || barCCount ) && l->next) {
	if (l->next->dat[8] == 'a') {
	  // printf("pass1.cc: next dat 8 %c\n", l->next->dat[8] );
	  l->padding = 1.3 * str_to_inch(min_d_w);}
	else if (l->next->dat[8] == 'r') {
	  // printf("pass1.cc: next dat 8 %c\n", l->next->dat[8] );
	  l->padding = 2.3 * str_to_inch(min_d_w);}
	else if (l->next->dat[8] == 's') {
	  // printf("pass1.cc: next dat 8 %c\n", l->next->dat[8] );
	  l->padding = 2.3 * str_to_inch(min_d_w);}
	else if (l->next->dat[8] == 't') {
	  // printf("pass1.cc: next dat 8 %c\n", l->next->dat[8] );
	  l->padding = 2.8 * str_to_inch(min_d_w);}
      }


      //      printf("pass1.cc: next dat 8 %c\n", l->next->dat[8] );
      
    rest:
      if ( j == *l_p - 1) {
	l->padding = 0.0;  // wbc jan 2025  
	weight = W_NONE;
      }
      else if ((j == *l_p - 2) && l->next && (l->next->dat[0] == '*')) {
	/* wbc jan 2025 for a letter * at the very end */
	l->padding = 0.0;  // wbc jan 2025  
	weight = W_NONE;
      }
      else if (l->next->dat[0] == 'F') {
	weight = W_NONE;
	l->padding +=0.11;
      }
      else if (l->next->dat[0] == 'G') {
	weight = W_NONE;
	l->padding +=0.11;
      }
      else if (strchr("Yy", l->next->dat[0])) {
	l->padding += str_to_inch(min_d_w);
	weight = W_NONE;		/* no expansion */
      }
      else if (strchr("Qq", l->next->dat[0])) {
	weight = W_NONE;		/* no expansion */
	if (c == '.') l->padding = str_to_inch(min_d_w);
      }
      else if (strchr("bB.Ai", l->next->dat[0])) { 
	//  this overrides what was set above when followed by a barline
	if (l->dat[0] != '*')
	  l->padding = str_to_inch(min_d_w); /* wbc was += */ //wbc jan 2025
	weight = W_NONE;		/* no expansion */
      }
      else if (strchr("bB.Ai", l->dat[0])) {  /// wbc jan 2025
	if (j == *l_p - 2 && (l->next->dat[0] == '*'))
	  l->padding = 0;
	else if (l->next->dat[0] == '*')
	  l->padding = str_to_inch(min_d_w);
	else 
	  l->padding += 0.6 * str_to_inch(min_d_w);}
      else {		        /* expansion - presumably notes here*/
	l->padding += str_to_inch(min_d_w);
	weight = W_ONE;
	if (f->flags & BIGNOTES)
	  weight = .7;
/*
 *  padding for bar lines barlines measures counting only here !!! 
 */

	/* all the following code does is to count bar lines barline measures */
	/* to compensate when a bourdon is next to a bar number */
	
	if (bar_count || barCount ||barCCount) {
	  char *ch  = l->dat;
	  char *nxt = l->next->dat;
	  char *prev = 0;
	  if (l->prev) prev = l->prev->dat;
	  /*  
	printf(" pass1.cc  488 here l->dat[0] %c l->dat %s l->next->dat[0] %c next->next->dat[0] %c\n",
	       l->dat[0], l->dat, l->next->dat[0], l->next->next->dat[0]);
	printf ("j %d l_p %d  \n", j, *l_p );
	  */
	if (*ch == 'b') {
  // printf(" pass1.cc  488 here ch %c l->dat[1] %s nxt %c next->next %c\n", *ch, l->dat, *nxt, l->next->next->dat[0]);
	      if (*nxt == '*') {
	      if ( l->next->next && (l->next->next->dat[0] ==  'b')) {
		//printf(" pass1.cc  491 here nxt %c next->next %c\n", *nxt, l->next->next->dat[0]);
	      }
	    }
	  }
	  else
	    /* printf(" pass1.cc  496 here ch %c nxt %c next->next %c\n", *ch,  *nxt, l->next->next->dat[0]) */;

	  if (( *ch == 'b' || *ch == 'B' || *ch == 'A')
	      && j+1 != *l_p
	      && j != *l_p
	      && *nxt != 'b'
	      && *nxt != 'B'
	      && bdot(l)
	      && *nxt != 'Q'
	      && ch[1] != 'X'
	      &&  (!(prev && *prev == '*' && l->prev->prev && (l->prev->prev->dat[0] ==  'b' ))))  /* wbc jan 2025  line 517*/
	    {
	    measures++;
	    if ( j+4 == *l_p && l->next && l->next->next && l->next->dat[0] == '*') {
	      measures --;
	    }
	    //	    fprintf (stderr, "pass 1: next is %s  measures %d\n", nxt, measures);
	    if ((bar_count && ! (measures % 5)) ||
		(j == 0 && barCount)) {
	      if ( nxt[8] != ' ' ) { // leave space for bar count number
		//		dbg1 (Warning,
		//		      "tab: pass1 :bar count expansion %c\n",
		//		      (void *)ch[0]);
		l->padding += 0.1;
	      }
	    }
	  }
	}
      }
      break;
    case 'v':     /* line break in original */
      weight = W_NONE;
      l->padding = f_a[0]->fnt->get_width(9);
      l->padding = 0;
      goto done;
    case 'L':
      l->padding =  f_a[0]->fnt->get_width(c);
      weight = W_NONE;
      //     weight = wp[0].weight;
      goto done;
    case 'J':
      l->padding = str_to_inch(wp[0].width);
      weight = wp[0].weight;
      goto done;
    case 'W':
      l->padding = str_to_inch(wp[1].width);
      weight = wp[1].weight;
      goto done;
    case 'w':
      l->padding = str_to_inch(wp[2].width);
      weight = wp[2].weight;
      goto done;
    case '0':
      l->padding = str_to_inch(wp[3].width);
      weight = wp[3].weight;
      goto done;
    case '1':
      l->padding = str_to_inch(wp[4].width);
      weight = wp[4].weight;
      goto done;
    case '2':
      l->padding = str_to_inch(wp[5].width);
      weight = wp[5].weight;
      goto done;
    case '3':
      l->padding = str_to_inch(wp[6].width);
      weight = wp[6].weight;
      goto done;
    case '4':
      l->padding = str_to_inch(wp[7].width);
      weight = wp[7].weight;
      goto done;
    case '5':		/* use the 4 width for now */
      l->padding = str_to_inch(wp[8].width);
      weight = wp[8].weight;
      goto done;
    case '6':
      weight = wp[8].weight/2;
      goto done;
    case '$':
    case 'O':			/* no size ! */
      l->padding = 0.0;
      weight = 0.0;
    done:
      if (breaksys) {
	//	printf("pass1 - breaksys\n");
	l->prev->padding += tlength;
	total_width += tlength;
	breaksys=0;
	breaksys2=1;
      }
      if (d[1] == '.' || d[1] == '@' || d[1] == 'W') {
	  l->padding *= 1.422;  /* Sept 2022 wbc was 1.6 */
          weight *=1.2;
      }
      else if (d[1] == 'B') {
	if (l->prev) {
	  l->prev->padding += .05;
	  total_width += .05;
	}
      }
      // this is sort of a test Oct 2003
      else if (l->next && l->next->dat[0] == '&') { // pad after ornaments
	// char jjj;
	for (i=2; i<8; i++ ){
	  if (l->next->dat[i] == 'x') {
	    ;
	  }
	}
	//	fprintf(stderr, "pass1: after ornament\n");
      }
      // end test
      break;
    case '~':
      l->padding = f_a[0]->fnt->get_width (55);	// the "wave" character
      weight = W_NONE;
      break;
    case 'x':
    case '@':  // a dot after a bar line after a flag  from Cosens
      l->padding = old_width;
      switch (old_flag) {
      case 'L':
      case 'B':
      case 'W':
      case 'w':
      case '0':
      case '1':
      case 't':
      case '2':
      case '3':
      case '4':
      case '5':
      case '~':
      case 'i':
      case 'v':
      case 'j':
      case 'd':
      case '$':
	weight = old_weight;
	if (d[0] == '@') {
	  l->padding = l->padding * 0.6;
	  weight = weight * 0.6;
	}
	break;
      default:
	if ( ! (f->m_flags & QUIET ))
	  dbg3(Warning, "x with no previous flag system %d chord %d after %c\n",
	       (void *)((int)n_system),
	       (void *)(int)(j+1), (void *)(int)old_flag);
	break;
      }
      break;

    case 'U':
      if (l->next && l->next->dat[0] == 'G' && (l->next->dat[1] == '2' || l->next->dat[1] == '3')) {
	l->padding = 1.27 *
	  f_a[0]->fnt->get_width(19);
	//	fprintf(stderr, "HERE 1\n");
	weight = W_ONE;
      }
      else {
	l->padding = 1.17 *
	  f_a[0]->fnt->get_width(19); /* add some space */
	weight = W_TWO;
      }
      break;
    case 'u':
      if (l->next && l->next->dat[0] == 'G' && (l->next->dat[1] == '2' || l->next->dat[1] == '3')) {
	l->padding = 1.27 *
	  f_a[0]->fnt->get_width(19);
	//	fprintf(stderr, "HERE 2\n");
	weight = W_ONE;
      }
      else {
	l->padding = 1.17 *
	  f_a[0]->fnt->get_width(20); /* add some space */
	weight = W_TWO;
      }
      break;

    case 'R':
    case 'S':
      //    case 'T':
    case 'V':
      l->padding = 1.15 * f_a[0]->fnt->get_width(c); /* add some space */
      weight = W_TWO;
      break;
    case 'Y':
    case 'y':
      if (strchr ("b.", l->dat[1])) {
	//	if (!l->next) {
	if ((j == *l_p - 1) || ( l->next && l->next->dat[0] == 'Q')) {
	  l->padding = 0.5 * f_a[0]->fnt->get_width(c);
	  weight = W_NONE;
	}
	else {
	  l->padding = 0.5 *
	    f_a[0]->fnt->get_width(c) + str_to_inch(min_d_w);
	  weight = W_TINY;
	}
      }
      else if (l->prev && strchr ("+^:", l->prev->dat[0])) {
	l->padding = f_a[0]->fnt->get_width(c);
	/*		l->padding -= f_a[0]->fnt->get_width('+'); */
	weight = W_TINY;
      }
      else {
	l->padding = f_a[0]->fnt->get_width(c);
	weight = W_TINY;
      }
      break;
    case 'm':
      l->padding = f_a[0]->fnt->get_width(191);
      weight = W_NONE;
      break;
    case 'Q':
    case 'q':
      l->padding = f_a[0]->fnt->get_width(c);
      weight = W_NONE;
      break;
    case 'D':		/* da capo */
    case '8':		/* 8va */
    case 'f':		/* fine */
    case '=':		/* W. = W */
      l->padding = 0.0;
      weight = W_NONE;
      break;
    case 'k':
      switch (d[1]) {
      case 'b':
      case 'd':
	l->padding = .17;
	break;
      case 'e':
      case 'a':
	l->padding = .28;
	break;
      case 'c':
	l->padding = 0.0;
	break;
      default:
	l->padding = .11;
      }
      l->padding += .06; /* use padding rather than expansion  was 09 */
      weight = W_NONE;
      break;
    case 'F':		/* font specification */
      if (l->dat[1] == 'M') {
	l->padding = f_a[0]->fnt->get_width('z'); /* g clef */
	l->padding *= 1.2;
	weight = W_NONE;
      }
      else {
	l->padding = 0.0;
	weight = W_NONE;
	if (f->flags & BIGNOTES ) weight = W_ONE;
	else weight = W_ONE;
      }
      break;
    case 'G':
      if (l->dat[1] == 'M') {
	l->padding = f_a[0]->fnt->get_width(252); /* g clef */
	l->padding *= 1.2;

	/*	if (f->flags & BIGNOTES ) {*/
	l->padding += 0.07;
	weight =/* W_ONE */ 0.0;
	/* }
	   else  weight = W_ONE;
	*/
      }
      else {
	if (j == 0) {	/* first, place to left of line */
	  l->padding = 0;
	  weight = W_NONE;
	}
	else if (strchr(l->dat, '-')) {
	  l->padding = .23;	// was .25
	  for (i=2; i<7; i++) {
	    if (l->next->dat[i] == 'd')
	      l->padding = .35;
	    if (l->next->dat[i] == 'G')
	      l->padding = .35;
	    else if (l->next->dat[i] == 'g')
              l->padding = .3;
	  }
	  weight = W_TWO;
	}
	else {
	  switch (l->dat[1]) {
	  case '0':
	    cc = 27;
	    break;
	  case '2':
	    cc = 'R';
	    break;
	  case '3':
	    cc = 'T';
	    break;
	  case '4':
	    cc = 'S';
	    break;
	  case '5':
	    cc = 'V';
	    break;
	  case '6':
	    cc = 'P';
	    break;
	  case 'O':  // this and next are old spanish signs
	    cc = 18;
	    break;
	  case 'o':
	    cc = 17;
	    break;
	  default:
	    cc = l->dat[1];
	    break;
	  }

	  if ((l->dat[1] == '3'||l->dat[1] == '2' ) &&
	      (l->next->dat[0] == 'U' || l->next->dat[0] == 'u')) {
	    weight = W_ONE;
	    l->padding = 1.27 * f_a[0]->fnt->get_width(cc);
	  }
          else if (cc == 'I') { 
            l->padding = 0.153; weight = W_ONE;}
          else if (cc == 'i') { 
            l->padding = 0.133; weight = W_ONE;}
	  else {
	    l->padding = 1.17 * f_a[0]->fnt->get_width(cc);
	    //	    l->padding += 0.17;
	    weight = W_ONE;
	  }
	}
      }
      break;
    case 'A':		/* again - repeat */
      l->padding = 2 * f_a[2]->fnt->get_width(l->dat[1])
	+ /* 2 */ .3 * str_to_inch(min_d_w);
      weight = W_NONE;
      break;
    case 'd':
      l->padding =
	f_a[0]->fnt->get_width(254); /* the del signe character */
      weight = W_ONE;
      break;
    case 'T':
      l->padding = 0.03;
      weight = W_ONE;
      if (l->text)
	l->text->words->in_sys = 1;
      break;
    case 'Z':
      l->padding = f_a[0]->fnt->get_width(251); /* the zap character */
      weight = W_ONE;
      break;
    case '*':
      if (d[1] == 32)
	printf ("pass1: you need a value afer a star\n");
      l->padding = f_a[2]->fnt->get_width(d[1]);
      l->padding = 0.0;
      weight = /* W_TWO */ /* 0.01 */ 0.0;
      break;
      //    case '$':			// flags
      //    case '-':
      //      fprintf(stderr,"pass1: here\n");
      //break;
    case '[':
      //      dbg0(Warning, " [pass1: not implemented\n");
      l->padding = 0;
      weight = 0;
      break;
    default:
      dbg3(Warning, "tab: pass1: unknown flag %d %s at index %d\n",
	   (void *)(int)c, (void *)l->dat, (void *)j );
      dbg2(Warning,
	   "pass1: unknown flag value in inter file %d %d\n",
	   (void *)n_system, (void *)(j+1));
      break;
    }

    /* check for music with no tab */

    if (l->dat[1] == 'M' &&
	l->dat[0] != 'Z' ) {	/* first of a series */
      struct list *ll, *lll;
      int found=0;

      if (l->prev &&
	  strchr("xJWw012345", l->prev->dat[0])) {
	ll = l->prev;	/* first in series */
	j--;
	total_width -= ll->padding; /* back up to tab note */
	total_weight -= ll->weight;

	do {
	  if (ll->notes &&
	      ( ll->notes->note != '0' ) ) {  /* could be the first one */
	    found = 0;
	    find_val( ll->notes->time, ll, &found,
		      &total_width,
		      &total_weight,
		      f->note_conv + 0, wp,
		      ll->notes->sharp);

	    /* this is an attempt to even up beamed notes
	       if (ll->notes2 && (
	       (ll->notes->time == 'w' && ll->notes2->time < '9')
	       || (ll->notes->time == 'R' && ll->notes2->time < '9')
	       )) {
	       find_val( ll->notes2->time, ll, &found,
	       &total_width,
	       &total_weight,
	       f->note_conv + 0, wp,
	       ll->notes2->sharp);
	       }
	    */

	    if (!found){
	      found = 0;
	      total_width += ll->padding;
	      total_weight += ll->weight;
	    }
	  }
	  else if (ll->notes2) {  /* could be the first one */
	    found = 0;
	    find_val( ll->notes2->time, ll, &found,
		      &total_width,
		      &total_weight,
		      f->note_conv + 0, wp,
		      ll->notes2->sharp);

	    if (!found){
	      found = 0;
	      total_width += ll->padding;
	      total_weight += ll->weight;
	    }
	  }
	  else {                /* !ll->notes */
	    found = 0;
	    find_val(ll->dat[0], ll, &found,
		     &total_width,
		     &total_weight,
		     f->note_conv + 0, wp, ll->dat[1]);

	    if (!found) { /* must be an x */
	      lll = ll->prev;
	      /* trouble here - june 98 */
	      while (lll && lll->prev &&
		     (strchr("bB^x:ij", lll->dat[0]) ||
		      lll->dat[1] == 'M')
		     && lll->prev )
		lll = lll->prev;

	      /* here lll points to the character */

	      find_val(lll->dat[0], ll, &found,
		       &total_width,
		       &total_weight,
		       f->note_conv + 0, wp, lll->dat[1]);
	    }
	  }
	  if (ll->next)
	    ll = ll->next;
	  else {
	    dbg0(Warning, "pass1: Music with no tablature at end of line\n");
	    goto end;
	  }
	  j++;

	} while  (ll->dat[1] == 'M'); // end do loop
	l = ll;
	j--;
	continue;
      }	                              // end if (l->dat[1] ==
    }                                 // end processing line of notes
  end:
    /* wbc added ij June 98  and v aug 2004 */
    if (! strchr("xb.+^&F8*:dGTijD$v[", c) && l->dat[1] != 'M') {
      old_flag = c;
      if (d[1] == '*' || d[1] == '.' ){
	old_width = l->padding / 1.6;
	old_weight = weight / 1.2;
      }
      else {
	old_width = l->padding;
	old_weight = weight;
      }
    }
    total_width  += l->padding;
    l->weight     = weight;
    total_weight += weight;
    l = l->next;
  } /* end of BIG loop */

  /* look for sharps in music */

  if ((f->flags & NOTES )) {
    l = ll;
    for ( j=0; j < (*l_p); j++ ) {
      if (l->notes){
	double extra = 1.1 * f_a[0]->fnt->get_width(22);
	if (( ! ( f->flags & SHARP_UP ))
	    && (l->notes->sharp == '+' || l->notes->sharp == '-')) {
	  if (l->prev
	      && l->prev->dat[0] != '^'
	      && l->prev->dat[0] != '+' ) {
	    l->prev->padding += (extra);
	    total_width += (extra);
	  }
	}
	else if (( ! ( f->flags & SHARP_UP ))               // flat
		 && (l->notes->sharp == '-' || l->notes->sharp == 'v')) {
	  extra = 1.62 * f_a[0]->fnt->get_width('?');
	  if (l->prev) {
	    l->prev->padding += (extra);
	    total_width += (extra);
	  }
	}
	else if ( l->notes->sharp == '.') {
	  l->padding += .12;
	  total_width += .12;
	}
      }
      if (l->notes || l->notes2) {
	if (l->next && l->next->dat[0] == 'b') {
	  l->padding += .02;
	  total_width += .02;
	}
      }
      l = l->next;
    }
  }  /* end f->flags & NOTES */

  /* another pass for all text */

  l = ll;

  if ( 1 ||  !(f->flags & NO_WORD)){
    double t_total=0.0;
    for ( j=0; j < (*l_p) && l; j++ ) {
      if (l->text)
	total_width += text_check(l, (*l_p) - j - 1, &total_weight,
				  l->text->words->in_sys, 1);
      if (l->text2)
	total_width += text_check(l, (*l_p) - j - 1, &total_weight,
				  l->text2->words->in_sys, 2);
      t_total += l->padding;
      
      if (f->flags & VERBOSE)
	fprintf(stderr,
		"tab: pass1: j %2d *l_p %d c %c width (padding) %f total width %f weight %f\n",
		j,
		*l_p,
		l->dat[0],
		l->padding,
		total_width,
		l->weight);
      l = l->next;
    }
  }

  /* find out how much we have to pad it out */
  if (total_weight <= 0.0) {
    dbg0(Warning, "pass1: total_weight is zero\n");
    *extra = 0.0;
  }
  if (staff_len - total_width < 0.0) {
    double v = staff_len - total_width;
    dbg0(Warning, "tab: pass1: total width greater than staff length\n");
    //    fprintf (stderr, "by %f\n", v);
    // wbc July 2019 added the below line to handle too much stuff in line
    *extra = (staff_len - total_width) / total_weight;
  }
  else
    *extra = (staff_len - total_width) / total_weight;
  /*
    fprintf (stderr, "tab: pass1: extra is %f staff_len %f t_width %f tot_weight %f\n",
	     *extra, staff_len, total_width, total_weight);
     */
  if (*extra < 0.0) {
    /*	printf ("tab: extra %f\n", *extra); */
    if (! (f->m_flags & QUIET ) )
      dbg1(Warning,"too many notes on one line, system %d \n",
	   (void *)n_system );
  }

  l = ll;

  for ( j=0; j < *l_p && l; j++) {
    l->space = (l->weight * *extra);  /* add in padding first */
    if (f->flags & NO_EXPAND)
      l->space = 0.0;	/* for testing only */
    l = l->next;
  }
  if (f->flags & NO_EXPAND) {
    printf("line length %f %f\n",
	   total_width, staff_len);
    staff_len = total_width;
  }
  //    printf("line length %f %f\n",
  //	   total_width, staff_len);
  return;
}

/* return width of text minus total width of chars to fit text in */
/* which is zero if there is already enough room */
/* side effect is to adjust the padding of all chords involved */
/* returns extra width needed, if needed */

double
text_check(
     struct list *l,		                     /* this chord */
     int line,			           /*  chords left in line */
     double *total_weight,	      /* this needs to be adjusted */
     int in_sys,                      /* is this text in the staff */
     int staff)                             /* which of two staffs */
{
    double t_size = 0.0;
    struct list *ll = l;
    double result;		/* extra space to fit the text */
    int notes=0;		/* notes to be expanded */
    int nospace_flag;
    int i;
    struct text *text=0;


    if (staff == 1 ) text = l->text;
    else if (staff == 2 ) text = l->text2;

    /*
    printf ("tab: text_check: line: %d text_size: %f in_sys: %d text: %s  \n",
	    line, text->size, text->words->in_sys,
	    text->words->words);
	    */

    if (text && text->words->nospace )
	nospace_flag = 1;
    else nospace_flag = 0;

    while (line &&
	   (nospace_flag ||
	   (text  && text->size - t_size > l->padding ))) {
	line--;
	if ( in_sys
	    || ( (staff == 1 && ll->next->text )
		 || (staff == 2 && ll->next->text2 ))
	    || line < 1         /* end of this text */
	    || strchr (".B", ll->next->dat[0])
	    || (ll->next->dat[0] == 'b' && line == 1)
	    || ( ll->next->next
		 && ll->next->dat[0] == 'b'
		 && strchr ("Bb", ll->next->next->dat[0]))) {

	    t_size += ll->padding;

	    notes++;
	    result = (text->size  - t_size);

	    if ( result < 0.0) {
	      if ( 0 )
			printf ("tab: text_check negative result!! %f %s %d %d\n",
			result,
			text->words->words,
			text->words->nospace,
			nospace_flag);
		result = 0.0;
	    }

	    if (nospace_flag) {
		t_size = text->size;	/* correct */
	    }

	    /* the trick is to divide result up fairly */

	    ll = l;

	    for (i=0; i < notes; i++) {
		ll->padding += result / notes;

		*total_weight -= ll->weight;
		ll->weight /= notes;
		if (result > .5) ll->weight /= 2.0;
		if (nospace_flag) ll->weight = 0.0;
		*total_weight += ll->weight;
		ll = ll->next;
	    }
	    if (result > 0.0) {
//		printf ("tab: text_check: line: %d text_size %f text %s  \n",
//			line, text->size, text->words->words);
//		printf("tab: text_check: result: %f\n", result);
	    }
	    return (result);
	}
	else {
	    t_size += ll->padding;
	    notes++;
	    ll = ll->next;
	}
    }
//    printf("tab: text_check: result: %f\n", 0.0);
    return (0.0);
}

/* find value of musical notes */
void
find_val(
     char val,
     struct list *ll,
     int *found,
     double *total_width, double *total_weight,
     int offset,
     struct w *wp,
     char dot)
{
    int k;

    for (k=0; (k + offset)< 10; k++) {
	if (vals[k] == val) {
	    *found=1;
	    ll->padding = str_to_inch(wp[k+offset].width);
	    if (dot == '.') ll->padding *= 1.8; 
	    ll->weight = wp[k-offset].weight;
	    *total_width += ll->padding;
	    *total_weight += ll->weight;
	    return;
	}
    }
    if (!*found && val == 'R' ) {
	    *found=1;
	    ll->padding = str_to_inch(wp[offset+3 /* quarter note */].width);
	    if (dot == '.') ll->padding *= 1.8;
	    ll->weight = wp[offset+3].weight;
	    *total_width += ll->padding;
	    *total_weight += ll->weight;
	    return;
    }
    if (!*found && val == 'Z') {
	*found = 1;
	ll->padding = 0;
	ll->weight = wp[offset+5].weight;
	*total_width += ll->padding;
	*total_weight += ll->weight;
	return;
    }
    if (!*found && val != 'x') {
	dbg3 (Warning,
		"tab: find_val: value not found %s time %c offset %d\n",
		(void *)ll->dat, (void *)val, (void *)offset);
    }
}



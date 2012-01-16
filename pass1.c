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

#include "tab.h"
#include <stdio.h>
#include <string.h>
/* #include <math.h> */

/* weights */

#define 	W_NONE  0.0
#define         W_TINY  0.000001
#define		W_ONE   1.0
#define		W_TWO   2.0
#define		W_THREE 3.0
#define		W_FOUR  4.0

/* weights of each flag when filling out line */

char min_0_w[] = "0.20 in";
char min_1_w[] = "0.14 in";	/* one flag */
char min_2_w[] = "0.12 in";
char min_3_w[] = "0.10 in";
char min_4_w[] = "0.09 in";	/* four flags */
char min_b_w[] = "0.03 in";	/* bar */
char min_d_w[] = "0.06 in";	/* repeat dot */
char min_O_w[] = "0.09 in";	/* ornament was 0.09 */
char min_F_w[] = "0.07 in";	/* a fingering number */
char min_tilde_w[] = "0.15 in";	/* waves */

extern char staff_len[];
extern char staff_height[];
extern char thick_bar[];

struct w {
    char *width;
    double weight;
};

struct w chart[9] = {
    {"0.26 in", 6.0},		/* B (J) */
    {"0.23 in", 5.0},		/* W */
    {"0.23 in", 5.0},		/* w */
    {min_0_w, W_FOUR},		/* 0 */
    {min_1_w, W_THREE},
    {min_2_w, W_TWO},
    {min_3_w, W_ONE},
    {min_4_w, W_ONE},
    {min_4_w, W_ONE}		/* 5 */
};
struct w big_chart[9] = {
    {min_0_w, W_FOUR},		/* B (J) */
    {min_1_w, W_THREE},		/* W */
    {min_2_w, W_TWO},		/* w */
    {min_3_w, W_ONE},		/* 0 */
    {min_4_w, W_ONE},		/* 1 */
    {min_4_w, W_ONE},		/* 2 */
    {min_4_w, W_ONE},		/* 3 */
    {min_4_w, W_ONE},		/* 4 */
    {min_4_w, W_ONE}		/* 5 */
};

char vals[] = "JWw012345";

FILE *ifd;

pass1(l_p, f, extra)
int *l_p;
struct file_info *f;
double *extra;
{
    int i, j;
    char c;
    static double old_width, old_weight;
    static char old_flag;
    static struct list *l;
    struct list *ll;		/* for second pass */
    char *d, cc;
    double weight;
    double total_width=0.0, total_weight=0.0;
    unsigned char dd;
    double text_check();
    struct w *wp;
    extern int n_measures;
    int measures=n_measures;


    if ( first_1 == 0 ) {
	l = listh;
	first_1++;
    }
    else if (l && l->next) l = l->next;
    else return;
    ll = l;

    if (f->flags & BIGNOTES) wp = big_chart;
    else wp = chart;

    /* figure out basic widths on the first pass through */



    for (j=0; j < (*l_p) && l; j++){	/* get widths */

	if ( l == 0 ){
	    printf("tab: pass1: nil pointer l\n");
	}

	d = l->dat;

/*	printf ("pass1 dat %d ->%s", j, l->dat);   */
/*	printf ("old weight %f width %f\n", old_width, old_weight); */

	if (f->flags & VERBOSE)  {
	    printf ("tab: pass1: %d l:%3d: ", n_system, j+1);
	    for (i=0; i<STAFF; i++) {
		dd = d[i];
		if (dd >= 'a' && dd <= 'p') 
		  printf ("  %1c", dd); 
		else if (dd >= 040 && dd <= 0100 )
		  printf ("  %1c", dd); 
		else if (dd >= 0202 && dd <= 0213)
		  printf ("  %1c", dd - 0202); 
		else if (dd >= 0214 && dd <= 0225)
		  printf ("  %1c", dd - 0214); 
		else if (dd >= 0226 && dd <= 0237)
		  printf ("  %1c", dd - 0226); 
		else printf (" %03d", dd); 
	    }
	    printf ("\n"); 
	}

	switch (c=d[0]) {
	  case '+':		/* ornaments */
	    l->padding = str_to_inch(min_O_w);
	    if (strchr("Yy", l->next->dat[0])) 
	      l->padding = str_to_inch(min_d_w);
	    else if (strchr("{}[]()UX", c)) 
	      l->padding = 0.01;
	    else {
		if ( l->prev->padding > 0.04 ) {
			l->prev->padding -= 0.04;
			total_width -= 0.04;
			}
		else {
			/* l->prev->padding = 0.01; */
			/* total_width = 0.01; */
/*			printf ("tab: pass1: negative padding\n"); */
/*			printf ("tab: pass1: %s\n", l->prev->dat); */
			}
	    }
	    {
		int Q_found=0;
		int orn_found=0;
		for (i = 2; i < STAFF; i++ ) {
		    if (d[i] == 'Q') 
		      Q_found=1;
		    if (d[i] != ' ')
		      orn_found=1;
		    if (d[i] != ' ' && d[i] != 'Q' 
			&& l->next->dat[i] == 'D')
		      l->padding += 0.03;

		}
		if (!orn_found && Q_found) l->padding = 0.0;
	    }
	    weight = W_NONE;
	    break;
	  case '&':		/* ornaments after the note*/
	    l->padding = 0;
	    weight = W_NONE;
	    break;
	  case '^':		/* fingerings */
	    l->padding = str_to_inch(min_F_w);
	    weight = W_NONE;
	    {
		for (i = 2; i < STAFF; i++ ) {
		    if (d[i] != ' ' 
			&& l->next->dat[i] == 'D')
		      l->padding += 0.07;

		}
	    }

	    break;
	  case 'i':     /* insert a space */
	    weight = W_NONE;
	    l->padding = str_to_inch(min_d_w);
	    goto rest;
	  case 'B':
	    weight = W_NONE;
	    l->padding = str_to_inch(thick_bar);
	    goto rest;
	  case '.':
	  case 'b':
	    l->padding = str_to_inch(staff_height);
	    l->padding = 0.0;
rest:
	    if ( j == *l_p - 1) {
		weight = W_NONE;
	    }
	    else if (strchr("Yy", l->next->dat[0])) {
		l->padding += str_to_inch(min_d_w);
		weight = W_NONE;		/* no expansion */
	    }
	    else if (strchr("Qq", l->next->dat[0])) {
		weight = W_NONE;		/* no expansion */
		if (c == '.') l->padding = str_to_inch(min_d_w);
	    }
	    else if (strchr("bB.A", l->next->dat[0])) {
		l->padding += str_to_inch(min_d_w);
		weight = W_NONE;		/* no expansion */
	    }
	    else {		        /* expansion - presumably notes here*/
		l->padding += str_to_inch(min_d_w);
		weight = W_ONE;
		if (bar_count || barCount) {
		    char *ch  = l->dat;
		    char *nxt = l->next->dat;

		    if (j+1 != *l_p && *ch == 'b'
			&& *nxt != 'b'
			&& *nxt != '.'
			&& *nxt != 'Q'
			&& *nxt != 'q'
			) {	
			if (ch[1] != '!') {
			    measures++;	                
			    if ((bar_count && ! (measures % 5)) || 
				(j == 0 && barCount))
			      if ( nxt[8] != ' ' ) {
				  printf ("tab: pass1 :bar count expansion\n");
				  l->padding += 0.1;
			      }
			}
		    }
		}
	    }
	    break;
	  case 'L':
	    l->padding = get_width(0, c);
	    weight = W_NONE;
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
	    l->padding = str_to_inch(wp[7].width);
	    weight = wp[7].weight;
	  done:
	    if (d[1] == '.' || d[1] == '@' || d[1] == 'W') 
	      l->padding *= 1.6, weight *=1.2;
	    break;
	  case '~':
	    l->padding = str_to_inch(min_tilde_w);
	    weight = W_NONE;
	    break;
	  case 'x':
	    l->padding = old_width;
	    switch (old_flag) {
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
		weight = old_weight;
		break;
	      default:
		p_err(n_system, j+1, "x with no previous flag");
		break;
	    }
	    break;
	  case 'U':
	    l->padding = 1.15 * get_width(0, 19); /* add some space */
	    weight = W_TWO;
	    break;
	  case 'u':
	    l->padding = 1.15 * get_width(0, 20); /* add some space */
	    weight = W_TWO;
	    break;
	  case 'R':
	  case 'S':
	  case 'T':
	  case 'V':
	    l->padding = 1.15 * get_width(0, c); /* add some space */
	    weight = W_TWO;
	    break;
	  case 'Y':
	  case 'y':
	    if (strchr ("b.", l->dat[1])) {
		l->padding = 0.5 * get_width(0, c) + str_to_inch(min_d_w);
		weight = W_TINY;
	    }
	    else {
		l->padding = get_width(0, c);
		weight = W_TINY;
	    }
	    break;
	  case 'Q':
	  case 'q':
	    l->padding = get_width(0, c);
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
	      default:
		l->padding = .11;
	    }
	    l->padding += .06; /* use padding rather than expansion  was 09 */
	    weight = W_NONE;
	    break;
	  case 'F':
	    l->padding = 0.0;
	    weight = W_NONE;
	    break;
	  case 'G':
	    if (l->dat[1] == 'M') {
		l->padding = get_width(0, 252); /* g clef */
		l->padding *= 1.2;
		if (f->flags & BIGNOTES ) weight = W_ONE;
		else weight = W_ONE;
	    }
	    else { 
		if (j == 0) {	/* first, place to left of line */
		    l->padding = 0;
		    weight = W_NONE;
		}
		else if (strchr(l->dat, '-')) {
		    l->padding = .25;
		    weight = W_TWO;
		}
		else {
		    switch (l->dat[1]) {
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
		      default:
			cc = l->dat[1];
			break;
		    }
		    l->padding = 1.33 * get_width(0, cc);
		    weight = W_THREE;
		}
	    }
	    break;
	  case 'A':		/* again - repeat */
	    l->padding = 2 * get_width(2, l->dat[1], f) 
	      + 2 * str_to_inch(min_d_w);
	    weight = W_NONE;
	    break;
	  case 'd':
	    l->padding = get_width(0, 254, f); /* the del signe character */
	    weight = W_ONE;
	    break;
	case 'Z':
	    l->padding = get_width(0, 251, f); /* the zap character */
	    weight = W_ONE;
	break;
	  default:
	    printf("tab: pass1: error %d %s\n", c, l->dat );
		p_err(n_system, j+1, 
		      "pass1: unknown flag value in intermediate file ");
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
		    if (ll->notes) {  /* could be the first one */
			found = 0;
			find_val( ll->notes->time, ll, &found, 
				 &total_width, 
				 &total_weight, 
				 f->note_conv + 0, wp,
				 ll->notes->sharp);

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
			    while (lll && 
				   (strchr("bB^x", lll->dat[0]) ||
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
		    ll = ll->next;
		    j++;
		    
		} while  (ll->dat[1] == 'M');
		l = ll;
		j--;
		continue;
	    }
	}
	
	if (! strchr("xb.+^&F8", c) && l->dat[1] != 'M') {
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
    }

    /* look for sharps in music */

    if (f->flags & NOTES && ( ! f->flags & SHARP_UP )) {
	l = ll;
	for ( j=0; j < (*l_p); j++ ) {
	    if (l->notes){
		double extra = .7 * get_width(0, 22);
		if ( l->notes->sharp == '+' ||
		    l->notes->sharp == '^') {
		    if (l->prev) {
			l->prev->padding += extra;
			total_width += extra;
		    }
		}
	    }
	    l = l->next;
	}
    }

    /* another pass for all text */

    l = ll;

    if ( !(f->flags & NO_WORD)){
	for ( j=0; j < (*l_p) && l; j++ ) {

	    if (l->text) 
	      total_width += text_check(l, f, (*l_p) - j - 1, &total_weight);
	    
	    if (f->flags & VERBOSE)
	      printf (
		  "tab: pass1: j %d c %c width %f total width %f weight %f\n", 
		       j, l->dat[0], l->padding,  total_width, l->weight);   
	    l = l->next;
	}
    }
    
    /* find out how much we have to pad it out */
    *extra = (str_to_inch (staff_len) - total_width) / total_weight;

/* printf ("tab: pass1: extra is %f t_width %f tot_weight %f\n",
	    *extra, total_width, total_weight);
*/

    if (*extra < 0) {
	printf ("tab: extra %f\n", *extra);
	p_err(n_system, j+1, "too many notes on one line");
    }

    l = ll;

    for ( j=0; j < *l_p && l; j++) {
	l->space = (l->weight * *extra);  /* add in padding first */
	if (f->flags & NO_EXPAND) 
	  l->space = 0.0;	/* for testing only */
	l = l->next;
    }
    if (f->flags & NO_EXPAND) {
	printf ("line length %f %d\n", total_width, strlen (staff_len));
	sprintf (staff_len, "%f in", total_width);
    }
    return;
}

/* return width of text minus total width of chars to fit text in */
/* which is negative if there is already enough room */
/* side effect is to adjust the padding of all chords involved */

double
text_check(l, f, line, total_weight)
     struct list *l;		                     /* this chord */
     struct file_info *f;
     int line;			           /*  chords left in line */
     double *total_weight;	      /* this needs to be adjusted */
{
    double t_size = 0.0;
    struct list *ll = l;
    double result;		/* extra space to fit the text */
    int notes=0;		/* notes to be expanded */
    int nospace_flag;
    int i;

/*    printf ("text_check: line %d text_size %f text %s\n", 
	  line, l->text->size, l->text->words->words);
	  */
    if (l->text && l->text->words->nospace ) 
      nospace_flag = 1;
    else nospace_flag = 0;

    while (line && 
	   (nospace_flag || 
	    l->text->size - t_size > l->padding) ) {
	line--;
	if (ll->next->text 
	    || line < 1         /* end of this text */
	    || strchr (".B", ll->next->dat[0])
	    || (ll->next->dat[0] == 'b' && line == 1)
	    || ( ll->next->next 
		&& ll->next->dat[0] == 'b' 
		&& strchr ("Bb", ll->next->next->dat[0]))) {

	    t_size += ll->padding;

	    notes++;
	    result = (l->text->size  - t_size);

	    if ( result < 0.0) {
		printf ("tab: text_check negative result!! %f %s %d %d\n",
			result, 
			l->text->words->words,
			l->text->words->nospace,
			nospace_flag);
		result = 0.0;
	    }
	      
	    if (nospace_flag) {
		t_size = l->text->size;	/* correct */
	    }

	    /* the trick is to divide result up fairly */

	    ll = l;

	    for (i=0; i < notes; i++) {
		ll->padding += result / notes;

		*total_weight -= ll->weight;
		ll->weight /= notes /* -1 */;
		if (result > .5) ll->weight /= 2.0;
		if (nospace_flag) ll->weight = 0.0;
		*total_weight += ll->weight;
		ll = ll->next;
	    }

	    return (result);
	}
	else {
	    t_size += ll->padding;
	    notes++;
	    ll = ll->next;
	}
    }
    return (0.0);
}

/* find value of musical notes */
find_val(val, ll, found, total_width, total_weight, offset, wp, dot)
     char val;
     struct list *ll;
     int *found;
     double *total_width, *total_weight;
     int offset;
     struct w *wp;
     char dot;
{
    int k;

    for (k=0; (k + offset)< 9; k++) {
	if (vals[k] == val) {
	    *found=1;
	    ll->padding = str_to_inch(wp[k+offset].width);
	    if (dot == '.') ll->padding *= 1.8;
	    ll->weight = wp[k+offset].weight;
	    *total_width += ll->padding;
	    *total_weight += ll->weight;
	    return;
	}
    }
    if (!*found && val != 'x') {
	printf (
		"tab: find_val: value not found %s time %c offset %d\n",
		ll->dat, val, offset);
    }
}



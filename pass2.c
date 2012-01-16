/* pass2.c
 */

    /* second pass which adds in padding and outputs the characters */

#include <stdio.h>
#include "tab.h"

extern char interspace[];

pass2(l_p, f, extra)
     int *l_p;
     struct file_info *f;
     double *extra;
{
    static struct list *l, *ll;
    int j;
    int i_space = str_to_dvi(interspace);
#ifdef X_WIN
    extern double x_space[9][LIST];
    extern int num_staff;
#endif /* X_WIN */

    if ( !first_2 ) {
	l = listh;
	first_2++;
    }
    else l = l->next;  

    ll = l;

    for ( j=0; j < *l_p && l; j++) {	/* loop through line of notes */

#ifdef X_WIN
	if (j == 0) x_space[num_staff][j] = space + l->padding;
	else  x_space[num_staff][j] = 
	  space + l->padding + x_space[num_staff][j-1];
/* 	printf("   %f %f\n",  j, x_space[num_staff][j]); */
#endif /* X_WIN */

/*	printf ( "pass2: dat %s space %f pad %f text %f %s\n",
		l->dat,
		l->space, l->padding, l->text ? l->text->size: 0,
		l->text? l->text->words:  " ");
 */

	if (f->flags & MANUSCRIPT) { 
	    score(j, l->space, l->padding, f, l->dat, l->prev->dat);
	}
	else dvi_format(l_p, j, f, l);

	if (! l->next) break;
	l = l->next;
    }

    getloc(0);

    do_system(f);			/* draw the lines */
    if (f->flags & MANUSCRIPT) {
	p_movev(6 * i_space);
	do_system(f);			
    }	
}

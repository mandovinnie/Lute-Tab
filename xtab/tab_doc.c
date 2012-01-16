/*

 */

#include <stdio.h>
#include "../tab.h"

double x_space[9][LIST];		/* inter char spacings */

get_char_position(x, y)
unsigned int x, y;
{
    extern int num_staff;
    char *c;
    int i;
    struct list *l;

    l = listh;

    printf (" %d\n", d_to_x(inch_to_dvi(1.0)));

    for (i=0; i< LIST; i++) {

	c = l->dat;

	if (d_to_x(inch_to_dvi(x_space[0][i]))/2.3 + 
	    98 /* d_to_x(inch_to_dvi(0.75))*/ > x) { 

	    printf (
   "Xx %d Xy %d data number %d %f %c %c %c %c %c %c %c %c %c %c\n", 
		    x, y,
		    i, x_space[0][i], 
		    *c, c[1], c[2], c[3], c[4], c[5], 
		    c[6], c[7],c[8], c[9], c[10]);
	    return(0);
	}
	l = l->next;
    }
    printf ("not found\n");

    return(0);
}

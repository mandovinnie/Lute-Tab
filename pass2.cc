/* pass2.c
 */

    /* second pass which adds in padding and outputs the characters */

#include "tab.h"
#include "print.h"
#include "tfm.h"
#include "system.h"
#include "ascii.h"
#include "nmidi.h"

/* EXTERNAL */
extern char mus_space[];
extern char interspace[];

static int first_2=0;

ascii *ap=0;

void
pass2(print *p, i_buf *i_b, font_list *f_a[], int *l_p, struct file_info *f, double *extra)
{
    static struct list *l/* , *ll */;
    int j;
    extern struct list *listh;

    if (1 /* !first_2 */ ) {
	l = listh;
	first_2++;
    }
    else l = l->next;  

    p->push();

    if (f->m_flags & ASCII) {
      ap = new ascii();
      f->utility = ap;
    }

    for ( j=0; j < *l_p && l; j++) {	/* loop through line of notes */
      //	printf ("pass 2: %s\n", l->dat);

	if (f->flags & MANUSCRIPT) { 
	    f->line_flag = BETWEEN_LINE;
	    score(p, l, f, i_b, f_a);
	}
	else if (f->m_flags & ASCII ) {
	  //	  dbg0 (Warning, "pass2: ascii\n");
	  do_ascii(l, f, i_b);
	}
	else if (f->m_flags & NMIDI ) {
	  do_nmidi(l, f, i_b);
	}
	else 
	  dvi_format(p, i_b, f_a, l_p, j, f, l);

	if (! l->next) break;
	l = l->next;
	//	free (l->prev);
    }
    
    if (ap) delete (ap);

    while (l->prev) {
      if (l->notes)   free (l->notes);
      if (l->notes2)  free (l->notes2);
      
      if (l->text) {
	
	if (l->text->words) {
	   if (l->text->words->words)  free (l->text->words->words);
	   free (l->text->words);
	}
	
	free (l->text);
      }
      
      if (l->text2){
	if (l->text2->words)  {
	  if (l->text2->words->words)  free (l->text2->words->words);
	  free (l->text2->words);
	}
	free (l->text2);
      }
      if (l->special) free (l->special);
      l = l->prev;
      free(l->next);
    }
    free (l);
    l=NULL;
    
    listh = 0;

    p->comment("%% about to pop out of pass 2\n");
    p->pop();

    do_system(p, f);			/* draw the lines */
#ifndef MAC
    p->comment("%%%% end system lines  here\n");
#endif

    if (f->flags & MANUSCRIPT) {
	p->p_movev(6 * str_to_dvi(mus_space));
	do_system(p, f);			
    }	
}

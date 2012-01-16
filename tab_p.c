/* make a page of tablature 
 */

#include <stdio.h>
#include <setjmp.h>
#include "tab.h"


/* extern char staffspace[];*/
extern char staff_len[];
extern char staff_height[];
extern char italian_offset[];
extern char interspace[];
extern char flag_to_staff[];
int num_staff=0;

/* return 0 on completion, 1 if there is more iniput */

double hor_offset=0;		/* for two systems on a line */

format_page(f)
struct file_info *f;
{
    int i;
    SIGNED_C c, r=0, getisystem();
    char cmdbuf[120];
    double staff_space;
    static int staff_page = 0;
    static int sys_count  = 0;
    int one_sys_done          = 0;
    int l_p[STAFF_PAGE * MAX_PAGE];
    int title_done=0;
    extern double text_sp;	/* space for each line of text */
    extern double m_space;	/* music interline space */
    extern double st_text;	/* space between music and tab */
    extern int st_italian;	/* is this long italian flags */
    double dvi_to_inch();
    double flag_h=0.0;

    if (f->flags & DRAFT && f->flags & PS ) {
	printf ("Draft\n");
	ps_draft();
    }
    if (f->flags & COPYRIGHT && f->flags & PS ) {
	printf ("Copyright\n");
	ps_copyright();
    }
    if (!sys_count) {
	if ( red == 1.0 ) {
	    if (f->flags & MANUSCRIPT) staff_page = 4;
	    else staff_page = 6;
	}
	else if (red == 0.88880){
	    staff_page = 7;
	    strcpy(staff_len, "7.3132 in");
	}
	else if (red == 0.777770){
	    staff_page = 8;
	    strcpy(staff_len, "8.4 in");
	}
	else {
	    staff_page = 9;
	    strcpy(staff_len, "9.5 in");
	}
    }

    if (f->flags & ROTATE ) strcpy (staff_len, "9.0 in");
    if (f->flags & LSA_FORM) {
	   st_text = .58;
	   m_space = .089;
	}

    printf("[");

    if (f->flags & PAGENUM) {
	char num[3];
	sprintf(num, "%d", pagenum);
	format_pagenum (num, f);
    }

    flag_h = get_height(0, f->flag_flag == ITAL_FLAGS ? 217 : 195);

    while (c = geti()) {
	ungeti(c);

	switch (c) {
	  case '{':
	    if (one_sys_done /*sys_count */) {	/* title after a system, correct space */
		if (f->flags & LSA_FORM ) 
		  fmovev( str_to_inch("1.0 in"));
		else if (f->flags & WALLACE ) 
		  fmovev( str_to_inch("0.5 in"));
		else 
		  fmovev( str_to_inch("0.5 in")); /* to bottom of title */
	    }
/* else just do title */
	    (void) dotitle(f);
	    title_done++;
	    if (f->flags & MARKS) put_rule("0.5 in", "0.01 in");
	    if ((c = geti()) != '{') {                    /* no more text */
		if (f->flags & LSA_FORM ) 
		  fmovev( str_to_inch("0.50 in"));
		else if (f->flags & WALLACE )        /* always with music */
		  fmovev( str_to_inch("0.3 in"));
		else 
		  fmovev( str_to_inch("0.40 in")); 
	    }
	    else {		                      /* yes, more title  */
		fmovev( str_to_inch(/* interspace */ "5.0 pt" )
		       + get_height(2, 'A'));              /* the em quad */
	    }
	    ungeti(c);
	    break;
	  case '[':
	    fmovev( -1 * ( str_to_inch(flag_to_staff)
			  + flag_h 
			  + str_to_inch(flag_to_staff)
			  - get_height(2,'A') / 1.02)); 
	    (void) dotitle(f);
	    if (f->flags & MARKS) put_rule("0.5 in", "0.01 in");
	    fmovev(  str_to_inch(flag_to_staff)
		   + flag_h 
		   + str_to_inch(flag_to_staff) ); 
	    if (f->flags & MARKS) put_rule("0.5 in", "0.01 in");
	    break;
	  case 'P':
	    {
		char nbuf[STAFF+2];
		char *p = nbuf;
		(void)geti();
		while ((*p = geti()) != '\n' && *p!= (char)EOF) 
		  p++;
		format_pagenum (nbuf, f);
	    }
	    break;
	  case '-':
	    i=0;
	    while (( c = geti()) != '\n') 
	      cmdbuf[i++] = c;
	    cmdbuf[i] = '\0';
	    args_from_string(cmdbuf, f);
	    break;
	  case 'e':
	  case EOF:
	    printf ("-eof-]");
	    return(END_OK);
	  case 'p':
	    (void)geti();
/*	    printf ("tab_p.c: Got a p\n");*/
	    title_done = 0;
	    one_sys_done = 0;
	    printf("]");
	    return(END_MORE);
	  case '\n':
	  case '\t':
	  case ' ':
	    c = geti(); 
	    if ((c = geti()) == 'e') {
		printf ("]");
		return(END_OK);
	    }
 	    else ungeti(c);

	    break;
	  case '%':
	    while (( c = geti()) != '\n');
	    break; 
	    break;
	  default:		/* assume that it must be lines of chords */

	    if (!more()) {
/*		ungeti(c); wbc June 94 */

		printf( "]");
		return(END_MORE);
	    }

	    r = getisystem(&l_p[sys_count], f);           /* get line tab */

	    if (f->flag_flag == ITAL_FLAGS) {
		flag_h = get_height(0, 217);
		if ( !st_italian) {
		    st_italian++;
		    st_text += (flag_h - get_height(0, 195));
		    printf ("st_text %f\n", st_text);
		}
	    }
	    else {
		flag_h = get_height(0, 195);
		st_italian = 0;
	    }
	    
	    if (f->flags & NOTES) {
		for (i=0; i< 5; i++ ) {
		    put_rule(staff_len, staff_height);
		    fmovev(m_space);
		}	
		fmovev( st_text);
		fmovev( text_sp * f->n_text + f->c_space); 
	    }
	    else {		/* compensate for flag height here */
		fmovev(flag_h + str_to_inch(flag_to_staff));
	    }

	    printsystem(&l_p[sys_count], f);

	    if ( r == END_OK ) {
		printf ("]\n");
		return (END_OK);
	    }

	    staff_page--;
	    one_sys_done++;
	    sys_count++;

	    if (f->flags & MARKS)
	      f_put_rule(str_to_inch("1 in"), str_to_inch("0.02 in")); 

	    if (f->flags & SAMELINE){
		f->flags &= ~SAMELINE;
		hor_offset = 6.5 / 2.0;
		fmoveh(hor_offset);
	    }
	    else {
		fmoveh( -1.0 * hor_offset);
		hor_offset = 0.0;
		
		staff_space = 6 * str_to_inch(interspace) /* to bottom */
		  + str_to_inch("0.33 in");
		if (f->flags & LSA_FORM ) staff_space += .5;
		fmovev(staff_space);    /* only place staffspace is used */
		/* staffspace is percieved space between  */
		/* bot line of system, top of flags */
		if (f->flags & MARKS)
		  f_put_rule(str_to_inch(".3 in"), str_to_inch("0.05 in")); 
	    }

	    if (~f->flags & NOTES && f->flags & TXT ) {
		fmovev( text_sp * f->n_text + f->c_space); 
	    }

	    if (f->flags & NOTES) f->flags &= ~NOTES;
	    if (f->flags & TXT) f->flags &= ~TXT;
	    f->n_text = 0;
	}
    }
    printf("]");
    return(END_OK);
}


/* draw a blank staff 
 */

static int i_space = 0;

do_system(f)
     struct file_info *f;
{
    int lines = 6;
    int i;
    double str_to_inch();

    if (i_space == 0 ) i_space = str_to_dvi(interspace);

    saveloc(0);

    if (f->flags & FIVE || f->flags &  MANUSCRIPT) lines = 5;

    if (f->line_flag == ON_LINE) p_movev(i_space / 2);
    else p_movev(i_space);		/* go beyond top line */

    if (f->flags & CON_SEV ){ /* 7 line italian conversion */
	p_movev(i_space);
    }

    for (i=0; i< lines; i++ ) {
	put_rule(staff_len, staff_height);
	if ( i != lines -1) p_movev(i_space);
    }		
    getloc(0);
}

SIGNED_C
getisystem(l_p, f)		/* get a system */
     int *l_p;			/* and print it */
     struct file_info *f;
{
    SIGNED_C c;
    SIGNED_C getiline();

    *l_p = 0;
    while (( c = getiline(l_p, f)) != END_SYSTEM && c != END_FILE ) 
      ;

    if (c == END_FILE) return(END_OK);
    else return (END_MORE);		/* more to come */
}

SIGNED_C
getiline(l_p, f)
     int *l_p;
     struct file_info *f;
{
    int i=0;
    SIGNED_C c;
    static struct list *l;
    int got_text = 0;		/* for multiple lines of text */
    char cmdbuf[BUFSIZ], *p;

    if (listh == NULL) {
	listh = (struct list *) my_malloc (sizeof(struct list));
	l = listh;
	l->next = NULL;
	l->prev = NULL;
    }
    else {
	l->next = (struct list *) my_malloc (sizeof(struct list));
	if (! l->next) {
	    printf ("tab: bad malloc in getiline\n");
	}
	l->next->prev = l;
	l = l->next;
    }


    if (*l_p == 0) l->prev = NULL;

    l->notes = NULL;
    l->text = NULL;
    l->special = NULL;

    do {                      /* get a line */
	c = geti();
	if ( i == 0 ) {
/*	    printf ("new line %c\n", c); */
	    switch (c) {
	      case '-':		/* command options */
		p = cmdbuf;
		*p = '-';
		p++;
		while((*p = geti()) != '\n')
		  p++;
		args_from_string(cmdbuf, f);
		l->prev->next = 0;
		l = l->prev;
		return ('\n');
	      case EOF:
	      case 'e':
		return END_FILE;
/* the following represents end of the system, something different follows */
	      case '\n': 	/* end of system */
	      case '{':		/* text */
		ungeti (c);
		return(END_SYSTEM);
	      case '%':		/* shouldn't have a comment here! */
		printf("tab: getiline: comment in intermediate file!\n");
		break;
	      case 'F':
		l->special = (char *)malloc(80);
		break;
	      default:
		break;
	    }
	} 
	if ( i == STAFF ) {
	    char d;
	    switch (c) {
	      case 'M':
		f->flags |= NOTES;
		c = ' ';
		l->notes = (struct notes *)my_malloc(sizeof(struct notes));
		l->notes->parent = l;
		d = geti();
		l->notes->time  = d;
		d = geti();
		l->notes->note  = d;
		if (l->notes->note == 'C') 
		    f->c_space = 0.025;
		d = geti();
		l->notes->sharp = d;
		if (( d = geti() ) == 'r' || d == '#' || d == 'x') { 
		    l->notes->special = d;
		    break;
		}                   /* else fall through */
		else {
		    ungeti(d);
		    l->notes->special = '\0';
		}
		if (( d = geti() ) == '\n') { /* was != 'T' */
		    ungeti(d);
		    break;
		}                   /* else fall through */
	      case 'T':
		f->flags |= TXT;
		{
		    char *p, *pp=0, buff[64];
		    int n = 0;
		    double tmp_text=0.0;
		    int font;
		    struct t_words *tp;

		    if (f->flags & PS )font= 1;
		    else font = 2;

		    /* initialize a text buffer */

		    l->text = (struct text *)my_malloc(sizeof (struct text));
		    l->text->size = 0.0; 
		    l->text->parent = l;
		    l->text->length = 0;
		    l->text->words = /* grab first buffer */
		      (struct t_words *)my_malloc(sizeof (struct t_words));
		    tp = l->text->words;
		    tp->next = NULL;
		    tp->prev = NULL;
		    tp->nospace = 0;

		    p = buff;
		    while (tp) {
			while (1) {
			    *p = geti();
			    if (*p == '\n') {
				*p = '\0';
				ungeti('\n');
				if (pp && *pp == '|' ) {
				    tp->nospace = 1;
				    tmp_text -= get_width(font, '|');
				    *pp = ' ';
				}
				else if (pp && *pp != '-') {
				    tmp_text += get_width(font, '-');
				}
				break;
			    }
			    else if (*p == '\t') {
				got_text++;
				*p = '\0'; /* erase tab */
				if (pp && *pp == '|' ) {
				    tmp_text -= get_width(font, '|');
				    tp->nospace = 1;
				}
				else if (pp && *pp != '-') {
				    tmp_text += get_width(font, '-');
				}
				p++;

				tp->next = (struct t_words *)
				    my_malloc(sizeof (struct t_words));
				tp->next->prev    = tp;
				tp->next->words   = 0;
				tp->next->length  = 0;
				tp->next->size    = 0.0;
				tp->next->next    = 0;
				tp->next->nospace = 0;
				break;
			    }
			    else {
				switch (*p) {
				  case ']':	/* a tie, add space */
				    tmp_text += get_width(0, 0201);
				    if (p != buff && *(p-2) == '<')
				      /* a tie after an ' accent */
				      tmp_text += 0.2 *
					get_width(0, 0201);
				    break;
				  case '<':
				  case '[':
				  case '>':
				  case ':':
				  case '\\': /* do nothing */
				    break;
				  default:
				    tmp_text += get_width(font, *p);
				    break;
				}
			    }
			    pp = p;
			    p++; n++;
			    if ( n > sizeof(buff) ) 
			      break;
			}
			/* get the maximum */
			l->text->size = 
			  l->text->size > tmp_text ? 
			    l->text->size : tmp_text;
			l->text->length = 
			  l->text->length > n ?
			    l->text->length : n;
			
			tp->words = (char *)my_malloc(n+1);
			strncpy(tp->words, buff, n+1);
			tp->length = n;
			tp->size = tmp_text;
			tp = tp->next;
			
			tmp_text = 0.0;
			n = 0;
			p = buff;
		    }
		    if ((got_text + 1) > f->n_text) 
		      f->n_text = got_text + 1;
		}
		break;
	      default:
		break;
	    }
	} /* if i == staff */
	if (i < STAFF ) l->dat[i] = c;
	if (l->special) l->special[i] = c;
	i++;
    } while (c  != EOF && c != '\n'); 
    
/*    printf ("getichar: l->dat %s\n", l->dat); */
    *l_p = *l_p + 1;

    return (c);
}

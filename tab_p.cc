/* Make a page of tablature 
 */



/*#include <setjmp.h>*/
#include "tab.h"
#include "print.h"
#include "tfm.h"
#include "i_buf.h"
#include "system.h"
/* EXTERNAL */
int num_staff=0;
extern int n_measures;


/* return 0 on completion, 1 if there is more iniput */
static int ssys=0;
double hor_offset=0;		/* for two systems on a line */
extern int title_font;
extern int text_font;

int orig_v, orig_h;		// for milticol output
int setflag(file_info *f, char * string, pass pass);
int set_string(const char *arg, 
		const char *val, struct file_info *f, pass pass);
int get_text(struct list *l, struct text **text, 
	     i_buf *i_b, font_list *f_a[], 
	     struct file_info *f, int font);
double my_max(double a, double b) {
  if (a > b ) return a;
  return b;
}

int format_page(print *p, i_buf *i_b, 
		font_list *f_a[], struct file_info *f)
{
    int i;
    signed char c, r=0;
    char cmdbuf[120];
    double staff_space;
    static int staff_page = 0;	/* number of systems per page */
    static int sys_count  = 0;	/* systems done on a page so far */
    int one_sys_done      = 0;	/* has a system been done - for title */
    /* after a system */
    int title_done        = 0;	/* if a title has been done - not tested */
    int tit_last          = 0;	/* was the last thing a title?  */
    int l_p[STAFF_PAGE * MAX_PAGE]; /* a ponter to each line */
    double flag_h=0.0;
    int return_val;

//    dbg0(Flow, "in format page\n");

    if (f->flags & DRAFT && f->flags & PS ) {
	if (!(f->m_flags & QUIET)) dbg0 (Warning, "Draft\n");
	p->ps_draft();
    }

    if (f->flags & COPYRIGHT && f->flags & PS ) {
	dbg0 (Warning, "Copyright\n");
	p->ps_copyright();
    }
    if (!sys_count) {
	if ( red == 1.0 ) {
	    staff_page = 6;
	}
	else if (red == 0.94440){
	    staff_page = 6;
	    staff_len = 6.88;
	}
	else if (red == 0.88880){
	    staff_page = 8;
	    staff_len =7.3132;
	}
	else if (red == 0.777770){
	    staff_page = 8;
	    staff_len = 8.4;
	}
	else {
	    staff_page = 9;
	    staff_len = 9.5;
	}
    }

    if (f->m_flags & A4) staff_len *= 0.96426;
    
    if (f->flags & ROTATE ) {
	if ( red == 1.0 ) {
	    staff_len = 9.0;
	    staff_page = 4;
	}
	else if (red == 0.94440){
	    staff_page = 5;
	    staff_len = 9.5;
	}
	else if (red == 0.88880){
	    staff_page = 5;
	    staff_len = 10.1260;
	}
	else if (red == 0.777770){
	    staff_page = 6;
	    staff_len = 11.57154;
	}
	else {
	    staff_page = 7;
	    staff_len =13.50135;
	}
	if (f->m_flags & A4) staff_len *= 1.077; // rotated
    }

    //    o_staff_len = staff_len;

    if (f->flags & PAGENUM && f->flags && ROTATE 
	&& red == 0.94440 ) {	/* shift whole page for book */
	int num = p->get_page_number();
	p->push();

	staff_len -= .3;
	if (num%2) {		/* left side */
	    p->moveh(0.3);
	}
	/* right side does nothing*/
	
    }
    if (f->flags & PAGENUM) {
	char num[6];
#ifdef MAC
	my_sprintf(num, "%d", p->get_page_number());
#else
	sprintf(num, "%d", p->get_page_number());
#endif
	format_pagenum (p, f_a, num, f);
    }

    switch (f->flag_flag) {
    case S_ITAL_FLAGS:
	flag_h = f_a[0]->fnt->get_height(0350);
	break;
    case ITAL_FLAGS:
	flag_h = f_a[0]->fnt->get_height(0332);
	break;
    case THIN_FLAGS:
	flag_h = f_a[0]->fnt->get_height(0363);
	break;
    case CONTEMP_FLAGS:
	flag_h = f_a[0]->fnt->get_height(0312);
	break;
    case BOARD_FLAGS:
	flag_h = f_a[0]->fnt->get_height(0356);
	break;
    default:
	flag_h = f_a[0]->fnt->get_height(0302);
	break;
    }
    if (f->flags &  MANUSCRIPT ) {
	flag_h = -1 * str_to_inch (interspace);
    }    
    st_text = flag_h + .05;	/* flag height plus space */

    if (f->flags & LSA_FORM) {
	st_text = .58;
	m_space = .089;
    }


    p->get_current_loc(&orig_h, &orig_v); // in case we need to come back
	
    while (c = i_b->GetByte()) {
	i_b->unGet(c);

 /* return if there isn't enough room */
	if (p->more() && ! (f->m_flags & NO_AUTO_END )) { 
	    return_val = END_MORE;
	    goto leave;
	}

	dbg1(Flow, "in format page loop %c\n", (void *)((int)c));

	if (f->flags & MARKS) p->put_rule("2.0 in", "0.01 in");

	switch (c) {
	case '?':
	    tit_last=0;
	    //	    p->moveh(1.5);
	    p->put_rule(1.0, 0.01); /* width, height */
	    p->put_rule(0.01, 1.0);
	    p->push();
	    p->movev(0.5);
	    p->put_a_char(c);
	    p->pop();
	    p->movev(1.0);
	    p->put_rule(1.0, 0.01);
	    p->saveloc(8);
	    p->moveh(1.0);
	    p->put_rule(0.01, -1.0);
	    p->getloc(8);
	    p->put_rule(0.01, 1.0);
	    while ((c = i_b->GetByte()) != NEWLINE)
		;
	    break;
	case '{':
	    if (tit_last)  
		p->movev(-.085);
	    tit_last++;
	    if (one_sys_done) {    /* title after a system, add some space */
	      /*
		if (f->flags & LSA_FORM ) 
		p->movev("1.0 in");
		else 
		*/
		  if (f->flags & WALLACE ) 
		    p->movev("0.5 in");
		else 
		    p->movev("0.1 in"); /* from system to bottom of title */
	    }
	    one_sys_done = 0;
/* else just do title */
	    if ( f->flags & PS ) {
		int val = pt_to_dvi(f->font_sizes[2] - 12.0);
		double v = f->font_sizes[2] - 12.0;
		p->movev(pt_to_dvi(f->font_sizes[2] - 12.0));
	    }
	    /* we presume a new song here, so reset the bar count */
	    n_measures=0;
	    dotitle(p, i_b, f_a, f);
	    title_done++;
	    if (f->flags & MARKS) p->put_rule("0.5 in", "0.01 in");
	    /* space from title to music */
	    if ((c = i_b->GetByte()) != '{') {            /* no more text */
	        if (f->flags & WALLACE )        /* always with music */
		    p->movev( str_to_inch("0.3 in"));
		else if (f->flags & ROTATE  && baroque) {
		    p->movev( str_to_inch("0.3 in"));
		}
		else 
		    p->movev( str_to_inch("0.40 in")); 
	    }
	    else {		                      /* yes, more title  */
		if (f->m_flags & ALTTITLE) 
		    p->movev( str_to_inch(/* interspace */ "12.0 pt" )
			  + f_a[3]->fnt->get_height('A'));    /* the em quad */
		else
		    p->movev( str_to_inch(/* interspace */ "12.0 pt" )
			  + f_a[2]->fnt->get_height('A'));    /* the em quad */
	    }
	    i_b->unGet(c);
	    break;
	case '[':
	    dotitle(p, i_b, f_a, f);
	    if (f->flags & MARKS) p->put_rule("0.5 in", "0.01 in");
	    break;
	case 'K':		// move to next col for text
	{			// this is a hack for a book project
	    int h, v;
	    one_sys_done = 0;
	    tit_last = 0;
	    while (( c = i_b->GetByte()) != NEWLINE) 
		;
	    p->get_current_loc(&h, &v);
	    p->moveh(3.5);
	    if (f->flags & DVI_O) 
		p->movev(orig_v - v);  // dvi
	    else if  (f->flags & PS) 
		p->movev(v - orig_v);
	    break;
	}

	case '$':
	{
	    char buf[80];
	    char *bp;
	    bp = &buf[0];
	    while (( c = i_b->GetByte()) != NEWLINE) {
		*bp=c;
		bp++;
	    }
	    *bp='\n';
	    (void)setflag(f, &buf[1], second);
	    break;
	}

	case 'P':
	    tit_last=0;
	    {
		static char nbuf[STAFF+4];
		char *pp = nbuf;
		(void)i_b->GetByte();
		(void)i_b->GetByte();
		while ((*pp = i_b->GetByte()) != NEWLINE && *pp!= (char)EOF) 
		    pp++;
		pp++;
		*pp='\0';
		//		printf("tab_p.cc: getting page number %s\n", nbuf);

		format_pagenum (p, f_a, &nbuf[0], f);
	    }
	    break;
	case '-':
	    i=0;
	    while (( c = i_b->GetByte()) != NEWLINE) 
		cmdbuf[i++] = c;
	    cmdbuf[i++] = '\0';
	    cmdbuf[i++] = '\0';
	    dbg1(Inter, "tab_p: %s\n", (void *)cmdbuf);
	    args_from_string(cmdbuf, f);

	    break;
	case 'F':
	{
	    char cc[2];
	    int fontnum;
	    c = i_b->GetByte();	// throw away space
	    c = i_b->GetByte();	// throw away space
	    if ( c == ' ' )
	      c = i_b->GetByte(); // the magic number
	    cc[0] = c;
	    cc[1] = '\0';
	    fontnum = atoi(cc);
	    if ( ! (f->m_flags & QUIET))
	      printf("tab_p: fontnum %s %d\n", cc, fontnum);
	    switch (fontnum) {
	      case 1:
	      case 5:
		text_font = fontnum;
		break;
	      case 2:
	      case 3:
		title_font = fontnum;
		break;
	    }
//	    (void)i_b->GetByte(); // the newline
	}
	    break;
	case 'e':
	case EOF:
	    return_val = END_OK;
	    goto leave;
	case 'p':
	    (void)i_b->GetByte();
	    return_val = END_MORE;
	    goto leave;
	case NEWLINE:
	case '\t':
	case ' ':
	    c = i_b->GetByte(); 
	    if ((c = i_b->GetByte()) == 'e') {
		return_val = END_OK;
		goto leave;
	    }
 	    else i_b->unGet(c);
	    
	    break;
	case 'V':		/* vertical skip */
	{
	    double skip;
	    char buf[80], *bufp = buf;
	    *bufp = '\0';
	    while (( c = i_b->GetByte()) != NEWLINE) {
//		    printf ("%c %d\n", c, c);
		if (c == '"') {
		    while (1) {
			*bufp = i_b->GetByte();
			if ( *bufp == '"' ) {
			    *bufp = '\0';
			    skip = str_to_inch(buf);
			    break;
			}
			bufp++;
		    }
		}
	    }
	    p->movev(skip);
	}
	break;
	case '%':
	    while (( c = i_b->GetByte()) != NEWLINE);
	    break; 
	default:		/* assume that it must be lines of chords */
	    tit_last=0;
	    

	    r = getisystem(i_b, f_a, &l_p[sys_count], f);  /* get line tab */

	    //	    fprintf(stderr, "sys_count %d start system %d\n", 
	    //   sys_count,f->start_system );

	    if ( sys_count  >= f->start_system ) {

	      if (f->flags & NOTES) {        /* thick vert line at beginning */
		if (f->m_flags & TWOSTAFF ) {
		  p->push();
		  // note 7 * interspace gives room 
		  // for sharp and flat above staff
		  p->movev(
			   10 * m_space + 2 * text_sp * f->n_text +
			   2 * f->c_space  + 7 * str_to_inch(interspace) 
			   + flag_h + str_to_inch(flag_to_staff) -.011
			   - (f->line_flag == ON_LINE ? 
			      str_to_inch(interspace)/2 : 0.0 ));
		  p->put_rule(0.01, 
			      ( 10 * m_space + 2 * text_sp * f->n_text +
				2 * f->c_space  + 7 * str_to_inch(interspace) 
				+ flag_h + str_to_inch(flag_to_staff) -.01
				- (f->line_flag == ON_LINE ? 
				   str_to_inch(interspace)/2 : 0.0 )));
		  /* for a double line before the staves 
		     p->moveh(-0.06);
		     p->put_rule(0.03, 
		     ( 10 * m_space + 2 * text_sp * f->n_text +
		     2 * f->c_space  + 7 * str_to_inch(interspace) 
		     + flag_h + str_to_inch(flag_to_staff)
		     - (f->line_flag == ON_LINE ? 
		     str_to_inch(interspace)/2 : 0.0 )));
		  */
		  p->pop();
		  for (i=0; i< 5; i++ ) {
		    p->put_rule(staff_len, str_to_inch(staff_height));
		    p->movev(m_space);
		  }	

		  p->movev( text_sp * f->n_text + f->c_space); 
		  p->movev( str_to_inch(interspace) );
		}
		else {  /* just one line of music, thick vert line */
		  p->push();
		  p->movev(
			   5 * m_space + text_sp * f->n_text +
			   f->c_space  + 6 * str_to_inch(interspace)
			   + flag_h + str_to_inch(flag_to_staff) 
			   -.011 
			   - (f->line_flag == ON_LINE ? 
			      str_to_inch(interspace)/2 : 0.0 ));
		  p->put_rule( 0.03,
			       5 * m_space + text_sp * f->n_text +
			       f->c_space  + 6 * str_to_inch(interspace)
			       + flag_h + str_to_inch(flag_to_staff) -.011
			       - (f->line_flag == ON_LINE ? 
				  str_to_inch(interspace)/2 : 0.0 ));
		  p->pop();
		}
		for (i=0; i< 5; i++ ) {
		  p->put_rule(staff_len, str_to_inch(staff_height));
		  p->movev(m_space);
		}	
		p->movev( st_text);
		p->movev( text_sp * f->n_text + f->c_space); 
	      }
	      else if (f->flags & ROTATE  && baroque) {
		p->movev(flag_h );
		if (red < .9999) {
		  p->movev("3 pt");
		}
	      }
	      else {		/* compensate for flag height here */
		p->movev(flag_h + str_to_inch(flag_to_staff));
	      }

	      printsystem(p, i_b, f_a, &l_p[sys_count], f);
	    }

	    if ( r == END_OK ) {
		return_val = END_OK;
		goto leave;
	    }

	    staff_page--;
	    one_sys_done++;
	    sys_count++;

	    if (f->flags & MARKS)
		p->put_rule(str_to_inch("1 in"), str_to_inch("0.02 in")); 

	    if (f->flags & SAMELINE){
		f->flags &= ~SAMELINE;
		hor_offset = 6.5 / 2.0;
		p->moveh(hor_offset);
	    }
	    else if (f->flags & MANUSCRIPT ) {
		p->moveh( -1.0 * hor_offset);
		hor_offset = 0.0;
		p->movev(7 * str_to_inch(mus_space)); /* middle c to low c */
		p->movev(.3);
		if (f->flags & MARKS)
		    p->put_rule(str_to_inch(".3 in"), str_to_inch("0.05 in")); 
	    }
	    else {		/* not Manuscript */
		int num_lines = 6;
		if (f->flags & FIVE) num_lines = 5;
		if (f->flags & FOUR) num_lines = 4;
		p->moveh( -1.0 * hor_offset);
		hor_offset = 0.0;

		staff_space = num_lines * str_to_inch(interspace) /* to bottom */
		    + str_to_inch("0.33 in");

		if (f->sys_skip > 0.0) staff_space += f->sys_skip;
		if (f->flags & LSA_FORM ) staff_space += .5;
		if (f->flags & JAM ) staff_space -= .1;
		if (f->flags & ROTATE  && baroque)staff_space -= .1;

		p->movev(staff_space);    /* only place staffspace is used */
		/* staffspace is percieved space between  */
		/* bot line of system, top of flags */
		if (f->flags & MARKS)
		    p->put_rule(str_to_inch(".3 in"), str_to_inch("0.05 in")); 
	    }

	    if (!(f->flags & NOTES) && f->flags & TXT ) {
		/* text no music */
		p->movev( str_to_inch(interspace)); /* only for 7th course! */
		p->movev( f->n_text * text_sp );
	    } 

	    if (f->flags & NOTES) f->flags &= ~NOTES;
	    if (f->flags & TXT) f->flags &= ~TXT;
	    f->n_text = 0;
	}
    }
    return_val = END_OK;
leave:
    sys_count = 0;
    one_sys_done = 0;
    title_done = 0;
    tit_last = 0;
    if (f->flags & PAGENUM && f->flags && ROTATE 
	&& red == 0.94440) {
	p->pop();		/* end shift whole page for book */
    }
    return (return_val);
}


/* draw a blank staff 
 */

static int i_space = 0;
extern char mus_space[];

void do_system(print *p, struct file_info *f)
{
    int lines = 6;
    int i;

    if (i_space == 0 ) i_space = str_to_dvi(interspace);

    p->push();

    if (f->flags & FIVE || f->flags &  MANUSCRIPT) lines = 5;
    if (f->flags & FOUR) lines = 4;
    if (f->m_flags & SEVEN) lines = 7;
    if (f->flags &  MANUSCRIPT) i_space = str_to_dvi(mus_space);

    if (f->line_flag == ON_LINE) p->p_movev(i_space / 2);
    else p->p_movev(i_space);		/* go beyond top line */

    if (f->flags & CON_SEV ){ /* 7 line italian conversion */
	p->p_movev(i_space);
    }

    for (i=0; i< lines; i++ ) {
	p->put_rule(staff_len, str_to_inch(staff_height));
	if ( i != lines -1) p->p_movev(i_space);
    }		
    p->pop();
}

signed char getisystem(i_buf *i_b, font_list *f_a[], 
		       int *l_p, struct file_info *f)
{
    signed char c;

//    dbg0(Flow, "getisystem: \n");

    *l_p = 0;
    while (( c = getiline(i_b, f_a, l_p, f)) != END_SYSTEM && c != END_FILE ) 
      ;

    if (c == END_FILE) return(END_OK);
    else return (END_MORE);		/* more to come */
}

int got_text = 0;		/* for multiple lines of text */

signed char getiline(i_buf *i_b, font_list *f_a[], 
		     int *l_p, struct file_info *f)
{
    int i=0;
    signed char c;
    static struct list *l;
    char cmdbuf[BUFSIZ], *p;
    extern struct list *listh;
    int font=text_font;
    
    got_text = 0;		

    // a new listh is allocated for each line

    if (listh == NULL) {
	listh = (struct list *) malloc (sizeof(struct list));
	if (listh == NULL) dbg0(Error, "tab: Malloc failed in getiline\n");
	l = listh;
	l->next = NULL;
	l->prev = NULL;
    }
    else {
	l->next = (struct list *) malloc (sizeof(struct list));
	if (! l->next) {
	    dbg0 (Error, "tab: bad malloc in getiline\n");
	}
	l->next->prev = l;
	l = l->next;
    }

    l->next = NULL;
    l->space= 0.0;
    l->padding = 0;
    l->weight = 0.0;
    memset(l->dat, 0, sizeof (l->dat));

    l->notes   = NULL;
    l->notes2  = NULL;
    l->text    = NULL;
    l->text2   = NULL;
    l->special = NULL;

    do {                      /* get a line */
	c = i_b->GetByte();

	if ( i == 0 ) {
//	    dbg1(Flow, "getiline: new line %c\n", (void *)c);
	    switch (c) {
	    case '-':		/* command options */
		p = cmdbuf;
		*p = '-';
		p++;
		while((*p = i_b->GetByte()) != NEWLINE)
		    p++;
		args_from_string(cmdbuf, f);
		l->prev->next = NULL;
		l = l->prev;
		return (NEWLINE);
	    case EOF:
	    case 'e':
		l = l->prev;
		free (l->next);
		l->next = NULL;
		return END_FILE;
/* the following represents end of the system, something different follows */
	    case NEWLINE: 	/* end of system */
	    case '{':		/* text */
	        l = l->prev;
		free (l->next);
	        l->next = NULL;
	        i_b->unGet(c);
		return(END_SYSTEM);
	    case '%':		/* shouldn't have a comment here! */
		dbg0(Warning, "tab: getiline: comment in intermediate file!\n");
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
		c = NEWLINE;	/* wbc was a blank */
		l->notes = (struct notes *)malloc(sizeof(struct notes));
		if (l->notes == NULL)
		    dbg0(Error, "tab_p; bad malloc\n");
		l->notes->parent = l;
		d = i_b->GetByte();
		l->notes->time  = d;
		d = i_b->GetByte();
		l->notes->note  = d;
		if (l->notes->note == 'C') 
		    f->c_space = my_max(f->c_space, 0.035);
		d = i_b->GetByte();
		l->notes->sharp = d;
		l->notes->special = '\0';
		l->notes->staff = 1;
		l->notes->reversed = 0;

		if (( d = i_b->GetByte() ) == 'r' || d == '#' 
		    || d == 'x' || d == '-' || d == '=') {
		    l->notes->special = d;
		    d = i_b->GetByte();
		    if (d == 'M') 
		      goto music2;
		    if (d != 'T')
			break;
		}                   /* else fall through */
		else if (d == NEWLINE) {
		    break;
		}
		else if (d == 'M') {
		music2:
		    l->notes2 = (struct notes *)malloc(sizeof(struct notes));
		    if (l->notes2 == NULL)
			dbg0(Error, "tab_p; bad malloc\n");
		    l->notes2->parent = l;
		    d = i_b->GetByte();
		    l->notes2->time  = d;
		    d = i_b->GetByte();
		    l->notes2->note  = d;
		    if (l->notes2->note == 'C') 
		      f->c_space = my_max(f->c_space, 0.035);
		    d = i_b->GetByte();
		    l->notes2->sharp = d;
		    l->notes2->special = '\0';
		    l->notes2->staff = 2;
		    l->notes2->reversed = 0;

		    if (( d = i_b->GetByte() ) == 'r' || d == '#' 
			|| d == 'x' || d == '-' || d == '=') {
			l->notes2->special = d;
		    }
		    else 	
			i_b->unGet(d);

		    d = i_b->GetByte();
		    if (d != 'T') {
			i_b->unGet(d);
			//				break;
		    }
		}	                   /* else fall through */
		else if (d != 'T') {
		  break;
		}
	    case 'T':
		{
		    int ret;
		    d = i_b->GetByte();
		    if (d == '^')
		      get_text(l, &l->text2, i_b, f_a, f, font);
		    else if (d == '\n') ;
		    else {
		      i_b->unGet(d);
		      ret = get_text(l, &l->text, i_b, f_a, f, font);
		      if (ret)  get_text(l, &l->text2, i_b, f_a, f, font);
		    }
		}
		c = NEWLINE ;
		break;
	    default:
		break;
	    }
	} /* if i == staff */
	if (i < STAFF ) l->dat[i] = c;
	if (l->special) l->special[i] = c;
	i++;
    } while (c  != EOF && c != NEWLINE && c != '\0'); 

				// I don't think this
				// does a whole lot.. Jan 30 97
    if (l->special) {
	if (l->special[i-1] == NEWLINE ) l->special[i-1] ='\0';
	else l->special[i] ='\0';	/* strip of newline */
	if (1 || f->flags & PS) {
	    char *p;
	    char cc[2];
	    int fontnum;
	    
	    p=&(l->special[2]);

	    cc[0] = *p;
	    cc[1] = '\0';
	    fontnum = atoi(cc);
//	    printf("getisystem: p is %d\n", fontnum);
	    switch (fontnum) {
	      case 1:
	      case 5:
		text_font = fontnum;
		font = text_font;
		break;
	      case 2:
	      case 3:
		title_font = fontnum;
		break;
	    }
	}
    }
    l->dat[STAFF+1] = '\0';
//    dbg1(Flow, "tab: getiline returns: l->dat %s\n", l->dat);
    *l_p = *l_p + 1;

    return (c);
}

int setflag(file_info *f, char * string, pass pass) 
{
    int i;
    char *p=string;
    char arg[40], val[80];

    for (i=0;*p != '\0' && *p != NEWLINE && *p != '='; p++, i++) {
	arg[i] = *p;
    }
    //   if (pass == first)
    //     dbg0 (Warning, "setflag: first\n");
    //  else
    //   dbg0 (Warning, "setflag: second\n");

    arg[i] = '\0';
    p++;
    //    dbg1 (Warning, "setflag: %s = ", (void *)arg);

    for (i=0 ;*p != '\0' && *p != NEWLINE; p++, i++) {
	val[i] = *p;
    }
    val[i] = '\0';
    //    dbg1 (Warning, "%s\n", (void *)val);
    return (set_string(arg, val, f, pass));
}


int get_text(struct list *l, struct text **textt, 
	     i_buf *i_b, font_list *f_a[], 
	     struct file_info *f, int font)
{
    text *text;
    char *p, *pp=0, buff[256];
    int n = 0;
    double tmp_text=0.0;
    struct t_words *tp;
    int more=0;

    /* initialize a text buffer */

    text = (struct text *)malloc(sizeof (struct text));
    *textt = text;
    text->size = 0.0; 
    text->parent = l;
    text->length = 0;
    text->words = /* grab first buffer */
	(struct t_words *)malloc(sizeof (struct t_words));
    tp = text->words;
    tp->next = NULL;
    tp->prev = NULL;
    tp->nospace = 0;
    tp->in_sys = 0;

    p = buff;
    while (tp) {
	while (1) {
	    *p = i_b->GetByte();
	    if (*p == NEWLINE || *p == '^') { /* end of text, */
		
		if (*p == '^')
		    more=1;
		*p = '\0'; /*  end of this chord */
		if (pp && *pp == '|' ) {
		    tp->nospace = 1;
		    tmp_text -= 
			f_a[font]->fnt->get_width('|');
		    *pp = ' ';
		}
		else if (pp && *pp != '-') {
		    tmp_text += 
			f_a[font]->fnt->get_width('-');	// was '-'
		}
		break;
	    }
	    else if (*p == '\t') {
		got_text++;
		*p = '\0'; /* erase tab */
		if (pp && *pp == '|' ) {
		    tmp_text -= f_a[font]->fnt->get_width('|');
		    tp->nospace = 1;
		}
		else if (pp && *pp != '-') {
		    tmp_text += f_a[font]->fnt->get_width('-');
		}
		p++;

		tp->next = (struct t_words *)
		    malloc(sizeof (struct t_words));
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
		    tmp_text += f_a[0]->fnt->get_width(0201);
		    if (p != buff && *(p-2) == '<')
			/* a tie after an ' accent */
			tmp_text += 0.2 *
			    f_a[0]->fnt->get_width(0201);
		    break;
		case '\\': /* do nothing */
		    tmp_text += 
			get_special_width(&p, i_b, 
					  f_a[font], f);
		    //p++;p++;
		    n++;n++;
		    break;
		case '?': /* replace with upside down ?? */
		{
		    tmp_text += 
			f_a[font]->fnt->get_width(*p);
		    char c = i_b->GetByte();
		    if ( c == '`') *p = 0076;
		    else i_b->unGet(c);
		}
		break;
		case '!': /* replace with upside down ?? */
		{
		    tmp_text += 
			f_a[font]->fnt->get_width(*p);
		    char c = i_b->GetByte();
		    if ( c == '`') *p = 0074;
		    else i_b->unGet(c);
		}
		break;
		case 'f': /* replace with ff with ff */
		{
		    char c = i_b->GetByte();
		    if (f->flags & DVI_O &&	
			c == 'f') *p = 0013; /* ff */
		    else if ( c == 'i') { /* fi */
			*p = 0014; 
			tmp_text += 
			    f_a[font]->fnt->get_width(0015);
		    }
		    else if ( c == 'l') { /* fl */
			*p = 0015;
			tmp_text += 
			    f_a[font]->fnt->get_width(*p);
		    }
		    else {
			i_b->unGet(c);
			tmp_text += 
			    f_a[font]->fnt->get_width(*p);
		    }
		}
		break;
		default:
		    tmp_text += f_a[font]->fnt->get_width(*p);
		    break;
		}
	    }
	    pp = p;
	    p++; n++;
	    if ( n > sizeof(buff) ) 
		break;
	}
	/* get the maximum */
	text->size = 
	    text->size > tmp_text ? 
	    text->size : tmp_text;
	text->length = 
	    text->length > n ?
	    text->length : n;
			
	tp->words = (char *)malloc(n+2);
	dbg1(Inter, "tab_p: get_text: text: %s\n", buff);
	strncpy(tp->words, buff, n);
	tp->words[n] = '\0';
	tp->length = n;
	tp->size = tmp_text;
	tp = tp->next;
			
	tmp_text = 0.0;
	n = 0;
	p = buff;
    }
    if ((got_text + 1) > f->n_text) 
	f->n_text = got_text + 1;
    //        printf ("get text: length %d size %f %s\n", 
    //	text->length, text->size, buff);
    if ( text->length == 0 ) {
      *textt = NULL;
      free(text->words);
      free(text);
      text = NULL;
    }
    if (more) return (1);
    else return (0);
}

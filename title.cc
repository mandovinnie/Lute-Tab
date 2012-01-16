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
#include "system.h"
#include "print.h"
#include "tfm.h"
#include "i_buf.h"

extern int line;
static int centerline;
int title_font = 2;
int text_font = 1;

/* used in get_misc */
#define TITLE 1
#define ARGS 2

/* LOCAL */

void get_misc(int flag, dvi_print *p, i_buf *i_b);
void format_title(print *p, i_buf *i_b, font_list *f_a[], struct file_info *f);

#define EN 'I'
#define LLINE 256

void
format_title(print *p, i_buf *i_b, font_list *f_a[], struct file_info *f)
{
    char c, cc;
    double cur = 0.0;		/* current position */
    int font = title_font;
    double en = f_a[font]->fnt->get_width(EN);
    char * l_bp=NULL;
    //    int i;
    char bbuf[LLINE];
    //    double b_total=0.0;
    char *b_bp=&bbuf[0];
    int italic = 1;		/* for wallace titles */

    centerline = 0;

    if (f->m_flags & ALTTITLE) {
      //      printf("using alternate font\n");
      font = 3;
    }

    p->use_font(font);

    p->push();

    while ((*b_bp = i_b->GetByte()) != (char)EOF) { /* get the text */
	/*	printf ("title: %c %2d\n", *b_bp, *b_bp);*/
	//	if (f_a[0]->fnt->is_defined(*b_bp))
	  //		printf(" char %d is defined\n", *b_bp);
	switch (c=*b_bp) {
	  case '^':
	    {
		char c[2];
		c[0] =  i_b->GetByte();
		c[1] =  i_b->GetByte();
		title_font = font = atoi(c);
		//		dbg1(Warning, "title %d\n", (void*)font);
		p->use_font(font);
	    }
	    break;
	  case 0:
	    goto done;
	    //	    break;
	  case ' ':
	    p->moveh(en);
	    cur += en;
	    break;
	  case '/':		/* lots of hfill */
	    *b_bp = '\0';	/* we are at the end of the first half */

	    if ((c = i_b->GetByte()) != '/' && c != (char)EOF) {
		char lbuf[LLINE];
		double total=0.0;
		l_bp = &lbuf[0];
		int i = 0;

		*l_bp = c;

		while ( *l_bp != (char)EOF && i++ < sizeof(lbuf) ) {
		    if (*l_bp == '}') break;
		    switch (*l_bp) {
		      case ' ':
		      case '\t':
			total += en;
			*l_bp = ' ';
			break;
		      case ']':
			total += f_a[0]->fnt->get_width(0201);
			break;
		      case '|':
			total += 0;
			break;
		      case '?': /* replace with upside down ?? */
			{
			    total += 
			      f_a[font]->fnt->get_width(*l_bp);
			    char c = i_b->GetByte();
			    if ( c == '`') *l_bp = 0076;
			    else i_b->unGet(c);
			}
			break;
		      case '!': /* replace with upside down ?? */
			{
			    total += 
			      f_a[font]->fnt->get_width(*l_bp);
			    char c = i_b->GetByte();
			    if ( c == '`') *l_bp = 0074;
			    else i_b->unGet(c);
			}
			break;
		      case 'f': /* replace with ff with ff */
			{
			    char c = i_b->GetByte();
			    if (f->flags & DVI_O ) {
				if ( c == 'f') *l_bp = 0013;
				else if ( c == 'i') *l_bp = 0014;
				else if ( c == 'l') *l_bp = 0015;
				else i_b->unGet(c);
			    }
			    else if (f->flags & PS ){
				if ( c == 'i') *l_bp = (char)0256;
				else if ( c == 'l') *l_bp = (char)0257;
				else i_b->unGet(c);
			    }
			    total += 
			      f_a[font]->fnt->get_width(*l_bp);
			}
			break;
		      case '\\':
			total += 
			  get_special_width(&l_bp, i_b, f_a[font], f);
			break;
		      default:
			total += f_a[font]->fnt->get_width(*l_bp);
			break;
		    }
		    l_bp++;
		    *l_bp = i_b->GetByte();
		}
		*l_bp = '\0';

//		printf("title staff_len %f total %f cur %f\n",
//		       staff_len, total,  cur);
//		printf("title: %s\n", lbuf);
		if (centerline) {
		    p->moveh ( (staff_len - total - cur)/2.0);
		    centerline = 0;
		}
		else 
		  p->moveh (staff_len - total - cur);

		l_bp = &lbuf[0];

		while (*l_bp) {
		    switch (*l_bp) {
		    case ' ':
			p->moveh(en);
			break;
		    case ']':	/* and this one */
			p->use_font(0);
			p->set_a_char(0201);
			p->use_font(font);
			break;
		    case '}':	// end of text
			break;
		    case '\\':
			print_special_char(&l_bp, p, i_b, 
					   f_a[font], f, italic);
			break;
		    default:
			p->set_a_char(*l_bp);
			break;
		    }
		    l_bp++;
		}
		goto done;
	    }
	    else p->set_a_char ('/');
	    break;
	  case NEWLINE :
	    //	    printf("title: newline\n");
	    p->pop();
	    b_bp=&bbuf[0];
	    p->movev("14.0 pt");
	    p->push();
	    break;
	  case '}':
	    goto done;
	  case ']':		/* word tie */
	    p->use_font(0);
	    p->set_a_char(0201);
	    cur += f_a[0]->fnt->get_width(0201);
	    p->use_font(font);
	    p->set_a_char(cc=i_b->GetByte());
	    cur += f_a[font]->fnt->get_width(cc);
	    break;
	  case '?':		/* replace with upside down ?? */
	    {
		cur += 
		  f_a[font]->fnt->get_width(*b_bp);
		char cc = i_b->GetByte();
		if ( cc == '`') *b_bp = 0076;
		else i_b->unGet(cc);
		p->set_a_char(*b_bp);
	    }
	    break;
	  case '!':		/* replace with upside down !! */
	    {
		cur += 
		  f_a[font]->fnt->get_width(*b_bp);
		char c = i_b->GetByte();
		if ( c == '`') {
		    *b_bp = 0074;
		}
		else i_b->unGet(c);
		p->set_a_char(*b_bp);
	    }
	    break;
	  case 'f':		/* replace with ff with ff */
	    {
		char cf = i_b->GetByte();
		if (cf == 'f' && f->flags & DVI_O ) {
		    *b_bp = 0013;
		}
		else if ( cf == 'i') {
		    if (f->flags & PS) *b_bp = (char)0256;
		    else *b_bp = 0014;
		}
		else if ( cf == 'l') {
		    if (f->flags & PS) *b_bp = (char)0257;
		    else *b_bp = 0015;
		}
		else 
		  i_b->unGet(cf);

		cur += 
		  f_a[font]->fnt->get_width(*b_bp);
		p->set_a_char(*b_bp);
	    }
	    break;
	  case '\\':
	    cur += get_special_width(&b_bp, i_b, f_a[font], f);
	    b_bp--;
	    b_bp--;
	    print_special_char(&b_bp, p, i_b, f_a[font], 
			       f, italic);
	    break;	
	  default:
	    p->set_a_char(*b_bp);
	    cur += f_a[font]->fnt->get_width(*b_bp); 
//	    printf ("cur %f\n", cur);
	    b_bp++;
	    break;
	}
    }
    *b_bp = '\0';
  done:
				/* print the first half */
    if (*b_bp != (char)EOF) {
	while ((*b_bp = i_b->GetByte()) != (char)NEWLINE &&
	      *b_bp != (char)EOF ) /* swallow line */
	  ;
    }
    p->pop();
    //    exit;
    return;
}

void
dotitle(print *p, i_buf *i_b, font_list *f_a[], struct file_info *f)
{
    i_b->GetByte();			/* throw away first { */

    // wbc oct 99    n_measures = 0;
    
    format_title(p, i_b, f_a, f);
    if (f->flags & MARKS)
      p->put_rule("0.25 in", "0.02 in");
    return;
}


void do_sp(unsigned char c, int font, print *p /* , struct file_info *f */)
{
    switch (c) {
      case '@':			/*  ./. */
	p->use_font(0);
	p->set_a_char(57);
	p->use_font(font);
	break;
      default:
	break;
    }
}

double special(char **pp, print *p, i_buf *i_b, font_list *f_a, int print,
	       struct file_info *f, int italic);
double special(char **pp, print *p, i_buf *i_b, font_list *f_a, int print,
	       struct file_info *f, int italic)
{
  unsigned char c, d;
  int dontset=0;

  if (**pp == '\\') {		/* slash, special, character */
    *++*pp;
    if (print) {
      c = (unsigned char)**pp;		/* special */
      *++*pp;
      d = (unsigned char)**pp;		/* character */
    } 
    else {
      **pp = (char)i_b->GetByte();
      c = (unsigned char)**pp;		/* special */
      *++*pp;
      **pp = (char)i_b->GetByte();
      d = (unsigned char)**pp;		/* character */
    }
    if (c == 's' && d == 'l' ) {/* long s */
      if (f->flags & PS) d = 0246; 
      else d = 's';
//	    printf("long s %f\n", f_a->fnt->get_width(d));
    }
    else if (c == 's' && d == 's') d = 0031; /* fs */
    else if (c == 'a' && d == 'e') d = 0032; /* ae */
    else if (c == 'A' && d == 'E') d = 0035; /* AE */
    else if (c == 'o' && d == 'e') d = 0033; /* oe */
    else if (c == 'O' && d == 'E') d = 0036; /* OE */
    else if (c == 'o' && d == 'o') d = 0034; /* orsted */
    else if (c == 'O' && d == 'O') d = 0037; /* Orsted */
    else if (c == ')') {	/* backwards compatibility */
      if (print) *--*pp;
      else i_b->unGet(d);
      d = c;
    }
    else if (c == '(') {
      if (print) *--*pp;
      else i_b->unGet(d);
      d = c;
    }
    else if (c == ']' || c == '[') {
      if (print) *--*pp;
      else i_b->unGet(d);
      d = c;
    }
    else if (c == '}' || c == '{') {
      if (print) *--*pp;
      else i_b->unGet(d);
      d = c;
    }

    if (print) {
      p->push();
      if (c == '`' ) {
	if (isupper(d))
	  p->movev(f_a->fnt->get_width('e') 
		   - f_a->fnt->get_width('E'));
	if (f->flags & PS) {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width(022))/2.5);
	  p->put_a_char(0301);
	}
	else {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	  p->put_a_char(0022);
	}
      }
      else if (c == '\'' ) {
	if (isupper(d))
	  p->movev(f_a->fnt->get_width('e') 
		   - f_a->fnt->get_width('E'));
	if (f->flags & PS) {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width(023))/2.5);
	  p->put_a_char(0302);
	}
	else {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	  p->put_a_char(0023);
	}
      }
      else if (c == '^' ) {
	if (isupper(d))
	  p->movev(f_a->fnt->get_width('e') 
		   - f_a->fnt->get_width('E'));
	if (f->flags & PS) {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width(0136))/2.5);
	  p->put_a_char(0303);
	}
	else {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	  p->put_a_char(0136);
	}
      }
      else if (c == '"' ) {
	//		printf("here\n");
	if (isupper(d))
	  p->movev(f_a->fnt->get_height('e') 
		   - f_a->fnt->get_height('E'));
	if (f->flags & PS) {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width(0022))/2.5);
	  p->put_a_char(0310);
	}
	else {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	  p->put_a_char(0177);
	}
      }
      else if (c == '~' ) {
	if (isupper(d))
	  p->movev(f_a->fnt->get_height('e') 
		   - f_a->fnt->get_height('E'));
	if (f->flags & PS) {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width(0022))/2.);
	  p->put_a_char(0304);
	}
	else {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	  p->put_a_char(0176);
	}
      }
      else if (c == '=' ) {
	if (isupper(d))
	  p->movev(f_a->fnt->get_height('e') 
		   - f_a->fnt->get_height('E'));
	if (f->flags & PS) {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width(0022))/2.5);
	  p->put_a_char(0305);
	}
	else {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	  p->put_a_char(0026);
	}
      }
      else if (c == '.' ) {
	if (isupper(d))
	  p->movev(f_a->fnt->get_width('e') 
		   - f_a->fnt->get_width('E'));
	if (f->flags & PS) {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width(0022))/2.5);
	  p->put_a_char(0307);
	}
	else {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	  p->put_a_char(0137);
	}
      }
      else if (c == 'u' ) {
	if (isupper(d))
	  p->movev(f_a->fnt->get_width('e') 
		   - f_a->fnt->get_width('E'));
	if (f->flags & PS) {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width(0022))/2.5);
	  p->put_a_char(0306);
	}
	else {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	  p->put_a_char(0025);
	}
      }
      else if (c == 'v' ) {
	if (isupper(d))
	  p->movev(f_a->fnt->get_width('e') 
		   - f_a->fnt->get_width('E'));
	if (f->flags & PS) {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width(0022))/2.5);
	  p->put_a_char(0317);
	}
	else {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	  p->put_a_char(0024);
	}
      }
      else if (c == 'H' ) {
	if (isupper(d))
	  p->movev(f_a->fnt->get_width('e') 
		   - f_a->fnt->get_width('E'));
	if (f->flags & PS) {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width(0022))/2.5);
	  p->put_a_char(0315);
	}
	else {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	  p->put_a_char(0175);
	}
      }
      else if (c == 'c' ) {    	/* cedilla */
	if (f->flags & PS) {
	  p->moveh ((f_a->fnt->get_width('c')
		     /* - f_a->fnt->get_width(0030) */ )/4.1 /* 2.0 */);
	  p->put_a_char(0313);
	}
	else {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	  p->put_a_char(0030);
	}
	dontset++;
      }
      else if ( c == 'C') {        /* center line in title */
	centerline = 1;
	dontset++;
      }
      else if (c == ']') {
	p->set_a_char(']');
	//		p->moveh (f_a->fnt->get_width('{'));
	dontset++;
      }
      else if (c == '[') {
	p->set_a_char('[');
	dontset++;
      }
      else if (c == '{') {
	p->set_a_char('{');
	//		p->moveh (f_a->fnt->get_width('{'));
	dontset++;
      }
      else if (c == '}') {
	p->set_a_char('}');
	dontset++;
      }
      else if ( c != 'a' && c != 'A' && c != 'e' && c != 'E'
		&& c != 's' && c != 'o' && c != 'O' 
		&& c != '(' && c != ')' && c != ' ') {
	if (! (f->m_flags & QUIET) )dbg2(Warning, 
					 "tab: special: unknown character %c %d\n", 
					 (void *)c, (void *)c );
      }
      p->pop();
	    
      /* the tex to ps char substitution is now in set_a_char */

      if (! dontset) { /* print */
	if (d == 'i') {
	  if (f->flags & PS) {
	    /*			int high = f_a->fnt->p_get_h(d);
				p->ps_clipped_i(d, high); 
	    */
	    p->set_a_char(0365);
	  } 
	  else p->set_a_char(020); /* dotless i */
	}
	else if (d == 'j') {
	  if (f->flags & PS) {
	    int high = f_a->fnt->p_get_h(d);
	    p->ps_clipped(d, high);
	  }
	  else p->set_a_char(021); /* dotless j */
	}
	else p->set_a_char(d);
      }
      else dontset = 0;
    }
    if (d == 0246) 
      return (f_a->fnt->get_width(d));
    else if ( c == 'c' || c == ']') {
      return /* (f_a->fnt->get_width(c)) */  (0.0);
    }
    else 
      return (f_a->fnt->get_width(d));
  }
  /* else not a '\\' */
  else {
    dbg0(Error, "tab: special: called without backslash\n");
    return (0.0);
  }
}

double get_special_width(char **pp, i_buf *i_b, font_list *f_a,
struct file_info *f)
{
  print *p=NULL; 
  return (special(pp, p, i_b, f_a, 0, f, 0));
}
double print_special_char(char **pp, print *p, i_buf *i_b, 
   font_list *f_a,struct file_info *f, int italic)
{ 
  return (special(pp, p, i_b, f_a, 1, f, 0)); 
}

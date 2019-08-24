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
#include "system.h"
#include "print.h"
#include "tfm.h"
#include "i_buf.h"
#if defined _WIN32
#define bzero(s,n) memset((s), 0, (n))
#endif /* WIN32 */

extern int line;
static int centerline;
int title_font = 2;
int text_font = 1;
double gap=0.0;

/* used in get_misc */
#define TITLE 1
#define ARGS 2

/* LOCAL */

void get_misc(int flag, dvi_print *p, i_buf *i_b);
void format_title(print *p, i_buf *i_b, font_list *f_a[], struct file_info *f);

double special(char **pp, print *p, i_buf *i_b, font_list *f_a[], int font, int print,
	       struct file_info *f, int italic);

#define EN 'I'
#define LLINE 512

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
    char *b_bp=&bbuf[0];
    int italic = 1;		/* for wallace titles */
    static char *tp;			// title pointer
    static char title_done=0;		// this seems to be for midi titles
    double font_scale = f->font_sizes[font]/12.0;  // wbc apr 17

    centerline = 0;
    bzero(bbuf, LLINE);

    if (f->m_flags & ALTTITLE) {
      //      printf("using alternate font\n");
      font = 3;
    }

    p->use_font(font);

    p->push();

    while ((*b_bp = i_b->GetByte()) != (char)EOF) { /* get the text */
      if (f->m_flags & NMIDI & ! title_done) {
	if (*b_bp == '}') {
	  p->pop();
	  *tp++ = 0;
	  title_done = 1;
	  return;
	}
	if (! f->title) {
	  f->title = (char *)malloc(LLINE);
	  tp = f->title;
	}
	*tp++ = *b_bp;
	//	printf("%c", *b_bp);
      }
      else {
	//  printf ("title: %c %2d\n", *b_bp, *b_bp);
	//	if (f_a[0]->fnt->is_defined(*b_bp))
	//		printf(" char %d is defined\n", *b_bp);

	//	printf ("--first pass character %c\n", *b_bp);
	switch (c=*b_bp) {
	case '^':
	  {
	    char c[2];
	    c[0] =  i_b->GetByte();
	    c[1] =  i_b->GetByte();
	    title_font = font = atoi(c);
	    //		dbg1(Warning, "title %d\n", (void*)font);
	    p->use_font(font);
	    font_scale = f->font_sizes[font]/12.0;  //apr 17
	    *b_bp--; // wbc sept 2015
	  }
	  break;
	case ' ':
	  cur += en;
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
	case 0:
	  goto done;
	case '?':		/* replace with upside down ?? */
	  {
	    char cc = i_b->GetByte();
	    cur += font_scale * f_a[font]->fnt->get_width(*b_bp);
	    if ( cc == '`') *b_bp = 0076; //0277 is upside down question
	    else { i_b->unGet(cc);}
	  }
	  break;
	case '!':		/* replace with upside down !! */
	  {
	    char cc = i_b->GetByte();
	    cur += font_scale * f_a[font]->fnt->get_width(*b_bp);
	    if ( cc == '`') *b_bp = 0074; //  0241 is exclaimation
	    else { i_b->unGet(cc); }
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
	      font_scale * f_a[font]->fnt->get_width(*b_bp);
	  }
	  break;
	case '\\':		// special escape
	  {
	    // char *tl;
	    double temp;
	    temp = font_scale * get_special_width(&b_bp, i_b, f_a, font,  f);
	    //	    	    printf("temp is %f\n", temp);
	    // printf("case 2 %c  %s\n", *b_bp, bbuf);
	    cur += temp;
	    //	    b_bp--;
	    // p->put_a_char ('.');
	  }
	  break;
	case ']':		/* word tie - this swallows the next chracter */
	  cc=i_b->GetByte();
	  if (cc == '}' || cc == '/' || cc == ' ') {
	    cur += font_scale * f_a[font]->fnt->get_width(c);
	    i_b->unGet(cc);
	  }
	  else {
	    cur += font_scale * f_a[0]->fnt->get_width(0201);
	    cur += 0.005;	// this is sheer voodo!!
	    i_b->unGet(cc);
	  }
	  break;
	case '/':		/* lots of hfill */

	  // 	printf("title staff_len %f  cur %f\n",
	  //  		       staff_len,  cur);
	  if (centerline) {
	    dbg0(Warning, "centerline and split in the same title line\n");
	  }
	  break;
	default:
	  {
	    cur += font_scale * (f_a[font]->fnt->get_width(c));
	    //	    printf ("title width %f\n",
	    //		   font_scale * (f_a[font]->fnt->get_width(c)));
	    break;
	  }
	}
	b_bp++;
      }
      //            printf("title cur %f\n", cur);
    }
  done:
    b_bp++;
    *b_bp = '\0';
    gap = staff_len - cur;

    //    printf("first pass done ->%s\n", bbuf);

    if (centerline) {
      //      printf("first pass done ->%s gap is %f\n", bbuf, gap);
      p->moveh(gap/2.0);
      centerline = 0;
    }

    b_bp=&bbuf[0];
    while (*b_bp != 0) {
      //      printf ("--second pass character %c\n", *b_bp);
      switch (*b_bp) {
      case ' ':
	p->moveh(en);
	break;
      case '}':
	goto end;
	break;
      case 'f':
	p->set_a_char(*b_bp);
	break;
      case ']':			// word tie
	if (*(b_bp+1) == '}' || *(b_bp+1) == '/' || *(b_bp+1) == ' ' ) {
	  p->set_a_char(*b_bp);
	}
	else {
	  p->use_font(0);
	  p->set_a_char(0201); /* 0201 is the word tie - half circle 129 decimal */
	  p->use_font(font);
	}
	break;
      case '\\':
	print_special_char(&b_bp, p, i_b, f_a, font,
			   f, italic);
	break;
      case '/':
	p->moveh(gap);
	break;
      default:
	p->set_a_char(*b_bp);
	break;
      }
      b_bp++;
    }
 end:
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


double get_special_width(char **pp, i_buf *i_b, font_list *f_a[],
			 int font, struct file_info *f)
{
  print *p=NULL;
  return (special(pp, p, i_b, f_a, font, 0, f, 0));
}
double print_special_char(char **pp, print *p, i_buf *i_b,
			  font_list *f_a[], int font, struct file_info *f, int italic)
{
  return (special(pp, p, i_b, f_a, font, 1, f, 0));
}

double special(char **pp, /* buffer of title */
	       print *p,  /* do we print or return spacing */
	       i_buf *i_b, /* inout stream */
	       font_list *f_a_in[], /* all the fonts */
	       int font, /* the main font used */
	       int print,
	       struct file_info *f, int italic)
{
  unsigned char c, d;
  int dontset=0;
  font_list *f_a = f_a_in[font];
  double font_scale  = f->font_sizes[font]/12.0;

  if (**pp != '\\')  {
    dbg0(Error, "tab: special: called without backslash\n");
    return (0.0);
  }
  else {
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
    }
    else if (c == 's' && d == 's') d = 0031; /* fs */
    else if (c == 'a' && d == 'e') d = 0032; /* ae */
    else if (c == 'A' && d == 'E') d = 0035; /* AE */
    else if (c == 'o' && d == 'e') d = 0033; /* oe */
    else if (c == 'O' && d == 'E') d = 0036; /* OE */
    else if (c == 'o' && d == 'o') d = 0034; /* orsted */
    else if (c == 'O' && d == 'O') d = 0037; /* Orsted */
    else if (c == 'w' && d == 't') d = 0201; /* word tie */

    else if (c == ')') {	/* backwards compatibility */
      if (!print) i_b->unGet(d);
      d = c;
    }
    else if (c == '(') {
      if (!print) i_b->unGet(d);
      d = c;
    }
    else if (c == ']' || c == '[') {
      if (!print) i_b->unGet(d);
      d = c;
    }
    else if (c == '}' || c == '{') {
      if (!print) i_b->unGet(d);
      d = c;
    }
    else if (c == '!' || c == '?') {
      if (!print) i_b->unGet(d);
      d = c;
    }
    else if ( c == 'C') {        /* center line in title */
      if (print) {
	if (d == 'L') {
	  *++*pp;
	}
      }
      else {
	if (d == 'L') {
	  *++*pp;
	  **pp = (char)i_b->GetByte();  // swallow / from \CL/ if necessary
	  if (**pp != '/')
	    dbg0(Warning, "incorrcte Centerline specification\n");
	  *--*pp; *--*pp;
	}
      }
      centerline = 1;
    }
    else if (c != '`' && c != '\'' && c != '^' && c != '"' && c != '~' && c != '.'
	     && c != 'u' && c != 'v' && c != 'H' && c != 'c' && c != 'w' &&
	     c != 'C' && c != '=' ) {
      if (!print) i_b->unGet(d);
       d = c;
    }


    if (print) {
      //      p->push();
      /*
      if (c == '`' ) {
	if (isupper(d))
	  p->movev(f_a->fnt->get_height('e')
		   - f_a->fnt->get_height('E'));
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
      */
      /*
      else if (c == '\'' ) {
	if (isupper(d))
	  p->movev(font_scale * (f_a->fnt->get_width('e')
				 - f_a->fnt->get_height('E')));
	if (f->flags & PS) {
	  p->moveh (font_scale * (f_a->fnt->get_width(d)
		     - f_a->fnt->get_width(023))/2.5);
	  p->put_a_char(0302);
	}
	else {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	  p->put_a_char(0023);
	}
      }
      */
      /*
      else if (c == '^' ) {
	if (isupper(d))
	  p->movev(f_a->fnt->get_width('e')
		   - f_a->fnt->get_width('E'));
	if (f->flags & PS) {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width(0136))/2.5);
	  p->push();
	  p->moveh (0.002);
	  p->put_a_char(0303);
	  p->pop();
	}
	else {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	  p->put_a_char(0136);
	}
      }
      */
      if (c == '"' || c == '^' || c == '\'' ||
	  c == '`' || c == '~' || c == '='  ||
	  c == '.' || c == 'u' || c == 'v'  ||
	  c == 'H' ) {
	int printing;
	switch (c) {
	case '"':
	  if (f->flags & PS) printing = 0310;
	  else printing = 0177;
	  break;
	case '^':
	  if (f->flags & PS) printing = 0303;
	  else printing = 0136;
	  break;
	case '\'':
	  if (f->flags & PS) printing = 0302;
	  else printing = 0023;
	  break;
	case '`':
	  if (f->flags & PS) printing = 0301;
	  else printing = 0022;
	  break;
	case '~':
	  if (f->flags & PS) printing = 0304;
	  else printing = 0176;
	  break;
	case '=':
	  if (f->flags & PS) printing = 0305;
	  else printing = 0026;
	  break;
	case '.':
	  if (f->flags & PS) printing = 0307;
	  else printing = 0137;
	  break;
	case 'u':
	  if (f->flags & PS) printing = 0306;
	  else printing = 0025;
	  break;
	case 'v':
	  if (f->flags & PS) printing = 0317;
	  else printing = 0024;
	  break;
	case 'H':
	  if (f->flags & PS) printing = 0315;
	  else printing = 0175;
	  break;
	}
	if (f->flags & PS) {
	  if (isupper(d))
	    p->movev(font_scale * (f_a->fnt->get_height('e')
				   - f_a->fnt->get_height('E')));
	  p->moveh (font_scale * (f_a->fnt->get_width(d)
				  - f_a->fnt->get_width(0022))/2.0); // was 2.5
	  p->put_a_char(printing);

	  p->moveh (-1 * font_scale * (f_a->fnt->get_width(d)
				       - f_a->fnt->get_width(0022))/2.0);
	  if (isupper(d))
	    p->movev(-1 * font_scale * (f_a->fnt->get_height('e')
					- f_a->fnt->get_height('E')));
	}
	else {
	  if (isupper(d))
	    p->movev( (f_a->fnt->get_height('e')
		       - f_a->fnt->get_height('E')));
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	  p->put_a_char(printing);
	  if (isupper(d))
	    p->movev( -1 * (f_a->fnt->get_height('e')
			    - f_a->fnt->get_height('E')));
	}

      }
      /*      else if (c == '~' ) {
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
      */
      /*     else if (c == '=' ) {
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
	} */
      /*      else if (c == '.' ) {
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
	} */
      /*      else if (c == 'u' ) {
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
	} */
      /*     else if (c == 'v' ) {
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
	}*/
      /*      else if (c == 'H' ) {
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
	}*/
      else if (c == 'c' ) {    	/* cedilla */
	if (f->flags & PS) {
	  //	  p->push();
	  p->moveh ((f_a->fnt->get_width(d))/3.1);
	  p->put_a_char(0313);
	  p->moveh (-1 * (f_a->fnt->get_width(d))/3.1);
	  //	  p->pop();
	  p->put_a_char(d);
	  c = d; /* this fixes the width, which is set below */
	}
	else {
	  p->moveh ((f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	  p->put_a_char(0030);
	  p->moveh (-1 * (f_a->fnt->get_width(d)
		     - f_a->fnt->get_width('e'))/2);
	}
	dontset++;
      }
      else if ( c == 'w') {
	p->use_font(0);
	p->put_a_char(0201); /* 0201 is the word tie - half circle 129 decimal */
	p->use_font(font);
	dontset++;
      }
      else if (c == ']') {
	p->set_a_char(']');
	dontset++;
      }
      else if (c == '[') {
	p->set_a_char('[');
	dontset++;
      }
      else if (c == '{') {
	p->set_a_char('{');
	dontset++;
      }
      else if (c == '}') {
	p->set_a_char('}');
	dontset++;
      }
      else if (c == '!') {
	p->set_a_char(0074);
	dontset++;
      }
      else if (c == '?') {
	p->set_a_char(0076);
	dontset++;
      }
      else if (c == 'C') {	// do nothing
	//	printf("d is %c\n", d);
	if ( d == 'L' ) {
	  dontset++;
	}
      }
      else if ( c != 'a' && c != 'A' && c != 'e' && c != 'E'
		&& c != 's' && c != 'o' && c != 'O'
		&& c != '(' && c != ')' && c != ' '
		&& c != '!' && c != '?' && c != 'w' ) {
	if (! (f->m_flags & QUIET) )dbg2(Warning,
					 "tab: special: unknown character %c %3d, printing it anyways\n",
					 (void *)(int)c, (void *)(int)c );
	p->set_a_char(c);
	dontset++;
      }
      //      p->pop();
      //      if (c == ']' || c == '[' || c == '{' || c == '}')
      if (dontset) { /* single code characters */
	if ( d == 0201) p->moveh (font_scale * f_a_in[0]->fnt->get_width(0201));
	else p->moveh (font_scale * f_a->fnt->get_width(c));

      }
      /* the tex to ps char substitution is now in set_a_char */
      if (! dontset ) { /* print */
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
	    p->print_clipped(d, high);
	  }
	  else p->set_a_char(021); /* dotless j */
	}
	else if (d == 0246 ) {	// long s
	  if (f->flags & PS ) {
	    p->moveh(f_a->fnt->get_kern(d));
	    //	    printf("title - long s kern %f\n", f_a->fnt->get_kern(d));
	    p->set_a_char(0246);
	  }
	  /* dvi unhandled */
	}
	else p->set_a_char(d);
      }
      else dontset = 0;
    }   /* end if print */

    if (d == 0246) {  // long s
      return (f_a->fnt->get_width(d) +  f_a->fnt->get_kern(d));
    }
    else if (  d == 0201 ) {
      return (f_a_in[0]->fnt->get_width(d));
    }
    else if ( c == 'c' ) {	// cedilla
      return (f_a->fnt->get_width(d));
    }
    else if (  c == ']') {
      return (f_a->fnt->get_width(c));
    }
    else if (  c == '[') {
      return (f_a->fnt->get_width(c));
    }
    else if ( c == 'C' && d == 'L' ) {
      return (0);
    }
    else
      return (f_a->fnt->get_width(d));
  }
}

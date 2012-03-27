
#include <stdio.h>
#include <stdlib.h>
#include "tab.h"
#include "ascii.h"
#include "i_buf.h"

ascii::ascii() 
{
  int i, j;
  
  n_lines = LINES;
  n_char  = 132;
  print_n_char  = 80;
  
  // fprintf (stderr, "starting ascii\n");
  
  ascii_h = 0;			// 0 is top line
  ascii_v = 0;
  n_staves = n_lines - 1;

  for (i=0; i < n_lines; i++) { 
    lines[i] = (char *)malloc(n_char+1+64);

    if ( i < 3 ) {      for (j=0; j< n_char; j++) 
	lines[i][j] = ' ';
    }
    else if ( i < n_staves -2 ) {
      for (j=0; j< n_char; j++) 
	lines[i][j] = '_';
    }
    else {
      for (j=0; j< n_char; j++) 
	lines[i][j] = ' ';
    }

    lines[i][n_char] = '\0';

  }
}

ascii::~ascii() 
{
  int i;

  // fprintf (stderr, "ending ascii\n");

  printf ("\n");

  print_n_char = print_n_char< ascii_h ? print_n_char : ascii_h;

  for (i=0; i < n_lines; i++) {

    lines[i][print_n_char] = '\0';

    printf("%s\n", lines[i]);
    free (lines[i]);
  }
}

void 
ascii::put_a_char(unsigned char c)
{
  lines[ascii_v][ascii_h] = c;
}

void 
ascii::put_a_char_next(unsigned char c)
{
  lines[ascii_v][ascii_h+1] = c;
}

void 
ascii::put_a_char_before(unsigned char c)
{
  lines[ascii_v][ascii_h-1] = c;
}
void
ascii::put_barline() 
{
  int i;
  
  for (i=0; i < 4; i++)
    lines[i][ascii_h] = ' ';
  for (; i< n_staves-2; i++) {
    lines[i][ascii_h] = '|';
  }
}

void
ascii::increment_h() {
  ascii_h++;
  if ( ascii_h > n_char ){
    dbg1(Warning, "ascii: character overflow %d\n", (void*)ascii_h);
  }
}

void
ascii::decrement_h() {
  if ( ascii_h > 1 )
    ascii_h--;
}
void
ascii::increment_v() {
  ascii_v++;
  if ( ascii_v > n_lines ){
    dbg1(Warning, "ascii: line overflow", (void*)ascii_h);
  }
}

void ascii::reset_v() { ascii_v = 0;}
void ascii::reset_h() { ascii_h = 0;}
void ascii::set_print_width(const int w) {
  print_n_char = w;
  if (print_n_char > 132)
    dbg1(Error, "ascii: print width set too big", (void*)w);
}

void
do_ascii(struct list *l, struct file_info *f, i_buf *i_b)
{
  char c;
  char *ch = l->dat;
  char cc = *ch;
  ascii *ap = (ascii*)f->utility;
  int i;
  int incr=1;

  // fprintf(stderr, "%sXXX\n", l->dat);

  ap->reset_v();
  if (! l->prev ) 
    ap->reset_h();

  if (f->m_flags & AWIDE)
    ap->set_print_width(132);

  switch (cc) {
  case 'b':
  case 'B':
    ap->put_barline();
    if (l->dat[1] == 'v' ) {
      ap->increment_v();   
      ap->increment_v();   
      ap->put_a_char ('v');
    }
    break;
  case 'y':
  case 'Y':
    if (!(f->flags & BIGNOTES) ) ap->increment_h();
    ap->put_a_char_before('/');
    ap->put_a_char('^');ap->put_a_char_next('\\');ap->increment_v();
    ap->put_a_char('O');ap->put_a_char_next(' ');ap->increment_v();
    ap->put_a_char(' ');
    if (l->dat[1] == '.')
      ap->put_a_char_next('.');
    else
      ap->put_a_char_next(' ');
    ap->increment_v();
    goto Notes;
  case 'W':
    if (!(f->flags & BIGNOTES) ) ap->increment_h();
    ap->put_a_char(' ');ap->put_a_char_next(' ');ap->increment_v();
    ap->put_a_char('O');ap->put_a_char_next(' ');ap->increment_v();
    ap->put_a_char(' ');
    if (l->dat[1] == '.')
      ap->put_a_char_next('.');
    else
      ap->put_a_char_next(' ');
    ap->increment_v();
    goto Notes;
  case 'w':
    if (!(f->flags & BIGNOTES) ) ap->increment_h();
    ap->put_a_char('|');ap->put_a_char_next(' ');ap->increment_v();
    ap->put_a_char('|');ap->put_a_char_next(' ');ap->increment_v();
    ap->put_a_char('O');
    if (l->dat[1] == '.')
      ap->put_a_char_next('.');
    else
     ap->put_a_char_next(' ');
    ap->increment_v();
    goto Notes;
  case '0':
    if (!(f->flags & BIGNOTES) ) ap->increment_h();
    ap->put_a_char('|');ap->put_a_char_next(' ');ap->increment_v();
    ap->put_a_char('|');ap->put_a_char_next(' ');ap->increment_v();
    ap->put_a_char('|');
    ap->put_a_char_next(' ');
    if (l->dat[1] == '.')
      ap->put_a_char_next('.');
    else
     ap->put_a_char_next(' ');
    ap->increment_v();
    goto Notes;
  case '1':
    if (!(f->flags & BIGNOTES) ) ap->increment_h();
    ap->put_a_char('|');ap->put_a_char_next('\\');ap->increment_v();
    ap->put_a_char('|');ap->put_a_char_next(' ');ap->increment_v();
    ap->put_a_char('|');
    if (l->dat[1] == '.')
      ap->put_a_char_next('.');
    else
     ap->put_a_char_next(' ');
    ap->increment_v();
    goto Notes;
  case '2':
    if (!(f->flags & BIGNOTES) ) ap->increment_h();
    ap->put_a_char('|');ap->put_a_char_next('\\');ap->increment_v();
    ap->put_a_char('|');ap->put_a_char_next('\\');ap->increment_v();
    ap->put_a_char('|');
    if (l->dat[1] == '.')
      ap->put_a_char_next('.');
    else
     ap->put_a_char_next(' ');
    ap->increment_v();
    goto Notes;
  case '3':
    if (!(f->flags & BIGNOTES) ) ap->increment_h();
    ap->put_a_char('|');ap->put_a_char_next('\\');ap->increment_v();
    ap->put_a_char('|');ap->put_a_char_next('\\');ap->increment_v();
    ap->put_a_char('|');ap->put_a_char_next('\\');ap->increment_v();
    goto Notes;
  case '4':
    dbg0(Warning, "ascii: ascii does not support 4 or higher flags\n");
    break;
  case 'x':
    if (!(f->flags & BIGNOTES) ) ap->increment_h();
    ap->increment_v();
    ap->increment_v();
    ap->increment_v();
    goto Notes;
  Notes:

    for (i=2 ; i< 9; i++) {
      if (l->dat[i] == ' ')
	;
      else if (l->dat[i] == ':')
	ap->put_a_char('"');
      else if (l->dat[i] == '.')
	ap->put_a_char('*');
      else
	ap->put_a_char(tolower(l->dat[i]));
      ap->increment_v();
    }
    ap->increment_h();
    break;

  case 'Q':
  case 'i':			// in this case, just do
    break;			// the defaullt increment
  case '+':
    ap->increment_v();ap->increment_v();ap->increment_v();
    for (i=2 ; i< 9; i++) {
      if (l->dat[i] == ' ')
	;
      else {
	if (!strchr("{}UX()", l->dat[i]))
	  ap->put_a_char(l->dat[i]);
      }
      ap->increment_v();
    }
    incr=0;
    break;
  case '&':
    ap->increment_v();ap->increment_v();ap->increment_v();
    for (i=2 ; i< 9; i++) {
      if (l->dat[i] == ' ')
	;
      else {
	if (!strchr("{}UX()", l->dat[i]))
	  ap->put_a_char_before(l->dat[i]);
      }
      ap->increment_v();
    }
    break;
  case '.':
    ap->increment_v();ap->increment_v();ap->increment_v();
    ap->increment_v();
    for (i=0; i<5; i++) {
      ap->put_a_char('*');ap->increment_v();
    }
    break;
  case '^':			// we don't do fingerings
    ap->decrement_h();
    break;
  case 'v':
    ap->increment_v();ap->increment_v();
    ap->put_a_char('v');
    break;
  default:
    dbg1(Warning, "ascii: time value %c not defined\n", 
	 (void *)((int)cc));
  }

  if (incr)
    ap->increment_h();
}

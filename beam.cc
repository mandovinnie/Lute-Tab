/* wbc
   april 16 98
   */

#include "win.h"
#include "beam.h"
#include "print.h"
#include "tfm.h"
#include "system.h"
#include "sizes.h"


#define BASE 10			/* base for saves note registers */
extern double stem;
extern double stem_w;

void
do_beam(print *p, font_list *f_a[], struct notes *nn, struct file_info *f)

{
  static beam *beam1=NULL;
  static beam *beam2=NULL;
  beam *b=0, *b_base, *last_b=0;
  int h, v;
  list *nn_p;
  notes* nn_pp;
  double c_total=0;		// average note value
  double count=0;
  int beamup;
  int l_stem;
  static int firsttime;
  int i;
  double bar_space=0.06;
  double slope;

  if (nn->special == '#') {	/* first */
    if (nn->staff == 1 ) {
      if (beam1) { delete beam1;beam1=NULL;}
      beam1 = new beam(nn);
      b = beam1;
    }
    else if (nn->staff == 2 ) {
      if (beam2) { delete beam2;beam2=NULL;}
      beam2 = new beam(nn);
      b = beam2;
    }
    firsttime=nn->time-'0';
  }
  else {
    if (nn->staff == 1 ) {
      b = new beam(beam1, nn);
    }
    else if (nn->staff == 2 ) {
      b = new beam(beam2, nn);
    }
    firsttime = (firsttime > nn->time-'0') ?
      nn->time-'0' : firsttime;
  }

  p->movev(-.043);       /* position up for drawing stem */
  p->get_current_loc(&h, &v);

  b->set_pos( h, v);

  /* check for last one */

  nn_p = nn->parent->next;

  while (nn_p
	 && nn_p->dat[0] != 'b'
	 && nn_p->dat[0] != 'B') {
    nn_pp = NULL;
    if (nn->staff == 1) {
      if (nn_p->notes) nn_pp = nn_p->notes;
    }
    else if (nn->staff == 2) {
      if (nn_p->notes2) nn_pp = nn_p->notes2;
    }
    else
      dbg0(Error, "tab: beam.cc: staff not 1 or 2\n");
    if ( nn_pp &&
	 (nn_pp->special == 'x'
	 || nn_pp->special == '=' )) {
      //      printf("not last\n");	// this is not the last one
      return;
    }
    if ( nn_pp &&  nn_pp->special == '#')
      break;

    nn_p = nn_p->next;
  }

  // printf("last one\n");

  if (nn->staff == 1 )
    b_base = beam1;
  else
    b_base = beam2;

  b = b_base;
  if (! b->next ) {
    dbg0(Warning, "beam with only one note\n");
    return;
  }

  while(b) {
    // printf ("note %d %d \n", b->note, (b->note + 29));
    if ( b->note >= 'a')
      c_total += b->note;
    else
      c_total += (b->note + 29);
    count++;
    if (!b->next)
      last_b = b;
    b = b->next;
  }
  //  printf("beam: value %f\n", c_total/count);

  if ( (c_total/count) < 10.0 /* b */) {
    beamup=1;
    l_stem = inch_to_dvi(-stem);
  }
  else {
    beamup=0;
    l_stem = inch_to_dvi(stem);
  }

  if (beamup) {
    int head_w = inch_to_dvi(f_a[0]->fnt->get_width(215)
	      - str_to_inch("0.59 pt"));
    b = b_base;
    while(b) {
      b->change_h(head_w);
      b = b->next;
    }
  }

  slope = (double)(last_b->get_v() - b_base->get_v()) /
     (double)(last_b->get_h() - b_base->get_h());

  b_base->stem_h = l_stem;
  b_base->stem_top = b_base->get_v() + l_stem;
  last_b->stem_h = l_stem;
  last_b->stem_top = last_b->get_v() + l_stem;

  b = b_base->next;
  while(b) {
    if ( ! b->next)
      break;
    else {
      int root_to_bottom;
      int a=0, c=0;
      int tt;
      root_to_bottom =
	(inch_to_dvi)(slope * dvi_to_inch(b->get_h() - b_base->get_h()));
      //      a = b->get_v();
      //      c = b_base->get_v();
      if (f->flags & PS ) {
	b->stem_top = b_base->stem_top + root_to_bottom;
	tt = b->get_v() - b_base->get_v();
	b->stem_h = tt + l_stem - root_to_bottom;
      }
      else if  (f->flags & DVI_O ) {
	b->stem_top = inch_to_dvi(b->stem_h) + c;
	b->stem_h += c - a;
      }
      b = b->next;
    }
  }

  /* draw the stems */

  b = b_base;

  while (b) {
    p->push();

    h = b->get_h();
    v = b->get_v();
    p->moveto(h,v);


    if (beamup) {
      if (f->flags & PS) {
	p->movev(b->stem_h);
	p->put_rule(inch_to_dvi(stem_w), b->stem_h);
      }
      else if (f->flags & DVI_O ) {
  	p->put_rule(stem_w, dvi_to_inch(-b->stem_h));
      }
    }
    else {
      if (f->flags & PS) {
	p->movev(b->stem_h);
      	p->put_rule(stem_w,  dvi_to_inch(b->stem_h));
      }
      else if (f->flags & DVI_O ){
	p->movev(b->stem_h);
	p->put_rule(stem_w, dvi_to_inch(b->stem_h));
      }
    }
    p->pop();

    b = b->next;
  }
  /* draw the beam */

  p->push();
  // firsttime;

  for (i=0; i < firsttime; i++){
    int bs = i * inch_to_dvi(bar_space);
    if (!beamup) bs = -bs;
    h = b_base->get_h();
    if (f->flags & PS )
      v = b_base->get_v() - b_base->stem_h - bs;
    else
      v = b_base->get_v() + b_base->stem_h + bs;
    p->moveto(h, v);
    p->saveloc(BASE);
    h = last_b->get_h();
    h += inch_to_dvi(stem_w);
    if (f->flags & PS )
      v = last_b->get_v() - last_b->stem_h - bs;
    else
      v = last_b->get_v() + last_b->stem_h + bs;
    p->moveto(h, v);
    p->saveloc(BASE+1);
    //    p->put_thick_slant(BASE, BASE+1);
    p->put_thick_slant(BASE, BASE+1);
  }
  p->pop();

  /* draw extra partial beams */
  p->push();
  {
    int bs = firsttime * inch_to_dvi(bar_space);
    if (!beamup) bs = -bs;
    b = b_base;
    while (b) {
      if (b->time-'0' > firsttime) {
	if (b->next &&
	    b->next->time == b->time) { // betwen this note and next
	  h = b->get_h();
	  if (f->flags & PS )
	    v = b->get_v() - b->stem_h - bs;
	  else
	    v = b->get_v() + b->stem_h + bs;
	  p->moveto(h, v);
	  p->saveloc(BASE);

	  h = b->next->get_h();
	  if (f->flags & PS )
	    v = b->next->get_v() - b->next->stem_h - bs;
	  else
	    v = b->next->get_v() + b->next->stem_h + bs;
	  p->moveto(h, v);
	  p->saveloc(BASE+1);

	  p->put_thick_slant(BASE, BASE+1);
	}
	// dot follows this
	else if (b->next && b->next->dot == '.') {
	  int hh;

	  h = b->get_h();
	  if (f->flags & PS )
	    v = b->get_v() - b->stem_h - bs;
	  else
	    v = b->get_v() + b->stem_h + bs;
	  p->moveto(h, v);

	  p->saveloc(BASE);
	  hh = (int)((b->next->get_h() - h)/3.5); //  was 2
	  v += (int)(slope * (double)hh);
	  p->moveto(h + hh, v);
	  p->saveloc(BASE+1);
	  p->put_thick_slant(BASE, BASE+1);
	}
	// dot preceeds this
	else if (b->prev && b->prev->dot == '.') {
	  int hh;

	  h = b->get_h();
	  if (f->flags & PS )
	    v = b->get_v() - b->stem_h - bs;
	  else
	    v = b->get_v() + b->stem_h + bs;
	  p->moveto(h, v);

	  p->saveloc(BASE);
	  hh = (int)((b->prev->get_h() - h)/3.5); // was 2
	  v += (int)(slope * (double)hh);
	  p->moveto(h + hh, v);
	  p->saveloc(BASE+1);
	  p->put_thick_slant(BASE, BASE+1);
	}
      }
      b = b->next;
    }
  }
  p->pop();
}


beam::beam()
{
  next=NULL;
  prev=NULL;
}

beam::beam(struct notes *nn)
{
  next=NULL;
  prev=NULL;
  time = nn->time;
   switch (nn->sharp) {
   case '.':
   case '^':
   case 'v':
   case 'N':
     dot = '.';
     break;
   default:
     dot = '\0';
   }
   /*
  dot  = nn->sharp == '.' ?
    '.' : '\0';
   */
  note = nn->note;
  note = nn->note >= 'a'?
	nn->note - 'a'+7: nn->note - 'A';
}

beam::beam(beam *root, struct notes *nn)
{
  beam *t=root;

  if (! t ){
    dbg0(Error,"tab: beam: no root for beam\n");
  }

  while (t->next) {
    t = t->next;
  }

  time = nn->time;
  dot  = nn->sharp == '.' ?
    '.' : '\0';
  note = nn->note;
  note = nn->note >= 'a'?
	nn->note - 'a'+7: nn->note - 'A';
  next=NULL;
  prev=t;
  t->next = this;

  //  printf("adding note to beam: note %c time %c\n", note, time);
}


beam::~beam()
{

}

void
beam::set_pos(int h_in, int v_in)
{
  h = h_in;
  v = v_in;
}



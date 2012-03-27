
#ifndef _BEAM_
#define _BEAM_

#include "tab.h"

class beam {
protected:
  int h;			/* dvi position */
  int v;
public:
  beam();
  beam(struct notes *nn);
  beam(beam * root, struct notes *nn);
  ~beam();
  beam *next;
  beam *prev;
  char time;
  char dot;
  char note;
  int stem_h;		/* stem height */
  int stem_top;
  void set_pos(int h, int v);
  void change_h(int offset) {h += offset;};
  int get_h() {return (h);};
  int get_v() {return (v);};
};

#endif /* _BEAM_ */

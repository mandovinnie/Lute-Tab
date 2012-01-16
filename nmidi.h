/*
 *
 * ascii output
 *
 */

#ifndef __NMIDI__
#define __NMIDI__

#define STRINGS 6		/* 7 for notes 3 for flags */

struct chord {
  struct chord *next;
  struct chord *prev;
  char dat[STRINGS];
  char stop[STRINGS];
  char start[STRINGS];
  int time;
  int bar;
};

class nmidi {
 protected:
  char used_strings[STRINGS];
  enum text_type {Sequence=0,Text=1,Copyright=2,Track=3,
		  Instrument=4,Lyrics=5,Marker=6,Cue=7};
  int instmnt;
  void program();
  void writeVarLen(unsigned long value);
  void midi_tail();
  unsigned char buf[2048 * 32];
  int count;
  char *ntitle;
  int npatch;
  double npulse;
  int format;

  void nmidi_head(unsigned char pulses);
  void ntext(enum text_type, const char *words);
  void TimeSig(int num, int denom, int clocks, int bb);
  void stop_stop(chord *c, int index);
  enum direction {Up, Down, Up2, Down2};
  void check_adjacent(const int string, direction d, int saved[]);
 public:
  void set_nmidi_title(char * title);
  void set_patch(int patch);
  void set_pulse(double c);
  nmidi();
  ~nmidi();
};


#endif  /*__NMIDI__ */


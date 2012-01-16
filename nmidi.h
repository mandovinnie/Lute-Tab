/*
 *
 * ascii output
 *
 */

#ifndef __NMIDI__
#define __NMIDI__

#include "i_buf.h"

#define STRINGS 7		/* 7 for notes 3 for flags */
                                /* this used to be 6 wbc feb 2011 */

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
  void midi_track_tail();
  void nmidi_track_head(const int len);
  unsigned char buf[2048 * 32];
  int count;
  char *ntitle;
  int npatch;
  int npulse;
  int format;
  int ntracks;
  int pulse_time;

  void nmidi_head(unsigned short int pulses);
  void ntext(enum text_type, const char *words);
  void TimeSig(int num, int denom, int clocks, int bb);
  void stop_stop(chord *c, int index);
  void stop_string(int string, int *chord, struct chord *mp);
  enum direction {Up, Down, Up2, Down2};
  void check_adjacent(const int string, direction d, int saved[]);
  int get_chord(chord * c);
 public:
  void set_nmidi_title(char * title);
  void set_patch(int patch);
  void set_pulse(double c);
  nmidi();
  ~nmidi();
  void do_tempo_map_track();
  void do_lute_music();
  void do_file_head();
};


#endif  /*__NMIDI__ */


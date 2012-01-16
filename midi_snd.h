#include <stdio.h>
#undef SHORT


class midi_snd : public sound {
private:
  unsigned char buf[2048 * 32];
  int count;
  int instmnt;
  char fname[120];
  void writeVarLen(unsigned long value);
  void midi_tail();
  enum text_type {Text=1,Copyright=2,Instrument=3};
  void text(enum text_type, const char *words);
  void program();
  int bar;
  unsigned int velocity;
public:
  midi_snd();
  midi_snd(const unsigned int instrument, const unsigned int volume);
  midi_snd(const unsigned int instrument, const unsigned int volume,
    const char *filename);
  ~midi_snd();
  void add(const int note);
  void play(const double time);
  void rest(const double time);
  void play_note(const int note, const double time);
  void add_bar();
  unsigned int get_volume();
  void set_volume(unsigned int value);
};

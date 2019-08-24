#pragma once
#include <stdio.h>
#undef SHORT

class sound {
private:

public:
  sound();
  virtual ~sound();
  virtual void add(const int note) = 0;
  virtual void play(const double time)= 0;
  virtual void rest(const double time)= 0;
  virtual void play_note(const int note, const double time) = 0;
  virtual void add_bar() = 0;
  virtual void set_volume(unsigned int value) = 0;
};

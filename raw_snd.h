#include <stdio.h>

#undef SHORT

struct snotes {
  double pitch;
  struct snotes *next;
};

class raw_snd : public sound {
private:
   int sample;
   FILE * f;
#ifdef SHORT
   unsigned short *dat;
#else
   unsigned char *dat;
#endif
   int dat_s;
   struct snotes *n;
public:
  raw_snd();
  ~raw_snd();
  void add(const int note);
  void play(const double time);
};

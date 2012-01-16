#include <stdio.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include "tab.h"
#include "sound.h"
#include "raw_snd.h"
#include <math.h>

raw_snd::raw_snd()
{
  sample = 8000;	/* 8000 samples / second */
  f = fopen("data", "w");
  n=0;
}

raw_snd::~raw_snd()
{
  printf ("Closing raw sound\n");
  fclose(f);
}
void
raw_snd::add(const int note)
{
  int octave;
  struct snotes *p;
  const double step=1.0594631;
  double s[] = {
    /*
    110.,			// A
    116.5409403,
    123.4708252,		// B
    130.8127825,		// C
    138.5913153,
    146.8323837,		// D
    156.9753390, // 155.5634915,
    163.5159781, // 164.8137780,		// E
    174.4170433, // 174.6141152,		// F
    184.9972107,
    196.2191737, // 195.9977172,                // G
    207.6523479,
    219.9999990			// a
    */
    110.,			// A
    116.5409403,
    123.4708252,		// B
    130.8127825,		// C
    138.5913153,
    146.8323837,		// D
    155.5634915,
    164.8137780,		// E
    174.6141152,		// F
    184.9972107,
    195.9977172,                // G
    207.6523479,
    219.9999990			// a
  };

  if (!n) {
    n = (struct snotes*)malloc((size_t)sizeof(struct snotes));
    n->next=0;
    p = n;
  }
  else {
    p = n;
    while (p->next) {
      p = p->next;
    }
    p->next = (struct snotes*)malloc((size_t)sizeof(struct snotes));
    p = p->next;
    p->next = NULL;
  }
  if (note < 0 )
    dbg1(Warning, "tab: raw_snd: note below zero %d\n", (void *)note);

  octave = note/12;
  p->pitch = s[(note % 12)];
  switch (octave) {
  case 0:
    break;
  case 1:
    p->pitch *=2;
    break;
  case 2:
    p->pitch *=4;
    break;
  case 3:
    p->pitch *=8;
    break;
  case 4:
    p->pitch *=16;
    break;
  case 5:
    p->pitch *=32;
    break;
  }
  //    printf("tab: add_note: %f\n", p->pitch);

}

#define FADE

unsigned  short
sine(int i, double val)
{
  int offset=0;


  double v;
  v = (15.0 - ((i < 11200) ? (double)i/800 : 14))
      * sin((double)(i+offset) / val);
  v += (7.5 - ((i < 4000) ? (double)i/800 : 5))
    * sin((double)(2*i+offset) / val);
  v += (5. - ((i < 4000) ? (double)i/800 : 5))
    * sin((double)(3*i+offset) / val);
  v += (3. - ((i < 400) ? (double)i/800 : .10))
    * sin((double)(5*i+offset) / val);
  v += (2.0 - ((i < 80) ? (double)i/800 : .05))
    * sin((double)(7*i+offset) / val);
  return((unsigned char)v);
}

void
raw_snd::play(const double time)
{
  int i;
  snotes *p, *pp;

  if (!n) 
    return;

  p = n;
  double val = sample / (p->pitch * 2.0 * 3.1415);
  double duration = time / 64.0;
  

  dat_s = (int)((double)sample * duration * 2);
  dat = (unsigned char *)malloc((size_t)dat_s);

  for (i=0; i< dat_s; i++) {
    dat[i] = (int)sine(i, val);
    dat[i] +=127;
  }

  while (p->next) {
    p = p->next;
    val = sample / (p->pitch * 2.0 * 3.1415);
    for (i=0; i< dat_s; i++) {
      dat[i] += sine(i, val);
    }
  }

  fwrite(dat, dat_s, 1, f);

  p = n;
  while (p) {
    pp = p;
    p = p->next;
    free (pp);  
  }
  free (dat);
  n=0;

}

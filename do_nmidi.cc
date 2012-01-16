#include <stdio.h>
#include <stdlib.h>
#include "tab.h"
#include "nmidi.h"
#include "buffer.h"
#include "i_buf.h"
#include "version.h"


static int old_time;
static int barline=0;

// int i_nmidi_strings[] = {55,60,65,69,74,79}; // italian
// 0 x2b 0x30 0x35 0x39 0x3e 0x43
int i_nmidi_strings[] = {43,48,53,57,62,67}; // italian

extern struct chord *midi_p;	// in nmidi.cc
extern int verbose;		// in nmidi.cc
extern int time_r[];

void do_nmidi(struct list *l, struct file_info *f, i_buf *i_b)
{
  int i;
  char *ch = l->dat;
  char cc = *ch;
  int time;

  //  printf("%s \n", f->title );

  if (f->flags & VERBOSE)
    verbose=1;

  switch (cc) {
  case 'x':
    time=old_time;break;
  case '5':
    time=1;break;
  case '4':
    time=2;break;
  case '3':
    time=4;break;
  case '2':
    time=8;break;
  case '1':
    time=16;break;
  case '0':
    time=32;break;
  case 'w':
    time=64;break;
  case 'W':
    time=128;break;
  case 'Y':
  case 'J':
    time=256;break;
  case 'L':
    time=512;break;
  case 'b':
    time = 0;break;
  }
  /*
  if (time > 1536) {
    printf("time error \n");
  }
  */
  switch(time){
  case 1:
    time_r[1]++; break;
  case 2:
    time_r[2]++; break;
  case 4:
    time_r[3]++; break; //  3
  case 8:
    time_r[4]++; break; //  2
  case 16:
    time_r[5]++; break; //  1
  case 32:
    time_r[6]++; break; //  0
  case 64:
    time_r[7]++; break; //  w
  case 128:
    time_r[8]++; break; //  W
  case 256:
    time_r[9]++; break;
  case 512:
    time_r[10]++; break;
  }

  if (ch[1] == '.') 
     time = time * 3/2;

  switch (cc) {
  case 'x':
  case '5':
  case '4':
  case '3':
  case '2':
  case '1':
  case '0':
  case 'w':
  case 'W':
  case 'Y':
  case 'J':
  case 'L':
    if (f->flags & VERBOSE)
      fprintf(stderr, "%s\n", l->dat);

    // midi 60 = middle c

    if (! midi_p) {
      fprintf(stderr, " the -nmidi flag can not be used in a tab file\n");
      exit(0);
    }
    midi_p->next = (chord *)malloc(sizeof (struct chord));
    midi_p->next->prev = midi_p; 
    midi_p = midi_p->next;
    midi_p->next = 0;
    if (barline) {
      midi_p->bar = 1;
      barline = 0;
    }
    else
      midi_p->bar = 0;

    for (i=0; i<STRINGS; i++) {
      l->dat[i+2] = tolower(l->dat[i+2]);
      if ((l->dat[i+2] >= '0') && (l->dat[i+2] <= '9')) {
	midi_p->dat[i] = i_nmidi_strings[i] + (l->dat[i+2] -'0');
      }
      else if (l->dat[i+2] == 'x') {
	midi_p->dat[i] = i_nmidi_strings[i] + 10;
      }
      else if ((l->dat[7-i] >= 'a') && (l->dat[7-i] <= 'i')) {
        midi_p->dat[i] = i_nmidi_strings[i] + (l->dat[7-i] -'a');
      } 
      else if (l->dat[7-i] == 'j' || l->dat[7-i] == 'k' || l->dat[7-i] == 'l') {
        midi_p->dat[i] = i_nmidi_strings[i] + (l->dat[7-i] -'a' - 1);
      } 
      else 
	midi_p->dat[i] = 0;

      midi_p->start[i] = 0;
      midi_p->stop[i] = 0;
    }
    midi_p->time = time;
    old_time=time;
    break;
  case 'b':
    barline = 1;
    break;
  }

}


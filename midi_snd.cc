#include <stdio.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include "tab.h"
#include "version.h"
#include "sound.h"
#include "midi_snd.h"
#include "i_buf.h"
#include <math.h>

i_buf *buffer;
void midi_head();
void midi_track(const int len);
void midi_tail();

midi_snd::midi_snd()
{
  char s[80];
  //  printf("Start midi_snd\n");
  buffer = new i_buf();
  midi_head();
  count = 0;
  sprintf(s, "tab %s copyright 1995-2003 by Wayne Cripps",  VERSION);
  text(Copyright, s);
  text(Text, "converted by TAB");
  instmnt = 0;
  program();
  fname[0] = '\0';
}

midi_snd::midi_snd(const unsigned int instrument)
{
  char s[80];
  //  printf("Start midi_snd\n");
  buffer = new i_buf();
  midi_head();
  count = 0;
  sprintf(s, "tab %s copyright 1995-2003 by Wayne Cripps",  VERSION);
  text(Copyright, s);
  text(Text, "converted by TAB");
  instmnt = instrument;
  program();
  fname[0] = '\0';
}

midi_snd::midi_snd(const unsigned int instrument,
		   const char * filename)
{
  char s[80];
  //  printf("Start midi_snd\n");
  buffer = new i_buf();
  midi_head();
  count = 0;
  sprintf(s, "tab %s copyright 1995-2003 by Wayne Cripps",  VERSION);
  text(Copyright, s);
  text(Text, "converted by TAB");
  instmnt = instrument;
  program();
  strncat (fname, filename, 120);
}

midi_snd::~midi_snd()
{
  int i;
  //  fprintf(stderr, "Stop midi_snd count is %d\n", count);
  midi_tail();

  midi_track(count);


  for (i=0;i<count;i++) {
    buffer->PutChar(buf[i]);
  }

  if ( ! strcmp (fname, "stdout")) 
    buffer->dump ("stdout", Creat);
  else
    buffer->dump ("data.mid", Creat);
  delete(buffer);
}

char t[12];			// hold the chord being built
int tt=0;

void
midi_snd::add(const int note)
{
  /* the 0 in tab is a b-flat two octaves below middle c
     60 is a midi middle c. so 60 - 24 (2 octaves) -1
     should be the conversion factor

     Oct 2002 wbc added -2 correction
  */
  t[tt] = note+35-2;
  tt++;
}

void
midi_snd::play(const double time)
{
  int i;


  for (i=0; i<tt; i++) {
    writeVarLen(0);	  /* delta time */
    buf[count++] = (char)0x90;  /* note on channel 0*/
    buf[count++] = (char)t[i]; /* the note - 60 = middle c*/
    buf[count++] = (char)64;	  /* velocity */
  }

  for (i=0; i<tt; i++) {
    if (i==0) writeVarLen((unsigned int) time * 3);	  /* delta time */
    else writeVarLen(0);

    buf[count++] = (char)0x80;  /* note on channel 0*/
    buf[count++] = (char)t[i]; /* the note - 60 = middle c*/
    buf[count++] = (char)64;	  /* velocity */
  }
  tt=0;
  //  printf("midi_snd play count = %d %f\n", count, time);
}

void midi_head()
{
  buffer->PutByte('M');  buffer->PutByte('T');
  buffer->PutByte('h');  buffer->PutByte('d');
  buffer->PutByte('\0'); buffer->PutByte('\0'); 
  buffer->PutByte('\0'); buffer->PutByte('\6');

  buffer->PutByte(0); buffer->PutByte(1); // midi type 0-single track 
				          // 1-mult track, or 2 
  buffer->PutByte(0); buffer->PutByte(1); /* how many tracks */
  buffer->PutByte(0); buffer->PutByte(0x60);  /* pulses per quarter note */
}

void midi_snd :: midi_tail() {
  buf[count++] = 0x00;		// delta time 0
  buf[count++] = 0xff;		// specail
  buf[count++] = 0x2f;		// end of track
  buf[count++] = 0x00; 
}

void midi_track(const int len)
{
  //rin printf ("midi track: len %d\n", len);
  buffer->PutByte('M');  buffer->PutByte('T');
  buffer->PutByte('r');  buffer->PutByte('k');
  buffer->PutByte(0);    buffer->PutByte(0);
  //  printf (" len %x %x %x\n", (len&0xff0000), (len&0xff00), (len&0xff));
  buffer->PutByte((char)((len&0xff00)>>8)); 
  buffer->PutByte((char)(len&0xff)); /* data length in bytes */
}

void midi_snd::text (enum text_type type, const char *words) 
{
  int len, i;

  len = strlen(words);
  buf[count++] = ('\0');
  buf[count++] = ('\377');
  buf[count++] = (type);
  buf[count++] = ((char)len);
  for (i=0;i<len;i++)
    buf[count++] = (words[i]);
}

void midi_snd::program()
{
  //      printf("midi program: %d\n", instmnt);
  buf[count++] = ('\0');
  buf[count++] = ('\300');
  buf[count++] = instmnt;	// the instrument
  /* 
  000 - 005 pianos ->006 hpsichord  010 other keyboards
  014 marimba
  016 tube bell
  030 nyguitar
  032  acguitar
  066  voices
  0100 soprano sax
  0110 piccolo
  0112 recorder
  */
}

void midi_snd::writeVarLen(unsigned long value)
{
  unsigned long buff;

  buff = value & 0x7f;

  while((value >>= 7)) {
    buff <<=8;
    buff |=  ((value & 0x7f) | 0x80);
  }

  while (1) {
    buf[count++] = (char)(buff & 0xff);
    if (buff & 0x80)
      buff >>= 8;
    else
      break;
  }
}

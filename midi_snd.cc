#include "win.h"
#include <stdio.h>
#ifndef __APPLE__
//#include <malloc.h>
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
// void midi_tail();

midi_snd::midi_snd()
{
  char s[80];
  //  printf("Start midi_snd 0 args\n");
  buffer = new i_buf();
  midi_head();
  count = 0;
  snprintf(s, 50, "tab %s copyright 1995-2024 by Wayne Cripps",  VERSION);
  text(Copyright, s);
  text(Text, "converted by TAB");
  instmnt = 0;
  program();
  fname[0] = '\0';
  velocity = 100;
}

midi_snd::midi_snd(const unsigned int instrument, const unsigned int volume,
		   const char * infile )
{
  char s[80];
  //  printf("Start midi_snd 1 arg\n");
  buffer = new i_buf();
  midi_head();
  count = 0;
  snprintf(s, sizeof(s), "tab %s copyright 1995-2024 by Wayne Cripps",  VERSION);
  text(Copyright, s);
  text(Text, "converted by TAB");
  text(Text, infile);
  instmnt = instrument;
  program();
  fname[0] = '\0';
  bar = 0;
  velocity = 100;
}

midi_snd::midi_snd(const unsigned int instrument,
		   const unsigned int volume,
		   const char *infile,
		   const char * filename)
{
  char s[80];
  //  printf("Start midi_snd 2 args\n");
  buffer = new i_buf();
  midi_head();
  count = 0;
  snprintf(s, sizeof(s), "tab %s copyright 1995-2024 by Wayne Cripps",  VERSION);
  text(Copyright, s);
  text(Text, "converted by TAB");
  text(Text, infile);
  instmnt = instrument;
  program();
  strncat (fname, filename, 120);
  // fprintf(stderr, "midi_snd::midi_snd initial velocity d %d X %X\n",
  //  volume, volume );

  velocity = volume;
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
  else {
    /*    buffer->dump ("data.mid", Creat); */
    if (strlen (fname))
      buffer->dump ( fname, Creat);
    else
      buffer->dump ("data.mid", Creat);
  }
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
  unsigned int t_velocity = velocity;

  if (bar) {
    //    printf ("BBAR\n");
    bar = 0;
    t_velocity += 16;
    if (t_velocity > 127) t_velocity = 127;
  }

//  fprintf(stderr, "midi_snd::play_midi velocity d %d  x %X\n",
//	  t_velocity,
//	  t_velocity );

  for (i=0; i<tt; i++) {
    writeVarLen(0);	  /* delta time */
    buf[count++] = (char)0x90;  /* note on channel 0*/
    buf[count++] = (char)t[i]; /* the note - 60 = middle c*/
    buf[count++] = (char)t_velocity;	  /* velocity */
  }

  for (i=0; i<tt; i++) {
    if (i==0) writeVarLen((unsigned int) time * 3);	  /* delta time */
    else writeVarLen(0);

    buf[count++] = (char)0x80;  /* note off channel 0*/
    buf[count++] = (char)t[i]; /* the note - 60 = middle c*/
    buf[count++] = (char)velocity;	  /* velocity */
  }
  tt=0;
  //  printf("midi_snd play count = %d %f\n", count, time);
}

void
midi_snd::rest(const double time)
{
  writeVarLen(0);	  /* delta time */
  buf[count++] = (char)0x90;  /* note on channel 0*/
  buf[count++] = (char)60; /* the note - 60 = middle c*/
  buf[count++] = (char)0;	  /* velocity */

  writeVarLen((unsigned int) time * 3);	  /* delta time */

  buf[count++] = (char)0x80;  /* note off channel 0*/
  buf[count++] = (char)60; /* the note - 60 = middle c*/
  buf[count++] = (char)velocity;	  /* velocity */

  tt=0;
}

// write a note on event, a delta time, and a note off ev ent

void midi_snd::play_note(const int note, const double time)
{
  unsigned int t_velocity = velocity;
  writeVarLen(0);	  /* delta time */
  buf[count++] = (char)0x90;  /* note on channel 0*/
  buf[count++] = (char)note; /* the note - 60 = middle c*/

  if (bar) {
    //    printf("bar\n");
    t_velocity += 16;
    if (t_velocity > 127) t_velocity = 127;
    bar = 0;
  }
  else {
    /* velocity = 64; */
  }
  printf("midi_snd::play_note midi_velocity %d\n", t_velocity );

  buf[count++] = (char)velocity;	  /* velocity */

  writeVarLen((unsigned int) time * 3);	  /* delta time */

  buf[count++] = (char)0x80;  /* note off channel 0*/
  buf[count++] = (char)note; /* the note - 60 = middle c*/
  buf[count++] = (char)t_velocity;	  /* velocity */
}

void midi_head()
{
  buffer->PutByte('M');  buffer->PutByte('T');
  buffer->PutByte('h');  buffer->PutByte('d');
  buffer->PutByte('\0'); buffer->PutByte('\0');
  buffer->PutByte('\0'); buffer->PutByte('\6');

  buffer->PutByte(0); buffer->PutByte(0); // midi type 0-single track
                                          // was 1
				          // 1-mult track, or 2
  buffer->PutByte(0); buffer->PutByte(1); /* how many tracks */
  buffer->PutByte(0); buffer->PutByte(0x60);  /* pulses per quarter note */
}

void
midi_snd :: midi_tail() {
  buf[count++] = 0x00;		// delta time 0
  buf[count++] = 0xff;		// special
  buf[count++] = 0x2f;		// end of track
  buf[count++] = 0x00;
}

void midi_track(const int len)
{
  //  fprintf (stderr, "midi track: len %d   %x\n", len, len);
  buffer->PutByte('M');  buffer->PutByte('T');
  buffer->PutByte('r');  buffer->PutByte('k');
  buffer->PutByte(0);    buffer->PutByte(0);
  // fprintf (stderr, " len %x %x %x\n", (len&0xff0000), (len&0xff00), (len&0xff));
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

//  fprintf (stderr, "midi_snd Program d %d\n", instmnt);

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

void midi_snd::add_bar()
{
  //  fprintf (stderr, "BAR\n");
  bar = 1;
}

void midi_snd::set_volume(unsigned int value)
{
  velocity = value;
}

unsigned int midi_snd::get_volume()
{
  return (velocity);
}

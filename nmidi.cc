#include <stdio.h>
#include <stdlib.h>
#include "tab.h"
#include "nmidi.h"
#include "buffer.h"
#include "i_buf.h"
#include "version.h"

static struct chord *midi_root;
static int midi_init=0;
struct chord *midi_p;
static int old_strings[STRINGS]= {0,0,0,0,0,0};
static char on[100];		// keep track of turned on notes

i_buf *nbuffer;
// void stop_stop(chord *c, int index);
// void nmidi_head(const unsigned char pulses);
//void nmidi_track(const int len);
//void nmidi_tail();
#define N_TIME 12
int time_r[N_TIME]={0,0,0,0,0,0};
int verbose=0;

nmidi::nmidi(){
  int i;

  if (!midi_init){
    midi_init = 1;

    midi_root = (chord *)malloc(sizeof (struct chord));
    midi_p = midi_root;
    midi_p->next = 0;
    midi_p->prev = 0; 
    midi_p->time = 0;
    midi_p->bar = 0;
    for ( i=0; i< STRINGS; i++) {
      midi_p->dat[i] = 0;
      midi_p->stop[i] = 0;
      midi_p->start[i] = 0;
      midi_p->start[i] = 0;
    }
    count = 0;
    npulse = 0;
    npatch = 0;
  }
}

nmidi::~nmidi(){
  int i;

  nbuffer = new i_buf();

  do_file_head();

  
  count=0;
   do_tempo_map_track();
  midi_track_tail();
  nmidi_track_head(count);
  
  for (i=0;i<count;i++) {
    nbuffer->PutChar(buf[i]);
  }
  count=0;
  
  do_lute_music();

  midi_track_tail();

  // need to know length here!
  nmidi_track_head(count);

  //then dump the track, including text and program and data and tail

  for (i=0;i<count;i++) {
    nbuffer->PutChar(buf[i]);
  }

  nbuffer->dump("nout.mid", Creat);

}

// --- Below RIPPED OFF from midi_snd

void 
nmidi::nmidi_head(unsigned short int pulses)
{
  format = 0;
  ntracks = 1;
  format = 1;
  nbuffer->PutByte('M');  nbuffer->PutByte('T');
  nbuffer->PutByte('h');  nbuffer->PutByte('d');
  nbuffer->PutByte('\0'); nbuffer->PutByte('\0'); 
  nbuffer->PutByte('\0'); nbuffer->PutByte('\6');

  nbuffer->PutByte(0); nbuffer->PutByte(format); // midi type 0-single track 
                                          // was 1
				          // 1-mult track, or 2 
  if (format == 0)
    ;
  else if (format == 1) {
    ntracks += 1;
  }
  else {
    fprintf(stderr," We don't handle type 3 or other midi formats\n");
  }
  nbuffer->PutByte((ntracks & 0xff00)>>8); 
  nbuffer->PutByte( ntracks & 0xff); /* how many tracks */

  nbuffer->PutByte((pulses & 0xff00)>>8); 
  nbuffer->PutByte( pulses & 0xff);  /* pulses per quarter note */
}

void nmidi::midi_track_tail() {
  buf[count++] = 0x00;		// delta time 0
  buf[count++] = 0xff;		// special
  buf[count++] = 0x2f;		// end of track
  buf[count++] = 0x00; 
}

void nmidi::nmidi_track_head(const int len)
{
  //  printf ("midi track: len %d %x\n", len, len);
  nbuffer->PutByte('M');  nbuffer->PutByte('T');
  nbuffer->PutByte('r');  nbuffer->PutByte('k');
  nbuffer->PutByte(0);    nbuffer->PutByte(0);
  //  printf (" len %x %x %x\n", (len&0xff0000), (len&0xff00), (len&0xff));
  nbuffer->PutByte((char)((len&0xff00)>>8));
  nbuffer->PutByte((char)(len&0xff)); /* data length in bytes */
}

void nmidi::ntext (enum text_type type, const char *words) 
{
  int len, i;

  if (! words)
    return; 
  len = strlen(words);

  buf[count++] = ('\0');
  buf[count++] = ('\377');
  buf[count++] = (type);
  buf[count++] = ((char)len);
  for (i=0;i<len;i++)
    buf[count++] = (words[i]);
}

void nmidi::TimeSig(int num, int denom, int clocks, int bb)
{
}

void nmidi::program()
{
  //      printf("midi program: %d\n", instmnt);

  buf[count++] = ('\0');
  buf[count++] = (0xc0);
  instmnt = 032;
  instmnt = 64;
  if (npatch)
    buf[count++] = npatch;	// the instrument
  else
    buf[count++] = instmnt;	// the instrument

  /* 
  000 - 005 pianos ->006 hpsichord  010 other keyboards
 (16) drawbar organ
 (19E (nicer)) church organ
  014 marimba
  016 tube bell
  030 nyguitar
  032 (26) steel  acoustic guitar
  (29) overdriven electric guitar
  (30) distortion electric guitar
  066  voices
  (51) synth strings
  (52) choral
  (53) choral
  (54) voice
  0077 (63) SynthBrass 2
  0100 (64) soprano sax
  0110 piccolo
  0112 recorder
  (79) ocharina
  (99) atmosphere (has attack and long sustain)
  (106) banjo
  (109) bagpipes
  (123) bird chirps (funny!)
  */
}

void nmidi::writeVarLen(unsigned long value)
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
/*
 * stop_stop
 *
 * go forward and clear the stop for the note you 
 * have just stopped 
 */
void
nmidi::stop_stop(chord *c, int index) {
  chord *cc;
  cc = c;
  while (cc->next)
    if (cc->next->stop[index] != 0 ) {
      on[cc->next->stop[index]] = 0;
      cc->next->stop[index] = 0;
      break;
    }
    else
      cc = cc->next;

}

/*
 * check_adjacent
 * 
 * check and stop note playing on adjacent string
 *
 */

void
nmidi::check_adjacent(const int string, direction d, int *saved ) {
  int o;

  if (d == Down) 
    o = string - 1;
  else if (d == Down2) 
    o = string - 2;
  else if (d == Up)
    o = string + 1;
  else if (d == Up2)
    o = string + 2;
  if (saved[string] != 0 && midi_p->start[o] != 0 ) {
    midi_p->stop[string] = saved[string];
    on[midi_p->stop[string]] = 0;
    saved[string] = 0;
    // remove old stop note
    if (midi_p->start[string] == 0 )
      stop_stop(midi_p, string);
  }
}

void
nmidi::set_nmidi_title(char *title){
  ntitle = title;
}

void 
nmidi::set_patch(int patch){
  if ((patch >= 0) && patch < 128)
    npatch = patch;
}

void
nmidi::set_pulse(double c)
{
  npulse = (int)c;
}

void nmidi::do_lute_music()
{
  int i;
  char s[80];
  int old_time, last_time, first;
  int saved[STRINGS];



  for (i=0; i<100; i++)
    on[i] = 0;

  for (i=0; i< STRINGS; i++)
    saved[i] = 0;

  sprintf(s, "tab %s copyright 1995-2005 by Wayne Cripps",  VERSION);
  ntext(Copyright, s);
  ntext(Text, "converted by TAB");
  ntext(Track, "lute");
  program();

  // FIRST PASS

  midi_p = midi_root->next;
  while(midi_p) {
    for (i=0; i< STRINGS; i++) {
      midi_p->start[i] = midi_p->dat[i];

      on[midi_p->start[i]] = 1;
      //      if (midi_p->start[i] == 0) {
      //	printf ("HERE  i is %d\n", i);
      //      }

      if ((old_strings[i] != 0) && (midi_p->dat[i] != 0)) {
	midi_p->stop[i] = old_strings[i];
	on[midi_p->stop[i]] = 0;
      }
      if (midi_p->dat[i] != 0)
	old_strings[i] = midi_p->dat[i];
    }
    midi_p = midi_p->next;
  }

  // SECOND PASS

  midi_p = midi_root->next;
  while(midi_p) {
    for (i=0; i< STRINGS; i++) {
      if (midi_p->prev->dat[i] != 0) {
	saved[i] = midi_p->prev->dat[i];
      }
    }
    
    // stop 6th string if something on 5th
    check_adjacent(0, Up, saved);

    // stop 6th string if something on 4th
    check_adjacent(0, Up2,saved);

    // stop 5th string if something on 6th
    check_adjacent(1, Down, saved);

    // stop 5th string if something on 4th string
    check_adjacent(1, Up, saved);

    // stop 4th string if something on 5th string
    check_adjacent(2, Down, saved);
    
    // stop a minor seventh below
    for (i=1; i< STRINGS-2; i++) {
      if((midi_p->start[i] + 10) == saved[i+2] ) {
	midi_p->stop[i+2] = saved[i+2];
	saved[i+2] = 0;
	if (midi_p->start[i+2] == 0) {
	  stop_stop(midi_p, i+2);
	}
      }
    }
    
    // stop a major seventh below
    for (i=1; i< STRINGS-2; i++) {
      if((midi_p->start[i] + 11) == saved[i+2] ) {
	midi_p->stop[i+2] = saved[i+2];
	saved[i+2] = 0;
	if (midi_p->start[i+2] == 0) {
	  stop_stop(midi_p, i+2);
	}
      }
    }
    
    for (i=1; i< STRINGS; i++) {
      if (saved [i-1] != 0) {
	int t = saved [i-1] - midi_p->start[i];
	if (t > -3 && t < 3) {
	  midi_p->stop[i-1] = saved [i-1];
	  on[midi_p->stop[i-1]] = 0;
	  saved [i-1] = 0;
	  if (midi_p->start[i-1] == 0 )
	    stop_stop(midi_p, i-1);
	}
      }
    }

    for (i=0; i< STRINGS-1; i++) {
      if (saved [i+1] != 0) {
	int t = saved [i+1] - midi_p->start[i];
	if (t > -3 && t < 3) {
	  midi_p->stop[i+1] = saved [i+1];
	  on[midi_p->stop[i+1]] = 0;
	  saved [i+1] = 0;
      	  if (midi_p->start[i+1] == 0 )
	    stop_stop(midi_p, i+1);
	}
      }
    }

    if (! midi_p->next) {
      for (i=0; i< STRINGS; i++) 
	if (saved[i] != 0 ) {
	  midi_p->stop[i] = saved[i];
	  saved[i] = 0;
	}
    }
    midi_p = midi_p->next;
  }


  // LAST (PRINTING) PASS

  midi_p = midi_root->next;

  while(midi_p) {
    int stopped=1;

    first=1;
    last_time = midi_p->time;
    old_time = midi_p->prev->time;
    if (verbose)
      printf ("%3x-    ", midi_p->time);
    for (i=0; i< STRINGS; i++) {
      if (verbose)
	printf("%02x ", midi_p->stop[i]);
      if (midi_p->stop[i] != 0) {
	if (first) {
	  writeVarLen((unsigned int) (old_time));   /* delta time */
	  first = 0;
	  stopped=0;
	}
	else writeVarLen(0);
	buf[count++] = (char)0x80;  /* note off channel 0*/
	buf[count++] = (char)midi_p->stop[i]; /* the note - 60 = middle c*/
	buf[count++] = (char)64;      /* velocity */
      }
    }
    if (stopped) {
      writeVarLen((unsigned int) (old_time));   /* delta time */
      buf[count++] = (char)0x80;  /* note off channel 0*/
      buf[count++] = (char)0; /* the note - 60 = middle c*/
      buf[count++] = (char)64;      /* velocity */
    }

    if (verbose)
      printf ("\nstart   ");
    for (i=0; i< STRINGS; i++) {
      if (verbose)
	printf("%02x ", midi_p->start[i]);
      if (midi_p->start[i] != 0) {
	writeVarLen(0);
	buf[count++] = (char)0x90;  /* note on channel 0*/
	buf[count++] = (char)midi_p->start[i]; /* the note - 60 = middle c*/
	if (midi_p->bar) 
	  buf[count++] = (char)70 /*velocity*/ ;        /* velocity */
	else
	  buf[count++] = (char)64 /*velocity*/ ;        /* velocity */
      }
    }
    if (verbose)
      printf ("\n");
    midi_p = midi_p->next;
  }
  if (verbose)
    printf("last %2d ", last_time);
  first = 1;
  for (i=0; i< STRINGS; i++) {
    if (verbose)
      printf("%02x ", old_strings[i]);
    //  below relies on old_strings being set from first pass
    //
    if (old_strings[i] != 0) {
      if (first) {
	writeVarLen((unsigned int)last_time);   /* delta time */
	first = 0;
      }
      else 
	writeVarLen(0);
      buf[count++] = (char)0x80;  /* note off channel 0*/
      buf[count++] = (char)old_strings[i]; /* the note - 60 = middle c*/
      buf[count++] = (char)64;      /* velocity */
      on[old_strings[i]] = 0;
    }
  }
  if (verbose)
    printf ("\n");

  for (i=1; i<100; i++) 
    if (on[i])
      printf("note %d %x is on\n", i, i);
  
}

void
nmidi::do_tempo_map_track() {

  // time signature

  buf[count++] = 0x00; 		// don't forget the delta time!
  buf[count++] = 0xff;		// FF
  buf[count++] = 0x58;		// 58 - time signature
  buf[count++] = 0x04;		// count
  buf[count++] = 0x04; 		// numerator
  buf[count++] = 0x02; 		// denominator as a power of 2
  buf[count++] = 0x24; 		// MIDI clocks per metronome tick (24 std)
  buf[count++] = 0x08; 		// number of 1/32 notes per 24 MIDI clocks (8 std)

  // set tempo
  
  buf[count++] = 0x00; 		// don't forget the delta time!
  buf[count++] = 0xff;		// FF
  buf[count++] = 0x51;		// 51 - set tempo
  buf[count++] = 0x03;		// count
  buf[count++] = 0x07; 		// should be 500000 for 120 q-notes per min
  buf[count++] = 0xa1; 
  buf[count++] = 0x20; 
  
  // sequence/track name

  ntext(Track, ntitle);

  // sequence number

  buf[count++] = 0x00; 		// don't forget the delta time!
  buf[count++] = 0xff;		// FF
  buf[count++] = 0x00;		// 
  buf[count++] = 0x02;		// count
  buf[count++] = 0x00; 
  buf[count++] = 0x01; 

  // marker

  // SMPTE offset

}

void nmidi::do_file_head()
{
  int i;
  int maxval=0, secondval=0;
  int maxi=0, secondi=0;
  unsigned char pulses;

  for (i=0; i< N_TIME; i++) {
    //    printf("index %d time counts %d\n", i, time_r[i] );
    if (time_r[i] > maxval) {
      if (time_r[i] > secondval ){
	secondval = maxval;
	secondi = maxi;
      }
      maxval = time_r[i];
      maxi = i;
    }
    else if (time_r[i] > secondval) {
      secondval = time_r[i];
      secondi = i;
    }
  }

 if ((time_r[maxi+1]) * 3 > time_r[maxi]) 
      maxi++;

  switch(maxi) {
  case 2:
    pulses = 4; break;		// 0x06 .. 0x18
  case 4:
    pulses = 8 ; break;		// 0x0c .. 0x30
  case 5:
    pulses = 16; break;		// 0x18 .. 0x60
  case 6:
    pulses = 32; break;		// 0x30 .. 0xc0
  case 7:
    pulses = 64 ; break;		// 0x60 .. 0x180
  case 8:
    pulses = 128; break;	        // 0xc0 .. 0x300
  case 9:
    pulses = 256; break;	        // 0xc0 .. 0x600
  default:
    fprintf (stderr, "uncaught time value %d\n", time_r[i]);
  }
  if( npulse ) {
    //    printf("npulse %d\n", npulse);
    nmidi_head((int)( 8 * npulse ));
  }
  else
    nmidi_head(pulses);


}


/*  
 *
 * midi-dump
 * 
 * a simple program to see what is in a midi file
 *
 * wbc Dec 2005
 * 
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

unsigned long readVarLen();
int event ();
static char *fp;
static int brief=0;
static char on[100];
static int count=0;

main(int argc, char *argv[]) {
  char fname[80];
  char b[80];
  int i;
  int f;
  struct stat statbuf;
  char *dat;
  int ntracks;
  int divisions;
  unsigned long length;
  int format;

  for (i=0; i< 100; i++) on[i] = 0;

  for (i=1; i< argc; i++) {
    if (*argv[i] == '-') {
      /*      printf("%c\n", argv[i][1]); */
      switch (argv[i][1]) {
      case 'b':
	brief=1;
	break;
      }
    }
    else {
      fname[0] = 0;
      for ( ; i< argc; i++ ) {
	strcat (fname, argv[i]);
      }
    }
  }

  f = open(fname, O_RDONLY );
  if (f == -1) {
    fprintf(stderr, "can't open %s\n", fname);
  }
  stat (fname, &statbuf);
  printf("File size is %d\n", statbuf.st_size);
  dat = (char *)malloc(statbuf.st_size);
  if (read(f, dat, statbuf.st_size ) != statbuf.st_size ) {
    fprintf(stderr, "file read error on %s\n", fname);
  }

  /*
   *  Header
   */
  fp = dat;

  for (i=0; i< 14; i++ ) 
    b[i] = *fp++;
  
  b[14] = 0;
  length = ((b[4]&0xff)<<24) +((b[5]&0xff)<<16) + ((b[6]&0xff)<<8) + (b[7]&0xff);
  format = ((b[8]&0xff)<<8) + (b[9]&0xff);
  ntracks = ((b[10]&0xff)<<8) + (b[11]&0xff);
  divisions = ((b[12]&0xff)<<8) + (b[13]&0xff);
  printf("header %s length %x format %x --  %x tracks %x pulses per q-note\n",
	  b, length, format, ntracks, divisions);


  /*
   * Track
   */
  /*  ntracks=4; */
  while(ntracks--){
    for (i=0; i< 8; i++ ) 
      b[i] = *fp++;
    b[8] = 0;
    printf("track  %s ", b);
    length = ((b[6]&0xff)<<8) + (b[7]&0xff);
    printf(" length  %d %x\n", length, length);
    
    while(i = event())
      ;
    printf("Event RETURNS %d End of Track\n", i);  
  }
  for (i=1; i<100; i++)
    if (on[i]) printf("note %d 0x%x is still on\n", i, i);
}

/*
 * event returns 1, returns 0 on end of track
 */

static int old_event=0;

int event () {
  int i;
  char b[80];
  int data;
  int event;
  int velocity;
  unsigned long length;
  char c;
  unsigned long deltime;
  int channel;

  deltime = readVarLen();

  event = *fp++;
  event &= 0xff;
  data = *fp++;
  data &= 0xff;
  if (brief) {
    if (deltime) {
      count++;
      printf("\nd-t %3d %3x ", count, deltime);
    }
  }
  else
    printf("d-time %3d %3x ", count, deltime);


  if (!(event & 0x80)) {		/* this may help on events */
    *fp--;
    *fp--;
    data = *fp++&0xFF;
    /*    printf("data fudge %x\n", data); */
    event = old_event;           /* where only the first is specified */
  }

  if (((event & 0xf0) >= 0x80) && ((event & 0xf0) <= 0xE0)) {
    channel = event & 0x0f;
    /*    if (event & 0x0f)
      (fprintf(stderr, " non zero channel set %x\n", event & 0x0f ));
    */
    if (!brief)
      printf ("channel %d ", channel);
    event &= 0xf0;
    old_event = event;
  }

  switch (event) {
  case 0x80:
    velocity = *fp++;
    if (brief) 
      printf("OFF %2x ", data);
    else
      printf("Note OFF  note %2x velocity %x \n", data, velocity);
    on[data]= 0;
    break;
  case 0x90:
    velocity = *fp++;
    if (brief) 
      printf("On  %2x ", data);
    else
    printf("Note ON   note %2x velocity %x \n", data, velocity);
    on[data]= 1;
    break; 
  case 0xb0:
    length = *fp++&0xFF;
    printf("Controller Change %d  %d\n", data, length);
    break;
  case 0xc0:
    printf("Program Change %x\n", data );
    break;
  case 0xd0:
    printf("Aftertouch %x\n", data );
    break;
  case 0xe0:
    {
      int f;
      f=*fp++&0xFF;
      printf("Pitch Bend  %x  %x      ", data, f );

      /* extreme hack */
      /*      for (i=0; i< (f >> 4); i++ ) 
	      printf(" %x ", *fp++);
      */
      printf("\n");
    }
    break;
  case 0xf0:
    printf("Sysex event %x\n", data);
    if (data & 0x80)
      fprintf(stderr, "Sysex event, length not handled\n");
    else {
      length = data;
      for (i=0; i< data; i++ ) 
	printf(" %x ", *fp++ & 0xff);
      printf("\n"); 
    }
    break;
  case 0xff:
    if (!(data & 0xf0)) 
      length = readVarLen(fp);
    else 
      length = *fp++;
    switch (data) {
    case 0:
      printf("Sequence Number");
      {
	int seq;
	if (length != 2)
	  fprintf(stderr, "Sequence Number length %x ERROR\n", length);
	for (i=0; i< length; i++ ) 
	  b[i] = *fp++;
	seq = ((b[0]&0xff)<<8) + (b[1]&0xff);
	printf("  %x\n", seq);
      }
      break;
    case 1:
      printf("Text Event length %d\n", length);
      while (length--)
	printf("%c", *fp++);
      printf("\n");
      break;
    case 2:
      printf("Copyright length %d\n", length);
      while (length--)
	printf("%c", *fp++);
      printf("\n");
      break;
    case 3:
      printf("Sequence/Track Name length %d  ", length);
      while (length--)
	printf("%c", *fp++);
      printf("\n");
      break;
    case 4:
      printf("Instrument Name length %d  ", length);
      while (length--)
	printf("%c", *fp++);
      printf("\n");
      /*     fprintf(stderr, "NOT Implimented Yet\n"); */
      break;
    case 5:
      printf("Lyric length %d  ", length);
      while (length--)
	printf("%c", *fp++);
      printf("\n");
      break;
    case 6:
      printf("Marker length %d  ", length);
      while (length--)
	printf("%c", *fp++);
      printf("\n");
      break;
    case 7:
      printf("Cue Point length %d  ", length);
      while (length--)
	printf("%c", *fp++);
      printf("\n");
      break;
    case 0x20:
      printf("MIDI Channel Prefix  channel");
      *fp++;
      printf("%x \n", *fp++);
      break;
    case 0x21:
      printf("Unknown - not on my list %x  ", length);
      while (length--)
	printf("%x", *fp++);
      printf("\n");
      break;
    case 0x51:
      {
	int i=0;
	int tmp=0;
	printf("Set Tempo microseconds/quarter note  ");
	if (length != 3)
	  fprintf(stderr, "Set Tempo length ERROR\n");
	{ 
	  int v;
	  while (length--) {
	    v = *fp++&0xff;
	    tmp += (v << (length * 8 ));
	    /*	    printf("%02x ", v); */
	  }
	  printf(" %x %d \n", tmp, tmp);
	}
      }
      break;
    case 0x53:
      printf("Unknown Meta Event 0x53 %d  ", length);
      fprintf(stderr, "NOT Implimented Yet\n");
      break;
    case 0x54:
      printf("SMPTE Offset length %x  ", length);
      while (length--)
	printf("%x ", *fp++);
      printf("\n");
      break;
    case 0x58:
      {
	int num, denom, clocks, bb;
	printf("Time Signature ");
	if (length != 4)
	  fprintf(stderr, "Time Signature length ERROR %d\n", length);
	num = *fp++&0xff;
	denom = *fp++&0xff;
	clocks = *fp++&0xff;
	bb = *fp++&0xff;
	printf(
	       "%d/%d   %x clocks per metronome tick, %x 32nd notes per 24 midi clocks\n",
	       num, (int)pow(2.0, (double)denom), clocks, bb);
      }
      break;
    case 0x59:
      {
	int sf, mi;
	printf("Key Signature ");
	if (length != 2)
	  fprintf(stderr, "Key Signature length ERROR\n");
	sf = *fp++&0xff;
	mi = *fp++&0xff;
	printf(
	       "num sharps or flats%d major or minor %x\n", sf, mi);
      }
      break;
    case 0x7f:
      printf("Sequencer Specific Meta-Event ");
      { 
	int v;
	while (length--) {
	  v = *fp++&0xff;
	  printf(" %x ", v);
	}
	printf("\n");
      }
      break;
    case 0x2f:
      printf("End of Track\n", length);
      return (0);
      break;
    }
    break;
  default:
    fprintf(stderr, "UNHANDLED EVENT %x\n", event);
    exit(0);
    break;
  }
  return (1);
}


unsigned long readVarLen()
{
  unsigned long val;
  int len;
  unsigned int tt;


  val = *fp & 0x7F;
  /*  fprintf(stderr, "val %x \n", val); */
  if(!(*fp & 0x80))
    *fp++;
  else {
    while (*fp & 0x80) {
      *fp++;
      val <<= 7;
      val += *fp & 0x7F;
      /*    fprintf(stderr, "+val %x \n", val); */
    }
    *fp++;
  }

  /*  fprintf(stderr, "final val %x \n", val); */
  return(val);
}

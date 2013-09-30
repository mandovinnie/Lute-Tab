
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define NEWLINE '\n'
/* #else
   #define MAC
   #include "mac.h"
   #define NEWLINE '\r'
*/

/* extern char *strstr(const char *, const char *); */
/* extern char *strchr(const char *, int ); */


#include "tfm.h"

enum dbg_type {File = 1, Path = 2, TFM = 4, Inter = 8, 
		 Flow=16, Warning=32, Error = 64, Stack=128, 
		 Proceedure=256, Fonts=512, Bug=1024, Widths=2048, 
		 z=4098, xx=8192, yy = 16384, Null =32768 };
extern int debug_flag;
void dbg0(const int type, const char *fmt);
void dbg1(const int type, const char *fmt, void *a);
void dbg2(const int type, const char *fmt, void *a, void *b);
void dbg3(const int type, const char *fmt, void *a, void *b, void *c);
void dbg4(const int type, const char *fmt, 
	  void *a, void *b, void *c, void *d);
void dbg5(const int type, const char *fmt, 
	  void *a, void *b, void *c, void *d, void *e);
void dbg_set(const dbg_type type);

#ifndef BUFSIZ
#define BUFSIZ 2048
#endif

#define STAFF 11
#define BL 120			/* default file name size */
#define STAFF_PAGE 9
#define MAX_PAGE 25

extern char version[];
extern double red;
extern int baroque;
extern int thin_renaissance;
extern int n_system;
extern int bar_count;
extern int barCount;
extern int barCCount;
extern int pagenum;

#define FONT_NAMES 8		/* number of font names allowed */

struct file_info {
  char *file;			/* file name */
  char *out_file;		/* output file name */
  unsigned int  flags;	/* general flags */
  unsigned int  m_flags;	/* more general flags */
  unsigned int  line_flag;    /* is the note on or between lines ? */
  unsigned int  flag_flag;    /* what kind of flags ? */
  unsigned int  char_flag;    /* what kind of characters ? */
  unsigned int  num_flag;
  unsigned int  note_flag;	/* for music */
  unsigned int  debug_flag;
  int n_text;			/* how many lines of text ? */
  int note_conv;		/* conversion between tab flags and notes */
  double c_space;		/* additional space for middle c in staff */  
  int page;			/* what page are we on? */
  int include;		/* are we included from somehwere */
  int cur_system;		/* what system are we in? */
  int start_system;           /* system to begin printing mostly for midi */
  int transpose;	       /* to transpose midi and maybe score */
  char *(font_names[FONT_NAMES]);
  double font_sizes[FONT_NAMES];
  double sys_skip;            /* extra space to skip between systems */
  unsigned int midi_patch;      /* a midi patch number - 0 is piano*/
  void *utility;		/* don't you hate old fashioned programmers? */
  char *scribe;			/* the person who enTABulated the piece */
  char *title;
  unsigned int left_margin;	/* the left margin in PostScript units for PS output only */
  unsigned int top_margin;	/* the top margin in PostScript units for PS output only */
  unsigned int midi_volume;     /* the volume or velocity for a midi file*/
  unsigned int extended_character_set; /* is q a character or ornament? */
};

struct font_list {
  char *name;
  int num;
  tfm_font * fnt;
  struct font_list *next;
  char * real_name;
  double size;
};

/* this is the basic chord data */

struct list {
     struct list *next;
     struct list *prev;
     char dat[STAFF+2];
     char filler[1];
     double padding;		/* default padding */
     double space;		/* extra space */
     double weight;
     struct notes *notes;
     struct notes *notes2;
     struct text *text;
     struct text *text2;
     char *special;
 };

 struct notes {
     int time;			/* time value of note */
     char note;			/* note from A to g */
     int sharp;			/* +, - or blank */
     char special;		/* for reversing, etc */
     double padding;
     double weight;
     int staff;	        /* are we on first of second staff */
     int reversed;		/* is this note stem reversed? */
     struct list *parent;
 };


 struct text {
     struct t_words * words;	/* the text */
     int length;			/* max number of characters */
     double size;			/* max length in inches */
     struct list *parent;
 };

 struct t_words {
     char *words;
     int length;			/* number of characters */
     double size;		/* length in inches */
     struct t_words *next;
     struct t_words *prev;
     int nospace;		/* run this word into the next */
     int in_sys;                 /* is it in the staff */
 };

 struct words {
     char let;
     double width;
 };

 struct w {
     char *width;
     double weight;
 };

 enum pass{first, second};

 /* return values */
 #define END_MORE    1
 #define END_OK      0
 #define END_FILE    -1
 #define END_SYSTEM  -2

 /* toggle flags */
 #define  NOTHING    0X00000000
 #define  MANUSCRIPT 0X00000001
 #define  DVI_O      0X00000002
 #define  PS         0X00000004
 #define  PDF        0X00000008
 #define  VERBOSE    0X00000010
 #define  FIVE       0X00000020
 #define  MARKS      0X00000040
 #define  WALLACE    0X00000080
 #define  DEBUG      0X00000100
 #define  BIGNOTES   0X00000200	/* notes have large values - like half */
 #define  NO_MUSIC_B 0X00000400
 #define  ROTATE     0X00000800
 #define  NO_EXPAND  0X00001000
 #define  NO_WORD    0X00002000
 #define  X_POSE     0X00004000
 #define  DPI600     0X00008000
 #define  LSA_FORM   0X00010000
 #define  FOUR       0X00020000
 #define  INCLUDE    0X00040000
 #define  SHARP_UP   0X00080000	/* accidentals above the note */
 #define  PAGENUM    0X00100000
 #define  SAMELINE   0X00200000
 #define  NOTES      0X00400000
 #define  BOARD      0X00800000
 #define  ITAL_TXT   0X01000000
 #define  TXT        0X02000000
 #define  CONVERT    0X04000000	/*  convert french to ital */
 #define  CON_SEV    0X08000000	/*  convert french to ital 7 course */
 #define  DRAFT      0X10000000 	/* print draft notice */
 #define  COPYRIGHT  0X20000000 	/* print copyright notice */
 #define  JAM        0X40000000  /* compress the lines to fit in 7 sys */
 #define  CONV_COR   0X80000000  /* a correction for some conversions */
 /* style  flags */

 #define ON_LINE		0X0001
 #define BETWEEN_LINE	        0X0002

 #define STAND_FLAGS		0X0004
 #define THIN_FLAGS		0X0008
 #define ITAL_FLAGS		0X0010
 #define BOARD_FLAGS		0X0020
 #define CONTEMP_FLAGS	        0x0040
 #define CAP_FLAGS		0X0080
 #define S_ITAL_FLAGS		0X0100
 #define NOT_FLAGS		0X0200

 #define STAND_CHAR		0X0100
 #define BOARD_CHAR		0X0200
 #define ROB_CHAR		0X0400
 #define MACE_CHAR		0X0800
 #define DOWLAND_CHAR	        0X1000

 #define STAND_NUM		0X1000
 #define ITAL_NUM		0X2000
 #define ADOBE_NUM		0X3000

 #define MOD_NOTES              0X4000
 #define ITAL_NOTES             0X8000

 /* more flags f->m_flags */
 #define NO_AUTO_END            0x0001
 #define DPI1200                0x0002
 #define QUIET			0x0004
 #define PAREN			0x0008
 #define SOUND                  0x0010
 #define SEVEN                  0x0020
 #define LONGBAR                0x0040
 #define FCLEF                  0x0080
 #define ALTTITLE               0x0100
 #define A4                     0x0200
 #define NOBOX                  0x0400
 #define TWOSTAFF               0x0800
 #define AUTOKEY                0x1000
 #define DSUP                   0x2000
 #define DSDOWN                 0x4000
 #define EPSF                   0x08000
 #define RED                    0x10000
 #define DPI2400                0x20000
 #define ASCII                  0x40000
 #define AWIDE                  0x80000
 #define MILAN                  0x100000
 #define NMIDI                  0x200000
 #define GUIT                   0x400000
 #define THIN_R                 0x800000
 #define TWELVEDOTS             0x1000000
 /* variables defined in sizes.c */




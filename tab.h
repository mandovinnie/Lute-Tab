/*
 *   globals and parameters
 *
 */

#include <setjmp.h>
#include <stdio.h>

#define DVI      		/* commment this out if you don't have dvi */
				/* preview or formatting faculities */
#define POSTSCRIPT 
/* #define X_WIN */
#define OLD_PRINTER           /* define this if your printer gives an */
				/* unefined command error on rectfill  */
				/* or xshow, or if it doesn't print */
#define LSA

/* #define VAX */		/* uncomment this if you are running VMS */
				/* actually, you don't need to, as the */
				/* vax c compiler defines it for you */
#ifdef VAX
#define bcopy(s1, s2, len) memcpy(s2, s1, len)
#define bzero(s, len)      memset(s, 0, len)
#undef dvi
#undef DVI
#define VAXDEBUGR
#ifndef OLD_PRINTER
#define OLD_PRINTER
#endif
#endif /* VAX */

#ifdef hpux			/* hack for machines that */
#define SIG			/* don't like signed char */
#endif
#ifdef sun
#define SIG
#endif
#ifdef VAX
#define SIG
#endif
#ifdef SIG
#define SIGNED_C char
#else
#define SIGNED_C signed char
#endif /* hp */


#define LIST 200
#define STAFF 11

/* extern int l_p; */
/* extern unsigned char list[]; */
/* extern char inter[]; */
extern int inter_p;
extern FILE *op;
extern jmp_buf env;

struct file_info {
    char *file;
    char *i_file;
    char *out_file;
    FILE *i_p;
    unsigned long flags;	/* general flags */
    unsigned long line_flag;    /* is the note on or between lines ? */
    unsigned long flag_flag;    /* what kind of flags ? */
    unsigned long char_flag;    /* what kind of characters ? */
    unsigned long num_flag;
    unsigned long note_flag;	/* for music */
    int n_text;			/* how many lines of text ? */
    int note_conv;		/* conversion between tab flags and notes */
    double c_space;		/* additional space for middle c in staff */  
    int page;			/* what page are we on? */
};

struct b {
	char *ib;			/* tab input buffer */
	int  ib_p;			/* pointer to current position */
	long ib_s;			/* size of buffer */
};

struct list {
    struct list *next;
    struct list *prev;
    char dat[STAFF];
    char filler[1];
    double padding;		/* default padding */
    double space;		/* extra space */
    double weight;
    struct notes *notes;
    struct text *text;
    char *special;
};

struct notes {
    int time;			/* time value of note */
    char note;			/* note from A to g */
    int sharp;			/* +, - or blank */
    char special;		/* for reversing, etc */
    double padding;
    double weight;
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
};

extern struct list *listh;

struct words {
    char let;
    double width;
};

#define STAFF_PAGE 9		        /*  max staves per page */
#define MAX_PAGE 30		        /*  max pages tab can handle*/
#define BL 120			                     /* file name length */

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
#define  X          0X00000008
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
#define  PAPER      0X00020000
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

/*
#define  FLAG_STYLE \
   ( ITALIAN | FRENCH | I_FLAGS | MOD_FLAGS | O_FLAGS | BOARD | THIN | NOTHING)
#define CHAR_STYLE \
   ( ITALIAN | O_FLAGS | MACE | ROBINSON | BOARD )
 */

/* style  flags */

#define ON_LINE			0X0001
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
#define DOWLAND_CHAR	0X1000

#define STAND_NUM		0X1000
#define ITAL_NUM		0X2000

#define MOD_NOTES               0X4000
#define ITAL_NOTES              0X8000

/* flags for misc input */
#define TITLE 1
#define ARGS 2

/* integer toggles */

#define PS_FONT_5 "Palatino-BoldItalic"       /* 10 pt italic text */
/* #define PS_FONT_4 "Palatino-Bold" */
#define PS_FONT_4 "Times-Roman"                  /* 12 pt time sig */
#define PS_FONT_3 "Palatino-Bold"   /* 28 pt big time sig not used */
/* #define PS_FONT_2 "Palatino-BoldItalic" */
#define PS_FONT_2 "Times-Roman"               /* 12 pt titles */
/* #define PS_FONT_1 "Palatino-Bold" */
#define PS_FONT_1 "Times-Roman"            /* 10 pt words to songs */

#define PS_SCALE_5 1.00           	/* scale tfm to real points  */
#define PS_SCALE_4 1.20           	/* scale tfm to real points  */
#define PS_SCALE_3 1.00           	/* scale tfm to real points  */
#define PS_SCALE_2 1.20           	/* scale tfm to real points  */
#define PS_SCALE_1 1.00           	/* scale tfm to real points  */
#define PS_SCALE PS_SCALE_2

extern int baroque;
extern int n_system;
extern int bar_count;		
extern int barCount;
extern int pagenum;

/* end integer toggles */

extern int pk_checksum[];
extern double red;
/* extern double: space; */
extern double red;
double get_width(), get_height(), get_depth(), pad(), str_to_inch();
double dvi_to_pt();
SIGNED_C get();
extern SIGNED_C geti();
extern char version[];

extern int max_b_w;
extern int max_b_h;
extern int max_off_w;
extern int max_off_h;

/* commands for output buffering */
#define MOVE    0x0001
#define RMOVE   0x0002
#define LINE    0x0003
#define SHOW    0X0004
#define GLP     0x0005
#define INIT    0x0006
#define RULE    0x0007
#define MOVEH   0x0008
#define MOVEV   0x0009
#define MOVEVH  0x000a
#define CHAR    0X000b
#define TIMES   0x000c
#define LUTE    0x000d
#define BIG     0x000e
#define SMALL   0x000f
#define PCHAR   0x0010
#define MED     0x0011
#define PS_CLIP 0x0012
#define ITAL    0x0013
#define TH_LINE 0x0014
#define PGRAY   0x0015
#define PTIE    0x0016
#define PDRAFT  0x0017
#define PCOPYRIGHT 0x0018
#define P_S_GRAY 0x0019
#define P_U_GRAY 0x0020
#define OTHER   0x0200

extern unsigned char ps_used[];
extern int first_1, first_2;
extern char highlight;

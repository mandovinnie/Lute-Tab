/* pkfile.h	Samuel W. Bent		1/23/91 */
/* Definitions for ADT PK file */

/* $Header: /usr/everest/wbc/src/lute/RCS/pkfile.h,v 1.1 92/08/18 13:08:48 wbc Exp $
 * $Log:	pkfile.h,v $
 * Revision 1.1  92/08/18  13:08:48  wbc
 * Initial revision
 * 
 */


#ifndef _PKfile_h_
#define _PKfile_h_

/******  Data Structures  ******/

typedef struct {
    char id;			/* identification byte */
    short comment_length;	/* length of comment */
    char comment[256];		/* comment */
    int design_size;		/* design_size (in 2^{-20} pts) */
    int checksum;		/* checksum (agrees with TFM file) */
    int hppp;			/* horizontal pixels per 2^{16} pt */
    int vppp;			/* vertical pixels per 2^{16} pt */
} PKPreamble;

typedef struct {
    unsigned char flag;		/* flag bits */
    int dyn_f;			/* threshold for 1-nybble run-encoding */
    long bitmap_length;		/* length of packet, from TFM to end */
    int address;		/* address of metric information */
    int code;			/* character code */
    int tfm_width;		/* TFM width (in DU's) */
    int dx, dy;			/* escapements (in 2^{-16} dots) */
    int height, width;		/* size of bitmap */
    int hoffset, voffset;	/* distance from top left to ref pt (dots) */
} PKPacketHeader;


/******  PK Commands ******/

#define PK_ID		89

#define PK_XXX1		240
#define PK_XXX2		241
#define PK_XXX3		242
#define PK_XXX4		243
#define PK_YYY		244
#define PK_POST		245
#define PK_NOOP		246
#define PK_PRE		247

#define PK_SHORT_PACKET		0
#define PK_EXTENDED_PACKET	1
#define PK_LONG_PACKET		2

#endif /* !_PKfile_h_ */




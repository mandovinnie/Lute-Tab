/* pk_input.c	Samuel W. Bent		1/23/91 */

/* routines for reading PK files */
#ifndef lint
static char *rcsid = "$Header: /usr/everest/wbc/src/lute/RCS/pk_input.c,v 1.5 93/03/19 21:00:46 wbc Exp $";
#endif /* lint */
/* $Log:	pk_input.c,v $
 * Revision 1.5  93/03/19  21:00:46  wbc
 * version 3.0
 * 
 * Revision 1.4  92/11/02  11:12:07  wbc
 * new arguments - numbers, transposition
 * 
 * Revision 1.3  92/09/02  14:51:38  wbc
 * fixes for vax
 * 
 * Revision 1.2  92/08/28  11:58:18  wbc
 *  ps improvements
 * 
 * Revision 1.1  92/08/18  13:08:04  wbc
 * Initial revision
 * 
 */

#include <stdio.h>
#include "pkfile.h"
#include "tab.h"

/* #ifdef POSTSCRIPT */

#ifndef VAX
#define GetByte(fp)	getc(fp)
#else
extern unsigned int GetByte();
#endif



/****************************************************************/
/*	Public Routines						*/
/****************************************************************/


pk_read_preamble(fp, pre)
    FILE *fp;
    PKPreamble *pre;
{
int cmd, k;
#ifdef VAX
    b_seek(fp, 0, 0);
#else
    fseek(fp, 0, 0);	/* preamble must be at beginning */
#endif /* VAX */
    cmd = GetByte(fp);
    if (cmd != PK_PRE)
	return 1;

    pre->id = GetByte(fp);			/* read the information */
    pre->comment_length = GetByte(fp);
    for (k=0; k<pre->comment_length; ++k) 
	pre->comment[k] = GetByte(fp);
    pre->comment[k] = '\0';
    pre->design_size = Get4Bytes();
    pre->checksum = Get4Bytes();
    pre->hppp = Get4Bytes();
    pre->vppp = Get4Bytes();

    return 0;
}


static int current_input_byte, input_flag=0;

int
pk_next_nybble(fp)
    FILE *fp;
{
int next_nybble;
    if (input_flag) {
	next_nybble = current_input_byte & 0xf;
    } else {
	current_input_byte = GetByte(fp);
	next_nybble = (current_input_byte >> 4) & 0xf;
    }
    input_flag = !input_flag;
    return next_nybble;
}

pk_prepare_for_bitmap()
{
    input_flag = 0;
}


/*
 * Get the next packed number; it's either a run-length or a repeat count.
 * Return the number itself in *result.  Return true if it's a repeat count,
 * false if it's a run count.
 */
int
pk_packed_number(fp, dyn_f, result)
    FILE *fp;
    int dyn_f;
    int *result;
{
register int n, m;
    n = pk_next_nybble(fp);

    if (n==0) {				/* Self-delimiting number */
	do {
	    m = pk_next_nybble(fp);		/* find its length */
	    ++n;
	} while (m==0);
	while (n>0) {				/* read the number */
	    m = m*16 + pk_next_nybble(fp);
	    --n;
	}
	*result =  m - 15 + (13-dyn_f)*16 + dyn_f;
	return 0;
    } else

    if (n<=dyn_f) {		/* short run-length */
	*result =  n;
	return 0;
    } else

    if (n<14) {			/* long run-length */
	m = pk_next_nybble(fp);
	*result =   (n-dyn_f-1)*16 + m + dyn_f + 1;
	return 0;
    } else

    if (n==14) {		/* variable repeat count */
	pk_packed_number(fp, dyn_f, result);
	return 1;
    } else

    if (n==15) {		/* repeat 1 */
	*result = 1;
	return 1;
    } else
    {  /* error */
	printf ("pk_packed_number Error %d\n", n);
	return 0;
    }
}


/*
 * Skip packet, but return character code
 */
pk_get_code_and_skip_packet(fp)
    FILE *fp;
{
unsigned int flag;
int code;
long bitmap_length;
    flag = GetByte(fp);
    switch (pk_packet_type(flag)) {
	case PK_SHORT_PACKET:
	    bitmap_length =  ((flag & 0x3) << 8) + GetByte(fp) - 8;
	    code = GetByte(fp);
	    Skip(8 + bitmap_length, fp);
	    break;
	case PK_EXTENDED_PACKET:
	    bitmap_length = ((flag & 0x3) << 16) + Get2Bytes() - 13;
	    code = GetByte(fp);
	    Skip(13 + bitmap_length, fp);
	    break;
	case PK_LONG_PACKET:
	    bitmap_length = Get4Bytes() - 28;
	    code = Get4Bytes();
	    Skip(28 + bitmap_length, fp);
	    break;
    }
    return code;
}


/*
 * Read character packet header
 */
pk_read_packet_header(fp, h)
    FILE *fp;
    PKPacketHeader *h;
{
    h->flag = GetByte(fp);
    h->dyn_f = h->flag >> 4;
    switch (pk_packet_type(h->flag)) {
	case PK_SHORT_PACKET:
	    pk_read_short_packet_header(fp, h);
	    break;
	case PK_EXTENDED_PACKET:
	    pk_read_extended_packet_header(fp,h);
	    break;
	case PK_LONG_PACKET:
	    pk_read_long_packet_header(fp, h);
	    break;
    }
#ifdef VAX
    h->address = b_ftell(fp);
#else
    h->address = ftell(fp);
#endif /* VAX */
}


/*
 * determine what kind of packet you've got
 */
int
pk_packet_type(flag)
    unsigned int flag;
{
    if ((flag & 0x7) == 7)
	return PK_LONG_PACKET;
    else if (flag & 0x4)
	return PK_EXTENDED_PACKET;
    else
	return PK_SHORT_PACKET;
}


/*
 * read a short packet header
 */
pk_read_short_packet_header(fp, h)
    FILE *fp;
    PKPacketHeader *h;
{
    h->bitmap_length =  ((h->flag & 0x3) << 8) + GetByte(fp) - 8;
    h->code = GetByte(fp);
    h->tfm_width = Get3Bytes();
    h->dx = GetByte(fp) << 16;
    h->dy = 0;
    h->width = GetByte(fp);
    h->height = GetByte(fp);
    h->hoffset = GetByte(fp);
    h->hoffset <<=24;  h->hoffset >>=24;	/* sign extend */
    h->voffset = GetByte(fp);
    h->voffset <<=24;  h->voffset >>=24;	/* sign extend */
}


/*
 * read an extended packet header
 */
pk_read_extended_packet_header(fp, h)
    FILE *fp;
    PKPacketHeader *h;
{
    h->bitmap_length = ((h->flag & 0x3) << 16) + Get2Bytes() - 13;
    h->code = GetByte(fp);
    h->tfm_width = Get3Bytes();
    h->dx = Get2Bytes() << 16;
    h->dy = 0;
    h->width = Get2Bytes();
    h->height = Get2Bytes();
    h->hoffset = Get2Bytes();
    h->hoffset <<= 16; h->hoffset >>=16; /* sign xtend */
    h->voffset = Get2Bytes();
    h->voffset <<= 16; h->voffset >>=16; /* sign xtend */
}

/*
 * read a long packet header
 */
pk_read_long_packet_header(fp, h)
    FILE *fp;
    PKPacketHeader *h;
{
    h->bitmap_length = Get4Bytes() - 28;
    h->code = Get4Bytes();
    h->tfm_width = Get4Bytes();
    h->dx = Get4Bytes();
    h->dy = Get4Bytes();
    h->width = Get4Bytes();
    h->height = Get4Bytes();
    h->hoffset = Get4Bytes();
    h->hoffset <<= 8; h->hoffset >>=8; /* sign xtend */
    h->voffset = Get4Bytes();
    h->voffset <<= 8; h->voffset >>=8; /* sign xtend */
}




unsigned char *
pk_read_bitmapped_char(fp, h)
    FILE *fp;
    PKPacketHeader *h;
{
int bytes_in_row = (h->width+7)/8;
unsigned char *data;
unsigned char *p, *q;
unsigned char mask;
int offset;
int k;
int row;
    data = (unsigned char *) malloc(bytes_in_row * h->height + 1);
    if (data == NULL) {
	printf ("tab: pk_read_bitmapped_char: not enough memory\n");
	return NULL;
    }
    p = q = data + bytes_in_row*h->height - h->bitmap_length + 1;
    for (k=0; k<h->bitmap_length; ++k)
	*p++ = GetByte(fp);
    
    p = data;
    offset = 0;
    mask = 0xff;
    for (row=0; row<h->height; ++row) {
	for (k=0; k<bytes_in_row; ++k) {
	    *p = ((*q << offset)) | ((*(q+1) >> (8-offset)) & mask);
	    ++p;  ++q;
	}
	if (offset < ((8 - h->width) & 0x7) ) 
	    --q;
	offset = (offset + h->width) & 0x7;
	mask = ~(0xff << offset);
    }

    return data;
}


static unsigned char Ones[8][9] = {
    {'\000','\200','\300','\340','\360','\370','\374','\376','\377'},
    {'\000','\100','\140','\160','\170','\174','\176','\177'},
    {'\000','\040','\060','\070','\074','\076','\077'},
    {'\000','\020','\030','\034','\036','\037'},
    {'\000','\010','\014','\016','\017'},
    {'\000','\004','\006','\007'},
    {'\000','\002','\003'},
    {'\000','\001'},
};

static unsigned char *BlackBits = NULL;

init_black_bits()
{
    int k;
    BlackBits = (unsigned char *) malloc(1000);
    if (BlackBits == 0) {
	printf ("tab: init_black_bits: not enough memory\n");
    }
    for (k=0; k<1000; ++k)
	BlackBits[k] = 0xff;
}


unsigned char *
pk_read_encoded_char(fp, h)
    FILE *fp;
    PKPacketHeader *h;
{
    int rowsleft = h->height;
    int bitsleft = h->width;
    int black = h->flag & 0x8;
    int bytes_in_row = (h->width + 7)/8;
    int count;
    int repeat_count = 0;
    unsigned char *data;
    unsigned char *curbyte;
    int offset;
    int b;

    if (BlackBits == NULL)
      init_black_bits();
    data = (unsigned char *) malloc(bytes_in_row * h->height);
    if (data == NULL) {
	printf ("pk_read_encoded_char: not enough memory\n");
	return NULL;
    }
    bzero(data, bytes_in_row * h->height);
    curbyte = data;
    offset = 0;

    pk_prepare_for_bitmap();
    while (rowsleft > 0) {

	if (pk_packed_number(fp, h->dyn_f, &count)) {
	    repeat_count = count;

	} else if (black) {
	    while (count >= bitsleft) {
		/* blacken rest of the current row */
		count -= bitsleft;
		b = (bitsleft>8-offset)? 8-offset : bitsleft;
		*curbyte |= Ones[offset][b];
		bitsleft -= b;
		bcopy(BlackBits, ++curbyte, bitsleft/8);
		curbyte += bitsleft/8;
		bitsleft = bitsleft & 0x7;
		if (bitsleft>0) {
		    *curbyte++ = Ones[0][bitsleft];
		}
		--rowsleft;
		/* repeat row */
		while (repeat_count-- > 0) {
		    bcopy(curbyte - bytes_in_row, curbyte, bytes_in_row);
		    curbyte += bytes_in_row;
		    --rowsleft;
		}
		bitsleft = h->width;
		offset = 0;
	    }

	    /* blacken a partial row */
	    if (count >= 8-offset) {
		*curbyte++ |= Ones[offset][8-offset];
		bitsleft -= 8-offset;
		count -= 8-offset;
		offset = 0;
		b = count/8;
		bcopy(BlackBits, curbyte, b);
		curbyte += b;
		bitsleft -= b*8;
		count -= b*8;
	    }

	    /* blacken a partial byte */
	    *curbyte |= Ones[offset][count];
	    offset += count;
	    bitsleft -= count;
	    black = !black;

	} else {
	    while (count >= bitsleft) {
		/* whiten rest of row */
		count -= bitsleft;
		if (offset > 0) {
		    ++curbyte;
		    bitsleft -= 8-offset;
		}
		curbyte += (bitsleft+7)/8;
		--rowsleft;
		/* repeat row */
		while (repeat_count-- > 0) {
		    bcopy(curbyte - bytes_in_row, curbyte, bytes_in_row);
		    curbyte += bytes_in_row;
		    --rowsleft;
		}
		bitsleft = h->width;
		offset = 0;
	    }
	    /* whiten a partial row */
	    if (count >= 8-offset) {
		++curbyte;
		bitsleft -= 8-offset;
		count -= 8-offset;
		offset = 0;
		b = count/8;
		curbyte += b;
		bitsleft -= b*8;
		count -= b*8;
	    }
	    /* whiten a partial byte */
	    bitsleft -= count;
	    offset += count;
	    black = !black;
	}
    }

    return data;
}

/*
 * read the bitmap into memory
 */
unsigned char *
pk_read_bitmap(fp, h)
    FILE *fp;
    PKPacketHeader *h;
{

#ifdef VAX
    b_seek (fp, h->address, 0);
#else
    fseek(fp, h->address, 0);
#endif
    if (h->dyn_f == 14) {
	return pk_read_bitmapped_char(fp, h);
    }
    else {
	return pk_read_encoded_char(fp, h);
    }

}

/* #endif */ /* POSTSCRIPT */

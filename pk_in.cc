/* pk_input.c	Samuel W. Bent		1/23/91 */

/* routines for reading PK files */
#ifndef __cplusplus
#ifndef lint
static char *rcsid = "$Header: /usr/everest/wbc/src/lute/RCS/pk_input.c,v 1.5 93/03/19 21:00:46 wbc Exp $";
#endif /* lint */
#endif /* cplusplus */
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


#ifdef sgi
#include <bstring.h>  /* for bzero */
#endif
#include "pkfile.h"
#include "tab.h"
#include "file_in.h"

/* #ifdef POSTSCRIPT */

void pk_read_short_packet_header(file_in *pk, PKPacketHeader *h);
void pk_read_extended_packet_header(file_in *pk, PKPacketHeader *h);
void pk_read_long_packet_header(file_in *pk, PKPacketHeader *h);
int pk_packet_type(unsigned int flag);
int pk_read_preamble(file_in *pk, PKPreamble *pre);
int pk_next_nybble(file_in *pk);
void pk_prepare_for_bitmap();
int pk_packed_number( file_in *pk, int dyn_f, int *result);
int pk_get_code_and_skip_packet(file_in *pk);
void pk_read_packet_header(file_in *pk, PKPacketHeader *h);
unsigned char * pk_read_bitmapped_char(file_in *pk, PKPacketHeader *h);
void init_black_bits();
unsigned char *
pk_read_encoded_char(file_in *pk, PKPacketHeader *h);
unsigned char *pk_read_bitmap(file_in *pk, PKPacketHeader *h);


/****************************************************************/
/*	Public Routines						*/
/****************************************************************/


int pk_read_preamble(
    file_in *pk,
    PKPreamble *pre)
{
int cmd, k;

    pk->Seek(0, 0);

    cmd =pk->GetByte();
    if (cmd != PK_PRE)
	return 1;

    pre->id = pk->GetByte();			/* read the information */
    pre->comment_length = pk->GetByte();
    for (k=0; k<pre->comment_length; ++k) 
	pre->comment[k] = pk->GetByte();
    pre->comment[k] = '\0';
    pre->design_size = pk->Get4Bytes();
    pre->checksum = pk->Get4Bytes();
    pre->hppp = pk->Get4Bytes();
    pre->vppp = pk->Get4Bytes();

    return 0;
}


static int current_input_byte, input_flag=0;

int
pk_next_nybble(file_in *pk)
{
    int next_nybble;
    if (input_flag) {
	next_nybble = current_input_byte & 0xf;
    } else {
	current_input_byte = pk->GetByte();
#ifdef MACXX
	if (current_input_byte == 10) {
		current_input_byte = 13;
			dbg0 (Bug, "input 10 converted to 13\n");
	}
	else if (current_input_byte == 13) {
		dbg0 (Bug, "input 13 converted to 10\n");
		current_input_byte = 10;
	}
#endif /* MAC */
//	 dbg1(Bug, "Current Input Byte %X\n", (void *)current_input_byte);
	next_nybble = (current_input_byte >> 4) & 0xf;
    }
    input_flag = !input_flag;
//    dbg1(Bug, "Next Nibble %d\n", (void *)next_nybble);
    return next_nybble;
}

void pk_prepare_for_bitmap()
{
    input_flag = 0;
}


/*
 * Get the next packed number; it's either a run-length or a repeat count.
 * Return the number itself in *result.  Return true if it's a repeat count,
 * false if it's a run count.
 */
int
pk_packed_number( file_in *pk, int dyn_f, int *result)
{
register int n, m;
    n = pk_next_nybble(pk);

    if (n==0) {				/* Self-delimiting number */
	do {
	    m = pk_next_nybble(pk);		/* find its length */
	    ++n;
	} while (m==0);
	while (n>0) {				/* read the number */
	    m = m*16 + pk_next_nybble(pk);
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
	m = pk_next_nybble(pk);
	*result =   (n-dyn_f-1)*16 + m + dyn_f + 1;
	return 0;
    } else

    if (n==14) {		/* variable repeat count */
	pk_packed_number(pk, dyn_f, result);
	return 1;
    } else

    if (n==15) {		/* repeat 1 */
	*result = 1;
	return 1;
    } else
    {  /* error */
	dbg1 (Warning, "pk_packed_number Error %d\n", (void *)n);
	return 0;
    }
}


/*
 * Skip packet, but return character code
 */
int pk_get_code_and_skip_packet(file_in *pk)
{
unsigned int flag;
int code;
long bitmap_length;
    flag = pk->GetByte();
    switch (pk_packet_type(flag)) {
	case PK_SHORT_PACKET:
	    bitmap_length =  ((flag & 0x3) << 8) + pk->GetByte() - 8;
	    code = pk->GetByte();
	    pk->Skip(8 + bitmap_length);
	    break;
	case PK_EXTENDED_PACKET:
	    bitmap_length = ((flag & 0x3) << 16) + pk->Get2Bytes() - 13;
	    code = pk->GetByte();
	    pk->Skip(13 + bitmap_length);
	    break;
	case PK_LONG_PACKET:
	    bitmap_length = pk->Get4Bytes() - 28;
	    code = pk->Get4Bytes();
	    pk->Skip(28 + bitmap_length);
	    break;
    }
    return code;
}


/*
 * Read character packet header
 */
void pk_read_packet_header(
    file_in *pk,
    PKPacketHeader *h)
{
    h->flag = pk->GetByte();
    h->dyn_f = h->flag >> 4;
    switch (pk_packet_type(h->flag)) {
	case PK_SHORT_PACKET:
	    pk_read_short_packet_header(pk, h);
	    break;
	case PK_EXTENDED_PACKET:
	    pk_read_extended_packet_header(pk,h);
	    break;
	case PK_LONG_PACKET:
	    pk_read_long_packet_header(pk, h);
	    break;
    }

    h->address = pk->Tell();
}


/*
 * determine what kind of packet you've got
 */
int pk_packet_type(unsigned int flag)
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
void pk_read_short_packet_header(file_in *pk, PKPacketHeader *h)
{
    h->bitmap_length =  ((h->flag & 0x3) << 8) + pk->GetByte() - 8;
    h->code = pk->GetByte();
    h->tfm_width = pk->Get3Bytes();
    h->dx = pk->GetByte() << 16;
    h->dy = 0;
    h->width = pk->GetByte();
    h->height = pk->GetByte();
    h->hoffset = pk->GetByte();
    h->hoffset <<=24;  h->hoffset >>=24;	/* sign extend */
    h->voffset = pk->GetByte();
    h->voffset <<=24;  h->voffset >>=24;	/* sign extend */
//    dbg2(Bug, "read_short_packet_header: length %d, voffset %d\n", 
//	   (void *)h->bitmap_length, (void *)h->voffset );
}


/*
 * read an extended packet header
 */
void pk_read_extended_packet_header(file_in *pk, PKPacketHeader *h)
{
    h->bitmap_length = ((h->flag & 0x3) << 16) + pk->Get2Bytes() - 13;
    h->code = pk->GetByte();
    h->tfm_width = pk->Get3Bytes();
    h->dx = pk->Get2Bytes() << 16;
    h->dy = 0;
    h->width = pk->Get2Bytes();
    h->height = pk->Get2Bytes();
    h->hoffset = pk->Get2Bytes();
    h->hoffset <<= 16; h->hoffset >>=16; /* sign xtend */
    h->voffset = pk->Get2Bytes();
    h->voffset <<= 16; h->voffset >>=16; /* sign xtend */
}

/*
 * read a long packet header
 */
void pk_read_long_packet_header(
    file_in *pk,
    PKPacketHeader *h)
{
    h->bitmap_length = pk->Get4Bytes() - 28;
    h->code = pk->Get4Bytes();
    h->tfm_width = pk->Get4Bytes();
    h->dx = pk->Get4Bytes();
    h->dy = pk->Get4Bytes();
    h->width = pk->Get4Bytes();
    h->height = pk->Get4Bytes();
    h->hoffset = pk->Get4Bytes();
    h->hoffset <<= 8; h->hoffset >>=8; /* sign xtend */
    h->voffset = pk->Get4Bytes();
    h->voffset <<= 8; h->voffset >>=8; /* sign xtend */
//    printf("read_long_packet_header: length %d\n", h->bitmap_length);
}




unsigned char *
pk_read_bitmapped_char(
    file_in *pk,
    PKPacketHeader *h)
{
    int bytes_in_row = (h->width+7)/8;
    unsigned char *data;
    unsigned char *p, *q;
    unsigned char mask;
    int offset;
    int k;
    int row;

//    printf ("tab: pk_read_bitmapped_char %d\n", h->code); 

    data = (unsigned char *) malloc(bytes_in_row * h->height + 1);
    if (data == NULL) {
	dbg1(Warning, "tab: pk_read_bitmapped_char: not enough memory %d\n", (void *)h->code);
	return NULL;
    }
    p = q = data + bytes_in_row*h->height - h->bitmap_length + 1;
    for (k=0; k<h->bitmap_length; ++k)
      *p++ = pk->GetByte();
    
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


static /* unsigned */ const char Ones[8][9] = {
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

void init_black_bits()
{
    int k;
    BlackBits = (unsigned char *) malloc(1000);
    if (BlackBits == 0) {
	dbg0 (Warning, "tab: init_black_bits: not enough memory\n");
    }
    for (k=0; k<1000; ++k)
	BlackBits[k] = 0xff;
}


unsigned char *
pk_read_encoded_char(
    file_in *pk,
    PKPacketHeader *h)
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

//    printf ("tab: pk_read_encoded_char %d\n", h->code); 

    if (BlackBits == NULL)
      init_black_bits();
    data = (unsigned char *) malloc(bytes_in_row * h->height + 1);
    if (data == NULL) 
	dbg1 (Error, "pk_read_encoded_char: not enough memory for %d bytes\n",
	      (void *)(bytes_in_row * h->height));

    memset(data, '\0', bytes_in_row * h->height);
    curbyte = data;
    offset = 0;

    pk_prepare_for_bitmap();
    while (rowsleft > 0) {

	if (pk_packed_number(pk, h->dyn_f, &count)) {
	    repeat_count = count;
//	    dbg2 (Bug, "pk read encoded char: count is %d tell %d\n",
//		    (void *)count, (void *)pk->Tell();

	} else if (black) {
//	    dbg3 (Bug, "pk read encoded char: black is %d tell %d left %d\n",
//		    (void *)count, (void *)pk->Tell(), 
//		  (void *)rowsleft);
	    while (count >= bitsleft) {
		/* blacken rest of the current row */
		count -= bitsleft;
		b = (bitsleft>8-offset)? 8-offset : bitsleft;
		*curbyte |= Ones[offset][b];
		bitsleft -= b;
		memcpy (++curbyte, BlackBits, bitsleft/8);
		curbyte += bitsleft/8;
		bitsleft = bitsleft & 0x7;
		if (bitsleft>0) {
		    *curbyte++ = Ones[0][bitsleft];
		}
		--rowsleft;
		/* repeat row */
		while (repeat_count-- > 0) {
		    memcpy (curbyte, curbyte - bytes_in_row, bytes_in_row);
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
		memcpy(curbyte, BlackBits, b);
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
//	    dbg3 (Bug, "pk read encoded char: else is %d tell %d left %d\n",
//		    (void *)count, (void *)pk->Tell(), 
//		  (void *)rowsleft;
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
		    memcpy(curbyte, curbyte - bytes_in_row, bytes_in_row);
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
pk_read_bitmap(file_in *pk, PKPacketHeader *h)
{
    
    pk->Seek (h->address, 0);

    if (h->dyn_f == 14) {
	return pk_read_bitmapped_char(pk, h);
    }
    else {
	return pk_read_encoded_char(pk, h);
    }

}

/* #endif */ /* POSTSCRIPT */

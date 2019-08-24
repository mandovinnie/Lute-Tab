/*

*/
#include "win.h"
#include "tab.h"
#include "pk_bit.h"
#include "pkfile.h"
#include "file_in.h"
#include "ps_print.h"

PKBit bits[257];
char *pk_buf;
int pk_ptr, pk_bufsize;
struct b **b;
extern struct b *ifb[];
signed char get_p(struct b *i);

extern int max_b_w;
extern int max_b_h;
extern int max_off_w;
extern int max_off_h;

/* LOCAL */
void read_pk_file(file_in *pk, ps_print *ps);
void save_bitmap(unsigned char *data , PKPacketHeader *h);
void pk_dump_special(file_in *pk, int length);
int ps_is_used(unsigned char c);
void print_data(unsigned char *data, PKPacketHeader *h);

#include "pk_input.h"
void read_pk_file(file_in *pk, print *ps)
{
    unsigned int cmd;
    int length;
    PKPacketHeader header;
    PKPreamble pre;
    unsigned char *bitmap;

    while ((cmd = pk->GetByte()) != EOF && ((int)cmd != 255 )
	   && (int)cmd != -1 && cmd != (unsigned int)EOF) {

	switch (cmd) {

	  case PK_PRE:
	    pk->GetByte();
	    length = pk->GetByte(); /* length of comment */
	    pk->Seek(length, 1);
	    pre.design_size = pk->Get4Bytes();
	    pre.checksum = pk->Get4Bytes();

//	    if ( pre.checksum != pk_checksum[0] ) {
//		printf ("tab: read_pk_file: checksum mismatch %X %X\n",
//			pre.checksum, pk_checksum[0]);
//	    }
	    pre.hppp = pk->Get4Bytes();
	    pre.vppp = pk->Get4Bytes();
	    break;

	  case PK_POST:
	    break;

	  case PK_NOOP:
	    break;

	  case PK_XXX1:
	    length = pk->GetByte();
	    pk_dump_special(pk, length);
	    break;

	  case PK_XXX2:
	    length = pk->Get2Bytes();
	    pk_dump_special(pk, length);
	    break;

	  case PK_XXX3:
	    length = pk->Get3Bytes();
	    pk_dump_special(pk, length);
	    break;

	  case PK_XXX4:
	    length = pk->Get4Bytes();
	    pk_dump_special(pk, length);
	    break;

	  case PK_YYY:
	    length = pk->Get4Bytes();
	    break;

	  default:
	    pk->unGet(cmd);
/*	    printf( "about to read %d\n", pk->Tell()); */
	    pk_read_packet_header(pk, &header);
/*	    printf( "header read %d %d\n", pk->Tell(), header.code);  */

	    dbg2(Bug, "read pk file: %d %c\n",
		 (void *)header.code,
		 (void *)header.code );

	    bitmap = pk_read_bitmap(pk, &header);
	    dbg1( Bug, "bitmap read %d\n", (void *)pk->Tell());

	    if (ps->print_used[header.code]){
		save_bitmap(bitmap, &header);
	    }
	    free(bitmap);
	    break;
	}
    }
}


void pk_dump_special(file_in *pk, int length)	/* swallow special commands */
{
    for (; length>0; --length) pk->GetByte();
}


void save_bitmap(unsigned char *data, PKPacketHeader *h)
{
    int row, col;
    unsigned char *p;
    PKBit *b;
    int byte_w;			/* width in bytes */

    b = &bits[h->code];
    b->bm_char = h->code;;
    b->bm_w = h->width;
    if (b->bm_w > max_b_w) max_b_w = b->bm_w;
    byte_w = (b->bm_w + 7 ) / 8;
    b->bm_h = h->height;
    if (b->bm_h > max_b_h) max_b_h = b->bm_h;
/*    printf ("wbc save_bitmap %08X\n", h->hoffset); */
    b->bm_h_off = h->hoffset;
    if (b->bm_h_off > max_off_w) max_off_w = b->bm_h_off;
    b->bm_v_off = h->voffset;
    if (b->bm_v_off > max_off_h) max_off_h = b->bm_v_off;
    if (byte_w * b->bm_h == 0)
      dbg1 (Warning, "tab: save_bitmap: 0 size bitmap %d\n",
	    (void *)h->code);
    else {
	b->bm_bits = (char *) malloc ( byte_w * b->bm_h);
	if (!b->bm_bits)
	  dbg1 (Error, "tab: save_bitmap: malloc failed !!!!%d\n",
		 (void *)h->code);
    }
    p = data;

    for (row= h->height; row ; --row) {
	for (col=0; col  < byte_w; ++col) {
	    b->bm_bits[((row-1) * byte_w) + col] = *p;
	    ++p;
	}
    }
}

/* void print_data(
    unsigned char *data,
    PKPacketHeader *h)
{
int row, col;
int k;
char c;
unsigned char *p;
    p = data;
    printf("     ");
    for (col=0; col<h->width; col+=5)
	printf("|%-4d", col-h->hoffset);
    printf("\n");
    for (row=0; row<h->height; ++row) {
	printf("%4d ", h->voffset - row);
	for (col=0; 8*col<h->width; ++col) {
	    for (k=7; k>=0; --k) {
		c = (8*col+7-k>=h->width)? '-' : (*p & (1<<k))? 'M' : '.';
		printf("%c", c);
	    }
	    ++p;
	}
	printf("\n");
    }
}
*/



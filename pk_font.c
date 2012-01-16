/*

 */
#include <stdio.h>
#include <string.h>
/*
#include <sys/types.h>
#include <sys/stat.h>
*/
#include "tab.h"
#include "pk_bit.h"

static FILE *fp;
PKBit bits[257];
char *pk_buf;
int pk_ptr, pk_bufsize;

#ifdef POSTSCRIPT
#define PK_USED
#endif

#ifdef X_WIN
#define PK_USED
#endif

#ifndef VAX
#define GetByte(f) getc(f)
#else

unsigned char GetByte(f)
FILE * f;
{
    unsigned char val;
    val = pk_buf[pk_ptr];

    if (pk_ptr >= (pk_bufsize)){
	return (EOF);
    }
    pk_ptr++;
    return ((int)(val & 0xff));
}

b_seek(stream, offset, ptrname)
FILE *stream; long offset; int ptrname;	/* 0 rew 1 cur 2 end */
{
    switch (ptrname) {
      case 0:
	pk_ptr = 0 + (offset);
	break;
      case 1:
	pk_ptr += (offset);
	break;
      case 2:
	pk_ptr = pk_bufsize-1;
	pk_ptr -= offset;
	break;
      default:
	break;
    }
}
b_ungetc(c, stream)
unsigned char c; FILE *stream;
{
    pk_ptr--;
}
b_ftell(stream)
FILE *stream;
{
    return(pk_ptr);
}
#endif /* VAX */

/* #ifdef PK_USED */

open_pk_file(f, ff)
char * f;
struct file_info *ff;
{
    char path[600];
    char file_path[120];
    char *p, *pp;
#ifdef vax
    int nread;
#endif /* vax */

#ifdef TFM_PATH

    strcpy (path, TFM_PATH );
    p = path;

    while (fp == NULL) {	/* try system location */
	pp = strchr(p, ':');
	if ( pp ) {
	    *(pp++) = '\0';
	}
	strcpy (file_path, p);
	strcat (file_path, f);
/*	printf ("tfm_file: path is %s\n", file_path); */
	fp = fopen (file_path, "r");
	if ( pp )
	  p = pp;
	else break;
    }
#else 
    fp = fopen(f, "r");
#endif /* TFM_PATH */

    if (fp == NULL) {
	printf ("tab: open_pk_file: Can't open pk font file %s\n", f);
#ifdef TFM_PATH
	printf ("font path is %s\n", TFM_PATH);
#endif /* TFM_PATH */
	exit (-1);
    }
    if (ff->flags & DEBUG ) {
	printf ("tab: open_pk_file: path is %s\n", file_path);
    }

#ifdef VAX
    fseek(fp, 0L, 2);
    pk_bufsize = ftell(fp);
    fseek(fp, 0L, 0);
    pk_buf = (char *)malloc(pk_bufsize);
    if(pk_buf <= 0) {
	printf ("tab: open_pk_file: malloc failed\n");
	printf ("tab: trying to get %d bytes\n", pk_bufsize);
	exit (-1);
    }
/* this always fails! */
    if ((nread = fread (pk_buf, 1, pk_bufsize, fp)) != pk_bufsize) {
	printf ("tab: open_pk_file %s %d wanted %d read\n", 
		f,
		pk_bufsize, nread);
    }
 /* */
#endif /* VAX */
}


/* this was copied from Sam Bent */

int input_error;

/* read next two bytes and return as integer */
Get2Bytes()
{
    unsigned int n, a,b;
    a = GetByte(fp);   b = GetByte(fp);
    n = (a<<8) | b;
    n <<= 16;			/* extend the sign */
    n >>= 16;
    input_error = (b==EOF);
    return n;
}


/* read next three bytes and return as integer */
Get3Bytes()
{
    unsigned int n, a,b,c;
    a = GetByte(fp);   b = GetByte(fp);   c = GetByte(fp);
    n = (((a<<8) | b) << 8) | c;
    n <<= 8;			/* extend the sign */
    n >>= 8;
    input_error = (c==EOF);
    return n;
}


/* read next four bytes and return as integer */
Get4Bytes()
{
    unsigned int n, a,b,c,d;
    a = GetByte(fp);   b = GetByte(fp);   c = GetByte(fp);  d = GetByte(fp);
    n = (((((a<<8) | b) << 8) | c) << 8) | d;
    input_error = (d==EOF);
    return n;
}


/* Skip ahead n bytes in file */
Skip(n, f)
    int n;
    FILE *f;
{
#ifdef VAX
    input_error = b_seek(f, n, 1);
#else
    input_error = fseek(f, n, 1); 
#endif /* VAX */
}


#include "pk_input.h"
read_pk_file()
{
int cmd;
int length;
PKPacketHeader header;
PKPreamble pre;
unsigned char *bitmap;

    while ((cmd = GetByte(fp)) != EOF && cmd != 255) {

	switch (cmd) {

	    case PK_PRE:
		GetByte(fp);
		length = GetByte(fp); /* length of comment */
#ifdef VAX
		b_seek(fp, length, 1);
#else
		fseek(fp, length, 1); /* comment  */
#endif /* VAX */
		pre.design_size = Get4Bytes();
		pre.checksum = Get4Bytes();
		
		if ( pre.checksum != pk_checksum[0] ) {
		    printf ("tab: read_pk_file: checksum mismatch %X %X\n",
			    pre.checksum, pk_checksum[0]);
		}
		pre.hppp = Get4Bytes();
		pre.vppp = Get4Bytes();
		break;

	    case PK_POST:
		break;

	    case PK_NOOP:
		break;

	    case PK_XXX1:
		length = GetByte(fp);
		pk_dump_special(fp, length); 
		break;

	    case PK_XXX2:
 		length = Get2Bytes();
		pk_dump_special(fp, length);
		break;

	    case PK_XXX3:
		length = Get3Bytes(); 
		pk_dump_special(fp, length);
		break;

	    case PK_XXX4:
 		length = Get4Bytes(); 
		pk_dump_special(fp, length);
		break;

	    case PK_YYY:
		length = Get4Bytes();
		break;

	    default:
#ifdef VAX
		b_ungetc(cmd, fp);
#else
		ungetc(cmd, fp);
#endif /* VAX */
		pk_read_packet_header(fp, &header);
		bitmap = pk_read_bitmap(fp, &header);
#ifdef vax
		if (cmd == 255 || cmd == -1) {
		    printf ("read_pk: cmd is EOF\n");
		}
/*		if ( !header.code )
		  printf ("read_pk: header code %c %d\n",
			header.code, header.code);
*/
#endif /* VAX */
		if (ps_used[header.code]){
		    save_bitmap(bitmap, &header);
		    /* 		print_data(bitmap, &header); */
		    free(bitmap);
		}
		break;
	}
    }
}


pk_dump_special(f, length)	/* swallow special commands */
    FILE *f;
    int length;
{
    for (; length>0; --length) GetByte(f);
}


save_bitmap(data, h)
unsigned char *data;
PKPacketHeader *h;
{
    int row, col;
    unsigned char *p;
    PKBit *b;
    int byte_w;			/* width in bytes */
    char *my_malloc();

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
    b->bm_bits = (char *)my_malloc ( byte_w * b->bm_h); 
    if (!b->bm_bits) {
	printf ("tab: save bitmap: malloc failed !!!!!!!!!!!!!!!!!%d\n",
		 h->code);
	exit (-1);
    }

    p = data;

    for (row= h->height; row ; --row) {
	for (col=0; col  < byte_w; ++col) {
	    b->bm_bits[((row-1) * byte_w) + col] = *p; 
	    ++p;
	}
    }
}

print_data(data, h)		/* by sam, completely */
    unsigned char *data;
    PKPacketHeader *h;
{
int row, col;
int k;
char c;
unsigned char *p;
    p = data;
    printf("     ");
    for (col=0; col<h->width; col+=5)
	printf("|%-4d", col-h->hoffset);
    putchar('\n');
    for (row=0; row<h->height; ++row) {
	printf("%4d ", h->voffset - row);
	for (col=0; 8*col<h->width; ++col) {
	    for (k=7; k>=0; --k) {
		c = (8*col+7-k>=h->width)? '-' : (*p & (1<<k))? 'M' : '.';
		putchar(c);
	    }
	    ++p;
	}
	putchar('\n');
    }
}
/* #endif */ /* PK_USED */

static malloc_init=0;
static char *malloc_buf=0;
static char *malloc_ptr;
static int malloc_total=0;
static int grand_total=0;
static int buffers = 1;

#define BFSIZ 1024 * 8  /* if BFSIZ is 1024 it crashes in malloc */

char *
my_malloc(b)
unsigned int b;
{
    char *ptr;
    int s;
	int align_s;	/* size to align on */

    if (! malloc_init) {
	malloc_init++;
	malloc_buf = (char *)malloc((unsigned int)BFSIZ);
	malloc_ptr = malloc_buf;
	if (!malloc_buf  ) {
	    printf (
		     "tab: pk_font: my_malloc: can't get %d bytes of memory!\n",
		     BFSIZ);
	    exit (-3);
	}
/*	printf ( "my_malloc - new buffer\n"); */

    }
    /* be sure to align the block for pointers */

    if (b > BFSIZ/6) {
	ptr = (char *)malloc (b);
	if (!ptr) printf ("tab: my_malloc: out of memory\n");
	return (ptr);
    }
	align_s = sizeof(char * );
#ifdef hpux
	align_s = sizeof(double *) * 2;
#endif /* hpux */
    s = (((b-1)/align_s) + 1) * align_s;
    if (malloc_total + s >= BFSIZ - 100) {
	malloc_buf = 0;
	malloc_buf = (char *)malloc((unsigned int)BFSIZ);
	malloc_ptr = malloc_buf;
	malloc_total = 0;
	printf ("#");
	if (!malloc_buf) {
	    printf (
		   "tab: pk_font: my_malloc: can't get %d bytes of memory!\n",
		     BFSIZ);
	    exit (-3);
	}
	buffers++;
    }
    ptr = malloc_ptr;
    malloc_ptr += s;
    malloc_total += s;
    grand_total += s;
/*    printf ("ptr %X\n", ptr); */
    return (ptr);
}

malloc_stat()
{
    printf ("tab: %d bytes out of %d in heap used\n", 
		grand_total, buffers * BFSIZ);
}

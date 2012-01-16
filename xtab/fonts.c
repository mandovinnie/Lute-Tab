
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include "../pk_bit.h"

extern Display *dis;
extern Widget art;
extern GC gc;
extern struct font_d *fonts[];

XImage *(xim[256]);		/* global to font and draw routines */

x_clear_page()
{
    XClearWindow(dis, XtWindow(art));
}

xp_put_a_char(c, x, y)		/* text font */
     unsigned char c;
     int x, y;			/* x and y from origin */
{
    XDrawString( dis, XtWindow(art), gc,
		x, y, &c, 1);
}

xp_put_char(c, x, y)
     unsigned char c;
     int x, y;			/* x and y from origin */
{
#define LUTE_FONT 0
    extern PKBit bits[];
    
    XPutImage( dis, XtWindow(art), gc,
	     xim[c], 0, 0, x, y - bits[c].bm_v_off, 
	      xim[c]->width, xim[c]->height );
}

xp_put_rule(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
    XDrawRectangle( dis, XtWindow(art), gc,
	      x1, y1, x2, y2);
}
xp_put_line(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
    XDrawLine( dis, XtWindow(art), gc,
	      x1, y1, x2, y2);
}

get_font()
{
    extern PKBit bits[];
    extern int ps_used[];
    int i;

    for (i=0; i < 256; i++) ps_used[i]++;

    open_tfm_file(0, "lute9.tfm");
    open_pk_file("lute9.118pk");
    read_pk_file();

    for (i=0; i < 254; i++) if (bits[i].bm_w) get_char_map(i);

    printf ("fonts read\n");
}
	

get_char_map(num)
     int num;
{
    extern PKBit bits[];
    PKBit *b;
    XImage *m;
    int i, j, k,  byte_w;
    char t_b[64 * 64 * 64];		/* temp space for reordering */
    char t;
    char s_b();

    b = &bits[num];
    byte_w = (b->bm_w + 7) / 8;

    bcopy (b->bm_bits, t_b, b->bm_h * byte_w ); 
    for (i=0; i < b->bm_h ; i++) { 
	for (j = 0; j < byte_w; j++) {
	    if (XBitmapBitOrder(dis) == LSBFirst) { /* dec */
		b->bm_bits[((b->bm_h - 1) - i) * byte_w + j] = 
		  s_b((unsigned char)  t_b[i * byte_w + j]);
	    }
	    else {/* BitmapBitOrder MSB First - sun */
		b->bm_bits[((b->bm_h - 1) - i) * byte_w + j] = 
		  ((unsigned char)  t_b[i * byte_w + j]);
	    }
	}
    }
    
    m = XCreateImage(dis, XtWindow(art), 
	    1           /* depth */,
	    XYBitmap    /* format */,
            0           /* offset */,
	    b->bm_bits  /* data */,
	    b->bm_w     /* width */,
	    b->bm_h     /* height */,
            XBitmapPad  /* bitmap pad */,
	    byte_w      /* bytes_per_line */  );
    xim[num] = m;  
}

do_box(box)			/* print all characters */
     Widget box;
{
    extern PKBit bits[];
    int i, line=50, col=25;

    for (i=0; i< 255; i++) {
	if (bits[i].bm_w) {
	    col += 30;
	    if (col > 800) {
		col = 25;
		line += 100;
	    }
	    xp_put_char(i, col, line);
	}
    }
}

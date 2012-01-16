#include <stdio.h>

/* flip bits around
 */

char s_b(b)
     unsigned char b;
{
    unsigned char t;
    int i;

    t = 0;
    b &= 0x00ff;

    if (b & 0X0001) t |= 0X80;
    if (b & 0X0002) t |= 0X40;
    if (b & 0X0004) t |= 0X20;
    if (b & 0X0008) t |= 0X10;
    if (b & 0X0010) t |= 0X08;
    if (b & 0X0020) t |= 0X04;
    if (b & 0X0040) t |= 0X02;
    if (b & 0X0080) t |= 0X01;

    return (t);
}

/*
 *  dump a bitmap to a terminal!
 *
 */

p_c(h, w, b)
int h, w;
char *b;
{
    int i, j, k;
    
    for ( i=0; i < h; i++) {
	for (j = 0; j < w; j++) {
	    for (k=7; k >= 0; k--){
		if (b[i*w+j] & 1<<k) putchar('X');
		else putchar('.');
	    }
	}
	putchar('\n');
    }
}


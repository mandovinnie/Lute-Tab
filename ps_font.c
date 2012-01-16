/*

 */

#include <stdio.h>
#include "tab.h"
#include "pk_bit.h"

#ifdef POSTSCRIPT

#define CHARS 256
#define MYCHARS 255		/* leave room for /.notdef */

make_ps_font(o, f)
     FILE *o;
     struct file_info *f;
{
    int i;
    char name[8];
    PKBit *b;
    extern PKBit bits[];
    double dvi_to_dots = 72.0 / 300.0;

    if (f->flags & DPI600) dvi_to_dots /= 2.0;

    fprintf (o, "10 dict begin\n");
    fprintf (o, "/FontType 3 def\n");
    fprintf (o, "/FontMatrix [%f 0 0 %f 0 0] def\n",
	     dvi_to_dots, dvi_to_dots);
    fprintf (o, "/FontBBox [%d %d %d %d] def\n",
	     - max_off_w, - max_off_h,
	     max_b_w, max_b_h); /* set to max b_box */

    fprintf (o, "/En {Encoding} def\n");
    fprintf (o, "/Encoding 256 array def\n");
    fprintf (o, "0 1 %d {Encoding exch /.notdef put} for\n", MYCHARS);
    for (i=0; i < 254; i++) {
	if (bits[i].bm_w && ps_used[i])
	  fprintf (o, "En %d /b_%d put\n", i, i);
    }

    fprintf (o, "/CharProcs %d dict def\n",
	     CHARS);
    fprintf (o, "CharProcs begin\n");
    fprintf (o, " /.notdef { 1 1 add pop } bind def\n");

/* bitmaps loaded here */

    for (i=0; i < 254; i++) {
	if (bits[i].bm_w && ps_used[i]) {
	    sprintf ( name, "b_%d", i);
	    bit_char(name, i);
	} 
    }
    fprintf (o, "end %% of CharProcs\n");

    fprintf (o, "/CharWidths %d dict def CharWidths begin\n", 
	     CHARS);
	fprintf (o, "/.notdef [ 0 0 0 0 ] def\n");
    for (i=0; i < 254; i++) {
	b = &bits[i];
	if (b->bm_w && ps_used[i])	/* assume null chars have 0 width */
	  fprintf (o, "/b_%d [%d %d %d %d ] def\n", i,
		   b->bm_w - b->bm_h_off -1, b->bm_v_off,
		   - b->bm_h_off, b->bm_v_off - b->bm_h + 1);
    }
    fprintf (o, "end %% of CharWidths\n");
      
    fprintf (o, "/BuildGlyph {\n");
/* x width, y width, x, y lower left bounding box, x, y upper right b box */
    fprintf (o, " exch dup /CharWidths get 3 copy exch pop exch get dup dup dup\n");
    fprintf (o, " 0 get /MaxXVal exch def 1 get /MaxYVal exch def \n");
    fprintf (o, " 2 get /MinXVal exch def 3 get /MinYVal exch def pop \n");

    fprintf (o, " MaxXVal 0 MinXVal MinYVal MaxXVal MaxYVal\n");
    fprintf (o, " setcachedevice\n"); 
    fprintf (o, " /CharProcs get exch\n");
    fprintf (o, " 2 copy known not {pop /.notdef} if\n");
    fprintf (o, " get exec\n");
    fprintf (o, "} bind def\n");

    fprintf (o, "/BuildChar {\n");
    fprintf (o, " 1 index /Encoding get exch get\n");
    fprintf (o, " 1 index /BuildGlyph get exec\n");
    fprintf (o, "}bind def\n");

    fprintf (o, "currentdict\n");
    fprintf (o, "end              %% of dict\n");
    fprintf (o, "/LuteFont exch definefont pop\n"); /* define big dict */

    fprintf (o, "/Rule { 2 copy 0 rlineto 0 exch rlineto neg 0 rlineto \n");
    fprintf (o, " 0 exch neg rlineto fill} def\n");

    fprintf (o, "/RM {rmoveto} def\n");
    fprintf (o, "/MT {moveto} def\n");
    fprintf (o, "/S {show} def\n");
    fprintf (o, "/languagelevel where {pop languagelevel} {1} ifelse\n");
    fprintf (o, "2 lt { \n");
    fprintf (o, "/X {gsave show grestore} def\n");
    fprintf (o, "} { \n");
    fprintf (o, "/X {[0 0] xshow} def\n");
    fprintf (o, "} ifelse\n");
    fprintf (o, "/FF {findfont} def /SF {scalefont setfont} def\n");
    fprintf (o, "/doslur { /delta exch def\n");
    fprintf (o, "gsave 1 setlinecap 0.7 setlinewidth\n");
    fprintf (o, "/delta delta 3 div def /height 5 def\n");
    fprintf (o, "currentpoint 2 copy 2 copy\n");
    fprintf (o, "/y1 exch def /x1 exch def /y2 exch def /x2 exch def\n");
    fprintf (o, "/y3 exch def /x3 exch def /x1 x1 delta add def\n");
    fprintf (o, "/y1 y1 height add def /x2 x2 delta 2 mul add def\n");
    fprintf (o, "/y2 y1 def /x3 x3 delta 3 mul add def\n");
    fprintf (o, "x1 y1 x2 y2 x3 y3 curveto stroke grestore } def\n");
    fprintf (o, "\n");
}

bit_char(char_name, char_num)
char *char_name;
int char_num;
{
    int i;
    PKBit *b;
    extern PKBit bits[];
    int byte_w;

    b = &bits[char_num];
    byte_w = (b->bm_w + 7 ) / 8; 

    fprintf (op, "/%s {", char_name);
    fprintf (op, " %d %d true [1 0 0 1 %d %d]",
	     b->bm_w, b->bm_h,
	     b->bm_h_off , b->bm_h - b->bm_v_off );
/*    printf (" wbc bm_h_off %d %X\n", b->bm_h_off, b->bm_h_off); */
/* bitmap */

    fprintf (op, "\n{<");
    for (i=0; i < byte_w * b->bm_h; i++) {
	fprintf (op, "%02X", b->bm_bits[i] & 0xff);
	if (!((i+1) % 40)) fprintf (op, "\n");
    }
    fprintf (op, ">}\n imagemask } bind def\n");


/*  looks like data compression won't work on our printers - sigh! wbc
    fprintf (op, " currentfile /RunLengthDecode filter ");
    fprintf (op, " imagemask");

    fprintf (op, "\n<");
    rle(op, byte_w * b->bm_h, b->bm_bits);
    fprintf (op, "> \n" );

    fprintf (op, " } bind def\n");
 */
 /* end bitmap */
}

rle(op, size, data)
FILE * op;
int size;
char *data;
{
    int i, start;
    int count=0;
    char *p = data;

/* encode runs > 3 */

    start = -1;
    for (i=0; i < size; i++) {
	if (i % 64 ) {
	    while (count) {
		fprintf (op, "%02X", count);
		fprintf (op, "%02X", *p & 0xff);
		p++;
		count--;
	    }
	}
	count++;
    }
    fprintf (op, "%02X", 128);	/* End Of Data */
}

change_ps_font(line)
     char *line;
{
    int num;
    char name[80];

    name[0] = '\0';

    sscanf ( line, "F %d %s", &num, name);
    set_ps_font (num, name);

}

/* copy a new name into font name array */

set_ps_font(num, name)
int num;
char * name;
{
    int len;
    extern char * ps_font[];

/*    printf ("tab: setting font %d to %s\n", num, name);
 */
    len = strlen(name);
    strcpy (ps_font[num], name);
}
#endif /* POSTSCRIPT */


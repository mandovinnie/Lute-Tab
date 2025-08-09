/*
 *
 *
 */
#include "win.h"
#if defined _WIN32
#include <io.h>
#endif /* WIN32 */

#include "version.h"
#include "tab.h"
#include "print.h"
#include "ps_print.h"
#include "ps.h"
#include "file_in.h"
#include "i_buf.h"
#include <time.h>
#include "pk_bit.h"
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif /* _WIN32 */
#include <fcntl.h>


void read_pk_file(file_in *pk, print *ps);

int format_page(print *d_p, i_buf *i_b, font_list *f_l[], struct file_info *f);
void bit_char(i_buf *ps, int char_num, int sixh);
int my_isprint(int c);

int max_b_w=0;                  /* max bitmap width for ps */
int max_b_h=0;                  /* max bitmap height for ps */
int max_off_w=0;                /* max bitap width offset for ps */
int max_off_h=0;                /* max bitmap height offset for ps */

extern char *font_path;
int bar_number_font=1;		/* number of font to use numbering bars */

ps_print::ps_print(font_list *font_array[], file_info *f)
{
    int j;
    int tmp;

    //    printf ("val %d\n", inch_to_dvi(0.4218));
    //    printf ("val %d\n", mm_to_dvi(17.6));
    //    printf ("val %f\n", dvi_to_inch(98670000));
    npages=0;
    if (f->m_flags & A4 ) { // a4 is  297mm*210mm
      if (!(f->flags & ROTATE))
	ps_top_of_page = 98670000 +  6563672/* 6712847 */;
      else
	ps_top_of_page = 98670000;
    }
    else
      ps_top_of_page = 98670000;
    /* printf("ps_print: top of page %f in  %f mm  %u dvi-units \t%u=32.42mm\n",
    	   dvi_to_inch(ps_top_of_page),
	   dvi_to_mm(ps_top_of_page),
	   ps_top_of_page,
	   mm_to_dvi(32.42)); */
    /*     for (int i=0; i< 256; i++) ps_used[i] = 0; */
    // int pt_to_dvi(double pt)
    tmp = (72 - f->top_margin);
    // printf("ps_print: tmp %d\n", tmp);
    tmp = pt_to_dvi((double)tmp);
    // tmp = -2;
    // printf("ps_print: f->top_margin: %d %d tmp: %d %d\n", f->top_margin, pt_to_dvi(f->top_margin), (72 - f->top_margin), tmp);
    ps_top_of_page += tmp;
    // printf("ps_print: top of page %d\n", ps_top_of_page);
    f_i = f;
    f_a = font_array;
    f_name[0] = '\0';
    for ( j=0; j<FONT_NAMES; j++)
      font_sizes[j] = f->font_sizes[j];
    if (*f->out_file) {
      strcat(f_name, f->out_file);
    }
    highlight_type=Gray;
    if (f->m_flags & SOUND)
      nodump = 1;
    else nodump = 0;
    left_margin = f->left_margin;
    bar_number_font= f->bar_number_font;
}

ps_print::~ps_print()
{
    file_head();		// file head dumps header
    file_trail();
    if (f_name[0]) {
      if (!nodump)
	pr_out->dump( f_name, Append);
    }
    else
      pr_out->dump( "out.ps", Append);
    //should we expliciteliy delete things here?
    // need to delete ps_print object
    // need to delete print object
}


void ps_print::file_head()
{
    i_buf ps_header;
    time_t t;
    char pk_name[300];

    char *p = NULL;


    if (nodump)
      return;

    ps_header.PutString("%!PS-Adobe-3.0");
    if (f_i->m_flags & EPSF )
      ps_header.PutString(" EPSF-3.0");
    ps_header.PutString("\n");

    if (!(f_i->flags & ROTATE) && npages < 2) {
	int val;
	if ( ! (f_i->m_flags & NOBOX) ) {
	  ps_header.PutString("%%BoundingBox:");
	  /* */
	  ps_header.Put10(55 - (72 - left_margin));
	  ps_header.PutString(" ");
	  val = (int)((dvi_to_inch(dvi_v) - 2.75 ) * 72.27 * red);
	  ps_header.Put10(val);
	  /* */
	  ps_header.PutString(" 555 735\n");
	}
    }
    else if (ROTATE) {	// yes, rotate
	if ( ! (f_i->m_flags & NOBOX) ) {
	  ps_header.PutString("%%BoundingBox:  45 68 "); // 45 725
	  ps_header.PutString("620");
	  ps_header.PutLine(" 725\n"); // full line
	}
    }
    else /* no rotate, many pages */
	ps_header.PutString("%%BoundingBox:  50 65 555 740\n");
    ps_header.PutString("%%Creator: (tab, version ");
    ps_header.PutString(VERSION);
    ps_header.PutString(", by Wayne Cripps)\n");
    ps_header.PutString("%%Creation Date: ");
    t = time(0);
    ps_header.PutString( ctime(&t));
    ps_header.PutString("%%Orientation: ");
    if (!(f_i->flags & ROTATE))
	ps_header.PutString("Portrait\n");
    else
	ps_header.PutString("Landscape\n");
    ps_header.PutString("%%Pages: ");
    ps_header.Put10(npages);
    ps_header.PutString("\n");
    ps_header.PutString("%%PageOrder: Ascend\n");
#ifndef MAC
    ps_header.PutString("%%Title: (");
    ps_header.PutString(f_i->file);
    ps_header.PutString(")\n");
#endif
    ps_header.PutString("%%EndComments\n");

    ps_header.PutString("%%BeginDefaults\n");
//    ps_header.PutString("%%PageBoundingBox: 50 65 555 740\n");
    ps_header.PutString("%%PageOrientation: ");
    if (!(f_i->flags & ROTATE))
	ps_header.PutString("Portrait\n");
    else
	ps_header.PutString("Landscape\n");
    ps_header.PutString("%%EndDefaults\n");

    ps_header.PutString("%%BeginProlog\n");
    ps_header.PutString("%%EndProlog\n");
    ps_header.PutString("%%BeginSetup\n");
    if (f_i->flags & DPI600)  {
	ps_header.PutString("%%Feature: *Resolution 600\n");
        // printf ("ps_print.cc: 179: 600 \n");
    }
    else if (f_i->m_flags & DPI1200)
	ps_header.PutString("%%Feature: *Resolution 1200\n");
    else if (f_i->m_flags & DPI2400)
	ps_header.PutString("%%Feature: *Resolution 2400\n");
    else if (f_i->m_flags & DPI360) {
	ps_header.PutString("%%Feature: *Resolution 360\n");
        // printf ("ps_print.cc: 187: 360 \n");
    }
    else ps_header.PutString("%%Feature: *Resolution 300\n");

#ifdef MAC
    strcpy(pk_name, "");
#else

      if (font_path) {
	//	fprintf (stderr, "ps_print.c - setting font path %s from command line\n", font_path);
	p = font_path;
      }
      else
	p = getenv("TABFONTS");

    if (p == NULL ) {
      char *font_file = (char*)"fonthome";
      struct stat buf;
      int ffd;
      int nr;
      char *c;
      char ffd_name[300];

      if (!stat(font_file, &buf)) { // stat returns 0 on success
#ifdef BUILD_FOR_WINDOWS
	ffd = _open(font_file, O_RDONLY);
#else
	ffd = open(font_file, O_RDONLY);
#endif
	if (ffd != -1 ) {
#ifdef BUILD_FOR_WINDOWS
	  nr = _read(ffd, ffd_name, sizeof(ffd_name));
#else
	  nr = read(ffd, ffd_name, sizeof(ffd_name));
#endif
	  if (nr < sizeof(ffd_name))
	    ffd_name[nr] = 0;
	  c = strchr(ffd_name, '\n');
	  if (c) *c = 0;
	  p=ffd_name;
	}
      }
    }
    if (p == NULL )

#ifdef TFM_PATH
	strcpy(pk_name, TFM_PATH);
#else
    strcpy(pk_name, ".");
#endif /* TFM_PATH */
    else
	strcpy(pk_name, (char *)p);

    strcat (pk_name, "/");
#endif /* MAC */
    if (f_i->font_names[0]) {
      strcat (pk_name, f_i->font_names[0]);
    }
    else {
      if (baroque) {
	strcat (pk_name, "blute");
      }
      else if (thin_renaissance) {
	strcat (pk_name, "tlute");
      }
      else
	strcat (pk_name, "lute");
    }
    if (red == 1.00)
	strcat (pk_name, "9");
    else if (red == 0.94440)
	strcat (pk_name, "85");
    else if (red == 0.88880)
	strcat (pk_name, "8");
    else if (red == 0.777770)
	strcat (pk_name, "7");
    else
	strcat (pk_name, "6");

    if (f_i->m_flags & DPI1200)
	strcat (pk_name, ".1200pk");
    else if (f_i->m_flags & DPI2400)
	strcat (pk_name, ".2400pk");
    else if (f_i->flags & DPI600)
	strcat (pk_name, ".600pk");
    else if (f_i->flags & DPI360)
	strcat (pk_name, ".360pk");
    else
	strcat (pk_name, ".300pk");

    dbg1(Fonts, "ps_print.cc: font name is %s\n", pk_name);


    file_in pk_in(pk_name, "rb");

        // fprintf(stderr, "ps_print: pk font is %s %s\n", pk_name, " TFM_PATH ");

    read_pk_file(&pk_in, this);

    make_ps_font(&ps_header);

    ps_header.PutString("%%EndSetup\n");

    if (f_name[0]) {
	ps_header.dump( f_name, Creat);
    }
    else
	ps_header.dump("out.ps", Creat);
}

void ps_print::define_all_fonts()
{
    int i;
    font_list **ff = f_a;
    for (i=0; i< MAXFONTS; i++) {
	if ((*ff) && (*ff)->name) {
	    dbg1(Fonts, "define all fonts %s\n", (*ff)->name);
	    define_font((*ff)->num, (*ff)->name);
	}
	(*ff++);
    }
}

void ps_print::page_head()
{
  // parens around name string for Rainer Jan 2002 wbc
    pr_out->PutString("%%Page: (");
    pr_out->PutString(f_i->file);
    pr_out->PutString(") ");
    pr_out->Put10( npages + 1 );
    pr_out->PutString("\n");

    pr_out->PutString("%%PageOrientation: ");
    if (!(f_i->flags & ROTATE))
	pr_out->PutString("Portrait\n");
    else
	pr_out->PutString("Landscape\n");

    pr_out->PutString("%%BeginPageSetup\n");
    pr_out->PutString("/pgsave save def\n");
    pr_out->PutString("%%EndPageSetup\n");
    /* NOTE - INDENT PAGE HERE */
    /* you can change the left margin by changing the 72 below */
    if (f_i->flags & LSA_FORM )
	pr_out->PutString("72 -42 translate\n");
    else if (f_i->flags & ROTATE)
	/* for fun try [ 0 1 1 0 -250 72 ] concat */
	pr_out->PutString("[0 1 -1 0  818 72 ] concat\n");
    else {
	pr_out->Put10(left_margin);
	pr_out->PutString(" -35 translate\n");
    }
    pr_out->PutString("/LuteFont FF setfont\n");
/*    in case there is no default font */

    init_hv();
}

void ps_print::file_trail()		// write_postamble
{
    flush();
    pr_out->PutString("%%Trailer\n");
    pr_out->PutString("%%EOF\n");
}

void ps_print::page_trail()
{
// why? 	reset_dvi_vh();
    flush();
    pr_out->PutString("pgsave restore\n");
    pr_out->PutString("showpage\n");}

int ps_print::do_page(i_buf *i_b,  struct font_list *f_a[])
{
    npages++;
    set_slur_depth(f_i->slur_depth);
    return (format_page(this, i_b, f_a, f_i));
}

void ps_print::init_hv()
{
    dvi_h = 0;
    dvi_v = ps_top_of_page ;
    ps_command(MOVE, dvi_h, -dvi_v, 0, 0);
}

#define CHARS 256
#define MYCHARS 255		/* leave room for /.notdef */

void ps_print::make_ps_font(i_buf *ps_header)
{
    int i;
    double dvi_to_dots = 72.0 / 300.0;
    PKBit *b;
    extern PKBit bits[];
    //printf ("ps_print.cc: 382: dvi_to_dots %f\n", dvi_to_dots);
    if (f_i->flags & DPI600) {
      dvi_to_dots /= 2.0;
      //printf ("ps_print.cc: 357: dvi_to_dots %f\n", dvi_to_dots);
    }
    else if (f_i->m_flags & DPI360) {
      // dvi_to_dots /= (5.0/6.0);             //adS
      //printf ("ps_print.cc: 361: dvi_to_dots %f\n", dvi_to_dots);
    }
    else if (f_i->m_flags & DPI1200) {
      dvi_to_dots /= 4.0;             //adS
      //printf ("ps_print.cc: 361: dvi_to_dots %f\n", dvi_to_dots);
    }
    else if (f_i->m_flags & DPI2400) {
      dvi_to_dots /= 8.0;
      //     printf ("ps_print.cc: 365: dvi_to_dots %f\n", dvi_to_dots);
    }

    ps_header->PutString("10 dict begin\n");
    ps_header->PutString("/FontType 3 def\n");
    ps_header->PutString("/FontMatrix [ ");
    ps_header->PutF(dvi_to_dots, 3);
    ps_header->PutString("0 0 ");
    ps_header->PutF(dvi_to_dots, 3);
    ps_header->PutString("0 0 ] def\n");
    ps_header->PutString("/FontBBox [");
    ps_header->Put10(- max_off_w);
    ps_header->Put10(- max_off_h);
    ps_header->Put10(max_b_w);
    ps_header->Put10(max_b_h);
    ps_header->PutString("] def\n");
    ps_header->PutString("/En {Encoding} def\n");
    ps_header->PutString("/Encoding 256 array def\n");
    ps_header->PutString("0 1 ");
    ps_header->Put10(MYCHARS);
    ps_header->PutString("{Encoding exch /.notdef put} for\n");
    for (i=0; i < 256; i++) {
	if (bits[i].bm_w && print_used[i]) {
	    ps_header->PutString("En ");
	    ps_header->Put10(i);
	    ps_header->PutString("/b_");
	    ps_header->Put10(i);
	    ps_header->PutString("put\n");
	}
    }
    ps_header->PutString("/CharProcs ");
    ps_header->Put10(CHARS);
    ps_header->PutString("dict def\n");
    ps_header->PutString("CharProcs begin\n");
    ps_header->PutString(" /.notdef { 1 1 add pop } bind def\n");
    for (i=0; i < 256; i++) {
	if (bits[i].bm_w && print_used[i]) {
	  /*	    printf ( "ps_print: 391: b_%d\n", i); */
	  bit_char(ps_header, i, f_i->flags & DPI600 );
	}
    }
    ps_header->PutString("end %% of CharProcs\n");
    ps_header->PutString("/CharWidths ");
    ps_header->Put10(CHARS);
    ps_header->PutString("dict def CharWidths begin\n");
    ps_header->PutString("/.notdef [ 0 0 0 0 ] def\n");
    for (i=0; i < 256; i++) {
	b = &bits[i];
	if (b->bm_w && print_used[i] ) {	/* assume null chars have 0 width */
	    ps_header->PutString("/b_");
	    ps_header->Put10(i);
	    ps_header->PutString("[ ");
	    ps_header->Put10(b->bm_w - b->bm_h_off -1);
	    ps_header->Put10(b->bm_v_off);
	    ps_header->Put10(- b->bm_h_off);
	    ps_header->Put10(b->bm_v_off - b->bm_h + 1);
	    ps_header->PutString("] def\n");
	}
    }
    ps_header->PutString("end %% of CharWidths\n");
    ps_header->PutString("/BuildGlyph {\n");
/* x width, y width, x, y lower left bounding box, x, y upper right b box */
    ps_header->PutString(" exch dup /CharWidths get 3 copy exch pop exch get dup dup dup\n");
    ps_header->PutString(" 0 get /MaxXVal exch def 1 get /MaxYVal exch def \n");
    ps_header->PutString(" 2 get /MinXVal exch def 3 get /MinYVal exch def pop \n");
    ps_header->PutString(" MaxXVal 0 MinXVal MinYVal MaxXVal MaxYVal\n");
    ps_header->PutString(" setcachedevice\n");
    ps_header->PutString(" /CharProcs get exch\n");
    ps_header->PutString(" 2 copy known not {pop /.notdef} if\n");
    ps_header->PutString(" get exec\n");
    ps_header->PutString("} bind def\n");

    ps_header->PutString("/BuildChar {\n");
    ps_header->PutString(" 1 index /Encoding get exch get\n");
    ps_header->PutString(" 1 index /BuildGlyph get exec\n");
    ps_header->PutString("}bind def\n");

    ps_header->PutString("currentdict\n");
    ps_header->PutString("end %% of dict\n");
    ps_header->PutString("/LuteFont exch definefont pop\n"); /* define big dict */

/* Rule takes x, y */
    ps_header->PutString("/Rule { currentpoint 4 2 roll \n");
    ps_header->PutString(" 2 copy 0 exch rlineto \n");
    ps_header->PutString(" 0 rlineto neg 0 exch rlineto \n");
    ps_header->PutString(" neg 0 rlineto fill moveto } def\n");

    ps_header->PutString("/RM {rmoveto} def\n");
    ps_header->PutString("/MT {moveto} def\n");
    ps_header->PutString("/S {show} def\n");
    ps_header->PutString("/languagelevel where {pop languagelevel} {1} ifelse\n");
    ps_header->PutString("2 lt { \n");
    ps_header->PutString("/X {gsave show grestore} def\n");
    ps_header->PutString("} { \n");
    ps_header->PutString("/X {[0 0] xshow} def\n");
    ps_header->PutString("} ifelse\n");
    ps_header->PutString("/FF {findfont} def /SF {scalefont setfont} def\n");

// slurs !
/* do a slur, given length */
    ps_header->PutString("/doslur { /delta exch def\n");
    ps_header->PutString("gsave 1 setlinecap 0.7 setlinewidth\n");
    ps_header->PutString("/delta delta 3 div def /height 5 def\n");
    ps_header->PutString("currentpoint 2 copy 2 copy 2 copy\n");
    ps_header->PutString("/y0 exch def /x0 exch def\n");
    ps_header->PutString("/y1 exch def /x1 exch def /y2 exch def /x2 exch def\n");
    ps_header->PutString("/y3 exch def /x3 exch def /x1 x1 delta add def\n");
    ps_header->PutString("/y1 y1 height add def /x2 x2 delta 2 mul add def\n");
    ps_header->PutString("/y2 y1 def /x3 x3 delta 3 mul add def\n");
    ps_header->PutString("/y4 y2 1.8 add def ");
    ps_header->PutString("/y5 y1 1.8 add def ");
    ps_header->PutString("x1 y1 x2 y2 x3 y3 curveto ");
    ps_header->PutString("x2 y4 x1 y5 x0 y0 curveto\n");
    ps_header->PutString("fill  grestore } def\n");

/* do a reversed slur, given horizontal length */
    ps_header->PutString("/dorslur { /delta exch def\n");
    ps_header->PutString("gsave 1 setlinecap 0.8 setlinewidth\n"); /* wbc Feb 2014 width was 0.7 */
    /* height is the height of the slur, delta is the hor distance point to point*/
    ps_header->PutString("/delta delta 3 div def /height -5 def\n");
    ps_header->PutString("currentpoint 2 copy 2 copy 2 copy\n");
    ps_header->PutString("/y0 exch def /x0 exch def\n");
    ps_header->PutString("/y1 exch def /x1 exch def ");
    ps_header->PutString("/y2 exch def /x2 exch def\n");
    ps_header->PutString("/y3 exch def /x3 exch def ");
    ps_header->PutString("/x1 x1 delta add def\n");
    ps_header->PutString("/y1 y1 height add def /x2 x2 delta 2 mul add def\n");
    ps_header->PutString("/y2 y1 def /x3 x3 delta 3 mul add def\n");
    ps_header->PutString("/y4 y2 1.8 sub def ");
    ps_header->PutString("/y5 y1 1.8 sub def ");
    ps_header->PutString("x1 y1 x2 y2 x3 y3 curveto ");
    ps_header->PutString("x2 y4 x1 y5 x0 y0 curveto\n");
    ps_header->PutString("fill grestore } def\n");

/* do a wavy slur, given horizontal length */
    ps_header->PutString("/dowslur { /delta exch def\n");
    ps_header->PutString("gsave 1 setlinecap 0.7 setlinewidth\n");
    /* height is the height of the slur, delta is the hor distance point to point*/
    ps_header->PutString("/delta delta 6 div def /height 3.7 def\n");
    ps_header->PutString("currentpoint 2 copy 2 copy 2 copy 2 copy 2 copy 2 copy \n");
    ps_header->PutString(" 2 copy 2 copy 2 copy 2 copy \n");
    ps_header->PutString("/y0 exch def /x0 exch def\n");
    ps_header->PutString("/y1 exch def /x1 exch def ");
    ps_header->PutString("/y2 exch def /x2 exch def\n");
    ps_header->PutString("/y3 exch def /x3 exch def ");
    ps_header->PutString("/y5 exch def /x5 exch def\n");
    ps_header->PutString("/y6 exch def /x6 exch def ");
    ps_header->PutString("/y7 exch def /x7 exch def\n");
    ps_header->PutString("/y8 exch def /x8 exch def ");
    ps_header->PutString("/y9 exch def /x9 exch def\n");
    ps_header->PutString("/y10 exch def /x10 exch def ");
    ps_header->PutString("/y11 exch def /x11 exch def\n");
    ps_header->PutString("/y0 y0 height sub def\n");
    ps_header->PutString("/y8 y8 height .6  mul add def\n");
    ps_header->PutString("/x1 x1 delta add def\n");
    ps_header->PutString("/y1 y1 height add def /x2 x2 delta 2 mul add def\n");
    ps_header->PutString("/y2 y1 def /x3 x3 delta 3 mul add def\n");
    ps_header->PutString("/y4 y2 1.8 sub def ");
    ps_header->PutString("/y5 y1 1.8 sub def ");
    ps_header->PutString("/y6 y6 height sub def /x6 x6 delta 4 mul add def\n");
    ps_header->PutString("/y7 y7 height sub def /x7 x7 delta 5 mul add def\n");
    ps_header->PutString("/y9 y7 1.8 sub def ");
    ps_header->PutString("/y10 y6 1.8 sub def ");
    ps_header->PutString("/x9 x7 def /x10 x6 def /x11 x3 def \n");
    ps_header->PutString("/x8 x8 delta 6 mul add def\n");
    ps_header->PutString("/y3  y3  .2 sub def\n ");
    ps_header->PutString("/y11 y11 .2 add def\n ");
    ps_header->PutString("x0 y0 moveto ");
    ps_header->PutString("x1 y1 x2 y2 x3 y3 curveto \n");
    ps_header->PutString("x6 y6 x7 y7 x8 y8 curveto \n");
    ps_header->PutString("x9 y9 x10 y10 x11 y11 curveto \n");
    ps_header->PutString("x2 y4 x1 y5 x0 y0 curveto\n");
    ps_header->PutString("fill grestore } def\n");

/* do a reversed half slur, given length */
    ps_header->PutString("/dorhslur { /delta exch def\n");
    ps_header->PutString("gsave 1 setlinecap 0.7 setlinewidth\n");
    ps_header->PutString("/delta delta 3 div def /height -5 def\n");
    ps_header->PutString("currentpoint 2 copy 2 copy 2 copy\n");
    ps_header->PutString("/y0 exch def /x0 exch def\n");
    ps_header->PutString("/y1 exch def /x1 exch def /y2 exch def /x2 exch def\n");
    ps_header->PutString("/y3 exch def /x3 exch def /x1 x1 delta add def\n");
    ps_header->PutString("/y1 y1 height add def /x2 x2 delta 2 mul add def\n");
    ps_header->PutString("/y2 y1 def /x3 x3 delta 3 mul add def\n");
    ps_header->PutString("/y4 y2 1.8 sub def ");
    ps_header->PutString("/y5 y1 1.8 sub def ");
    ps_header->PutString("x1 y1 x2 y2 x3 y3 curveto ");
    ps_header->PutString("x2 y4 x1 y5 x0 y0 curveto\n");
    ps_header->PutString("fill ");
    ps_header->PutString(" grestore } def\n");

/* a general horizontal slur, takes delta x, delta y, and curve amount and returns to where it started */
/* a new routine added by wbc July 2019 */
    ps_header->PutString("/uslur { /curve exch def /deltay exch def  /deltax exch def\n");
    ps_header->PutString("gsave  1 setlinecap 0.3 setlinewidth\n");
    ps_header->PutString("%%% /curve -15 def  % how much curvature the slur has\n");
    ps_header->PutString("/thick 0.9 def\n");
    ps_header->PutString("/et .3 def   % end thickness\n");
    ps_header->PutString("currentpoint /cy exch def /cx exch def\n");
    ps_header->PutString("/x3 cx deltax add def /y3 cy deltay add def\n");
    ps_header->PutString("/y3a y3 et add def /y4a cy et add def\n");
    ps_header->PutString("/x1 cx deltax 3.0 div add  def /y1 cy curve add deltay 3.0 div add def\n");
    ps_header->PutString("/y1a y1  thick add def\n");
    ps_header->PutString("/x2 cx deltax 1.5 div add  def /y2 y1 deltay 3.0 div add def\n");
    ps_header->PutString("/y2a y2 thick add def x1 y1 x2 y2 x3 y3 curveto\n");
    ps_header->PutString("x3 y3a lineto x2 y2a x1 y1a cx y4a curveto closepath\n");
    ps_header->PutString("fill grestore } def  \n");

/* do a vertical slur, given heigth */
    ps_header->PutString("/dovslur { /delta exch def\n");
    ps_header->PutString("gsave 1 setlinecap 0.7 setlinewidth\n");
    ps_header->PutString("/delta delta 3 div def /width -5 def\n");
    ps_header->PutString("currentpoint 2 copy 2 copy\n");
    ps_header->PutString("/y1 exch def /x1 exch def /y2 exch def /x2 exch def\n");
    ps_header->PutString("/y3 exch def /x3 exch def /y1 y1 delta add def\n");
    ps_header->PutString("/x1 x1 width add def /y2 y2 delta 2 mul add def\n");
    ps_header->PutString("/x2 x1 def /y3 y3 delta 3 mul add def\n");
    ps_header->PutString("x1 y1 x2 y2 x3 y3 curveto stroke grestore } def\n");


/* do a tie, given length and height */
    ps_header->PutString("/dotie { /delta exch def /height exch def\n");
    ps_header->PutString("gsave 1 setlinecap 0.7 setlinewidth\n");
    ps_header->PutString("/delta delta 3 div def\n");
    ps_header->PutString("currentpoint 2 copy 2 copy\n");
    ps_header->PutString("/y1 exch def /x1 exch def /y2 exch def /x2 exch def\n");
    ps_header->PutString("/y3 exch def /x3 exch def /x1 x1 delta add def\n");
    ps_header->PutString("/y1 y1 height add def /x2 x2 delta 2 mul add def\n");
    ps_header->PutString("/y2 y1 def /x3 x3 delta 3 mul add def\n");
    ps_header->PutString("x1 y1 x2 y2 x3 y3 curveto stroke grestore } def\n");
    ps_header->PutString("\n");
    /*
      /radius 50 def
      /radius2 radius 3 sub def
      /delta 57 def
      currentpoint /y0 exch def /x0 exch def
      /y1 y0 radius2 add def
      /y2 y0 radius2 sub def
      /x1 x0 delta add def
      /x2 x0 delta sub def
      
      newpath
      x0 y0 radius 90 270 arc 
      x0 y2 moveto
      x2 y2 x2 y1 x0 y1 curveto fill
      
      x0 y0 moveto
      newpath
      x0 y0 radius 270 90 arc       
      x0 y1 moveto 
      x1 y1 x1 y2 x0 y2 curveto fill

      newpath
      x0 y0 radius 8 div 0 360 arc fill
    */
    /* x y angle 1 angle2 radius arc */
    /* do tempus perfectum perfect */  /* wbc jam 2026 */
    ps_header->PutString("/dotempusperfectum { \n");
    ps_header->PutString("gsave\n");
    ps_header->PutString("1.2 setlinewidth \n");
    ps_header->PutString("currentpoint /y0 exch def /x0 exch def \n");
    if (f_i->flag_flag & PERFECTUM) {
        ps_header->PutString("/radius 7.5 def /radius2 radius 0.9 sub def /delta 7.5 def \n");
	ps_header->PutString("/y1 y0 radius2 add def /y2 y0 radius2 sub def /x1 x0 delta add def /x2 x0 delta sub def \n");
	ps_header->PutString("/x1 x0 delta add def /x2 x0 delta sub def\n");
	ps_header->PutString("% left side\n");
	ps_header->PutString("newpath x0 y2 moveto x2 y2 x2 y1 x0 y1 curveto x0 y0 radius 90 270 arc closepath fill\n");
	ps_header->PutString("% right side\n");
	ps_header->PutString("newpath x0 y1 moveto x1 y1 x1 y2 x0 y2 curveto x0 y0 radius 270 90 arc fill \n");
	ps_header->PutString("newpath x0 y0 1.5 0 365 arc fill\n");
      /*
	ps_header->PutString("/radius 10 def /radius2 radius 0.9 sub def /delta 10 def \n");
	ps_header->PutString("/y1 y0 radius2 add def /y2 y0 radius2 sub def /x1 x0 delta add def /x2 x0 delta sub def \n");
	ps_header->PutString("/x1 x0 delta add def /x2 x0 delta sub def \n");
	ps_header->PutString("newpath x0 y0 radius 90 270 arc x0 y2 moveto x2 y2 x2 y1 x0 y1 curveto fill\n");
	ps_header->PutString("x0 y0 moveto newpath x0 y0 radius 270 90 arc x0 y1 moveto x1 y1 x1 y2 x0 y2 curveto fill\n");
      */
    }
    else {
      ps_header->PutString("currentpoint newpath 2 copy  7 0 360 arc stroke 1.5 0 365 arc fill\n");
    }
    ps_header->PutString("grestore } def\n");

    /* do tempus imperfectum needs a dot in the middle */
    ps_header->PutString("/dotempusimperfectum { \n");
    ps_header->PutString("gsave\n");
    ps_header->PutString("currentpoint /y0 exch def /x0 exch def \n");
    if (f_i->flag_flag & PERFECTUM) {
      ps_header->PutString("/radius 7.5 def /radius2 radius 0.9 sub def /delta 7.5 def \n");
      ps_header->PutString("/y1 y0 radius2 add def /y2 y0 radius2 sub def /x1 x0 delta add def /x2 x0 delta sub def \n");
      ps_header->PutString("/x1 x0 delta add def /x2 x0 delta sub def \n");
      ps_header->PutString("% left side \n");
      ps_header->PutString("newpath x0 y2 moveto x2 y2 x2 y1 x0 y1 curveto x0 y0 radius 90 270 arc closepath fill\n");
      ps_header->PutString("% right side \n");
      ps_header->PutString("x0 y0 radius 0.6 sub 55 90 arc stroke\n");
      ps_header->PutString("x0 y0 2 add radius 2.5 sub 50 90 arc stroke\n");
      ps_header->PutString("x0 y0 radius 0.6 sub 270 305 arc stroke\n");
      ps_header->PutString("x0 y0 2 sub radius 2.5 sub 270 310 arc stroke\n");
      ps_header->PutString("newpath x0 y0 1.5 0 365 arc fill\n");
      /*      
	      currentpoint /y0 exch def /x0 exch def 
	      ps_header->PutString("1.0 setlinewidth \n");
	      ps_header->PutString("/radius 10 def /radius2 radius 0.9 sub def /delta 10 def \n");
	      ps_header->PutString("/y1 y0 radius2 add def /y2 y0 radius2 sub def /x1 x0 delta add def /x2 x0 delta sub def \n");
	      ps_header->PutString("/x1 x0 delta add def /x2 x0 delta sub def \n");
	      ps_header->PutString("newpath x0 y0 radius 90 270 arc x0 y2 moveto x2 y2 x2 y1 x0 y1 curveto fill\n");
	      ps_header->PutString("x0 y0 radius 0.6 sub 55 90 arc stroke\n"); 
	      ps_header->PutString("x0 y0 2 add radius 2.5 sub 50 90 arc stroke\n");
	      ps_header->PutString("x0 y0 radius 0.6 sub 270 305 arc stroke\n");
	      ps_header->PutString("x0 y0 2 sub radius 2.5 sub 270 310 arc stroke\n");
	      //ps_header->PutString("x0 y0 moveto newpath x0 y0 radius 270 90 arc x0 y1 moveto x1 y1 x1 y2 x0 y2 curveto fill\n");
	      */
    }
    else {
      ps_header->PutString("1.2 setlinewidth \n");
      ps_header->PutString("currentpoint newpath \n"); // we need to make x y  into x y x y
      ps_header->PutString("2 copy 2 copy \n"); 
      ps_header->PutString("7 55 305 arc stroke 1.5 0 360 arc fill\n");
      // ps_header->PutString("1 0 365 arc fill\n");
      //ps_header->PutString("newpath x0 y0 1.5 0 365 arc fill\n");    
    }
    ps_header->PutString("grestore } def\n");
    
//    ps_header->PutString("%%%%end of header\n");
}
void ps_print::p_movev(const int ver)
{
    if (ver == 0) return;
    ps_command(MOVEV, 0, ver, 0, 0);
    dvi_v -= ver;
}

void ps_print::p_moveh(const int hor)
{
   if (hor == 0) return;
   ps_command(MOVEH, hor, 0, 0, 0);
   dvi_h += hor;
}
void ps_print::p_moveto(const int hor, const int ver)
{
//    printf("in moveto %d %d\n", hor, ver);
    ps_command(MOVE, hor, -ver, 0, 0);
    dvi_h = hor;
    dvi_v = ver;
}
void ps_print::p_put_rule(int w, int h)
{
    if (highlight==On){
      if (highlight_type==Red)
	ps_command(P_S_RED, 0, 0, 0, 0);
      else if (highlight_type==Blue)
	ps_command(P_S_BLUE, 0, 0, 0, 0);
      else
	ps_command(P_S_GRAY, 0, 0, 0, 0);
    }
    ps_command(RULE, w, h, 0, 0);
    if (highlight==On) {
	clear_highlight();
	ps_command(P_U_GRAY, 0, 0, 0, 0);
    }
}
void ps_print::put_a_char (unsigned char c)
{
    if (highlight==On)  {
	if (highlight_type == Paren ) {
	    moveh (-.08);
	    ps_command(PCHAR, (int)'(', 0, 0, 0);
	    moveh (.08);
	}
	else if (highlight_type == Red) {
	  ps_command(P_S_RED, 0, 0, 0, 0);
	}
        else if (highlight_type == Blue) {
          ps_command(P_S_BLUE, 0, 0, 0, 0);
	}
	else
	  ps_command(P_S_GRAY, 0, 0, 0, 0);
    }
    if (curfont == 0 && print_used[c] < 4)
	print_used[c]++;
    ps_command(PCHAR, (int)c, 0, 0,0);
    if (highlight==On) {
	clear_highlight();
	if (highlight_type == Paren) {
	  double www=f_a[curfont]->fnt->get_width(c);
	    moveh (www);
	    ps_command(PCHAR, (int)')', 0, 0, 0);
	    print_used['(']++;	print_used[')']++;
	    moveh (-www);
	}
	else if (highlight_type == Red)
	  ps_command(P_U_RED, 0, 0, 0, 0);
	else if (highlight_type == Blue)
	  ps_command(P_U_BLUE, 0, 0, 0, 0);
	else
	  ps_command(P_U_GRAY, 0, 0, 0, 0);
    }
}
void ps_print::set_a_char (unsigned char c)
{
  if (highlight==On)
    {
    if (highlight_type == Red) {
      ps_command(P_S_RED, 0, 0, 0, 0);
    }
    else if (highlight_type == Blue)
	  ps_command(P_S_BLUE, 0, 0, 0, 0);
    else {
      ps_command(P_S_GRAY, 0, 0, 0, 0);}
    }

  if (c == 0365)
    dvi_h += inch_to_dvi(f_a[curfont]->fnt->get_width('i'));
  else if ( c == 0074)
    dvi_h += inch_to_dvi(f_a[curfont]->fnt->get_width('!'));
  else if ( c == 0076)
    dvi_h += inch_to_dvi(f_a[curfont]->fnt->get_width('?'));
  else if ( c == 014 )  /* fi */
    dvi_h += inch_to_dvi(f_a[curfont]->fnt->get_width(0256));
  else if ( c == 015 )  /* fl */
    dvi_h += inch_to_dvi(f_a[curfont]->fnt->get_width(0257));
  else if ( c == 031 )  /* german ss */
    dvi_h += inch_to_dvi(f_a[curfont]->fnt->get_width(031));
  else
    dvi_h += inch_to_dvi(f_a[curfont]->fnt->get_width(c));

  if (curfont != 0) {
    //	printf("set_a_char: c is %d %c\n", c, c);
    if ( c == 0013) {
      c = 0146; // ff - just one f in postscript
      ps_command(CHAR, (int)c, 0, 0,0); // print the first one
    }
    else if ( c == 0014) c = 0256; // fi
    else if ( c == 0015) c = 0257; // fl
    else if ( c == 0031) c = 0373; // fs
    else if ( c == 0032) c = 0361; // ae
    else if ( c == 0035) c = 0341; // AE
    else if ( c == 0033) c = 0372; // oe
    else if ( c == 0036) c = 0352; // OE
    else if ( c == 0034) c = 0371; // oo
    else if ( c == 0037) c = 0351; // OO
    else if ( c == 0074) c = 0241; // ! inverted
    else if ( c == 0076) c = 0277; // ? inverted
  }

  if (curfont == 0 && print_used[c] < 4)
    print_used[c]++;
  ps_command(CHAR, (int)c, 0, 0,0);
  if (highlight==On) {
    clear_highlight();
    ps_command(P_U_GRAY, 0, 0, 0, 0);
  }
}
void ps_print::use_font(int fontnum)
{
    curfont = fontnum;
    switch (fontnum) {
    case 7:			/* page number font doesn't shrink 12 pt */
	ps_command( NUMFONT, 0, 0, 0, 0);
	break;
    case 6:			/* font 6 18 pt text */
	ps_command( MUSICS, 0, 0, 0, 0);
	break;
    case 5:			/* font 5 10 pt italic */
	ps_command( ITAL, 0, 0, 0, 0); // title italic
	break;
    case 4:			/* font 4 med */
	ps_command( MED, 0, 0, 0, 0);
	break;
    case 3:			/* font 3 big */
	ps_command( BIGIT, 0, 0, 0, 0);
	break;
    case 2:			/* font 2 */
	ps_command( ROMAN, 0, 0, 0, 0);	// title
	break;
    case 1:			/* font 1 */
	ps_command( SMALL, 0, 0, 0, 0); // words to songs
	break;
    case 0:		        /* lute */
	ps_command( LUTE, 0, 0, 0, 0);
	break;
    default:
	dbg1(Warning, "tab: ps_setfont: undefined font number %d, using 2\n", (void *)fontnum);
	ps_command( ROMAN, 0, 0, 0, 0);
	break;
    }
}

void ps_print::do_tie(double length)
{
    ps_command ( PTIE, inch_to_dvi(length), 0, 0, 0);
}
void ps_print::do_tie_reversed(double length)
{
    ps_command ( PRTIE, inch_to_dvi(length), 0, 0, 0);
}
void ps_print::do_half_tie(double length)
{
    ps_command ( PHTIE, inch_to_dvi(length), 0, 0, 0);
}
void ps_print::do_half_tie_reversed(double length)
{
    ps_command ( PHRTIE, inch_to_dvi(length), 0, 0, 0);
}
void ps_print::do_rtie(int bloc, int eloc)
{
    ps_command ( PTIE, (save_h[eloc] - save_h[bloc]), 0, 0, 0);
}
void ps_print::print_clipped(char c, int font/* acutally height */)
{
    ps_command ( PS_CLIP, c, font, dvi_h, dvi_v);
     dvi_h += inch_to_dvi(f_a[curfont]->fnt->get_width(c));
}

void ps_print::put_slash
      (int bloc, int eloc, int count, struct file_info *f)
{
    double nflags = (double)count;
    int thickness = str_to_dvi("0.005 in");

    if (f->flags & LSA_FORM) thickness = str_to_dvi("0.023 in");

    saveloc(REGS-1);
/*     move_n_v("0.19 in"); */
    while (count) {
	p_put_rule(save_h[eloc] - save_h[bloc], thickness);
	movev(str_to_inch("0.03 in") + 0.06 / nflags);
	count --;
    }
    getloc(REGS-1);
}

void ps_print::put_uline(int bloc, int eloc)
{
    saveloc(REGS-1);
    // printf ("HERE put_uline \n");
    ps_command ( PRTIE, save_h[eloc] - save_h[bloc], 0, 0, 0); // was PTIE2
    getloc(REGS-1);
}
void ps_print::put_r_uline(int bloc, int eloc)
{
    saveloc(REGS-1);
    ps_command ( PTIE, save_h[eloc] - save_h[bloc], 0, 0, 0); // was PTIE2
    getloc(REGS-1);
}
void ps_print::put_w_uline(int bloc, int eloc)
{
    saveloc(REGS-1);
    ps_command ( PWTIE, save_h[eloc] - save_h[bloc], 0, 0, 0); // was PTIE2
    getloc(REGS-1);
}
void ps_print::put_thick_slant(int bloc, int eloc)
{
  ps_command(TH_LINE, save_h[bloc], save_v[bloc], save_h[eloc], save_v[eloc]);
}
void ps_print::put_med_slant(int bloc, int eloc)
{
  // printf("put_med_slant \n");
  ps_command(MED_LINE, save_h[bloc], save_v[bloc], save_h[eloc], save_v[eloc]);
}
void ps_print::put_slant(int bloc, int eloc)
{
    saveloc(REGS-1);
    // printf("put_slant \n");
    ps_command(LINE, save_h[bloc], save_v[bloc], save_h[eloc], save_v[eloc]);

    getloc(REGS-1);
    return;
}

int  ps_print::more()
{
    double length;
    if ( red == 1.0 ) {
	length = 2.6;
    }
    else if (red == 0.94440){ /* 95 */
	length = 2.5;
    }
    else if (red == 0.88880){ /* 9 */
	length = 2.4;
    }
    else if (red == 0.777770){ /* 8 */
	length = 2.80;
    }
    else {
	length = 2.50;           /* 7 */
    }
    //        printf("ps more %f\n", dvi_to_inch(dvi_v));
    if (inch_to_dvi(length) > dvi_v) {
	return(1);		/* END_MORE */
    }
    return(0);			/* END_OK */
}
void ps_print::showsave(int reg)
   { dbg0(Error, "Undefined Proceedure showsave\n");}

void ps_print::set_slur_depth(double depth)
{
  slur_depth = depth;
}
double ps_print::get_slur_depth()
{
  return slur_depth;
}
void ps_print::p_num(int n)
{
#define N_S 5
    char string[N_S];
    int i;
    double total_width=0.0;
#ifdef MAC
#else
    snprintf(string, sizeof(string), "%d", n);
#endif
    push();
    movev("0.18 in");

    use_font(bar_number_font);  /* was 1 wbc july 2019 */
    for (i=0; i < N_S && string[i]; i++)
      total_width += f_a[curfont]->fnt->get_width(string[i]);

    //    printf("p_num, width is %f  %s\n", total_width, string);

    moveh(-total_width/2.0 + 0.018);

    for (i=0; i < N_S && string[i]; i++)
      set_a_char(string[i]);

    use_font(0);
    pop();
}

void ps_print::print_draft()
{
    ps_command(PDRAFT, 0, 0, 0, 0);
}
void ps_print::print_copyright()
{
    ps_command(PCOPYRIGHT, 0, 0, 0, 0);
}
void ps_print::define_font(int font_num, char *name)
{
dbg0(Error, "Undefined Proceedure Define font\n");}

void ps_print::vert_curve(int len)
{
    ps_command(PVCURVE, len, 0, 0, 0);
}

void ps_print::perfect()
{
  ps_command(PERFECT, 0, 0, 0, 0);
}
void ps_print::imperfect()
{
  ps_command(IMPERFECT, 0, 0, 0, 0);
}
void ps_print::stroke()
{   
  ps_command(STROKE, 0, 0, 0, 0);
} 
void ps_print::strokex()
{   
  ps_command(STROKEX, 0, 0, 0, 0);
} 
void ps_print::half_cross()
{   
  ps_command(HALF_CROSS, 0, 0, 0, 0);
} 
void ps_print::push()
{
//    ps_command(PPUSH, 0, 0, 0, 0);
//    printf("ps push %d, dvi_h %d, stack h %d\n",
//	   sp, dvi_h, stack_h[sp]);
    slp(sp, stack_h, stack_v); sp++;		  // also set the dvi_v, dvi_h
}

void ps_print::pop()
{
//    ps_command(PPOP, 0, 0, 0, 0);
//    sp--; dvi_h = stack_h[sp]; dvi_v = stack_v[sp];
    sp--;
    glp(sp, stack_h, stack_v);
//    printf("ps pop  %d, h %d, stack h %d\n", sp, dvi_h, stack_h[sp]);
}

int my_isprint(int c)
{
  if (c < 33 ) return (0);
  if (c > 126 ) return (0);
  return (1);
}

static int last_com=OTHER;
static int last_last_com=OTHER;
static int last_font=OTHER;

void ps_print::ps_command(int com, int h_n, int v_n, int hh_n, int vv_n)
{
  static int h, v, hh, vv;
  static int dvi_oh, dvi_ov;

  static int ooh, oov, oohh, oovv; // two previous
  static int dvi_ooh, dvi_oov;

  /*    extern double ps_size[]; */
  time_t timeval;
  int places=4;			// digits of floating pt numbers
  double currentgray =0.0;

  //    printf("ps_command com %d h %03.2f v %03.2f\n",
  //	   com, d_to_p (h_n), d_to_p(v_n));

  //    if (com == MOVE) printf("com - move h_n %d %f\n", h_n, d_to_p(h_n));
  //#define OPTI
#ifdef OPTI
  last_last_com = last_com;
  last_com = com;
  h = h_n; v = v_n;
  hh = hh_n; vv = vv_n;
  dvi_oh = dvi_h; dvi_ov = dvi_v;
#endif /* OPTI */

#ifndef OPTI

  if (com == last_com) {
    switch (com) {
    case MOVEH:
      h += h_n;
      return;
    case MOVEV:
      v += v_n;
      return;
    case MOVEVH:
      h += h_n;
      v += v_n;
      return;
    }
  }

  if (com == LUTE && last_font == LUTE) {
    switch (last_com) {
    case ROMAN:
    case BIGIT:
    case SMALL:
    case MED:
    case ITAL:
      break;
    default:
      return;
    }
  }

  //    if (com == PPOP && last_com == PPUSH ) {
  //	last_com = last_last_com;
  //	return;
  //    }

  if (com == MOVEH &&
      (last_com == MOVEV || last_com == MOVEVH)) {
    last_com = MOVEVH;
    h += h_n;
    return;
  }
  if (com == MOVEV &&
      (last_com == MOVEH || last_com == MOVEVH)) {
    last_com = MOVEVH;
    v += v_n;
    return;
  }
  if (last_com == MOVE && com == MOVE) {
    v = v_n;
    h = h_n;
    return;
  }
  if (last_com == MOVE &&
      com == MOVEV ) {
    v += v_n;
    return;
  }
  if (last_com == MOVE &&
      com == MOVEH ) {
    h += h_n;
    return;
  }
  if (last_com == MOVE &&
      com == MOVEVH ) {
    h += h_n;
    v += v_n;
    return;
  }
#endif /* OPTI */

  switch (last_com) {
  case MOVE:
    pr_out->PutF(d_to_p(h), places);
    pr_out->PutF(d_to_p(-v), places);
    pr_out->PutString(" MT\n");
    break;
  case RULE:		// now takes x, y
    pr_out->PutF(d_to_p(h), places);
    pr_out->PutF(d_to_p(v), places); //negate the v ???
    pr_out->PutString(" Rule\n"); // v, h
    break;
  case MOVEH:
    pr_out->PutF(d_to_p(h), places);
    pr_out->PutString(" 0 RM\n");
    break;
  case MOVEV:
    pr_out->PutString("0 ");
    pr_out->PutF(d_to_p(-v), places);
    pr_out->PutString(" RM\n");
    break;
  case MOVEVH:
    pr_out->PutF(d_to_p(h), places);
    pr_out->PutF(d_to_p(-v), places);
    pr_out->PutString(" RM\n");
    break;

  case LINE:  // the default line thickness set here July 2019 wbc
    pr_out->PutString("0.27 setlinewidth ");
    pr_out->PutF(d_to_p(h), places);
    pr_out->PutF(d_to_p(v), places);
    pr_out->PutString("MT ");
    pr_out->PutF(d_to_p(hh), places);
    pr_out->PutF(d_to_p(vv), places);
    pr_out->PutString("lineto stroke\n");
    break;
  case TH_LINE:
    pr_out->PutF(d_to_p(h), places);
    pr_out->PutF(d_to_p(v), places);
    pr_out->PutString(" 1.1 sub moveto 0.0 2.2\n");
    pr_out->PutString("rlineto ");
    pr_out->PutF(d_to_p(hh), places);
    pr_out->PutF(d_to_p(vv), places);
    pr_out->PutString("1.1 add lineto 0.0 -2.2 rlineto closepath fill\n");
    break;
  case MED_LINE:
    slur_depth = get_slur_depth();
    if (slur_depth == 0.0 ) {
      // printf("med line: straight slur depth %f\n", slur_depth);
      // old system - draws PS line
      pr_out->PutF(d_to_p(h), places);
      pr_out->PutF(d_to_p(v), places);
      pr_out->PutString(" 0.3 sub moveto 0.0 .6\n");
      pr_out->PutString("rlineto ");
      pr_out->PutF(d_to_p(hh), places);
      pr_out->PutF(d_to_p(vv), places);
      pr_out->PutString("0.3 add lineto 0.0 -.6 rlineto closepath fill\n");
      break;
    }
    else {
      // new system - draws arc July 2019 wbc
      // printf("med line: curve slur depth %f\n", slur_depth);
      pr_out->PutF(d_to_p(hh - h), places);
      pr_out->PutF(d_to_p(vv - v), places);
      pr_out->PutF(slur_depth, places);
      pr_out->PutString("uslur \n");
      break;
    }
  case CHAR:
  case PCHAR:
    if ( h == 050 || h == 051 || h == '\\') {
      pr_out->PutString("(\\");
      pr_out->PutChar(h);
      pr_out->PutString(")");
    }
    else if (my_isprint(h)) {
      pr_out->PutString("(");
      pr_out->PutChar(h);
      pr_out->PutString(")");
    }
    else {
      pr_out->PutString("<");
      pr_out->Put16(h);
      pr_out->PutString(">");
    }
    if ( last_com == CHAR) pr_out->PutString("S\n");
    else pr_out->PutString("X\n");
    break;
  case LUTE:
    if ( last_font != LUTE ) {
      pr_out->PutString("/LuteFont FF setfont\n");
      last_font = LUTE;
    }
    break;
  case SMALL:		/* words to songs, font 1 */
    if (f_a[1]->real_name)
      pr_out->PutString(f_a[1]->real_name);
    else {
      pr_out->PutString("/NewCenturySchlbk-Roman");
      //pr_out->PutString("/NewCenturySchlbk-Italic");
    }
    pr_out->PutString(" FF ");
    pr_out->PutF(font_sizes[1] * red, places);
    pr_out->PutString(" SF\n");
    last_font = SMALL;
    break;
  case ROMAN:		/* title, roman, font 2 */
    if (f_a[2]->real_name)
      pr_out->PutString(f_a[2]->real_name);
    else
      pr_out->PutString("/NewCenturySchlbk-Roman");
    pr_out->PutString(" FF ");
    pr_out->PutF(font_sizes[2] * red, places);
    pr_out->PutString("SF\n");
    last_font = ROMAN;
    break;
  case BIGIT:			/* title italic, font 3 */
    if (f_a[3]->real_name)
      pr_out->PutString(f_a[3]->real_name);
    else
      pr_out->PutString("/NewCenturySchlbk-Bold");
    pr_out->PutString(" FF ");
    pr_out->PutF(font_sizes[3] * red, places);
    pr_out->PutString("SF\n");
    last_font = BIGIT;
    break;
  case MED:			/* big time sigs, font 4 */
    pr_out->PutString("/NewCenturySchlbk-Roman");
    pr_out->PutString(" FF ");
    pr_out->PutF(font_sizes[4] * red, places);
    pr_out->PutString("SF\n");
    last_font = MED;
    break;
  case ITAL:		/* text, italic, font 5 */
    if (f_a[5]->real_name)
      pr_out->PutString(f_a[5]->real_name);
    else
      pr_out->PutString("/NewCenturySchlbk-Italic");
    pr_out->PutString(" FF ");
    pr_out->PutF(font_sizes[5] * red, places);
    pr_out->PutString("SF\n");
    last_font = ITAL;
    break;
  case NUMFONT:		/* font for page numbers */
    pr_out->PutString("/NewCenturySchlbk-Roman");
    pr_out->PutString(" FF ");
    pr_out->PutF(12.0, places);
    pr_out->PutString("SF\n");
    last_font = NUMFONT;
    break;
  case MUSICS:                /* time signature in music */
    pr_out->PutString("/NewCenturySchlbk-Roman");
    pr_out->PutString(" FF ");
    pr_out->PutF(font_sizes[6] * red, places);
    pr_out->PutString("SF\n");
    last_font = MUSICS;
    break;
  case PS_CLIP:
    pr_out->PutString("gsave\n");
    pr_out->PutString("currentpoint  /yval exch def /xval exch def pstack\n");
    //	printf("ps clip printing %c\n", h);
    pr_out->PutString("(");
    //	pr_out->Put10(h);
    pr_out->PutString((char*)&h);
    pr_out->PutString(") false charpath clip\n");
    //#ifdef OLD_PRINTER
    //	pr_out->PutString("newpath\n");
    //	pr_out->PutString("xval %.2f moveto ", d_to_p(vv));      /* x, y */
    //	pr_out->PutString("(%c) stringwidth pop 0 rlineto 0 %f rlineto\n",
    //		 h, d_to_p(v)); /* was .73 but the dvi file changed */
    //	pr_out->PutString("(%c) stringwidth pop neg 0 rlineto\n", h);
    //	pr_out->PutString("closepath fill\n");
    //#else /* OLD_PRINTER */
    pr_out->PutString("xval ");
    pr_out->PutF(d_to_p(vv), places);      /* x, y */
    pr_out->PutString("(");
    pr_out->PutChar(h);
    pr_out->PutString(") stringwidth pop ");
    pr_out->PutF(d_to_p(v), places);
    pr_out->PutString(" rectfill\n");
    //#endif /* OLD_PRINTER */
    pr_out->PutString("grestore\n");
    pr_out->PutString(" (");
    pr_out->PutChar(h);
    pr_out->PutString(") stringwidth pop 0 rmoveto\n");
    break;
  case PTIE:
  case PHTIE:
    pr_out->PutF(d_to_p(h), places);
    pr_out->PutString("doslur\n");
    break;
  case PRTIE:

    pr_out->PutF(d_to_p(h), places);
    //  wbc July 2019 new slurs!
    // pr_out->PutF(0.0, places) ; // horizontal
    // pr_out->PutF(10.0, places); // curvature
    //pr_out->PutString("uslur\n");
    pr_out->PutString("dorslur\n");
    break;
  case PHRTIE:
    pr_out->PutF(d_to_p(h), places);
    pr_out->PutString("dorhslur\n");
    break;
  case PTIE2:
    pr_out->PutString("-6.0 ");
    pr_out->PutF(d_to_p(h), places);
    pr_out->PutString("dotie\n");
    break;
  case PWTIE:
    pr_out->PutF(d_to_p(h), places);
    pr_out->PutString("dowslur\n");
    break;
  case NTIE:
    pr_out->PutF(d_to_p(h), places);  // x length of slur
    pr_out->PutF(d_to_p(v), places);  // y how much higher or lower the end is
    printf("\n");
    pr_out->PutF(d_to_p(hh), places);  // depth of the curve 0 is a straight line
    pr_out->PutString("uslur\n");
    break;
  case PERFECT:
    pr_out->PutString("dotempusperfectum\n");
    break;
  case IMPERFECT:
    pr_out->PutString("dotempusimperfectum\n");
    break;
  case PDRAFT:
    pr_out->PutString("gsave\n");
    pr_out->PutString("/Times-Roman findfont dup 150 scalefont setfont\n");
    pr_out->PutString("0.70 setgray \n");
    pr_out->PutString("25 650 moveto \n");
    pr_out->PutString("[.7 -.7 .7 .7 0 0] concat\n");
    pr_out->PutString("(DRAFT) show\n");
    timeval = time((time_t *)0);
    pr_out->PutString("25 scalefont setfont -330 425 moveto (");
    pr_out->PutString(ctime(&timeval));
    pr_out->PutString(") show\n");
    pr_out->PutString("grestore \n");
    break;
  case PVCURVE:
    pr_out->PutF(d_to_p(h), places);
    pr_out->PutString("dovslur\n");
    break;
  case PCOPYRIGHT:
    pr_out->PutString("gsave\n");
    pr_out->PutString("/Times-Roman findfont \n");
    pr_out->PutString("dup length dict begin\n");
    pr_out->PutString("  {1 index /FID ne {def} {pop pop}");
    pr_out->PutString("ifelse} forall\n");
    pr_out->PutString("  /Encoding ISOLatin1Encoding def\n");
    pr_out->PutString("currentdict end\n");
    pr_out->PutString("/Times-Roman-ISO exch definefont pop\n");
    pr_out->PutString("/Times-Roman-ISO findfont\n");
    pr_out->PutString("12 scalefont setfont\n");
    pr_out->PutString("0 -690 rmoveto\n");
    pr_out->PutString("( \251 ) show\n");
    pr_out->PutString("(TAB by Wayne Cripps 2020)show\n");
    pr_out->PutString("grestore \n");
    break;
  case P_S_GRAY:
    pr_out->PutString("/mygray currentgray def\n");
    pr_out->PutString("0.5 setgray\n");
    break;
  case P_U_GRAY:
    pr_out->PutString("mygray setgray\n");
    break;
  case P_S_RED:
    pr_out->PutString("/mygray currentgray def\n");
    pr_out->PutString("0.0 1.0 1.0 0.0  setcmykcolor\n");
    break;
  case P_U_RED:
    pr_out->PutString("0.0 0.0 0.0 1.0  setcmykcolor\n");
    break;
  case P_S_BLUE:
    pr_out->PutString("/mygray currentgray def\n");
 /*   pr_out->PutString("1.0 1.0 0.0 0.0  setcmykcolor\n"); */
    pr_out->PutString("0.5 0.5 0.0 0.0  setcmykcolor\n");
    break;
  case P_U_BLUE:
    pr_out->PutString("0.0 0.0 0.0 1.0  setcmykcolor\n");
    break;
  case PPUSH:
    //	pr_out->PutString("gsave\n");
    pr_out->PutString("currentpoint\n");
    break;
  case PPOP:
    //	pr_out->PutString("grestore\n");
    pr_out->PutString("moveto\n");
    break;
  case RAW:
    pr_out->PutString((char *)&h);
    break;
  case STROKE:
    pr_out->PutString("gsave\n");
    pr_out->PutString("0 0 rmoveto 0 6 rlineto stroke\n");
    pr_out->PutString("grestore\n");
    break;
  case STROKEX:
    pr_out->PutString("gsave\n");
    pr_out->PutString("currentpoint 0 -1 rmoveto 0 6 rlineto stroke\n");
    pr_out->PutString("/LuteFont findfont 14 scalefont  setfont\n");
    pr_out->PutString("gsave 0 0 moveto \(Here)  show grestore\n");
    pr_out->PutString("grestore\n");
    break;
  case HALF_CROSS:
    pr_out->PutString("gsave\n");
    pr_out->PutString("/xorig 3  def /yorig 0 def ");
    pr_out->PutString("currentpoint xorig yorig rmoveto 0 6 rlineto stroke\n");
    pr_out->PutString("moveto -1  yorig 3 add rmoveto xorig 1 add  0 rlineto stroke\n");
    pr_out->PutString("grestore\n");
    break;
  case OTHER:
    break;
  default:
    dbg1(Warning, "tab: ps_command: unknown command %d\n", (void *)last_com);
    break;
  }

#ifndef OPTI
  last_last_com = last_com;
  last_com = com;

  ooh = h; oov = v;
  h = h_n; v = v_n;

  oohh = hh; oovv = vv;
  hh = hh_n; vv = vv_n;

  dvi_ooh = dvi_oh; dvi_oov = dvi_ov;
  dvi_oh = dvi_h; dvi_ov = dvi_v;

#endif /* OPTI */
  return;
}

void bit_char(i_buf *ps, int char_num, int sixh)
{
    int i;
    PKBit *b;
    extern PKBit bits[];
    int byte_w;

    b = &bits[char_num];
    byte_w = (b->bm_w + 7 ) / 8;


    ps->PutString("/b_");
    ps->Put10(char_num);
    ps->PutString(" {");  // char_name);

    if (char_num == 81 && sixh ) {    /* vector char 0121 81 decimal */
/* at 600 dpi scaled this character seems to be 331 412 */
/* these dimensions are scaled -R9 */
/*      ps->Put10(b->bm_w); ps->Put10(b->bm_h); */
      ps->PutString("%% Hand drawn by Wayne \n");
      ps->PutString("\n/P {");
      ps->PutF(red, 5);
      ps->PutString(" mul } def\n");
      ps->PutString("5 P setlinewidth 0 0 moveto\n");
      ps->PutString("18 P 216 P 30 P 379 P 45 P 405  P curveto stroke\n");
      ps->PutString("14 P setlinewidth 45 P 405 P moveto\n");
      ps->PutString("76 P 390 P 70 P 65 P 102 P 60 P curveto stroke\n");
      // second up
      ps->PutString("5 P setlinewidth 102 P 60 P moveto\n");
      ps->PutString("110 P 74  P 112 P 297 P 135 P 334 P curveto stroke\n");
      ps->PutString("14 P setlinewidth 135 P 334 P moveto\n");
      ps->PutString("140 P 284 P 144 P 100 P 168 P 110 P curveto stroke\n");
      // third up
      ps->PutString("5 P setlinewidth 168 P 110 P moveto\n");
      ps->PutString("180 P 120 P 182 P 230 P 190 P 266 P curveto stroke\n");
      ps->PutString("14 P setlinewidth 190 P 266 P moveto\n");
      ps->PutString("198 P 240 P 200 P 152 P 215 P 142 P curveto stroke\n");
      // last squiggle
      ps->PutString("5 P setlinewidth 215 P 140 P moveto\n");
      ps->PutString("265 P 145 P 290 P 200 P 300 P 290 P curveto stroke\n");
      ps->PutString("15 P setlinewidth 300 P 290 P moveto\n");
      ps->PutString("311 P 230 P 312 P 270 P 315 P 270 P curveto stroke\n");
      // ps->PutString("stroke\n");
    }
    else if (char_num == 033 && 0 ) {  /* 27  Perfect circle */
      // fprintf(stderr, "HERE\n");
      ps->PutString("\n/P {");
      ps->PutF(red, 5);
      ps->PutString(" mul } def\n");
      ps->PutString("5 P setlinewidth 0 0 moveto\n");
      ps->PutString("25 P 25 P moveto stroke");
    }
    else {         /* bitmapped character */
      ps->Put10(b->bm_w); ps->Put10(b->bm_h);
      ps->PutString("true [1 0 0 1 ");
      ps->Put10(b->bm_h_off);     ps->Put10(b->bm_h - b->bm_v_off );
      ps->PutString("]\n{<");
      /* bitmapt */

      for (i=0; i < byte_w * b->bm_h; i++) {
	ps->Put16(b->bm_bits[i] & 0xff);
	if (!((i+1) % 40)) ps->PutString("\n");
      }
      ps->PutString(">}\n imagemask");
    }
    ps->PutString("} bind def\n");
}


void ps_print::glp(int reg, int h[], int v[])
{
    if (reg >= REGS)
      dbg1 (Error, "tab: getloc: illegal register %d\n", (void *)reg);

    h_diff = h[reg] - dvi_h;
    v_diff = v[reg] - dvi_v;
    dvi_h = h[reg];
    dvi_v = v[reg];

    dbg5(Stack, "print: glp: reg %d v_diff %d h_diff %d dvi_v %d dvi_h %d\n",
	 (void *)reg,
	 (void *)v_diff, (void *)h_diff,
	 (void *)dvi_v, (void *)dvi_h );

    ps_command(MOVE, dvi_h, -dvi_v, 0, 0);

}

void ps_print::comment(const char *string)
{
  //    char cc[80], *c;
    //    strncpy(cc, string, 80);
    //    c = cc;
    //    while ( *c ) {
	    //	ps_command(RAW, (int)*c, 0, 0, 0);
	    //	c++;
	    //    }
}

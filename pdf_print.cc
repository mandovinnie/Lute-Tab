/*
 *
 *
 */
#include "win.h"
#if defined WIN32
#include <windows.h>
#endif /* WIN32 */

#include "version.h"
#include "tab.h"
#include "print.h"
#include "pdf_print.h"
#include "pdf.h"
#include "file_in.h"
#include "i_buf.h"
#include <time.h>
#include "pk_bit.h"

void read_pk_file(file_in *pk, print *pdf);

int format_page(print *d_p, i_buf *i_b, font_list *f_l[], struct file_info *f);
void pdf_bit_char(i_buf *pdf, int char_num);
int my_pdf_isprint(int c);

void print_xref_table();
static struct xref_entry xref_list;
void new_xref_entry(const int offset);
void new_xref_list();

int pdf_max_b_w=0;                  /* max bitmap width for pdf */
int pdf_max_b_h=0;                  /* max bitmap height for pdf */
int pdf_max_off_w=0;                /* max bitap width offset for pdf */
int pdf_max_off_h=0;                /* max bitmap height offset for pdf */

extern char *font_path;

pdf_print::pdf_print(font_list *font_array[], file_info *f)
{
    int j;

    //    printf ("val %d\n", inch_to_dvi(0.4218));
    //    printf ("val %d\n", mm_to_dvi(17.6));
    //    printf ("val %f\n", dvi_to_inch(98670000));
    npages=0;
    byte_count = 0;
    xref_offset = 0;
    generation = 0;
    memset (pdf_stream_b, 0, sizeof(pdf_stream_b));
    new_xref_list();

    if (f->m_flags & A4 ) { // a4 is  297mm*210mm
      if (!(f->flags & ROTATE)) 
	pdf_top_of_page = 98670000 +  6563672/* 6712847 */;
      else  
	pdf_top_of_page = 98670000;
    }
    else
      pdf_top_of_page = 98670000;
    /*    printf("top of page %f %f %u %u\n",  
	   dvi_to_inch(pdf_top_of_page),
	   dvi_to_mm(pdf_top_of_page),
	   pdf_top_of_page,
	   mm_to_dvi(32.42)); */
    for (int i=0; i< 256; i++) pdf_used[i] = 0;
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

    file_head();		// file head dumps header
}

pdf_print::~pdf_print()
{
    file_trail();
    if (f_name[0]) {
      if (!nodump)
	pr_out->dump( f_name, Append);
    }
    else
      pr_out->dump( "out.pdf", Append);
}


void pdf_print::file_head()
{
    i_buf pdf_header;
    // time_t t;
    char pk_name[300];
    char *p = NULL;

    if (nodump) 
      return;

    byte_count =  pdf_header.PutStringC("%PDF-1.4\n");
    byte_count += pdf_header.PutStringC("%’");
    byte_count += pdf_header.PutStringC("\n");
    /* byte_count += 1; */ /* wbc april test */
    fprintf(stderr, "file_head: byte count is %d\n", byte_count);
    byte_count += do_catalog();
    fprintf(stderr, "file_head: byte count is %d\n", byte_count);
    byte_count += do_page_tree();
    fprintf(stderr, "file_head: byte count is %d\n", byte_count);
    //    byte_count += do_page_leaf();
    //    byte_count += do_page_content();
    //    byte_count += do_page_resource();
    byte_count += pdf_fontdef();

    if (font_path) {
      //fprintf (stderr, "pdf_print - setting font path %s from command line\n", 
      //font_path);
      p = font_path;
    }
    else
      p = getenv("TABFONTS");
    if (p == NULL ) 
#ifdef TFM_PATH
	strcpy(pk_name, TFM_PATH);
#else
    strcpy(pk_name, ".");
#endif /* TFM_PATH */
    else
	strcpy(pk_name, (char *)p);

    /* free(p); windows */
    strcat (pk_name, "/");

    if (f_i->font_names[0]) {
      strcat (pk_name, f_i->font_names[0]);
    }
    else {
      if (baroque) {
	strcat (pk_name, "blute");
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
    else
	strcat (pk_name, ".300pk");

    file_in pk_in(pk_name, "rb");

    read_pk_file(&pk_in, this);

    //    make_pdf_font(&pdf_header);

    //    pdf_header.PutString("%%EndSetup\n");

    if (f_name[0]) {
	pdf_header.dump( f_name, Creat);
    }
    else
	pdf_header.dump("out.pdf", Creat);
}

void pdf_print::define_all_fonts()
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

void pdf_print::page_head()
{
  byte_count += do_page_leaf();

  init_hv();
}

void pdf_print::file_trail()		// write_postamble
{
  char buf[80];
  //   flush();

  file_xref();

  pr_out->PutString("trailer\n");
  sprintf (buf, "<< /Size %d\n", xref_count );
  pr_out->PutString(buf);
  pr_out->PutString("   /Root 1 0 R\n");
  pr_out->PutString(">>\n");

  pr_out->PutString("startxref\n");
  pr_out->Put10(byte_count);
  pr_out->PutString("\n");
  pr_out->PutString("%%EOF\n"); //  official last line!
}

void pdf_print::page_trail()
{
  //  byte_count += do_page_content();
  byte_count += do_page_resource();
// why? 	reset_dvi_vh();
//    flush();
}

int pdf_print::do_page(i_buf *i_b,  struct font_list *f_a[])
{
  npages++;
  byte_count += do_page_content(i_b, f_a);
  //  format_page(this, i_b, f_a, f_i);
  return (page_retval);
}

void pdf_print::init_hv()
{
    dvi_h = 0;
    dvi_v = pdf_top_of_page ;
    pdf_command(MOVE, dvi_h, -dvi_v, 0, 0);
}

#define CHARS 256
#define MYCHARS 255		/* leave room for /.notdef */

void pdf_print::make_pdf_font(i_buf *pdf_header)
{
    int i;
    double dvi_to_dots = 72.0 / 300.0;
    PKBit *b;
    extern PKBit bits[];

    if (f_i->flags & DPI600) dvi_to_dots /= 2.0;
    else if (f_i->m_flags & DPI1200) dvi_to_dots /= 4.0;             //adS
    else if (f_i->m_flags & DPI2400) dvi_to_dots /= 8.0;         

    pdf_header->PutString("10 dict begin\n");
    pdf_header->PutString("/FontType 3 def\n");
    pdf_header->PutString("/FontMatrix [ ");
    pdf_header->PutF(dvi_to_dots, 3);
    pdf_header->PutString("0 0 ");
    pdf_header->PutF(dvi_to_dots, 3);
    pdf_header->PutString("0 0 ] def\n");
    pdf_header->PutString("/FontBBox [");
    pdf_header->Put10(- pdf_max_off_w);
    pdf_header->Put10(- pdf_max_off_h);
    pdf_header->Put10(pdf_max_b_w);
    pdf_header->Put10(pdf_max_b_h);
    pdf_header->PutString("] def\n");
    pdf_header->PutString("/En {Encoding} def\n");
    pdf_header->PutString("/Encoding 256 array def\n");
    pdf_header->PutString("0 1 ");
    pdf_header->Put10(MYCHARS);
    pdf_header->PutString("{Encoding exch /.notdef put} for\n");
    for (i=0; i < 256; i++) {
	if (bits[i].bm_w && pdf_used[i]) {
	    pdf_header->PutString("En ");
	    pdf_header->Put10(i);
	    pdf_header->PutString("/b_");
	    pdf_header->Put10(i);
	    pdf_header->PutString("put\n");
	}
    }
    pdf_header->PutString("/CharProcs ");
    pdf_header->Put10(CHARS);
    pdf_header->PutString("dict def\n");
    pdf_header->PutString("CharProcs begin\n");
    pdf_header->PutString(" /.notdef { 1 1 add pop } bind def\n");
    for (i=0; i < 256; i++) {
	if (bits[i].bm_w && pdf_used[i]) {
//	    printf ( "b_%d", i);
	    pdf_bit_char(pdf_header, i);
	} 
    }
    pdf_header->PutString("end %% of CharProcs\n");
    pdf_header->PutString("/CharWidths ");
    pdf_header->Put10(CHARS);
    pdf_header->PutString("dict def CharWidths begin\n");
    pdf_header->PutString("/.notdef [ 0 0 0 0 ] def\n");
    for (i=0; i < 256; i++) {
	b = &bits[i];
	if (b->bm_w && pdf_used[i]) {	/* assume null chars have 0 width */
	    pdf_header->PutString("/b_");
	    pdf_header->Put10(i);
	    pdf_header->PutString("[ ");
	    pdf_header->Put10(b->bm_w - b->bm_h_off -1);
	    pdf_header->Put10(b->bm_v_off);
	    pdf_header->Put10(- b->bm_h_off);
	    pdf_header->Put10(b->bm_v_off - b->bm_h + 1);
	    pdf_header->PutString("] def\n");
	}
    }
    pdf_header->PutString("end %% of CharWidths\n");
    pdf_header->PutString("/BuildGlyph {\n");
/* x width, y width, x, y lower left bounding box, x, y upper right b box */
    pdf_header->PutString(" exch dup /CharWidths get 3 copy exch pop exch get dup dup dup\n");
    pdf_header->PutString(" 0 get /MaxXVal exch def 1 get /MaxYVal exch def \n");
    pdf_header->PutString(" 2 get /MinXVal exch def 3 get /MinYVal exch def pop \n");
    pdf_header->PutString(" MaxXVal 0 MinXVal MinYVal MaxXVal MaxYVal\n");
    pdf_header->PutString(" setcachedevice\n"); 
    pdf_header->PutString(" /CharProcs get exch\n");
    pdf_header->PutString(" 2 copy known not {pop /.notdef} if\n");
    pdf_header->PutString(" get exec\n");
    pdf_header->PutString("} bind def\n");

    pdf_header->PutString("/BuildChar {\n");
    pdf_header->PutString(" 1 index /Encoding get exch get\n");
    pdf_header->PutString(" 1 index /BuildGlyph get exec\n");
    pdf_header->PutString("}bind def\n");

    pdf_header->PutString("currentdict\n");
    pdf_header->PutString("end %% of dict\n");
    pdf_header->PutString("/LuteFont exch definefont pop\n"); /* define big dict */

/* Rule takes x, y */
    pdf_header->PutString("/Rule { currentpoint 4 2 roll \n");
    pdf_header->PutString(" 2 copy 0 exch rlineto \n");
    pdf_header->PutString(" 0 rlineto neg 0 exch rlineto \n");
    pdf_header->PutString(" neg 0 rlineto fill moveto } def\n");

    pdf_header->PutString("/RM {rmoveto} def\n");
    pdf_header->PutString("/MT {moveto} def\n");
    pdf_header->PutString("/S {show} def\n");
    pdf_header->PutString("/languagelevel where {pop languagelevel} {1} ifelse\n");
    pdf_header->PutString("2 lt { \n");
    pdf_header->PutString("/X {gsave show grestore} def\n");
    pdf_header->PutString("} { \n");
    pdf_header->PutString("/X {[0 0] xshow} def\n");
    pdf_header->PutString("} ifelse\n");
    pdf_header->PutString("/FF {findfont} def /SF {scalefont setfont} def\n");

// slurs !
/* do a slur, given length */
    pdf_header->PutString("/doslur { /delta exch def\n");
    pdf_header->PutString("gsave 1 setlinecap 0.7 setlinewidth\n");
    pdf_header->PutString("/delta delta 3 div def /height 5 def\n");
    pdf_header->PutString("currentpoint 2 copy 2 copy 2 copy\n");
    pdf_header->PutString("/y0 exch def /x0 exch def\n");
    pdf_header->PutString("/y1 exch def /x1 exch def /y2 exch def /x2 exch def\n");
    pdf_header->PutString("/y3 exch def /x3 exch def /x1 x1 delta add def\n");
    pdf_header->PutString("/y1 y1 height add def /x2 x2 delta 2 mul add def\n");
    pdf_header->PutString("/y2 y1 def /x3 x3 delta 3 mul add def\n");
    pdf_header->PutString("/y4 y2 1.8 add def ");
    pdf_header->PutString("/y5 y1 1.8 add def ");
    pdf_header->PutString("x1 y1 x2 y2 x3 y3 curveto ");
    pdf_header->PutString("x2 y4 x1 y5 x0 y0 curveto\n");
    pdf_header->PutString("fill  grestore } def\n");

/* do a reversed slur, given length */
    pdf_header->PutString("/dorslur { /delta exch def\n");
    pdf_header->PutString("gsave 1 setlinecap 0.7 setlinewidth\n");
    pdf_header->PutString("/delta delta 3 div def /height -5 def\n");
    pdf_header->PutString("currentpoint 2 copy 2 copy 2 copy\n");
    pdf_header->PutString("/y0 exch def /x0 exch def\n");
    pdf_header->PutString("/y1 exch def /x1 exch def /y2 exch def /x2 exch def\n");
    pdf_header->PutString("/y3 exch def /x3 exch def /x1 x1 delta add def\n");
    pdf_header->PutString("/y1 y1 height add def /x2 x2 delta 2 mul add def\n");
    pdf_header->PutString("/y2 y1 def /x3 x3 delta 3 mul add def\n");
    pdf_header->PutString("/y4 y2 1.8 sub def ");
    pdf_header->PutString("/y5 y1 1.8 sub def ");
    pdf_header->PutString("x1 y1 x2 y2 x3 y3 curveto ");
    pdf_header->PutString("x2 y4 x1 y5 x0 y0 curveto\n");
    pdf_header->PutString("fill grestore } def\n");

/* do a reversed half slur, given length */
    pdf_header->PutString("/dorhslur { /delta exch def\n");
    pdf_header->PutString("gsave 1 setlinecap 0.7 setlinewidth\n");
    pdf_header->PutString("/delta delta 3 div def /height -5 def\n");
    pdf_header->PutString("currentpoint 2 copy 2 copy 2 copy\n");
    pdf_header->PutString("/y0 exch def /x0 exch def\n");
    pdf_header->PutString("/y1 exch def /x1 exch def /y2 exch def /x2 exch def\n");
    pdf_header->PutString("/y3 exch def /x3 exch def /x1 x1 delta add def\n");
    pdf_header->PutString("/y1 y1 height add def /x2 x2 delta 2 mul add def\n");
    pdf_header->PutString("/y2 y1 def /x3 x3 delta 3 mul add def\n");
    pdf_header->PutString("/y4 y2 1.8 sub def ");
    pdf_header->PutString("/y5 y1 1.8 sub def ");
    pdf_header->PutString("x1 y1 x2 y2 x3 y3 curveto ");
    pdf_header->PutString("x2 y4 x1 y5 x0 y0 curveto\n");
    pdf_header->PutString("fill ");
    pdf_header->PutString(" grestore } def\n");

/* do a vertical slur, given heigth */
    pdf_header->PutString("/dovslur { /delta exch def\n");
    pdf_header->PutString("gsave 1 setlinecap 0.7 setlinewidth\n");
    pdf_header->PutString("/delta delta 3 div def /width -5 def\n");
    pdf_header->PutString("currentpoint 2 copy 2 copy\n");
    pdf_header->PutString("/y1 exch def /x1 exch def /y2 exch def /x2 exch def\n");
    pdf_header->PutString("/y3 exch def /x3 exch def /y1 y1 delta add def\n");
    pdf_header->PutString("/x1 x1 width add def /y2 y2 delta 2 mul add def\n");
    pdf_header->PutString("/x2 x1 def /y3 y3 delta 3 mul add def\n");
    pdf_header->PutString("x1 y1 x2 y2 x3 y3 curveto stroke grestore } def\n");


/* do a tie, given length and height */
    pdf_header->PutString("/dotie { /delta exch def /height exch def\n");
    pdf_header->PutString("gsave 1 setlinecap 0.7 setlinewidth\n");
    pdf_header->PutString("/delta delta 3 div def\n");
    pdf_header->PutString("currentpoint 2 copy 2 copy\n");
    pdf_header->PutString("/y1 exch def /x1 exch def /y2 exch def /x2 exch def\n");
    pdf_header->PutString("/y3 exch def /x3 exch def /x1 x1 delta add def\n");
    pdf_header->PutString("/y1 y1 height add def /x2 x2 delta 2 mul add def\n");
    pdf_header->PutString("/y2 y1 def /x3 x3 delta 3 mul add def\n");
    pdf_header->PutString("x1 y1 x2 y2 x3 y3 curveto stroke grestore } def\n");
    pdf_header->PutString("\n");
//    pdf_header->PutString("%%%%end of header\n");
}
void pdf_print::p_movev(const int ver)
{
    if (ver == 0) return;
    pdf_command(MOVEV, 0, ver, 0, 0); /* april 2019 wbc put a minus here */
    dvi_v -= ver;
}

void pdf_print::p_moveh(const int hor) 
{
   if (hor == 0) return;
   pdf_command(MOVEH, hor, 0, 0, 0);
   dvi_h += hor;	
}
void pdf_print::p_moveto(const int hor, const int ver)
{
//    printf("in moveto %d %d\n", hor, ver);
    pdf_command(MOVE, hor, -ver, 0, 0);
    dvi_h = hor;	
    dvi_v = ver;	
}
void pdf_print::p_put_rule(int w, int h) 
{ 
    if (highlight==On){
	pdf_command(P_S_GRAY, 0, 0, 0, 0);
    }
    pdf_command(RULE, w, h, 0, 0);
    if (highlight==On) { 
	clear_highlight();  
	pdf_command(P_U_GRAY, 0, 0, 0, 0);
    }
}
void pdf_print::put_a_char (unsigned char c) 
{
    if (highlight==On)  {
	if (highlight_type == Paren ) {
	    moveh (-.08);
	    pdf_command(PCHAR, (int)'(', 0, 0, 0);
	    moveh (.08);
	}
	else if (highlight_type == Red) {
	  pdf_command(P_S_RED, 0, 0, 0, 0);
	}
	else
	  pdf_command(P_S_GRAY, 0, 0, 0, 0);
    }
    if (curfont == 0 && pdf_used[c] < 4) 
	pdf_used[c]++;
    pdf_command(PCHAR, (int)c, 0, 0,0);
    if (highlight==On) { 
	clear_highlight();  
	if (highlight_type == Paren) {
	  double www=f_a[curfont]->fnt->get_width(c);
	    moveh (www);
	    pdf_command(PCHAR, (int)')', 0, 0, 0);
	    pdf_used['(']++;	pdf_used[')']++;
	    moveh (-www);
	}
	else if (highlight_type == Red)
	  pdf_command(P_U_RED, 0, 0, 0, 0);
	else
	  pdf_command(P_U_GRAY, 0, 0, 0, 0);
    }
}
void pdf_print::set_a_char (unsigned char c) 
{ 
    if (highlight==On) 
	pdf_command(P_S_GRAY, 0, 0, 0, 0);
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
	    pdf_command(CHAR, (int)c, 0, 0,0); // print the first one
	}
	else if ( c == 0014) c = 0256; // fi
	else if ( c == 0015) c = 0257; // fl
	else if ( c == 0031) c = 0373; // fs
	else if ( c == 0032) c = 0361; // ae
	else if ( c == 0035) c = 0341; // AE
	else if ( c == 0033) c = 0352; // oe
	else if ( c == 0036) c = 0372; // OE
	else if ( c == 0034) c = 0371; // oo
	else if ( c == 0037) c = 0351; // OO
	else if ( c == 0074) c = 0241; // ! inverted
	else if ( c == 0076) c = 0277; // ? inverted
    }

    if (curfont == 0 && pdf_used[c] < 4) 
      pdf_used[c]++;
    pdf_command(CHAR, (int)c, 0, 0,0);
    if (highlight==On) { 
	clear_highlight();  
	pdf_command(P_U_GRAY, 0, 0, 0, 0);
    }
}
void pdf_print::use_font(int fontnum) 
{ 
    curfont = fontnum;
    switch (fontnum) {
    case 7:			/* page nnumber font doesn't shrink 12 pt */
	pdf_command( NUMFONT, 0, 0, 0, 0);
	break;
    case 6:			/* font 6 18 pt text */
	pdf_command( MUSICS, 0, 0, 0, 0);
	break;
    case 5:			/* font 5 10 pt italic */
	pdf_command( ITAL, 0, 0, 0, 0); // title italic
	break;
    case 4:			/* font 4 med */
	pdf_command( MED, 0, 0, 0, 0);
	break;
    case 3:			/* font 3 big */
	pdf_command( BIGIT, 0, 0, 0, 0);
	break;
    case 2:			/* font 2 */
	pdf_command( ROMAN, 0, 0, 0, 0);	// title
	break;
    case 1:			/* font 1 */
	pdf_command( SMALL, 0, 0, 0, 0); // words to songs
	break;
    case 0:		        /* lute */
	pdf_command( LUTE, 0, 0, 0, 0);
	break;
    default:
	dbg0(Warning, "tab: pdf_setfont: undefined font number, using 2\n");
	pdf_command( ROMAN, 0, 0, 0, 0);
	break;
    }
}

void pdf_print::do_tie(double length) 
{ 
    pdf_command ( PTIE, inch_to_dvi(length), 0, 0, 0);
}
void pdf_print::do_tie_reversed(double length) 
{ 
    pdf_command ( PRTIE, inch_to_dvi(length), 0, 0, 0);
}
void pdf_print::do_half_tie(double length) 
{ 
    pdf_command ( PHTIE, inch_to_dvi(length), 0, 0, 0);
}
void pdf_print::do_half_tie_reversed(double length) 
{ 
    pdf_command ( PHRTIE, inch_to_dvi(length), 0, 0, 0);
}
void pdf_print::do_rtie(int bloc, int eloc) 
{ 
    pdf_command ( PTIE, (save_h[eloc] - save_h[bloc]), 0, 0, 0);
}
void pdf_print::print_clipped(char c, int font/* acutally height */)
{ 
    pdf_command ( PDF_CLIP, c, font, dvi_h, dvi_v);
     dvi_h += inch_to_dvi(f_a[curfont]->fnt->get_width(c));
}

void pdf_print::put_slash
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

void pdf_print::put_uline(int bloc, int eloc)
{ 
    saveloc(REGS-1);
    pdf_command ( PTIE2, save_h[eloc] - save_h[bloc], 0, 0, 0);
    getloc(REGS-1);
}
void pdf_print::put_thick_slant(int bloc, int eloc) 
{ 
  pdf_command(TH_LINE, save_h[bloc], save_v[bloc], save_h[eloc], save_v[eloc]);
}
void pdf_print::put_med_slant(int bloc, int eloc) 
{ 
   pdf_command(MED_LINE, save_h[bloc], save_v[bloc], save_h[eloc], save_v[eloc]);
}
void pdf_print::put_slant(int bloc, int eloc) 
{
    saveloc(REGS-1);

    pdf_command(LINE, save_h[bloc], save_v[bloc], save_h[eloc], save_v[eloc]);
    
    getloc(REGS-1);
    return;
}

int  pdf_print::more()
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
    //        printf("pdf more %f\n", dvi_to_inch(dvi_v));
    if (inch_to_dvi(length) > dvi_v) {
	return(1);		/* END_MORE */
    }
    return(0);			/* END_OK */
}
void pdf_print::showsave(int reg) 
   { dbg0(Error, "Undefined Proceedure showsave\n");}

void pdf_print::p_num(int n) 
{ 
#define N_S 5
    char string[N_S];
    int i;
    double total_width=0.0;
#ifdef MAC
#else
    sprintf(string, "%d", n);
#endif
    push();
    movev("0.18 in");

    use_font(1);
    for (i=0; i < N_S && string[i]; i++)
      total_width += f_a[curfont]->fnt->get_width(string[i]);

    //    printf("p_num, width is %f  %s\n", total_width, string);

    moveh(-total_width/2.0 + 0.018);

    for (i=0; i < N_S && string[i]; i++)
      set_a_char(string[i]);
	
    use_font(0);
    pop();
}

void pdf_print::print_draft() 
{ 
    pdf_command(PDRAFT, 0, 0, 0, 0);
}
void pdf_print::print_copyright() 
{ 
    pdf_command(PCOPYRIGHT, 0, 0, 0, 0);
}
void pdf_print::define_font(int font_num, char *name)
{
dbg0(Error, "Undefined Proceedure Define font\n");}

void pdf_print::vert_curve(int len)
{
    pdf_command(PVCURVE, len, 0, 0, 0);
}

void pdf_print::push() 
{
//    pdf_command(PPUSH, 0, 0, 0, 0);
//    printf("pdf push %d, dvi_h %d, stack h %d\n",
//	   sp, dvi_h, stack_h[sp]);
    slp(sp, stack_h, stack_v); sp++;		  // also set the dvi_v, dvi_h
}

void pdf_print::pop() 
{
//    pdf_command(PPOP, 0, 0, 0, 0);
//    sp--; dvi_h = stack_h[sp]; dvi_v = stack_v[sp];
    sp--;
    glp(sp, stack_h, stack_v);
//    printf("pdf pop  %d, h %d, stack h %d\n", sp, dvi_h, stack_h[sp]);
}

int my_pdf_isprint(int c)
{
  if (c < 33 ) return (0);
  if (c > 126 ) return (0);
  return (1);
}

static int last_com=OTHER;
static int last_last_com=OTHER;
static int last_font=OTHER;

void pdf_print::pdf_command(int com, int h_n, int v_n, int hh_n, int vv_n)
{
  static int h, v, hh, vv;
  static int dvi_oh, dvi_ov;

  static int ooh, oov, oohh, oovv; // two previous
  static int dvi_ooh, dvi_oov;

  /*    extern double pdf_size[]; */
  time_t timeval;
  int places=4;			// digits of floating pt numbers
  double currentgray =0.0;
  char ctemp[STREAM];

  //    printf("pdf_command com %d h %03.2f v %03.2f\n", 
  //	   com, d_to_p (h_n), d_to_p(v_n));

  //    if (com == MOVE  printf("com - move h_n %d %f\n", h_n, d_to_p(h_n));
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

  // printf ("  pdf_print.cc: 837: pdf_command %x\n", last_com);
  switch (last_com) {
  case MOVE:
    {
      double ttt=d_to_p(h); double tttt=d_to_p(-v);
      sprintf  (ctemp, "%.1f %.1f m \n", ttt, tttt);
      // printf ("     MOVE %.4f  %.4f\n",  ttt, tttt);
    }
    // printf ("     MOVE: %.4f %.4f\n",  d_to_p(h), d_to_p( -v));
    strcat (pdf_stream_b, ctemp);
    break;
  case RULE:		// now takes x, y to draw a box 0, 0, x, y
    // printf("pdf_print: RULE 847: %d %d\n", dvi_h, h);
    // 
    sprintf (ctemp, "q %.1f %.1f %.1f %.1f re s Q \n",
		    d_to_p(dvi_h), d_to_p(dvi_v),
		    d_to_p(h), d_to_p(-v));
    strcat (pdf_stream_b, ctemp);
    break;
  case MOVEH:
    sprintf  (ctemp, "%.1f 0 m \n", d_to_p(h));
    // printf ("     MOVEH %.4f\n",  d_to_p(h));
    strcat (pdf_stream_b, ctemp);
    break;
  case MOVEV:
    // if (d_to_p(dvi_v -v) < 610.0 ) { printf("HERE HERE\n");}
    sprintf (ctemp, "0 %.1f m \n", d_to_p(dvi_v -v));
    // printf ("     MOVEV: %.4f\n",  d_to_p(dvi_v -v));
    strcat (pdf_stream_b, ctemp);
    break;
  case MOVEVH:
    pr_out->PutF(d_to_p(h), places);
    pr_out->PutF(d_to_p(-v), places);
    pr_out->PutString(" RM %%MoveVH\n");
    break;

  case LINE:
    pr_out->PutString("0.2 setlinewidth ");
    pr_out->PutF(d_to_p(h), places);
    pr_out->PutF(d_to_p(v), places);
    pr_out->PutString("m ");
    pr_out->PutF(d_to_p(hh), places);
    pr_out->PutF(d_to_p(vv), places);
    pr_out->PutString("l\n");
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
    pr_out->PutF(d_to_p(h), places);
    pr_out->PutF(d_to_p(v), places);
    pr_out->PutString(" 0.3 sub moveto 0.0 .6\n");
    pr_out->PutString("rlineto ");
    pr_out->PutF(d_to_p(hh), places);
    pr_out->PutF(d_to_p(vv), places);
    pr_out->PutString("0.3 add lineto 0.0 -.6 rlineto closepath fill\n");
    break;

  case CHAR:
  case PCHAR:
    if ( h == 050 || h == 051 || h == '\\') {
      pr_out->PutString("(\\");
      pr_out->PutChar(h);
      pr_out->PutString(")");
    }
    else if (my_pdf_isprint(h)) {
      /*      pr_out->PutString("(");
	      pr_out->PutChar(h);
	      pr_out->PutString(")");
      */;
    }
    else {
      printf ("pdf_print.cc: 918: printing letters\n");
      pr_out->PutString("<");
      pr_out->Put16(h);
      pr_out->PutString(">");
      ; 
    }
    if ( last_com == CHAR) 
      /* pr_out->PutString("S\n") */;
    else 
      /* pr_out->PutString("X\n") */;
    break;
  case LUTE:
    if ( last_font != LUTE ) {
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
  case PDF_CLIP:
    pr_out->PutString("gsave\n");
    pr_out->PutString("currentpoint  /yval exch def /xval exch def pdftack\n");
    //	printf("pdf clip printing %c\n", h);
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
  case PDRAFT :
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
    pr_out->PutString("0 -520 rmoveto\n");
    pr_out->PutString("( \251 ) show\n");
    pr_out->PutString("(Copyright Karen Meyers, 1997)show\n");
    pr_out->PutString("grestore \n");
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
  case OTHER:
    break;
  default:
    dbg1(Warning, "tab: pdf_command: unknown command %d\n", (void *)last_com);
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

void pdf_bit_char(i_buf *pdf, int char_num)
{
    int i;
    PKBit *b;
    extern PKBit bits[];
    int byte_w;

    b = &bits[char_num];
    byte_w = (b->bm_w + 7 ) / 8; 


    pdf->PutString("/b_");
    pdf->Put10(char_num);
    pdf->PutString(" {");  // char_name);

    pdf->Put10(b->bm_w); pdf->Put10(b->bm_h);
    pdf->PutString("true [1 0 0 1 ");
    pdf->Put10(b->bm_h_off);     pdf->Put10(b->bm_h - b->bm_v_off );
    pdf->PutString("]\n{<");
/* bitmap */

    for (i=0; i < byte_w * b->bm_h; i++) {
	pdf->Put16(b->bm_bits[i] & 0xff);
	if (!((i+1) % 40)) pdf->PutString("\n");
    }
    pdf->PutString(">}\n imagemask } bind def\n");

 /* end bitmap */
}

void pdf_print::glp(int reg,int h[], int v[])
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

    pdf_command(MOVE, dvi_h, -dvi_v, 0, 0);
    
}

void pdf_print::comment(const char *string)
{
  //    char cc[80], *c;
    //    strncpy(cc, string, 80);
    //    c = cc;
    //    while ( *c ) {
    //	pdf_command(RAW, (int)*c, 0, 0, 0);
    //	c++;
    //    }
}


void new_xref_list()
{
  fprintf(stderr, "first xref entry\n");

  xref_root = (xref_entry *) malloc (sizeof (xref_entry));
  xref_root->byte_offset = 0;
  xref_root->generation = 65535;
  xref_root->use = 'f';
  xref_root->next=0;
  xref_count = 1;
}

void new_xref_entry(const int offset)
{
  xref_entry *t, *u;

  fprintf(stderr, "adding xref entry offset %u\n", offset);

  t = (xref_entry *) malloc (sizeof (xref_entry));
  if ( ! t ) { fprintf(stderr, "new_xref_entry: malloc failed\n");}

  t->byte_offset = offset;
  t->generation = 0;
  t->use = 'n';
  t->next = 0;

  xref_count += 1;
  
  u = xref_root;
  
  while (u->next) {
    //    fprintf(stderr, "increment xref list\n");
    u = u->next;
  }
  u->next = t;

}

// print out xref information
//
void pdf_print::file_xref()
{
  xref_entry *u;
  char tstring[36];

  pr_out->PutString("xref\n");
  snprintf (tstring, 19, "%d %d\n", 0, xref_count);
  pr_out->PutString(tstring);

  u = xref_root;
  if (! u ) {
    dbg0 (Error, "pdf_print: file_xref: uninitialized list");
  }

  while (u) {
    snprintf (tstring, 35, "%010d %05d %c \n", u->byte_offset, u->generation, u->use);
    pr_out->PutString(tstring);
    u = u->next;
    // free nodes here
  }
}

// do catalog, return byte counts in catalog
//
unsigned int pdf_print::do_catalog()
{
  unsigned int bytes = 0;

  fprintf (stderr, "Catalog\n");
  /* save byte position of first byte */
  new_xref_entry( byte_count /* offset */);
  bytes += pr_out->PutStringC("1 0 obj\n");
  bytes += pr_out->PutStringC("  << /Type /Catalog\n");
  bytes += pr_out->PutStringC("     /Pages 2 0 R\n");
  bytes += pr_out->PutStringC("  >>\nendobj\n"); 
  return (bytes);
}

unsigned int pdf_print::do_page_tree() 
{
  unsigned int bytes = 0;

  fprintf (stderr, "Page Tree\n");
  new_xref_entry( byte_count /* offset */);
  bytes += pr_out->PutStringC("2 0 obj\n");
  bytes += pr_out->PutStringC("  << /Type /Pages\n");
  bytes += pr_out->PutStringC("     /Kids [3 0 R]\n");
  bytes += pr_out->PutStringC("     /Count 1\n");
  bytes += pr_out->PutStringC("  >>\nendobj\n"); 
  return (bytes);
}

unsigned int pdf_print::do_page_leaf() 
{
  unsigned int bytes = 0;

  fprintf (stderr, "Page Leaf\n");
  new_xref_entry( byte_count /* offset */);
  bytes += pr_out->PutStringC("3 0 obj\n");
  bytes += pr_out->PutStringC("  << /Type /Page\n");
  bytes += pr_out->PutStringC("     /Parent 2 0 R\n");
  bytes += pr_out->PutStringC("     /MediaBox [0 0 621 792]\n");
  //bytes += pr_out->PutStringC("     /Contents 4 0 R\n");
  bytes += pr_out->PutStringC("     /Contents 6 0 R\n");
  bytes += pr_out->PutStringC("     /ProcSet 5 0 R\n");
  bytes += pr_out->PutStringC("     /Resources << /Font  << /F13 21 0 R >> >> \n");
  bytes += pr_out->PutStringC("  >>\nendobj\n"); 
  return (bytes);
}
unsigned int pdf_print::do_page_content(i_buf *i_b,  struct font_list *f_a[])
{
  unsigned int bytes = 0;
  char b[128];
  unsigned int scount ;
  char s_buf[STREAM];

  memset (s_buf, 0, sizeof(s_buf));
  fprintf (stderr, "Page Content\n");
  new_xref_entry( byte_count /* offset */);
  bytes += pr_out->PutStringC("4 0 obj\n");
  scount = do_rule_stream(i_b,  f_a, s_buf);

  // we need to know byte count of content stream here
  // count includes trailing newline
    
  sprintf (b, "<</Length %u>>\n", scount);
  bytes += pr_out->PutStringC(b);

  bytes += pr_out->PutStringC("stream\n");
  
  bytes += pr_out->PutStringC(s_buf);
  //  print_stream();
  // we go byte count in scount above
  bytes += pr_out->PutStringC("endstream\n");
  bytes += pr_out->PutStringC("endobj\n");

  bytes += do_text_stream(i_b, f_a, s_buf);
  return(bytes);
}

unsigned int pdf_print::do_page_resource() 
{
  unsigned int bytes = 0;

  fprintf (stderr, "Page Resource\n");
  new_xref_entry( byte_count /* offset */);
  bytes += pr_out->PutStringC("5 0 obj \n");
  bytes += pr_out->PutStringC("<<  /ProcSet [/PDF] \n");
 // bytes += pr_out->PutStringC("    /Font << /F13 21 0 R >> \n");
  bytes += pr_out->PutStringC(">> \n endobj \n");   
  return(bytes);
}

unsigned int pdf_print::pdf_fontdef()
{
  unsigned int bytes = 0;
   fprintf (stderr, "Define Font\n");
   new_xref_entry( byte_count /* offset */);
   bytes += pr_out->PutStringC("21 0 obj\n");
   bytes += pr_out->PutStringC("<<  /Type /Font \n   /Suntype /Type1 \n   /BaseFont /Helvetica \n >> \n");
   bytes += pr_out->PutStringC("endobj\n"); 
  return(bytes);
}

unsigned int pdf_print::do_rule_stream(i_buf *i_b,  struct font_list *f_a[],
				  char *s_buf) {
  unsigned int bytes = 0;

  fprintf(stderr,"pdf_print: do_rule_stream: bytes dec %d oct %o\n",
	  bytes, bytes);
 
  //  pdf_print *page_buf;
  
  page_retval = (format_page(this, i_b, f_a, f_i));
  
  strcat (s_buf,  pdf_stream_b );
  
  bytes = strlen(s_buf);
  printf("do_rule_stream: bytes %u \n", bytes);
  if (bytes > STREAM) { dbg1(Error, 
     "tab: pdf_print.cc: do_rule_stream: bytes greater than STREAM %d\n",
			     (void *)bytes); }
  return (bytes);
}

unsigned int pdf_print::do_text_stream(i_buf *i_b,  struct font_list *f_a[],
				  char *s_buf) {
  unsigned int bytes = 0;
  unsigned int scount = 0;
  char b[16];
  
  fprintf(stderr,"pdf_print: do_text_stream: bytes dec %d oct %o\n",
	  bytes, bytes);
  
  memset (s_buf, 0, sizeof(s_buf));
  strcpy (s_buf, "BT\n/F13 12 Tf\n 288 520 Tda\n (ABC) Tj\nET \n");
  scount = strlen(s_buf);
  bytes += pr_out->PutStringC("6 0 obj\n<</Length ");
  sprintf (b, "%d", scount);
  bytes += pr_out->PutStringC(b);
  bytes += pr_out->PutStringC(">>\nstream\n");
  bytes += pr_out->PutStringC(s_buf);
  bytes += pr_out->PutStringC("endstream\nendobj\n");
  return (bytes);
}


// we don't need this anymore, I hope
void pdf_print::print_stream() {
}


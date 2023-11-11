/*
 *
 *
 */
#include "win.h"
#include "tab.h"
#include "print.h"
#include "dviprint.h"
#include "dvi.h"
#include "i_buf.h"

int format_page(print *d_p, i_buf *i_b, font_list *f_l[], struct file_info *f);

dvi_print::dvi_print(font_list *font_array[], file_info *f)
{
    strcpy(tag, " TeX output by Wayne Cripps");
    last_bop = -1;
    page_number = 1;
    cur_loc = 0;
    f_i = f;
    f_a = font_array;
    f_name[0] = '\0';
    if (*f->out_file) {
      strcpy(f_name, f->out_file);
    }
    file_head();
}

dvi_print::~dvi_print()
{
    file_trail();
    if (f_name[0])
      pr_out->dump(f_name, Creat);
    else
      pr_out->dump("out.dvi", Creat);

}

void dvi_print::file_head()		// write_preamble()
{
    int i;
    PutByte(PRE);		/* first byte - preamble opcode */
    PutByte(2);			/* version - identification*/
    SignedQuad(NUM);		/* numerator */
    SignedQuad(DENOM);		/* denominator */
    SignedQuad(1000);		/* mag * 1000 */
    i = strlen(tag);
    PutByte(i);	/* length of comment */
    for (i=0;i < (signed int)strlen(tag); i++)	/* the comment */
      PutByte ((unsigned char)tag[i]);
}

void dvi_print::define_all_fonts()
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

void dvi_print::page_head()
{
    int prev_bop = cur_loc;
    int i;

    PutByte(BOP);
    SignedQuad(page_number++);
    for (i=1; i< 10; i++) SignedQuad(0);
    SignedQuad(last_bop);
    last_bop = prev_bop;
    define_all_fonts();

/*
    dvi_command(NO_OP, 0, 0);
    PutByte(EOP);
    reset_dvi_vh();
 */
}

void dvi_print::file_trail()		// write_postamble
{
    int loc;

    loc = cur_loc;
    PutByte(POST);
    SignedQuad((long)last_bop);	/* pointer to last bop */
    SignedQuad(NUM);	                  /* numerator */
    SignedQuad(DENOM);	                  /* denominator */
    SignedQuad(1000);		          /* mag * 1000 */
    SignedQuad((long)inch_to_dvi(11.0));  /* height plus depth tallest page */
    SignedQuad((long)inch_to_dvi(8.5));	  /* width of widest page */
    SignedPair(4);		          /* max stack depth */
    SignedPair((short)page_number-1);	  /* total num pages */
    /* font definitions */

    define_all_fonts();

    PutByte(POST_POST);	        /* post-post */
    SignedQuad((long)loc);      /* ptr to post that started postamble */
    PutByte(2);       	        /* identifier */
    PutByte(223);		/* filler 4 - 7 to pack file*/
    PutByte(223);
    PutByte(223);
    PutByte(223);
}

void dvi_print::page_trail()
{
	dvi_command(NO_OP, 0, 0);
	PutByte(EOP);

	reset_dvi_vh();
}

void dvi_print::p_movev( const int ver)
{
    if (ver == 0) return;

    dvi_v += ver;
    if (ver > 0) {
	if (ver & 0xff800000) {
	    dvi_command(DOWN4, ver, 0);
	}
	else if (ver & 0x007f8000) {
	    dvi_command(DOWN3, ver, 0);
	}
	else if (ver & 0x00007f80) {
	    dvi_command(DOWN2, ver, 0);
	}
	else if (ver & 0x0000007f) {
	    dvi_command(DOWN1, ver, 0);
	}
    }
    else {
	if (ver > -32767) {
	    dvi_command(DOWN2, ver, 0);
	}
	else if (ver & 0xff000000) {
	    dvi_command(DOWN4, ver, 0);
	}
    }
}

void dvi_print::p_moveh( const int hor)
{
   if (!hor) return;

   dvi_h += hor;
   if (hor & 0xff800000) {
       dvi_command(RIGHT4, hor, 0);
   }
   else if (hor & 0x007f8000) {
       dvi_command(RIGHT3, hor, 0);
   }
   else if (hor & 0x00007f80) {
       dvi_command(RIGHT2, hor, 0);
   }
   else if (hor & 0x0000007f) {
       dvi_command(RIGHT1, hor, 0); /* was RIGHT4 */
   }
}
void dvi_print::p_put_rule(int w, int h)
{
    dvi_command(PUT_RULE, h, w);
}
void dvi_print::put_a_char (unsigned char c)
{
  //    printf ("dviprint - put - c is %d %o %c\n", c, c, c);
    if (highlight==On) {
	moveh (-.07);
	dvi_command(PUT1, (int)'(', 0);
	moveh (.07);
    }
    dvi_command(PUT1, (int)c, 0);
    if (highlight==On) {
	moveh (.10);
	dvi_command(PUT1, (int)')', 0);
	moveh (-.10);
	clear_highlight();
    }
}
void dvi_print::set_a_char (unsigned char c)
{
//    if (highlight==On) {
//	dvi_command(PUT1, (int)'(', 0);
//    }
//    printf ("dviprint - set - c is %d %o %c\n", c, c, c);
    if (c == '{') {
	dvi_h += inch_to_dvi(f_a[curfont]->fnt->get_width(050));
	dvi_command(SET1, (int)050, 0);
    }
    else if (c == '}') {
	dvi_h += inch_to_dvi(f_a[curfont]->fnt->get_width(051));
	dvi_command(SET1, (int)051, 0);
    }
    else {
	dvi_h += inch_to_dvi(f_a[curfont]->fnt->get_width(c));
	dvi_command(SET1, (int)c, 0);
    }
//    if (highlight==On) {
//	clear_highlight();
//    }
}
void dvi_print::use_font(int fontnum)
{
    curfont = fontnum;
    dvi_command(FNT1, fontnum, 0);
}
void dvi_print::do_tie(double length)
{
    dbg0(Warning, "Undefined Proceedure dvi do_tie \n");
}
void dvi_print::do_tie_reversed(double length)
{
    dbg0(Warning, "Undefined Proceedure dvi do_tie_reversed \n");
}
void dvi_print::do_half_tie(double length)
{
    dbg0(Warning, "Undefined Proceedure dvi do_tie \n");
}
void dvi_print::do_half_tie_reversed(double length)
{
    dbg0(Warning, "Undefined Proceedure dvi do_tie_reversed \n");
}
void dvi_print::do_rtie(int bloc, int eloc)
{
    dbg0(Warning, "tab: do_rtie: ties are not printed in dvi mode \n");
}
void dvi_print::print_clipped(char c, int font)
{
//    dbg0(Warning, "Undefined Proceedure ps_clipped\n");
    switch (c) {
      case 'i':
	set_a_char(020);
	break;
      case 'j':
	set_a_char(021);
	break;
      default:
	set_a_char(c);
	break;
    }

}

void dvi_print::put_slash
      (int bloc, int eloc, int count, struct file_info *f)
{
    double nflags = (double)count;
    int thickness = str_to_dvi("0.005 in");

    if (f->flags & LSA_FORM) thickness = str_to_dvi("0.023 in");

    saveloc(REGS-1);
    while (count) {
	dvi_command(PUT_RULE,
		    thickness,
		    save_h[eloc] - save_h[bloc]);
	movev(str_to_inch("0.03 in") + 0.06/ nflags);
	count --;
    }
    getloc(REGS-1);
}
void dvi_print::put_uline(int bloc, int eloc)
{
    saveloc(REGS-1);

    dvi_command(PUT_RULE,
		str_to_dvi("0.005 in"),
		save_h[eloc] - save_h[bloc]);
    getloc(REGS-1);
}
void dvi_print::put_thick_slant(int bloc, int eloc)
{
    dbg0(Warning, "using thin slant instead of thick \n");
    put_slant(bloc, eloc);
}
void dvi_print::put_med_slant(int bloc, int eloc)
{
    dbg0(Warning, "using thin slant instead of medium\n");
    put_slant(bloc, eloc);
}
void dvi_print::put_slant(int bloc, int eloc)
{
    int length, height, inter;
    unsigned char cc;
    int c_h, c_v;
    extern char interspace[];
    int i_space = str_to_dvi(interspace);
    int i_ratio;

    saveloc(REGS-1);

    length = (save_h[eloc] - save_h[bloc]);
    height = (save_v[eloc] - save_v[bloc]);

    if (!length) {
	dbg0(Warning, "tab: put_slant: null length!\n");
	put_a_char(0);
	put_a_char('Q');
	return;
    }

    i_ratio = height * 256 / length;
    if (i_ratio > 10){
	cc = 3;
	p_movev(i_space);
    }
    else if ( i_ratio > -40 ) { /* no slope */
	dvi_command(PUT_RULE, str_to_dvi("0.005 in"),
		    (save_h[eloc] - save_h[bloc]));
	return;
    }
    else if (i_ratio > -100 ) cc = 4; /* shallow upwards */
    else cc = 1;		/* steep upwards */

    c_h = inch_to_dvi(f_a[0]->fnt->get_width(cc));
    c_v = inch_to_dvi(f_a[0]->fnt->get_height(cc));

    if ( c_h == 0 || c_v == 0 )
      dbg1 (Error, "tab: slant: font width compatibility problem %d\n",
	    (void *)(int)(cc));

    if (c_h == 0 || c_v == 0) {
	dbg1(Warning,
	     "tab: put_slant: bad font char width or height %d\n",
	     (void *)((int)cc));
	return;
    }
    if (height < 0) c_v = - c_v;

    while (length > c_h) {
	put_a_char((char)cc);
	p_moveh(c_h);
	p_movev(c_v);
	length -= c_h;
    }
    inter = length - c_h;
    p_moveh(inter);
    p_movev(inter * (((c_v*64 )/ c_h)) / 64 );

    put_a_char((char)cc);

    getloc(REGS-1);
    return;
}
int  dvi_print::more()		// return 1 if page full
{
    int foo=inch_to_dvi(8.8 / red);
//    printf("dvi more %f\n", dvi_to_inch(dvi_v));
    if (dvi_v  > foo)
      return(1);
    return(0);
}
void dvi_print::showsave(int reg)
   { dbg0(Error, "Undefined Proceedure showsave\n");}

void dvi_print::p_num(int n)
{
#define N_S 5
    char string[N_S];
    int i;
#ifdef MAC
	string[0] = 0;
#else
    snprintf(string, sizeof(string), "%d", n);
#endif
    push();
    movev("0.18 in");

    use_font(1);
    for (i=0; i < N_S && string[i]; i++)
      set_a_char(string[i]);

    use_font(0);
    pop();
}
void dvi_print::print_draft() { dbg0(Error, "Undefined Proceedure ps_draft\n");}
void dvi_print::vert_curve(int len)
      { dbg0(Error, "Undefined Proceedure dvi vert_curve\n");}
void dvi_print::print_copyright()
{
    dbg0(Error, "Undefined Proceedure print_copyright\n");
}
void dvi_print::define_font(int font_num, char *name)
{
    int i, j, l = strlen(name);
    font_list **ff = f_a;

    for (i=0; i< MAXFONTS; i++) {
	if ((*ff) && (*ff)->num == font_num) {
	    dbg1 (Fonts, "define font %s\n", (void *)((*ff)->name));
	    break;
	}
	(*ff++);
    }

    PutByte(FNT_DEF1);
    PutByte((unsigned char)font_num);	             /* font number */
    SignedQuad((long)(*ff)->fnt->get_check_sum());   /* checksum */
    SignedQuad((long)(*ff)->fnt->get_design_size()); /* loaded at size, */
			        /* scale factor applied to char widths */
    SignedQuad((long)(*ff)->fnt->get_design_size()); /* design size */
    PutByte(0);		                /* a - the length of directory */
    PutByte((unsigned char)l);		/* length of font name */
    for (j=0; j< l ; j++)
      PutByte((unsigned char)name[j]);
    /* font name */

    /* font area, or directory goes here */
}

static int o_com=NO_OP;

void dvi_print::dvi_command(int com, int val1, int val2)
{

    static int o_val1, o_val2;

    /*
    switch (com) {
    case FNT1:
      printf ("dvi_command: FNT1 %d\n", val1);
      break;
    case DOWN3:
      printf ("dvi_command: DOWN3\n");
      break;
    case DOWN2:
      printf ("dvi_command: DOWN2\n");
      break;
    case RIGHT4:
      printf ("dvi_command: RIGHT4\n");
      break;
    case SET1:
      printf ("dvi_command: SET1 %c\n", val1);
      break;
    default:
      printf ("dvi_command: %d\n", com);
    }
    */

    if (com == DOWN4) {
	switch (o_com) {
	  case DOWN3:
	  case DOWN4:
	    o_val1 += val1;
	    o_com = com;
	    return;
	  default:
	    break;
	}
    }
    else if (com == DOWN3 ) {
	switch (o_com) {
	  case DOWN3:
	  case DOWN4:
	    o_val1 += val1;
	    o_com = DOWN4;
	    return;
	  default:
	    break;
	}
    }
    if (com == RIGHT4) {
	switch (o_com) {
	  case RIGHT3:
	  case RIGHT4:
	    o_val1 += val1;
	    o_com = com;
	    return;
	  default:
	    break;
	}
    }

    switch (o_com) {
      case RIGHT4:
	PutByte((unsigned char)RIGHT4);
	SignedQuad(o_val1);
	break;
      case RIGHT3:
	PutByte((unsigned char)RIGHT3);
	SignedTrio(o_val1);
	break;
      case RIGHT2:
	PutByte((unsigned char)RIGHT2);
	SignedPair(o_val1);
	break;
      case RIGHT1:
	PutByte((unsigned char)RIGHT1);
	PutByte(o_val1);
	break;
      case DOWN4:
	PutByte((unsigned char)DOWN4);
	SignedQuad(o_val1);
	break;
      case DOWN3:
	PutByte((unsigned char)DOWN3);
	SignedTrio(o_val1);
	break;
      case DOWN2:
	PutByte((unsigned char)DOWN2);
	SignedPair(o_val1);
	break;
      case DOWN1:
	PutByte((unsigned char)DOWN1);
	PutByte(o_val1);
	break;
      case PUT_RULE:
	PutByte((unsigned char)PUT_RULE);
	SignedQuad(o_val1);
	SignedQuad(o_val2);
	break;
      case PUT1:
	PutByte((unsigned char)PUT1); PutByte(o_val1);
	break;
      case SET1:
	PutByte((unsigned char)SET1);
	PutByte(o_val1);
	break;
      case FNT1:
	PutByte((unsigned char)FNT1);
	PutByte(o_val1);
	break;
      case PUSH:
	PutByte((unsigned char)PUSH);
	break;
      case POP:
	PutByte((unsigned char)POP);
	break;
      case EOP:			/* this should flush */
	PutByte((unsigned char)EOP);
	break;
      case NO_OP:
	PutByte((unsigned char)NO_OP);
	break;
      default:
	dbg1(Warning, "tab: dvi_command: undefined command %d\n",
	     (void *)com);
	break;
    }

    o_val1 = val1;
    o_val2 = val2;
    o_com = com;
}

int dvi_print::do_page(i_buf *i_b,  struct font_list *f_a[])
{
    return (format_page(this, i_b, f_a, f_i));
}

void dvi_print::PutByte(unsigned char c)
  { cur_loc++;   pr_out->PutByte(c); }
void dvi_print::SignedPair(unsigned short c)
  { cur_loc +=2; pr_out->SignedPair(c); }
void dvi_print::SignedTrio(int c)
  { cur_loc +=3; pr_out->SignedTrio(c); }
void dvi_print::SignedQuad(int c)
  { cur_loc +=4; pr_out->SignedQuad(c); }

void dvi_print::comment(const char *string)
{
  //    char c = string[0];
}

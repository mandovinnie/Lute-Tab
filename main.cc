
#include <sys/stat.h>

#include "win.h"
#include "version.h"
#include "tab.h"
#include "i_buf.h"
#include "file_in.h"
#include "print.h"
#include "dviprint.h"
#include "ps_print.h"
#include "pdf_print.h"
#include "ascii.h"
#include "nmidi.h"

#include "sound.h"
#include "midi_snd.h"
#include "raw_snd.h"

#ifdef __unix__
#include <signal.h>
#include <unistd.h>
#endif /* unix */

void get_tab_file(file_in *fi, i_buf *ib, struct file_info *f);
void args(int argc, char ** argv, struct file_info *f);
struct font_list *init_font_list(int num, const char *name, double scale);
struct font_list  *delete_font_list(struct font_list *f);
void init(file_info *f);
void tfm_stuff(i_buf *b, file_info *f);
void file_stuff(i_buf *i_b, struct file_info *f);
#ifdef MAC
int my_main(int flags);
#endif /* MAC */

extern double conv;
sound *sp=0;
extern char staff_height[];

int baroque;
int thin_renaissance;
int n_system;
int bar_count;
int barCount;
int barCCount;
int pagenum;
double red;

void init(file_info *f)
{
    baroque = 0;
    thin_renaissance = 0;
    n_system = 0;
    strncat(staff_height, "0.0057 in", 80);

    /*    int barCount = 0; */
    /*    int pagenum = 0; */
    bar_count = 0;
    red = 1.0;
    f->file = (char *)malloc( BUFSIZ /* BL */ );  /* wbc Mar 28 2022 */
    f->file[0] = '\0';
    f->out_file = (char *)malloc( BUFSIZ /* BL */ );
    f->out_file[0] = '\0';
    f->line_flag = BETWEEN_LINE;
    f->flag_flag = STAND_FLAGS;
    f->char_flag = STAND_CHAR;
    f->num_flag = STAND_NUM;
    f->note_conv = 0 /*3*/;
    f->c_space = 0.045; /* was 0.0;	 */
    f->flags = 0;
    f->m_flags = 0;
    f->flags |=PS;
    f->flags |= DPI600;
    f->cur_system = 0;
    f->n_text = 0;
    f->font_sizes[0] =  0.0;	// not used
    f->font_sizes[1] = 10.0;	// used for song text, roman
    f->font_sizes[2] = 12.0;	// used for title, roman
    f->font_sizes[3] = 12.0;	// used for title italics
    f->font_sizes[4] = 24.0;	// used for time signature
    f->font_sizes[5] = 10.0;	// used for text italics
    f->font_sizes[6] = 17.0;	// used for time signatures in music
    f->font_sizes[7] = 0.0;	// not used
    f->note_flag = MOD_NOTES;
    f->sys_skip=0.0;
    f->cur_system = 0;
    f->include = 0;
    f->midi_patch = 0;
    f->midi_volume = 110;
    f->start_system = 0;
    f->transpose=0;
    f->scribe=0;
    f->title=0;
    f->left_margin=72;
    f->top_margin=72;
    f->extended_character_set = 0;
    f->bar_number_font = 1;
    f->slur_depth = -1.20;
}

char *
get_real_name(const char *short_name, int dump);
				/* this bit loads in the tfm metrics */
nmidi *np=0;

void tfm_stuff(i_buf *b, file_info *f)
{
    print **pp;
    dvi_print *pdp = 0;
    ps_print *psp = 0;
    pdf_print *pdfp = 0;
    struct font_list *f_a[MAXFONTS];
    int i, more=END_MORE;
    char lutefont[80];

    strcpy(lutefont, "");

    if (f->font_names[0]) {
      strcat (lutefont, f->font_names[0]);
    }
    else {
      if (baroque)
	strcat(lutefont, "blute");
      else if (thin_renaissance)
	strcat(lutefont, "tlute");
      else
	strcat(lutefont, "lute");
    }

    if (red == 1.0) {
	strcat (lutefont, "9");
    }
    else if (red == 0.9444 ) {
	strcat (lutefont, "85");
    }
    else if (red == 0.8888 ) {
	strcat (lutefont, "8");
    }
    else if (red == 0.777770) {
	strcat (lutefont, "7");
    }
    else {
	strcat(lutefont, "6");
    }

    f_a[0] = init_font_list(0, lutefont, 1.0);

    for (i=1; i< MAXFONTS; i++) { // was 3 ??
      f_a[i] = NULL;
    }

    if (f->flags & PS ) {
	if (f->font_names[1]) {	// words
	  f_a[1] = init_font_list(1, f->font_names[1], 1.2 );
	  f_a[1]->real_name = get_real_name(f->font_names[1], 0);
	}
	else
	  f_a[1] = init_font_list(1, "pncr", 1.0 );

	if (f->font_names[2]) {	// title
	  f_a[2] = init_font_list(2, f->font_names[2], f->font_sizes[2]/10. );
	  f_a[2]->real_name = get_real_name(f->font_names[2], 0);
	}
	else
	  f_a[2] = init_font_list(2, "pncr", f->font_sizes[2]/10.);

	if (f->font_names[3]) {	// italic title
	  f_a[3] = init_font_list(3, f->font_names[3], 1.2 );
	  f_a[3]->real_name = get_real_name(f->font_names[3], 0);
	}
	else
	  f_a[3] = init_font_list(3, "pncri", 1.2 );

	f_a[4] = init_font_list(4, "pncr", 2.4 );

	if (f->font_names[5]) {	// italic text
	  f_a[5] = init_font_list(5, f->font_names[5], 1.0 );
	  f_a[5]->real_name = get_real_name(f->font_names[5], 0);
	}
	else
	  f_a[5] = init_font_list(5, "pncri", 1.0 );
	f_a[6] = init_font_list(6, "pncr", 1.0 );
	f_a[7] = init_font_list(7, "pncr", 1.0 / red );

	psp = new ps_print (f_a, f);
	pp = (print **)&psp;
    }
    else if (f->flags & PDF ) {
      fprintf (stderr, "PDF!\n");
	if (f->font_names[1]) {	// words
	  f_a[1] = init_font_list(1, f->font_names[1], 1.2 );
	  f_a[1]->real_name = get_real_name(f->font_names[1], 0);
	}
	else
	  f_a[1] = init_font_list(1, "pncr", 1.0 );

	if (f->font_names[2]) {	// title
	  f_a[2] = init_font_list(2, f->font_names[2], f->font_sizes[2]/10. );
	  f_a[2]->real_name = get_real_name(f->font_names[2], 0);
	}
	else
	  f_a[2] = init_font_list(2, "pncr", f->font_sizes[2]/10.);

	if (f->font_names[3]) {	// italic title
	  f_a[3] = init_font_list(3, f->font_names[3], 1.2 );
	  f_a[3]->real_name = get_real_name(f->font_names[3], 0);
	}
	else
	  f_a[3] = init_font_list(3, "pncri", 1.2 );

	f_a[4] = init_font_list(4, "pncr", 2.4 );

	if (f->font_names[5]) {	// italic text
	  f_a[5] = init_font_list(5, f->font_names[5], 1.0 );
	  f_a[5]->real_name = get_real_name(f->font_names[5], 0);
	}
	else
	  f_a[5] = init_font_list(5, "pncri", 1.0 );
	f_a[6] = init_font_list(6, "pncr", 1.0 );
	f_a[7] = init_font_list(7, "pncr", 1.0 / red );

	pdfp = new pdf_print (f_a, f);
	pp = (print **)&pdfp;
    }
    else {			/* dvi */

	if (f->font_names[1])
	  f_a[1] = init_font_list(1, f->font_names[1], 1.0);
	else
	  f_a[1] = init_font_list(1, "cmr10", 1.0);

	if (f->font_names[2])
	  f_a[2] = init_font_list(2, f->font_names[2], 1.0);
	else
	  f_a[2] = init_font_list(2, "cmr12", 1.0);

	if (f->font_names[3])
	  f_a[3] = init_font_list(3, f->font_names[3], 1.0);
	else
	  f_a[3] = init_font_list(3, "cmr12", 1.0);

	f_a[4] = 0;

	if (f->font_names[5])
	  f_a[5] = init_font_list(5, f->font_names[5], 1.0);
	else f_a[5] = init_font_list(5, "cmti10", 1.0);

	f_a[6] = 0;
	f_a[7] = 0;

	pdp = new dvi_print(f_a, f);
	pp = (print **)&pdp;
    }
    // this seems to be where an old style midi file is started
    if (f->m_flags & SOUND) {
      if (!sp) {
	if (!f->midi_patch)  f->midi_patch = 34;
	if (!strncmp(f->out_file, "stdout", 6))
	  sp = new midi_snd(f->midi_patch, f->midi_volume, f->file, "stdout");
	else if (strlen(f->out_file))
	  sp = new midi_snd(f->midi_patch, f->midi_volume, f->file, f->out_file);
	else
	  sp = new midi_snd(f->midi_patch, f->midi_volume, f->file);
      }
      // wbc sept 2014
      //	else {			// no midi patch
      //	  if (!strncmp(f->out_file, "stdout", 6))
      //	    sp = new midi_snd(34, f->midi_volume , "stdout");
      //	  else
      //	    sp = new midi_snd;
      //	}
    }
    else if (f->m_flags & NMIDI) {
      np = new nmidi();
    }

    while (more == END_MORE) {
	(*pp)->page_head();
	more = (*pp)->do_page(b, f_a);
	(*pp)->page_trail();
    }
    if (sp) {
      delete sp;
      sp = 0;
    }
    if (np) {
      if (f->title)
	np->set_nmidi_title(f->title);
      if (f->midi_patch)
	np->set_patch(f->midi_patch);
      if (conv != 2) {
	np->set_pulse(conv);
      }
    }
    if (np) delete (np);
    if (psp) delete psp;
    if (pdp) delete pdp;
    if (pdfp) delete pdfp;
    if (f_a[0]) f_a[0] = delete_font_list(f_a[0]);
    if (f_a[1]) f_a[1] = delete_font_list(f_a[1]);
    if (f_a[2]) f_a[2] = delete_font_list(f_a[2]);
    if (f_a[3]) f_a[3] = delete_font_list(f_a[3]);
    if (f_a[4]) f_a[4] = delete_font_list(f_a[4]);
    if (f_a[5]) f_a[5] = delete_font_list(f_a[5]);
    if (f_a[6]) f_a[6] = delete_font_list(f_a[6]);
    if (f_a[7]) f_a[7] = delete_font_list(f_a[7]);

}

/*
 * read the input file into an intermediate buffer
 *
 */
void file_stuff(i_buf *i_b, struct file_info *f)
{
    file_in *ff;
#ifdef MAC
    ff = new file_in(/*f->file */);
#elif defined  WIN32
    ff = new file_in(f->file, "r" );
#else
    ff = new file_in(f->file, "rb" );
#endif /* MAC */

    get_tab_file(ff, i_b, f);

    delete (ff);
}

#ifdef __unix__			// this is for tab server timeouts
void  terminate(int val)
{
  dbg0(Error, "emergency exit, timer ran out\n");
  exit(3);
}
#endif /* unix */

#ifdef _WIN32
int main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif /* _WIN32 */
{
    char buf[BUFSIZ];
    i_buf b;
    struct file_info f;
    int i;

    for (i=0; i < FONT_NAMES; i++) f.font_names[i] = 0;

    init(&f);

    dbg_set(Warning);
    dbg_set(Error);

#ifdef __unix__			// this is for tab server timeouts
    signal(SIGALRM, terminate );
    (void) alarm(17000);
#endif /* unix */

    argc--;
    *argv++;
    args (argc, argv, &f);

    //  dbg_set(Proceedure);
    //	dbg_set(Widths);
    //	dbg_set(TFM);
    //	dbg_set(Stack);
    //	dbg_set(Flow);
    //	dbg_set(Inter);
    //  dbg_set(Fonts);

    if ( ! (f.m_flags & QUIET) ) {
      dbg2(Warning, "tab %s copyright 1995-2025 by Wayne Cripps%c",
	   (void *) VERSION,
	   (void *) NEWLINE );
#ifdef _WIN32
      dbg1(Warning, "Windows 32 bit%c", (void *) NEWLINE);
#endif
#ifdef _WIN64
      dbg1(Warning, "Windows 64 bit%c", (void *) NEWLINE);
#endif
    }
    file_stuff(&b, &f);
    b.Seek(0, rew);

    for (;;) {
      b.GetLine(buf, BUFSIZ);
      if ((signed char)buf[0] == EOF) break;
      dbg1(Inter, "main: %s", (void *)buf);
    }
    b.Seek(0, rew);

    tfm_stuff(&b, &f);

    if (f.scribe)
      free (f.scribe);
    free (f.file);
    free (f.out_file);

    if (f.m_flags & GS ) {
       char command[120];
       struct stat statbuf;
       int ret=0;

       strcpy ( command, "/opt/homebrew/bin/gs");
       ret = stat ( command, &statbuf );
       if ( ret == -1 ) {
          strcpy ( command, "/usr/bin/gs");
          ret = stat ( command, &statbuf );
          if ( ret == -1 ) {
             strcpy ( command, "/usr/local/bin/gs");
             ret = stat ( command, &statbuf );
          }
       }
       if ( ret == -1 ) { printf ("main: stat failed, can\'t find a copy of gs \n"); exit (-1);}

       strcat (command, " -q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sPAPERSIZE=letter -sOutputFile=out.pdf out.ps");
       system (command);
    }

    return(0);
}



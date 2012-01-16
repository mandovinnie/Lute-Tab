
#include "version.h"
#include "tab.h"
#include "i_buf.h"
#include "file_in.h"
#include "print.h"
#include "dviprint.h"
#include "ps_print.h"

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

extern jmp_buf b_env;


int baroque;
int n_system;
int bar_count;
int barCount;
int barCCount;
int pagenum;
double red;

void init(file_info *f)
{
    baroque = 0;
    n_system = 0;

    /*    int barCount = 0; */
    /*    int pagenum = 0; */
    bar_count = 0;
    red = 1.0;
    f->file = (char *)malloc( BL );
    f->file[0] = '\0';
    f->out_file = (char *)malloc( BL );
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
}

char *
get_real_name(const char *short_name, int dump);
				/* this bit loads in the tfm metrics */
void tfm_stuff(i_buf *b, file_info *f)
{
    print **pp;
    dvi_print *pdp = 0;
    ps_print *psp = 0;
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

//    if (f->m_flags && SOUND) 
//      ;

    while (more == END_MORE) {
	(*pp)->page_head(); 
	more = (*pp)->do_page(b, f_a);
	(*pp)->page_trail();
    }
    if (psp) delete psp;
    if (pdp) delete pdp;
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

#ifdef MAC
int my_main(int flags)
#else
main(int argc, char **argv)
#endif
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

#ifdef MAC
    if (flags & PS) {
      f.flags |= PS;
      f.flags &= ~DVI_O;
    }
#else
    argc--;
    *argv++;
    args (argc, argv, &f);
#endif
    
    //	dbg_set(Proceedure);
    //	dbg_set(Widths);
    //	dbg_set(TFM);
    //	dbg_set(Stack);
    //	dbg_set(Flow);
    
    if ( ! (f.m_flags & QUIET) )
      dbg2(Warning, "tab %s copyright 1997 by Wayne Cripps%c",
	   (void *)VERSION,
	   (void *) NEWLINE );
    
    file_stuff(&b, &f);
    b.Seek(0, rew);
    
    for (;;) {
      b.GetLine(buf, BUFSIZ);   
      if ((signed char)buf[0] == EOF) break;
      dbg1(Inter, "main: %s", (void *)buf);
    }
    b.Seek(0, rew);
    
    tfm_stuff(&b, &f);
    return(0);
}



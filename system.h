/*
 *
 * proceedure definitions
 * system means system of music, not operating system
 *
 */

/* proceedures */

#include "print.h"
#include "dviprint.h"

void args_from_string(char *buf, struct file_info *f);
int more();
signed char getisystem
   (i_buf *i_b, font_list *f_a[], int *l_p, struct file_info *f);
signed char getiline  
   (i_buf *i_b, font_list *f_a[], int *l_p, struct file_info *f);
int format_page
   (print *p, i_buf *i_b, font_list *f_a[], struct file_info *f);
void dotitle
   (print *p, i_buf *i_b, font_list *f_a[], struct file_info *f);
void printsystem
   (print *p,  i_buf *i_b, font_list *f_a[], int *l_p, struct file_info *f);
void do_system (print *p, struct file_info *f);
void mystrncpy(char *s1, const char *s2, int n);

void pass1 (font_list *f_a[], int *l_p, struct file_info *f, double *extra);
void pass2 (print *p, i_buf *i_b, font_list *f_a[], 
	    int *l_p, struct file_info *f, double *extra);

void dvi_format(print *p, i_buf *i_b, font_list *f_a[], int *l_p, int j,	struct file_info *f, struct list *l);

void do_system(print *p, struct file_info *f);
void format_pagenum(print *p, font_list *f_a[], char *num, struct file_info *f);


void
do_special(unsigned char c, unsigned char next, int font, 
	   print *p, i_buf *i_b, font_list *f_a[], struct file_info *f);

void
do_text(print *p, i_buf *i_b, font_list *f_a[], 
	struct list *l, struct text *text, 
	struct file_info *f, int in_sys, int staff);
void
do_notes(print *p, font_list *f_a[], struct notes *n, struct file_info *f);

void do_sp(unsigned char c, int font, print *p/* , struct file_info *f */);

void score(print *p, struct list *l,
      struct file_info *f, i_buf *i_b, font_list *f_a[]);

void
my_underline(print *p, font_list *f_a[],int *skip_spaces, int s1, int line);

void 
do_uline(print *p, int *skip_spaces, int s1, int s2, int line);

void 
do_r_uline(print *p, int *skip_spaces, int s1, int s2, int line);

void 
do_w_uline(print *p, int *skip_spaces, int s1, int s2, int line);

void
do_thick_uline(print *p, int *skip_spaces, int s1, int s2, int line);

double get_special_width(char **pp, class /* struct */ i_buf *i_b, 
			 font_list *f_a[], int font, struct file_info *f);
double print_special_char(char **pp, print *p, i_buf *i_b, 
			  font_list *f_a[], int font,  struct file_info *f, int italic );
struct font_list *add_font(char *name, double scale);
char * ps_map(char * name);

void do_ascii(struct list *l, struct file_info *f, i_buf *i_b);
void do_nmidi(struct list *l, struct file_info *f, i_buf *i_b);

extern double st_text;
extern double m_space;
extern char flag_to_staff[];
extern int st_italian;
extern char interspace[];
extern double staff_len;
extern char staff_height[];
extern double text_sp;
extern char mus_space[];
extern double o_staff_len;
extern int n_measures;	/* in dvi_f, reset measure number */


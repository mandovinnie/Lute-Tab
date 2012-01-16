#ifndef _PS_PRINT_
#define _PS_PRINT_		
#include "i_buf.h"
#include "tfm.h"
#include "dvi.h"
#include "print.h"
#include "ps.h"

int str_to_dvi(const char *string);
int inch_to_dvi(const double inch);
double str_to_inch(const char *string);

class ps_print : public print {
    char tag[35];
    int last_bop;
    int npages;
    int page_number;
    int cur_loc;
    void define_all_fonts();
    void define_font(int font_num, char *name);
    void ps_command(int com, int h_n, int v_n, int hh_n, int vv_n);
    struct font_list **f_a;
    struct file_info *f_i;
    double d_to_p(int n) {return ((double)(n) * 0.00000762);}
    void make_ps_font(i_buf *ps_header);
    void init_hv();
    int ps_top_of_page;
    char f_name[120];		// this should be in ps_print.h 
    double font_sizes[FONT_NAMES];				    
  public:
    char ps_used[256];

    void PutByte(const char c);    
    void PutLine(const char * l);

    ps_print(font_list *f[], file_info *ff);
    ~ps_print();
    void file_head();
    void page_head();
    void file_trail();
    void page_trail();
    int do_page(i_buf *b, font_list *f_l[]);

    void put_rule(char *w, char *h);
    void p_moveh(const int hor);
    void p_movev(const int ver);
    void p_moveto(const int hor,const int ver);
    void p_put_rule(int w, int h);
    void put_a_char (unsigned char c);	
    void set_a_char (unsigned char c);	
    void use_font(int fontnum);
    void use_named_font(int fontnum, char *name);
    void do_tie(double length);
    void do_tie_reversed(double length);
    void do_half_tie(double length);
    void do_half_tie_reversed(double length);
    void do_rtie(int bloc, int eloc);
    void ps_clipped(char c, int font);
    void push();
    void pop();
    void glp(int reg,int h[], int v[]);
    void put_slash
      (int bloc, int eloc, int count, struct file_info *f); 
    void put_uline(int bloc, int eloc);
    void put_thick_slant(int bloc, int eloc);
    void put_med_slant(int bloc, int eloc);
    void put_slant(int bloc, int eloc);
    int more();
    int is_used(char c) { return ps_used[(unsigned char)c]; }
    void showsave(int reg);	
    void p_num(int n); 
    void ps_draft();
    void ps_copyright();
    void vert_curve(int len);
    void flush() {ps_command ( OTHER, 0, 0, 0, 0); }
    int ps_top() { return (ps_top_of_page);}
    int get_page_number() {return (npages);}
    void comment(const char *string);
};
#endif /* _PS_PRINT_ */

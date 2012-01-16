#ifndef _DVI_PRINT_
#define _DVI_PRINT_		
#include "i_buf.h"
#include "tfm.h"
#include "dvi.h"
#include "print.h"


int str_to_dvi(const char *string);
int inch_to_dvi(const double inch);
double str_to_inch(const char *string);

class dvi_print : public print {
private:
    char tag[35];
    int last_bop;
    int page_number;
    int cur_loc;
    void define_all_fonts();
    void define_font(int font_num, char *name);
    void dvi_command(int com, int val1, int val2);
    struct font_list **f_a;
    struct file_info *f_i;
    char f_name[120];

  public:
    void PutByte(unsigned char c);
    void SignedPair(unsigned short c); 
    void SignedTrio(int c);
    void SignedQuad(int c);
    
    dvi_print(font_list *f[], file_info *ff);
    ~dvi_print();
    void file_head();
    void page_head();
    void file_trail();
    void page_trail();
    int do_page(i_buf *b, font_list *f_l[]);

    void put_rule(char *w, char *h);
    void p_moveh(const int hor);
    void p_movev(const int ver);
    void p_moveto(const int hor, const int ver) 
      {p_moveh(hor - get_h()); p_movev(ver - get_v());}
    void p_put_rule(int w, int h);
    void put_a_char (unsigned char c);	
    void set_a_char (unsigned char c);	
    void use_font(int fontnum);
    void do_tie(double length);
    void do_tie_reversed(double length);
    void do_half_tie(double length);
    void do_half_tie_reversed(double length);
    void do_rtie(int bloc, int eloc);
    void print_clipped(char c, int font);
    void put_slash
      (int bloc, int eloc, int count, struct file_info *f); 
    void put_uline(int bloc, int eloc);
    void put_r_uline(int bloc, int eloc) { put_uline(bloc, eloc); };
    void put_w_uline(int bloc, int eloc) { put_uline(bloc, eloc); };
    void put_thick_slant(int bloc, int eloc);
    void put_med_slant(int bloc, int eloc);
    void put_slant(int bloc, int eloc);
    int more();
    void showsave(int reg);	
    void p_num(int n); 
    void print_draft();
    void vert_curve(int len);
    void print_copyright();
    int get_page_number() { return (page_number); }
    void push()  { slp(sp, stack_h, stack_v); sp++; }
    void pop()   { sp--; glp(sp, stack_h, stack_v); }
    void comment(const char *string);
};
#endif /* _DVI_PRINT_ */


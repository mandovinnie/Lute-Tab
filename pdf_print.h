#ifndef _PDF_PRINT_
#define _PDF_PRINT_		
#include "i_buf.h"
#include "tfm.h"
#include "dvi.h"
#include "print.h"
#include "ps.h"

int str_to_dvi(const char *string);
int inch_to_dvi(const double inch);
double str_to_inch(const char *string);

class pdf_print : public print {
    char tag[35];
    int last_bop;
    int npages;
    int page_number;
    int cur_loc;
    void define_all_fonts();
    void define_font(int font_num, char *name);
    void pdf_command(int com, int h_n, int v_n, int hh_n, int vv_n);
    struct font_list **f_a;
    struct file_info *f_i;
    double d_to_p(int n) {return ((double)(n) * 0.00000762);}
    void make_pdf_font(i_buf *pdf_header);
    void init_hv();
    int pdf_top_of_page;
    char f_name[120];		// this should be in pdf_print.h 
    double font_sizes[FONT_NAMES];				    
    int nodump;

    unsigned int byte_count;
    unsigned int xref_offset;
    unsigned int generation;

    int page_retval;
    
  public:
    char pdf_used[256];

    void PutByte(const char c);    
    void PutLine(const char * l);

    pdf_print(font_list *f[], file_info *ff);
    ~pdf_print();
    void file_head();
    void file_xref();
    void file_trail();
    void page_head();
    void page_trail();
    int  do_page(i_buf *b, font_list *f_l[]);
    unsigned int  do_catalog();
    unsigned int  do_page_tree();
    unsigned int  do_page_leaf();
    unsigned int  do_page_content(i_buf *i_b,  struct font_list *f_a[]);
    unsigned int  do_page_resource();
    unsigned int  do_stream(i_buf *i_b,  struct font_list *f_a[], char * s_buf);
    void print_stream();
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
    void print_clipped(char c, int font);
    void push();
    void pop();
    void glp(int reg,int h[], int v[]);
    void put_slash
      (int bloc, int eloc, int count, struct file_info *f); 
    void put_uline(int bloc, int eloc);
    void put_r_uline(int bloc, int eloc) { put_uline(bloc, eloc); };
    void put_w_uline(int bloc, int eloc) { put_uline(bloc, eloc); };
    void put_thick_slant(int bloc, int eloc);
    void put_med_slant(int bloc, int eloc);
    void put_slant(int bloc, int eloc);
    int more();
    int is_used(char c) { return pdf_used[(unsigned char)c]; }
    void showsave(int reg);	
    void p_num(int n); 
    void print_draft();
    void print_copyright();
    void vert_curve(int len);
    void flush() {pdf_command ( OTHER, 0, 0, 0, 0); }
    int pdf_top() { return (pdf_top_of_page);}
    int get_page_number() {return (npages);}
    void comment(const char *string);
};

struct xref_entry {
  unsigned int byte_offset;
  unsigned int generation;
  char use;
  struct xref_entry *next;
};

static xref_entry *xref_root;
static unsigned int xref_count;

#endif /* _PDF_PRINT_ */

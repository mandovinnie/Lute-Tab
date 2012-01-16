/*
 * printer file definitions, primitaves
 *
 *
 */
#ifndef _PRINT_
#define _PRINT_		
#include "i_buf.h"
#include "tfm.h"
#include "dvi.h"


int str_to_dvi(const char *string);
int inch_to_dvi(const double inch);
double str_to_inch(const char *string);

enum pr_out_type  {Ps, Dvi, MusicTeX };
const int REGS = 45;

class print {
  protected:
    int dvi_h;
    int dvi_v;
    int h_diff; int v_diff;
    int save_h[REGS];
    int save_v[REGS];
    int stack_h[REGS];
    int stack_v[REGS];
    int sp;
    int curfont;
    i_buf *pr_out;
    enum highlight {On, Off} highlight;
    enum highlight_type {Paren, Gray} highlight_type;
  public:
    print();
    ~print();
    virtual void file_head() = 0;
    virtual void page_head() = 0;
    virtual void file_trail() = 0;
    virtual void page_trail() = 0;

    virtual int do_page(i_buf *b, font_list *f_l[]) = 0;
    void reset_dvi_vh()   { dvi_h=dvi_v=0;}
    virtual void p_moveh(const int hor) = 0;
    virtual void moveh(const double h)   { p_moveh (inch_to_dvi(h)); }
    virtual void moveh(const char  *h)   { p_moveh ( str_to_dvi(h)); }
    virtual void moveh(const int h)      { p_moveh (h); }
    virtual void move_n_h(const char *h) { p_moveh(- str_to_dvi(h)); }
    virtual void p_movev(const int ver) = 0;
    virtual void movev(const double v)   { p_movev (inch_to_dvi(v)); }
    virtual void movev(const char  *v)   { p_movev (str_to_dvi(v)); }
    virtual void movev(const int    v)   { p_movev (v); }
    virtual void move_n_v(const char *v) { p_movev(-1 * str_to_dvi(v)); }
    virtual void p_moveto(const int hor, const int ver) = 0;
    virtual void moveto(const double h, const double v)
      { p_moveto (inch_to_dvi(h), inch_to_dvi(v)); }
    virtual void moveto(char * h, char * v)
      { p_moveto (str_to_dvi(h), str_to_dvi(v)); }
    virtual void moveto(int h, int v)
      { p_moveto (h, v); }
    void put_rule(const char *ww, const char *h)
          { p_put_rule(str_to_dvi(ww), str_to_dvi(h)); }
    void put_rule(double ww, double h)
          { p_put_rule(inch_to_dvi(ww), inch_to_dvi(h)); }
    void put_rule(int ww, int h)
          { p_put_rule(ww, h); }
    virtual void p_put_rule(int w, int h) = 0;
    virtual void put_a_char (unsigned char c) = 0;	
    virtual void set_a_char (unsigned char c) = 0;	
    virtual void use_font   (int fontnum) = 0;
    int get_font()        { return (curfont); }
    virtual void do_tie(double length) = 0;
    virtual void do_tie_reversed(double length) = 0;
    virtual void do_half_tie(double length) = 0;
    virtual void do_half_tie_reversed(double length) = 0;
    virtual void do_rtie(int bloc, int eloc) = 0;
    virtual void ps_clipped(char c, int font) = 0;
    virtual void saveloc(int reg) { slp(reg, save_h, save_v);} 
    virtual void glp(int reg,int h[], int v[]);
    void slp(int reg, int h[],int v[]);
    void getloc(int reg)       { glp(reg, save_h, save_v); }	
    void get_current_loc(int *h, int *v) { *h = dvi_h; *v = dvi_v; }
    virtual void push()=0;//           { slp(sp, stack_h, stack_v); sp++; }
    virtual void pop()=0;//            { sp--; glp(sp, stack_h, stack_v); }
    void look()           { sp--; glp(sp, stack_h, stack_v); sp++; }
    void showloc()        {  dbg2(Warning, "tab: showloc: hor %d vert %d\n",
				    (void *)dvi_h, (void*)dvi_v); }
    int get_h(){return (dvi_h);}
    int get_v(){return (dvi_v);}
    virtual void put_slash
      (int bloc, int eloc, int count, struct file_info *f) = 0; 
    virtual void put_uline(int bloc, int eloc) = 0;
    virtual void put_thick_slant(int bloc, int eloc) = 0;
    virtual void put_med_slant(int bloc, int eloc) = 0;
    virtual void put_slant(int bloc, int eloc) = 0;
    virtual int  more() = 0;
    virtual void showsave(int reg) = 0;	
    virtual void p_num(int n) = 0; 
    virtual void ps_draft() = 0;
    virtual void vert_curve(int len)= 0;
    virtual void ps_copyright() = 0;
    virtual int ps_top() { return( 0);}
    virtual int get_page_number()= 0;
    virtual void comment(const char *string) = 0;
    void set_highlight() { 
      highlight = On;
    }
    void clear_highlight() { highlight = Off;}
    void gray_highlight() { highlight_type = Gray;}
    void paren_highlight() 
      { 
//	printf("print.h: paren_highlight here\n");
	highlight_type = Paren;
      }
};
#endif /* _PRINT_ */

//
//  input a set of font metrics
//
#ifndef __TFM_FONT__
#define __TFM_FONT__
#define MAXFONTS 8
#define MAXWIDTHS 256
#define MAX_W_VAL 64
#define NAME_SIZE 32 /* was 20 - is that 20 hex ? */
#define MAX_H_VAL 16

class tfm_font {
    short f_l;	/* length */
    short f_lh;	/* header length */
    int   f_bc;	/* beginning character */
    int   f_ec;	/* end character */
    short f_nw;	/* number of different width sizes */
    short f_nh;	/* number of different heights */
    short f_nd;	/* number of different depths */
    short f_ics;	/* number of different italics */
    short f_ligproglen;
    short f_n_kerns;
    short f_extproglen;
    short f_params;
    unsigned int f_type_len; /* length of type comment */
    char commnt[NAME_SIZE];  /* comment */
    unsigned int   f_name;   /* length of font name */
    char names[NAME_SIZE];   /* font name */
    int   f_check_sum;
    int   f_scaled_size;
    int   f_design_size;
    int   n_widths; /* number of characters in font */
    int   f_widths[MAXWIDTHS]; /* font width table */
    double f_width[MAX_W_VAL]; /* the widths as double  */
    int   f_i_width[MAX_W_VAL]; /* the widths as int */
    int   f_heights[MAXWIDTHS];
    double f_height[MAX_H_VAL];
    int   f_i_height[MAX_H_VAL];
    int   f_depths[MAXWIDTHS];
    double f_depth[MAX_H_VAL];
    int   f_italics[MAXWIDTHS]; /* italic correction indexes */
    int   f_tags[MAXWIDTHS];    /* tags table, whatever that is */
    int   f_remain[MAXWIDTHS];  /* remainder fields */
    int   f_ligkerns[MAXWIDTHS]; /* ligature - kerning index */
    int   f_kerns[MAXWIDTHS];	/* kerning index */
    double f_ps_red;		/* PostScript scaling factor */

    void tfm_input(double scale);
    void read_tfm_word(char *b0, char *b1, char *b2, char *b3);
    void read_tfm_short(char *b0, char *b1);

    int get_short();
    long get_long();
    int p_get_w(unsigned char c);
  public:
    tfm_font(const char *font_name, double scale);
    ~tfm_font();
    double get_width (unsigned char c);
    double get_height(unsigned char c);
    double get_depth (unsigned char c);
    int is_defined(unsigned char c) {return (f_width[f_widths[c]] == 0.0 ? 0 : 1); }
    int p_get_h(unsigned char c);
    int get_design_size() { return (f_design_size); }
    int get_check_sum()   { return (f_check_sum); }
};
#endif /* __TFM_FONT__ */

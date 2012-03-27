/*
 *
 * dvi operation macros
 *
 */
double dvi_to_pt(int dvi);
double dvi_to_inch(int dvi);
double dvi_to_mm(int dvi);
int inch_to_dvi(const double inch);
int pt_to_dvi(double pt);
int mm_to_dvi(double mm);
int str_to_dvi(const char *string);

#define NUM 12700000 
#define DENOM 473628672

#define SET_CHAR_0 0
#define SET1 128
#define SET_RULE 132
#define PUT1 133
#define PUT_RULE 137
#define NO_OP 138
#define BOP 139			/* 0213 */
#define EOP 140			/* 0214 */
#define PUSH 141
#define POP 142
#define RIGHT1 143
#define RIGHT2 144
#define RIGHT3 145
#define RIGHT4 146
#define W0 147
#define W1 148
#define X0 152
#define X1 153
#define DOWN1 157
#define DOWN2 158
#define DOWN3 159
#define DOWN4 160
#define Y0 161
#define Y1 162
#define Z0 166
#define Z1 167
#define FNT_NUM_0 171		/* 0253 */
#define FNT_NUM_1 172
#define FNT_NUM_2 173
#define FNT_NUM_3 174
#define FNT_NUM_4 175
#define FNT_NUM_5 176
#define FNT1 235		/* 0353 */
#define FNT_DEF1 243		/* 0363 */
#define PRE 247
#define POST 248
#define POST_POST 249



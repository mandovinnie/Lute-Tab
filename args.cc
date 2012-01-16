/*
   This program is copyright 1991 by Wayne Cripps,
   P.O. Box 677 Hanover N.H. 03755.
   All rights reserved.  It is supplied "as is" 
   without express or implied warranty.

   Permission is granted to use, copy, modify and distribute
   this software without fee, provided that this notice appears
   in all copies, and that a copy of this notice is provided to
   anyone who recieves a binary copy without sources.

   This software may not be used for commercial purposes
   without explicit, prior written permission.

   Please mail bug reports, suggestions, and improvements
   to wbc@sunapee.dartmouth.edu.

 */

#include "tab.h"
#include <setjmp.h>
#include "system.h"
#include "tree.h"

#define N_ARGS 12

/* EXTERN */

void args(int argc, char ** argv, struct file_info *f);
void set_ps_size(int num, int size);
void set_ps_font(int num, char * name);
void ps_text_tfm(struct file_info *f, int fontnum);
extern char flag_to_staff[];
extern jmp_buf m_env;
extern int title_font, text_font;
char *get_real_name(const char *short_name, int dump);
int setflag(file_info *f, char * string, pass pass);

void set_font_flag(const char *value, struct file_info *f) 
{
  // vals 1 2 3 4 
  // this used to swap between italic and text
}
void set_C(const char *value, struct file_info *f) {  bar_count++; }
void set_c(const char *value, struct file_info *f) {  barCount++; }
void set_CC(const char *value, struct file_info *f) {  
  barCCount++; 
}
void set_b(const char *value, struct file_info *f) {  baroque++; }
void set_d(const char *value, struct file_info *f) {  f->flags |= DEBUG; }
void set_B(const char *value, struct file_info *f) { 
  f->line_flag = BETWEEN_LINE;
  f->flag_flag = BOARD_FLAGS;	
  f->char_flag = BOARD_CHAR;
  f->num_flag  = STAND_NUM;
  f->note_conv = 0;
  f->note_flag = ITAL_NOTES;}
void set_D(const char *value, struct file_info *f) { 
  f->line_flag = BETWEEN_LINE;
  f->flag_flag = THIN_FLAGS;	
  f->char_flag = STAND_CHAR;
  f->num_flag  = STAND_NUM;
  f->note_conv = 0;
  f->note_flag = ITAL_NOTES;}
void set_e(const char *value, struct file_info *f) {  f->flags |= ROTATE; }
void set_E(const char *value, struct file_info *f) {  f->flags |= DRAFT; }
void set_ff(const char *value, struct file_info *f) { 
  f->flag_flag = STAND_FLAGS; }
void set_fc(const char *value, struct file_info *f) { 
  f->char_flag = STAND_CHAR;}
void set_f(const char *value, struct file_info *f) { 
  f->line_flag = BETWEEN_LINE;
  f->flag_flag = STAND_FLAGS;
  f->char_flag = STAND_CHAR;
  f->num_flag  = STAND_NUM;
  f->note_flag = ITAL_NOTES;}
void set_F(const char *value, struct file_info *f) { 
  f->line_flag = BETWEEN_LINE;	
  f->flag_flag = CONTEMP_FLAGS;
  f->char_flag = STAND_CHAR;
  f->num_flag  = STAND_NUM;
  f->note_flag = ITAL_NOTES;}
void set_G(const char *value, struct file_info *f) { f->flags |= COPYRIGHT;}
void set_h(const char *value, struct file_info *f) { 
  if ( f->flags & CONVERT ) f->flags &= ~CONVERT;
  else f->flags |= CONVERT;
  f->num_flag = ITAL_NUM;
  f->note_flag = ITAL_NOTES;
  f->flag_flag = S_ITAL_FLAGS;}
void set_H(const char *value, struct file_info *f) { 
  if ( f->flags & CON_SEV) f->flags &= ~CON_SEV;
  else f->flags |= CON_SEV;
  set_h(value, f);}
void set_i(const char *value, struct file_info *f) {   
  if (f->flags & MANUSCRIPT) { 
    f->line_flag = BETWEEN_LINE;
  }
  else if ( f->flags & CONVERT) { 
    if (f->line_flag == ON_LINE )
      f->line_flag = BETWEEN_LINE;
    else 
      f->line_flag = ON_LINE;
  }
  else 
    f->line_flag = ON_LINE;
  if (!baroque) 
      f->flag_flag = S_ITAL_FLAGS;
  f->char_flag = STAND_CHAR;
  f->num_flag = ITAL_NUM;
  f->note_flag = ITAL_NOTES;}
void set_I(const char *value, struct file_info *f) {
  f->line_flag = BETWEEN_LINE;
  f->flag_flag = S_ITAL_FLAGS;
  f->char_flag = STAND_CHAR;
  f->num_flag = ITAL_NUM;
  f->note_flag = ITAL_NOTES;}
void set_K(const char *value, struct file_info *f) {f->m_flags |= NO_AUTO_END;}
void set_x(const char *value, struct file_info *f) {
  f->line_flag = ON_LINE;
  f->flag_flag = THIN_FLAGS;
  f->char_flag = STAND_CHAR;
  f->num_flag = ITAL_NUM;
  f->note_flag = ITAL_NOTES;}
void set_O(const char *value, struct file_info *f) {
  f->line_flag = ON_LINE;
  f->flag_flag = CAP_FLAGS;
  f->char_flag = STAND_CHAR;
  f->num_flag = ITAL_NUM;
  f->note_flag = ITAL_NOTES;}
void set_s(const char *value, struct file_info *f) { f->flags |= CON_SEV;}
void set_T(const char *value, struct file_info *f) {
  f->line_flag = BETWEEN_LINE;
  f->flag_flag = THIN_FLAGS;
  f->char_flag = STAND_CHAR;
  f->num_flag = STAND_NUM;
  f->note_flag = ITAL_NOTES;}
void set_w(const char *value, struct file_info *f) {
  f->line_flag = ON_LINE;
  f->flag_flag = S_ITAL_FLAGS; 
  f->char_flag = STAND_CHAR; 
  f->num_flag = ITAL_NUM; 
  f->note_flag = ITAL_NOTES; 
  f->flags |= WALLACE; 
  f->flags |= NO_MUSIC_B; 
  f->flags |= BIGNOTES; 
  f->flags |= ROTATE;
  f->flags |= SHARP_UP;
  f->note_conv = 0;}
void set_Z(const char *value, struct file_info *f) {
  f->line_flag = ON_LINE;
  f->flag_flag = S_ITAL_FLAGS; 
  f->char_flag = STAND_CHAR;
  f->note_flag = ITAL_NOTES;
  f->num_flag = ITAL_NUM; 
  f->flags |= NO_MUSIC_B; 
  f->flags |= BIGNOTES; 
  f->note_conv = 0;}
void set_j(const char *value, struct file_info *f) { f->flags |= CONV_COR;}
void set_m(const char *value, struct file_info *f) { f->flags |= MANUSCRIPT;}
void set_M(const char *value, struct file_info *f) { f->flags |= MARKS;}
void set_n(const char *value, struct file_info *f) { 
  f->char_flag = MACE_CHAR; }
void set_N(const char *value, struct file_info *f) { 
  f->char_flag = ROB_CHAR;}
void set_p(const char *value, struct file_info *f) { f->flags |= PAGENUM;}
void set_P(const char *value, struct file_info *f) {
  f->flags |= PS;
  f->flags &= ~DVI_O;}
void set_2(const char *value, struct file_info *f) {
  f->m_flags |= DPI1200;
  red = 1.0;}
void set_6(const char *value, struct file_info *f) {
  f->flags |= DPI600;
}
void set_7(const char *value, struct file_info *f) {
  f->m_flags |= SEVEN;
}
void set_Q(const char *value, struct file_info *f) {f->flags |= BIGNOTES;}
void set_q(const char *value, struct file_info *f) {f->m_flags |= QUIET;}
void set_r(const char *value, struct file_info *f) {baroque=0;}
void set_S(const char *value, struct file_info *f) {f->flags |= SAMELINE;}
void set_t(const char *value, struct file_info *f) {f->line_flag |= ON_LINE;}
void set_v(const char *value, struct file_info *f) {
  f->flags |= VERBOSE;
  dbg_set(Inter);}
void set_V(const char *value, struct file_info *f) {}
void set_W(const char *value, struct file_info *f) {
  f->line_flag = BETWEEN_LINE;
  f->flag_flag = S_ITAL_FLAGS; 
  f->char_flag = STAND_CHAR; 
  f->num_flag = ITAL_NUM; 
  f->note_flag = ITAL_NOTES; 
  f->flags |= WALLACE; 
  f->flags |= NO_MUSIC_B; 
  f->flags |= BIGNOTES; 
  f->flags |= ROTATE;
  f->flags |= SHARP_UP;
  f->flags |= CONVERT;
  f->note_conv = 0;}
void set_X(const char *value, struct file_info *f) { f->flags |= NO_EXPAND;}
void set_Y(const char *value, struct file_info *f) { f->flags |= NO_WORD;}
void set_y(const char *value, struct file_info *f) {}
void set_z(const char *value, struct file_info *f) {}
void set_4(const char *value, struct file_info *f) { f->flags |= FOUR;}
void set_5(const char *value, struct file_info *f) { f->flags |= FIVE;}
void set_0(const char *value, struct file_info *f) { 
  f->flags &= ~FOUR;
  f->flags &= ~FIVE;}
void set_l(const char *value, struct file_info *f) {
  if ( ! strncmp ("reset", value, 5))
    staff_len = o_staff_len;
  else {
    o_staff_len = staff_len;
    staff_len = str_to_inch(value);
  }
}
void set_R95(const char *value, struct file_info *f) { red = 0.94440;}
void set_R9(const char *value, struct file_info *f)  { red = 0.88880;}
void set_R8(const char *value, struct file_info *f)  { red = 0.777770;}
void set_R7(const char *value, struct file_info *f)  { red = 0.6666666;}
void set_listfonts(const char *value, struct file_info *f) {
  (void)get_real_name("pncr", 1);
}

void set_o(const char *value, struct file_info *f)  {
#ifndef MAC
  char * ptr;
  strcpy (f->out_file, value);
  if (ptr = strstr (f->out_file, ".tab")) {
    *ptr = '\0';
  }
  if (ptr = strstr (f->out_file, ".ps")) {
    *ptr = '\0';
  }
#ifndef NO_STDOUT
  if (f->flags & PS ) { 
    if ( strcmp (f->out_file, "stdout")) {
      strcat (f->out_file, ".ps");
    }
  }
#endif /* NO_STDOUT */
  else strcat (f->out_file, ".dvi");
  if ( ! (f->m_flags & QUIET)) {
      dbg1 (Warning, "tab: sending output to %s\n", (void *)f->out_file);
  }
#endif /* MAC */
}

void set_tuning(const char *value, struct file_info *f)  { 
  extern char *arg_str;
  //  dbg1(Warning, "in set_tuning %s\n", (void *) value);
  arg_str=(char *)malloc(strlen(value)+1);
  strcpy(arg_str, value);
}

void set_sharp_up(const char *value, struct file_info *f)
{
   f->flags |= SHARP_UP;
}

void set_sharp_normal(const char *value, struct file_info *f)
{
   f->flags &= ~SHARP_UP;
}

void set_highlight_paren(const char *value, struct file_info *f)
{
	f->m_flags |= PAREN;
}

void set_sound(const char *value, struct file_info *f)
{
	f->m_flags |= SOUND;
	f->flags |= MANUSCRIPT;
#ifndef NO_STDOUT
	strcat (f->out_file, "/dev/null");
#endif
}

void set_alttitle(const char *value, struct file_info *f)
{
  //        printf("args: setting alt title\n");
	f->m_flags |= ALTTITLE;
}
void set_alttitle_off(const char *value, struct file_info *f)
{
  //        printf("args: turning off alt title\n");
	f->m_flags &= ~ALTTITLE;
}
void set_a4(const char *value, struct file_info *f)
{
	f->m_flags |= A4;
}
void set_nobox(const char *value, struct file_info *f)
{
	f->m_flags |= NOBOX;
}
void set_sItalNotes(const char *value, struct file_info *f)
{
        f->flag_flag = S_ITAL_FLAGS;   
}
void set_italFlags(const char *value, struct file_info *f)
{
        f->note_flag = ITAL_NOTES; 
}

void set_twostaff(const char *value, struct file_info *f)
{
        f->m_flags |= TWOSTAFF; 
}

void set_longbar(const char *value, struct file_info *f)
{
        f->m_flags |= LONGBAR; 
}
void set_comp_staff(const char *value, struct file_info *f)
{
        strcpy(interspace, "9.0 pt");
	m_space = 0.068;
}
void set_autoKey(const char *value, struct file_info *f)
{
        f->m_flags |= AUTOKEY;
}
void set_allDsup(const char *value, struct file_info *f)
{
        f->m_flags |= DSUP;
	f->m_flags &= ~DSDOWN;
}
void set_allDsdown(const char *value, struct file_info *f)
{
        f->m_flags |= DSDOWN;
	f->m_flags &= ~DSUP;
}
void set_ModNotes(const char *value, struct file_info *f)
{
        f->note_flag |= MOD_NOTES;
}
void set_EPSF(const char *value, struct file_info *f)
{
        f->m_flags |= EPSF;
}

void args(int argc, char ** argv, struct file_info *f)
{
    char *aa=0;
    //    void (*r) (const char *, struct file_info *f);
    void (*r)(const char *, struct file_info *);

    struct tuple onearg={"af", (void*)set_font_flag};

    struct tuple arglist[] = {
      {"af", (void*)set_font_flag},
      {"C", (void*)set_C},
      {"c", (void*)set_c},
      {"CC", (void*)set_CC},
      {"b", (void*)set_b},
      {"B", (void*)set_B},
      {"D", (void*)set_D},
      {"e", (void*)set_e},
      {"E", (void*)set_E},
      {"f", (void*)set_f},
      {"fc", (void*)set_fc},
      {"ff", (void*)set_ff},
      {"F", (void*)set_F},
      {"G", (void*)set_G},
      {"H", (void*)set_H},
      {"h", (void*)set_h},
      {"i", (void*)set_i},
      {"I", (void*)set_I},
      {"K", (void*)set_K},
      {"x", (void*)set_x},
      {"O", (void*)set_O},
      {"s", (void*)set_s},
      {"T", (void*)set_T},
      {"w", (void*)set_w},
      {"Z", (void*)set_Z},
      {"j", (void*)set_j},
      {"l", (void*)set_l},
      {"m", (void*)set_m},
      {"M", (void*)set_M},     //not used
      {"0", (void*)set_0},
      {"2", (void*)set_2},
      {"4", (void*)set_4},
      {"5", (void*)set_5},
      {"6", (void*)set_6},
      {"7", (void*)set_7},
      {"N", (void*)set_N},
      {"P", (void*)set_P},
      {"Q", (void*)set_Q},
      {"R7", (void*)set_R7},
      {"R8", (void*)set_R8},
      {"R9", (void*)set_R9},
      {"R95", (void*)set_R95},
      {"S", (void*)set_S},
      {"V", (void*)set_V},
      {"W", (void*)set_W},
      {"X", (void*)set_X},
      {"Y", (void*)set_Y},
      {"listfonts", (void*)set_listfonts},
      {"n", (void*)set_n},
      {"o", (void*)set_o},
      {"p", (void*)set_p},
      {"q", (void*)set_q},
      {"r", (void*)set_r},
      {"t", (void*)set_t},
      {"v", (void*)set_v},
      {"y", (void*)set_y},
      {"z", (void*)set_z},
      {"tuning", (void*)set_tuning},
      {"sharpUp",    (void*)set_sharp_up},
      {"sharpNorm",    (void*)set_sharp_normal},
      {"highlightparen", (void*)set_highlight_paren},
      {"sound", (void*)set_sound},
      {"midi", (void*)set_sound},
      {"alttitle", (void*)set_alttitle},
      {"alttitle-off", (void*)set_alttitle_off},
      {"a4", (void*)set_a4},
      {"nobox", (void*)set_nobox},
      {"sItalNotes", (void*)set_sItalNotes},
      {"italFlags", (void*)set_italFlags},
      {"twostaff", (void*)set_twostaff},
      {"longbar", (void*)set_longbar},
      {"compressStaff", (void*)set_comp_staff},
      {"autoKey", (void*)set_autoKey},
      {"allDsup", (void*)set_allDsup},
      {"allDsdown", (void*)set_allDsdown},
      {"modernNotes", (void*)set_ModNotes},
      {"EPSF", (void*)set_EPSF},
      {0, 0}
    };

    static tree at(arglist);

/* check for blank after flag */

    while (argc > 0) {

	if (**argv == '-') {
	    (*argv)++;
	    //r = (void *)at.get(*argv);
	    //	    r = (void (*)(const char *, file_info *))at.get(*argv);
	    r = (void(*)(const char*, file_info*))at.get(*argv);
	    
	    if (r) {
	      aa = argv[1];
	           (*r)(aa, f);
	      //   setit(r, aa, f);
	    }
	    else {
	      aa = argv[0];
	      dbg1(Warning, "tab: args: unknown flag %c\n", (void*)aa);
	    }

	    switch (argv[0][0]){
	    case 't':
	      if ( argv[0][1] != 'u') break;
	    case 'a':
	      if ( argv[0][1] != 'f') break;
	    case 'l':
	    case 'o':
	      *argv++;
	      argc--;
	      break;
	    default:
	      break;
	    }

	    *argv++;
	    argc--;
	}
	else if (**argv == '$') {
	  (*argv)++;
	  (void)setflag(f, *argv, first);
	  //	  dbg1(Warning, "tab: $ parameters not allowed on command line: %s\n", 
	  //       *argv);
	  *argv++;
	  argc--;	}
	/* assume what is left is filename */
	else {			/* not - */
	    strcpy (f->file, *argv);
#ifdef WIN32
	    if (!strstr(f->file, ".")) {
#else
	    if (!strstr(f->file, ".tab")) {
#endif /* WIN32 */
		strcat (f->file, ".tab");
	    }
	    if ( ! (f->m_flags & QUIET))
	      dbg1 (Warning, "setting filename to %s\n", f->file);
	    *argv++;
	    argc--;
	}
    } 
}

#define ARG_LEN 120

void args_from_string(char *buf, struct file_info *f)
{
    int argc=0, i, j;
    char argv[ARG_LEN];
    char *argp[N_ARGS];
    int quote=0;

    memset(argv, 0, sizeof(argv));

    for (i=0,j=0; i<sizeof (argv) && buf[i] != (char)'\0'; /* i++, */ j++) {

	while (buf[i] == ' ') 
	  i++;	/*  strip leading spaces */
	if (buf[i] == '\0') break;
	if (buf[i] == NEWLINE) break;

	argp[argc] = &argv[j]; /* start a new arg */
	argc++;
	while (1) {
	    argv[j] = buf[i];

	    if (argv[j] == '"' || argv[j] == '\'') {
		if (quote) {	/* end quote */
		    quote = 0;
		    argv[j] = '\0';
		    i++;
		}
		else {
		    i++;
		    argv[j] = buf[i];
		    quote++;
		}
	    }
	    j++;
	    i++;
	    if (buf[i] == '\0') break;
	    if (buf[i] == NEWLINE) break;
	    if (!quote && buf[i] == ' ') {
		argv[j] = '\0';
		break;
	    }
	}
    }
    argv[i]='\0';

    memset(&argv[i], 0, ARG_LEN - i);
    for (j=argc; j < N_ARGS; j++ ) {
	argp[j]=0;
    }
    args(argc, argp, f);

}


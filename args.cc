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
#include "system.h"
#include "tree.h"

#define N_ARGS 12

/* EXTERN */

void args(int argc, char ** argv, struct file_info *f);
void set_ps_size(int num, int size);
void set_ps_font(int num, char * name);
void ps_text_tfm(struct file_info *f, int fontnum);
extern char flag_to_staff[];
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
void set_b(const char *value, struct file_info *f) {  
  if (thin_renaissance)
    dbg0(Error, "Args.cc: You can't use both -thin and -b at the same time \n");
  baroque++; 
}
void set_d(const char *value, struct file_info *f) {
  if (! strncmp(value, "File", 4)) {
    dbg0(Warning, "Setting debug to File\n");
    dbg_set(File);
  }
  if (! strncmp(value, "Path", 4)) {
    dbg0(Warning, "Setting debug to Path\n");
    dbg_set(Path);
  }
  else if (! strncmp(value, "TFM", 3)) {
    dbg0(Warning, "Setting debug to TFM\n");
    dbg_set(TFM);
  }
  else if (! strncmp(value, "Inter", 4)) {
    dbg0(Warning, "Setting debug to Inter\n");
    dbg_set(Inter);
  }
  else if (! strncmp(value, "Flow", 4)) {
    dbg0(Warning, "Setting debug to Flow\n");
    dbg_set(Flow);
  }
  else if (! strncmp(value, "Warning", 4)) {
    dbg0(Warning, "Setting debug to Warning\n");
    dbg_set(Warning);
  }
  else if (! strncmp(value, "Error", 4)) {
    dbg0(Warning, "Setting debug to Error\n");
    dbg_set(Error);
  }
  else if (! strncmp(value, "Stack", 4)) {
    dbg0(Warning, "Setting debug to Stack\n");
    dbg_set(Stack);
  }
  else if (! strncmp(value, "Proceedure", 4)) {
    dbg0(Warning, "Setting debug to Proceedure\n");
    dbg_set(Proceedure);
  }
  else if (! strncmp(value, "Fonts", 4)) {
    dbg0(Warning, "Setting debug to Font\n");
    dbg_set(Fonts);
  }
  else if (! strncmp(value, "Bug", 3)) {
    dbg0(Warning, "Setting debug to Bug\n");
    dbg_set(Bug);
  }
  else if (! strncmp(value, "Widths", 4)) {
    dbg0(Warning, "Setting debug to Widths\n");
    dbg_set(Widths);
  }
  f->flags |= DEBUG; 
}
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
void set_G(const char *value, struct file_info *f) 
{ f->flags |= COPYRIGHT;}
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
  // f->line_flag = BETWEEN_LINE;
  f->flag_flag = THIN_FLAGS;
//  f->char_flag = STAND_CHAR;
//  f->num_flag = STAND_NUM;
//  f->note_flag = ITAL_NOTES;}
}
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
  f->flags &= ~PDF;
  f->flags &= ~DVI_O;}
void set_dvi(const char *value, struct file_info *f) {
  f->flags |= DVI_O;
  f->flags &= ~PDF;
  f->flags &= ~PS;}
void set_pdf(const char *value, struct file_info *f) {
  f->flags &= ~DVI_O;
  f->flags &= ~PS;
  f->flags |= PDF;}
void set_2(const char *value, struct file_info *f) {
  f->m_flags |= DPI1200;
  red = 1.0;}
void set_6(const char *value, struct file_info *f) {
  f->flags |= DPI600;
}
void set_300(const char *value, struct file_info *f) {
  f->flags &= ~DPI600;
}
void set_24(const char *value, struct file_info *f) {
  f->m_flags |= DPI2400;
  red = 1.0;}
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
void set_wide(const char *value, struct file_info *f) {
  f->m_flags |= AWIDE;
}
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
void set_4(const char *value, struct file_info *f) { 
    f->flags |= FOUR;}
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
  if (!value) {
    dbg0 (Warning, "-o with no output file specified, using default\n");
    return;
  }
  strcpy (f->out_file, value);
  if (ptr = strstr (f->out_file, ".tab")) {
    *ptr = '\0';
  }
  if (ptr = strstr (f->out_file, ".ps")) {
    *ptr = '\0';
  }
  if (ptr = strstr (f->out_file, ".mid")) {
    *ptr = '\0';
  }
#ifndef NO_STDOUT
  if (f->m_flags & SOUND ) {
    strcat (f->out_file, ".mid");
  }
  else if (f->flags & PS ) { 
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
  // dbg1(Warning, "in set_tuning %s\n", (void *) value);
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

void set_highlight_red(const char *value, struct file_info *f)
{
	f->m_flags |= RED;
}

void set_sound(const char *value, struct file_info *f)
{
	f->m_flags |= SOUND;
	f->flags |= MANUSCRIPT;
	f->m_flags |= QUIET;
#ifndef NO_STDOUT
	strcat (f->out_file, "stdout");	//  was /dev/null
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

void set_ascii(const char *value, struct file_info *f)
{
        f->m_flags |= ASCII;
}
void set_fontpath(const char *value, struct file_info *f)
{
  extern char *font_path;
  //  dbg1(Warning, "in set_fontpath %s\n", (void *) value);
  font_path=(char *)malloc(strlen(value)+1);
  strcpy(font_path, value);
}
void set_milan(const char *value, struct file_info *f)
{
        f->m_flags |= MILAN;
}
void set_nmidi(const char *value, struct file_info *f)
{
        f->m_flags |= NMIDI;
	f->m_flags |= QUIET;
}
void set_amidi_patch(const char *value, struct file_info *f)
{
        f->midi_patch = atoi(value);
}
void set_guitar(const char *value, struct file_info *f)
{
        f->m_flags |= GUIT;
	f->flags |= MANUSCRIPT;
}

void set_line_thickness(const char *value, struct file_info *f)
{
  strncpy(staff_height, value, 20);
}

void set_thin_font(const char *value, struct file_info *f)
{
  if (baroque)
    dbg0(Error, "Args.cc: You can't use both -thin and -b at the same time \n");
  thin_renaissance=1;     
}
void set_tfmdump(const char *value, struct file_info *f)
{
	dbg_set(TFM);
}
void set_twelvedots(const char *value, struct file_info *f)
{
  f->m_flags |= TWELVEDOTS;
}

void args(int argc, char ** argv, struct file_info *f)
{
    char *aa=0;
    //    void (*r) (const char *, struct file_info *f);
    void (*r)(const char *, struct file_info *);

    struct tuple arglist[] = {
      {(char*)"af", (void*)set_font_flag},
      {(char*)"C", (void*)set_C},
      {(char*)"c", (void*)set_c},
      {(char*)"CC", (void*)set_CC},
      {(char*)"b", (void*)set_b},
      {(char*)"B", (void*)set_B},
      {(char*)"D", (void*)set_D},
      {(char*)"debug", (void*)set_d},
      {(char*)"e", (void*)set_e},
      {(char*)"E", (void*)set_E},
      {(char*)"f", (void*)set_f},
      {(char*)"fc", (void*)set_fc},
      {(char*)"ff", (void*)set_ff},
      {(char*)"F", (void*)set_F},
      {(char*)"G", (void*)set_G},
      {(char*)"H", (void*)set_H},
      {(char*)"h", (void*)set_h},
      {(char*)"i", (void*)set_i},
      {(char*)"I", (void*)set_I},
      {(char*)"K", (void*)set_K},
      {(char*)"x", (void*)set_x},
      {(char*)"O", (void*)set_O},
      {(char*)"s", (void*)set_s},
      {(char*)"T", (void*)set_T},
      {(char*)"w", (void*)set_w},
      {(char*)"Z", (void*)set_Z},
      {(char*)"j", (void*)set_j},
      {(char*)"l", (void*)set_l},
      {(char*)"m", (void*)set_m},
      {(char*)"M", (void*)set_M},     //not used
      {(char*)"0", (void*)set_0},
      {(char*)"2", (void*)set_2},
      {(char*)"1200", (void*)set_2},
      {(char*)"2400", (void*)set_24},
      {(char*)"300", (void*)set_300},
      {(char*)"4", (void*)set_4},
      {(char*)"5", (void*)set_5},
      {(char*)"6", (void*)set_6},
      {(char*)"600", (void*)set_6},
      {(char*)"7", (void*)set_7},
      {(char*)"N", (void*)set_N},
      {(char*)"P", (void*)set_P},
      {(char*)"dvi", (void*)set_dvi},
      {(char*)"pdf", (void*)set_pdf},
      {(char*)"Q", (void*)set_Q},
      {(char*)"R7", (void*)set_R7},
      {(char*)"R8", (void*)set_R8},
      {(char*)"R8.5", (void*)set_R8},
      {(char*)"R9", (void*)set_R9},
      {(char*)"R95", (void*)set_R95},
      {(char*)"R9.5", (void*)set_R95},
      {(char*)"S", (void*)set_S},
      {(char*)"V", (void*)set_V},
      {(char*)"W", (void*)set_W},
      {(char*)"X", (void*)set_X},
      {(char*)"Y", (void*)set_Y},
      {(char*)"listfonts", (void*)set_listfonts},
      {(char*)"n", (void*)set_n},
      {(char*)"o", (void*)set_o},
      {(char*)"p", (void*)set_p},
      {(char*)"q", (void*)set_q},
      {(char*)"r", (void*)set_r},
      {(char*)"t", (void*)set_t},
      {(char*)"v", (void*)set_v},
      {(char*)"y", (void*)set_y},
      {(char*)"z", (void*)set_z},
      {(char*)"tuning", (void*)set_tuning},
      {(char*)"sharpUp",    (void*)set_sharp_up},
      {(char*)"sharpNorm",    (void*)set_sharp_normal},
      {(char*)"highlightparen", (void*)set_highlight_paren},
      {(char*)"highlightred", (void*)set_highlight_red},
      {(char*)"sound", (void*)set_sound},
      {(char*)"midi", (void*)set_sound},
      {(char*)"alttitle", (void*)set_alttitle},
      {(char*)"alttitle-off", (void*)set_alttitle_off},
      {(char*)"a4", (void*)set_a4},
      {(char*)"nobox", (void*)set_nobox},
      {(char*)"sItalNotes", (void*)set_sItalNotes},
      {(char*)"italFlags", (void*)set_italFlags},
      {(char*)"twostaff", (void*)set_twostaff},
      {(char*)"longbar", (void*)set_longbar},
      {(char*)"compressStaff", (void*)set_comp_staff},
      {(char*)"autoKey", (void*)set_autoKey},
      {(char*)"allDsup", (void*)set_allDsup},
      {(char*)"allDsdown", (void*)set_allDsdown},
      {(char*)"modernNotes", (void*)set_ModNotes},
      {(char*)"EPSF", (void*)set_EPSF},
      {(char*)"ascii", (void*)set_ascii},
      {(char*)"wide", (void*)set_wide},
      {(char*)"fontpath", (void*)set_fontpath},
      {(char*)"milan", (void*)set_milan},
      {(char*)"nmidi", (void*)set_nmidi},
      {(char*)"midi-patch", (void*)set_amidi_patch},
      {(char*)"guitar", (void*)set_guitar},
      {(char*)"staff-line-thickness", (void*)set_line_thickness},
      {(char*)"thin", (void*)set_thin_font},
      {(char*)"tfmdump", (void*)set_tfmdump},
      {(char*)"twelvedots", (void*) set_twelvedots},
      {(char *)0, (void*)0}
    };

    static tree at(arglist);

/* check for blank after flag */

    while (argc > 0) {

	if (**argv == '-') {
	    (*argv)++;
	    r = (void(*)(const char*, file_info*))at.get(*argv);
	    
	    if (r) {
	      aa = argv[1];
	           (*r)(aa, f);
	      //   setit(r, aa, f);
	    }
	    else {
	      aa = argv[0];
	      dbg1(Warning, "tab: args: unknown flag %s\n", (void*)aa);
	    }

	    //swallow argument values here

	    switch (argv[0][0]){
	    case 't':
	      if (strncmp(argv[0], "tuning", 6 ))
		break;
	      else {
		*argv++;
		argc--;
		break;		
	      }
	    case 'd':
	      if ( strncmp(argv[0], "debug", 5 ))
		break;
	      else {
		*argv++;
		argc--;
		break;		
	      }
	    case 'f':
	      if ( strncmp(argv[0], "font", 4 ))
		break;
	      else {
		*argv++;
		argc--;
		break;		
	      }
	    case 'a':
	      if ( argv[0][1] != 'f') break;
	    case 'm':
	      if (strncmp(argv[0], "midi-patch", 10 )) 
		break;
	    case 's':
	      if (strncmp(argv[0], "staff-line-thickness", 18 )) 
		break;
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


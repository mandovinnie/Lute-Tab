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

#if defined WIN32
#include <windows.h>
#endif /* WIN32*/

#include "version.h"
#include "tab.h"
#include "dvi.h"
#include "file_in.h"

signed char get_p(struct b *i);
#ifdef __MWERKS__
struct b *
  mac_open_font_file(char *tfm_file, struct file_info *ff);
void mac_close_font_file();
#endif

void open_pk_file(char * f, struct file_info *ff);
double tfm_dvi_to_inch(int dvi);
char *font_path=0;		// allow setting font path from cammand line

#ifdef VAX
#undef TFM_PATH
#endif /* VAX */

/* LOCAL */
file_in *f;

tfm_font::tfm_font(const char *font_name, double scale)
{
    char font_n[80];
#if defined WIN32
    LPBYTE p;
    HKEY hKey, hSubKey;
    DWORD dwSize, dwType;
#else
    char *p = NULL;
#endif

    dbg1 (TFM, "\nstarting tfm_font %s\n", (void *)font_name);
    //    printf ("tfm.cc: scale is %f\n", scale);


#ifdef MAC
    strcpy(font_n, font_name);
#else  /* not MAC */
#if defined WIN32
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software", 0 ,KEY_READ, &hKey);
    RegOpenKeyEx(hKey, "Wayne Cripps", 0 ,KEY_READ, &hSubKey);
    RegCloseKey(hKey);
    hKey = hSubKey;
    RegOpenKeyEx(hKey, "tab", 0 ,KEY_READ, &hSubKey);
    RegCloseKey(hKey);
    hKey = hSubKey;
    RegOpenKeyEx(hKey, VERSION, 0 ,KEY_READ, &hSubKey);
    RegCloseKey(hKey);
    hKey = hSubKey;
    RegQueryValueEx(hKey,"tfmDir",NULL,&dwType,NULL,&dwSize);
    p=(LPBYTE)malloc(dwSize);
    if(RegQueryValueEx(hKey,"tfmDir",NULL,&dwType,p,&dwSize) != ERROR_SUCCESS)
#else    /* not WIN32 and not MAC */
      if (font_path) {
	//	fprintf (stderr, "tfm.c - setting font path %s from command line\n", font_path);
	p = font_path;
      }
    else 
	p = getenv("TABFONTS");
    if (p == NULL ) 
#endif /* WIN32 */
#ifdef TFM_PATH
      strcpy(font_n, TFM_PATH);
#else
      strcpy(font_n, ".");
#endif /* TFM_PATH */
    else
      strcpy(font_n, (const char *)p);
    
    strcat(font_n, "/");
    strcat(font_n, font_name);
#endif /* MAC */
    strcat(font_n, ".tfm");

    f = new file_in(font_n, "rb");
    tfm_input(scale);

    delete f;
}

tfm_font::~tfm_font()
{
  /*     if (f_ligkerns) free (f_ligkerns); */
    dbg0 (TFM, "ending tfm_font\n");
}



void tfm_font::read_tfm_word(char *b0, char *b1, char *b2, char *b3)
{
    *b0 = f->GetByte();
    *b1 = f->GetByte();
    *b2 = f->GetByte();
    *b3 = f->GetByte();
}
void tfm_font::read_tfm_short(char *b0, char *b1)
{
    *b0 = f->GetByte();
    *b1 = f->GetByte();
}

double tfm_conv;		/* dvi units per absolute TFM unit 1 for now */
extern int cur_loc;
extern int last_bop;
extern int page_num;
struct font_d *fonts[MAXFONTS];

void
tfm_font::tfm_input(double scale) // read current file to current struct
/*	  int nf,  struct file_info *ff */
{
    char  b0, b1, b2, b3; 
    int k;			/* index for loops */
    unsigned int x;
    char *p;
    int ps_text = 0;		// is this a text font or lute font?

    tfm_conv = 1.0 / 8.0;
    /* (25400000 / NUM ) * ( resolution DENOM / DENOM) / 16.0 */

    f_l  = get_short();	/* length */
    f_lh = get_short();	/* header len */

    dbg2 (TFM, "\ntab: tfm_input: length %d, header_len %d, ",
	    (void *)(int)f_l, (void *)(int)f_lh);

    dbg2 (Widths, "\ntab: tfm_input: length %d, header_len %d\n ",
	    (void *)(int)f_l, (void *)(int)f_lh);

    f_bc = get_short();	/* begin char */
    f_ec = get_short();	/* end char */


    dbg2(TFM, "begin char %d, end char %d\n",
	      (void *)f_bc, (void *)f_ec);

    if (f_ec < f_bc) f_bc = f_ec +1;

    /* check for too many widths */
    if (( n_widths = f_ec - f_bc + 1) > MAXWIDTHS) 
      dbg0(Error, "tab: tfm_input: too many widths\n");

    if (n_widths < 5) {	/* oops! */
	dbg1 (Warning, 
	      "tab: tfm_file: there are only %d characters in this font!!\n",
	      (void *)n_widths);
    }
    f_nw = get_short();	/* number of widths */
    f_nh = get_short();	/* number of heights */
    f_nd = get_short();
    f_ics = get_short();
    f_ligproglen = get_short();
    f_n_kerns = get_short();
    f_extproglen = get_short();
    f_params = get_short();

    dbg2 (TFM,  " %d widths, %d heights\n", (void *)(int)f_nw, (void *)(int)f_nh);
    dbg1 (TFM,  "program length is %d, ", (void *)(int)f_ligproglen);
    dbg2 (TFM,  "%d italics, %d kerns, ", (void *)(int)f_ics, (void *)(int)f_n_kerns);
    dbg1 (TFM,  "%d parameters\n",(void *) (int)f_params);

    for (k=1;k <= f_lh; k++) { /* check sum and design size */
	if ( k == 1 ) {
	    f_check_sum = (int)get_long();
/*	    pk_checksum[nf] = f_check_sum; */
	    dbg1 (TFM, "check sum is %X\n", (void *)f_check_sum);
	}
	else if (k == 2) {
	    f_design_size = (int)get_long() / 8;
	    dbg1 (TFM, "design size is %X\n", (void *)f_design_size);
	}
	else if (k == 3 ) {	/* seems to be 1 byte size 19 name */
	    int i;
	    p = commnt;
	    read_tfm_word(&b0, &b1, &b2, &b3);
	    i = f_type_len = (unsigned int)b0;
	    *p++ = b1; 
	    *p++ = b2; 
	    *p++ = b3; 
	    i-=3;
	    while (i>0) {
		read_tfm_word(&b0, &b1, &b2, &b3);
		k++;
		i-=4;
		*p++ = b0; 
		*p++ = b1; 
		*p++ = b2; 
		*p++ = b3; 
	    }
	    commnt[f_type_len-1] = '\0';
	    dbg2 (TFM, "type length %d, coding scheme %s\n",
		      (void *)f_type_len, (void *)commnt);
	    dbg2 (Widths, " tfm_input: type length %d coding scheme %s\n",
		      (void *)f_type_len, (void *)commnt);
	}
	else if ( k == 13) {
	    int i;
	    /* seems to be 1 byte size 19 bytes name */	    
	    p = names;
	    read_tfm_word(&b0, &b1, &b2, &b3); /* at word 14 */
	    i = f_name = (unsigned int)(b0/* & 0x000f */);
	    i-=3;
	    *p++ = b1; 
	    *p++ = b2; 
	    *p++ = b3;
	    /* while (k <= 18) { */
	    while (i>0 && k <= 18) {
		read_tfm_word(&b0, &b1, &b2, &b3);
		*p++ = b0; 
		*p++ = b1; 
		*p++ = b2; 
		*p++ = b3; 
		i-=4;
		k++;
	    }
	    names[f_name] = '\0';

	    dbg2 (TFM,    "tfm_input: namelen %d name %s\n",
		  (void *)f_name,(void *) names);
	    
	    dbg2 (Widths, "tfm_input: namelen %d name %s\n",
		  (void *)f_name,(void *) names);

	    ps_text = 0;
	    if (!strstr(names, "wbc") && strchr(names, 'P')) {
	      ps_text = 1;
	    }
	}
	else (void) get_long();
    }
    /* store char width indices at end of width table */
    for (k=0; k < n_widths; k++){
	read_tfm_word(&b0, &b1, &b2, &b3);
	f_widths[k]  = (unsigned)(b0 & 0xff);
	dbg4 (Widths, "Width for char %c %d %o is %d\n", 
	      (void *)k, (void *)k, (void *)k, (void *)f_widths[k] );

	f_heights[k] = (unsigned)(b1>>4 & 0xf);
	f_depths[k]  = (unsigned)(b1 & 0xf);
	f_italics[k] = (unsigned)(b2>>2 & 0x3f);
	f_tags[k]    = (unsigned)(b2 & 0x3);
	f_remain[k]  = (unsigned)(b3 & 0xff);
    }

    for (k = n_widths; k < 255; k++ ) 
		f_widths[k]  = (0);

    /* read and convert width values */

    for (k=0; k < f_nw; k++){
	x = (int)get_long();
	dbg2(Widths, "width %d is %d\n", (void *)k, (void *)x);
	if (ps_text) {  /* postscript text fonts */
	  f_width[k] = tfm_dvi_to_inch(x) * (double)f_design_size * scale
		  / (1048576.0);
	}
	else {
	  f_width[k] = dvi_to_inch(x) * (double)f_design_size * scale
		  / (1048576.0);
	}
	f_i_width[k] =(int)((scale * ( x * (( f_design_size * 16) 
		  / 0x100000 )) / 16));	

	/*  1048576 = 0x100000 design size for lute9 = 1179648 = 0x120000*/
    }    

    /* read and convert height values */

    for (k=0; k < f_nh; k++){
	x = (int) get_long();
	if (ps_text) {  /* postscript text fonts */
	    f_height[k] = tfm_dvi_to_inch(x) 
		* f_design_size * scale / 1048576.0;
	}
	else {
	    f_height[k] = dvi_to_inch(x) 
		* f_design_size * scale / 1048576.0;
	}
	f_i_height[k] = (int)((scale * x * 
		    (( f_design_size * scale * 16) / 0x100000)) / 16);
    }    

    /* read and convert depth values */

    for (k=0; k < f_nd; k++){
	x = (int)get_long();
	if (ps_text) {  /* postscript text fonts */
	    f_depth[k] = tfm_dvi_to_inch(x) 
		* f_design_size * scale / 1048576.0;
	}
	else {
	    f_depth[k] = tfm_dvi_to_inch(x) 
		* f_design_size * scale / 1048576.0;
	}
    }   
    /* read and convert italic values   0 -> f_ics -1 fix_word*/
    /* forget the values */
    for (k=0; k < f_ics; k++) {
	x = (int)get_long();
    }
    /* read and convert lig_kern values 0 -> f_ligproglen -1  
       lig_kern_command */ 

    if (f_ligproglen) {
      /*	
       *f_ligkerns = (int)malloc((int)f_ligproglen);
	if (!f_ligkerns) {
	    dbg0(Warning, "tab: ligkern error\n");
	}
	*/
    }
    for (k=0; k < f_ligproglen; k++) {
	x = (int)get_long();
	f_ligkerns[k] = x;
    }
    /* read and convert kern values     0 -> f_extproglen -1  
       extensible_recepie */ 
    for (k=0; k < f_n_kerns; k++) {
	x = (int)get_long();
    }
    /* read and convert param values    0 -> f_params -1  fix_word*/
    for (k=0; k < f_params; k++) {
	x = (int)get_short();
	    if (k==0) dbg1 (TFM, "  slant: %d ", (void *)x);
	    if (k==1) dbg1 (TFM, "space: %d ", (void *)x);
	    if (k==2) dbg1 (TFM, "space_stretch: %d ", (void *)x);
	    if (k==3) dbg1 (TFM, "space_shrink: %d ", (void *)x);
	    if (k==4) dbg1 (TFM, "x_height: %d ", (void *)x);
	    if (k==5) dbg1 (TFM, "quad: %d ", (void *)x);
	    if (k==6) dbg1 (TFM, "extra_space: %d\n", (void *)x);
    }
}

long tfm_font::get_long()
{
    char b0, b1, b2, b3;
    long val;
    read_tfm_word(&b0, &b1, &b2, &b3);

    val = ((unsigned long)b0 & 0xff) << 24;
    val += ((unsigned long)b1 & 0xff) << 16;
    val += ((unsigned long)b2 & 0xff) << 8;
    val += ((unsigned long)b3 & 0xff);
    return(val);
}
/* read two bytes and return integer */
int tfm_font::get_short()
{
    char b0, b1;

    read_tfm_short(&b0, &b1);
    return ((((unsigned long)b0 & 0xff) << 8) + (b1 & 0xff));
}

double tfm_font::get_width(unsigned char c)
{
    unsigned int cc;

    if (c > f_ec - f_bc) {
	dbg1 (Warning,
	      "tab: get_width: character %d greater than maximum\n", 
	      (void *)(int)c);
	return (0.0);
    } 
    else if ( c < f_bc ) {
	dbg2 (Warning,
	      "tab: get_width: character %d less than than minimum %d\n", 
	      (void *)(int)c, (void *)(int)f_bc);
	return (0.0);
    }
    cc = c - f_bc;
    return(f_width[f_widths[(int)cc]]);
}

double tfm_font::get_height(unsigned char c)
{
    return(f_height[f_heights[(int)c - f_bc]]);
}

double
tfm_font::get_depth(unsigned char c)
{
    return(f_depth[f_depths[(int)c - f_bc]]);
}

int tfm_font::p_get_w(unsigned char c)
{
    
    if ( c < f_bc ) {
	dbg1 (Warning, "tab: p_get_w: character below minimum %d\n",
		(void *)(int)c);
	return (0);
    }
    if ( c > f_ec ) {
	if ( c == 161) {/* inverted ! */
	    c = '\074'; /* the TeX inverted ! */
	}
	else if (c == 191 ) {/* inverted ? */
	    c = '\076'; /* the TeX inverted ? */
	}
	else {
	    dbg3 (Warning, 
		  "tab: p_get_w: character above maximum char %d max %d %s\n",
		  (void *)(int)c, (void *)f_ec, (void *)names);
	    return (0);
	}
    }
    return((int)(f_i_width[f_widths[(int)c - f_bc]]));
}

int tfm_font::p_get_h(unsigned char c)
{

    return((int)(f_i_height[f_heights[(int)c - f_bc]]));
}




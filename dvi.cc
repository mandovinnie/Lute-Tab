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
/*
 * dvi output stuff
 *
 * note that we use dvi units, and we use the tfm file for font
 * widths
 */

#include "dvi.h"
#include "tab.h"
#include <errno.h>
#include "print.h"
#include "dviprint.h"
#include "system.h"

double tfm_dvi_to_inch(int dvi);

/* LOCAL */

int pt_to_dvi(double pt);
int mm_to_dvi(double mm);
struct font_list *init_font_list(int num, const char *name, double scale);
struct font_list  *delete_font_list(struct font_list *f);

int max_font_num = 0;

#ifdef MAC
double atof(char *s);

double atof(char *s)
{
#define SZ 15
    int i, j=0, point=-1;
    double r=0.0;
    double mul=1.0;
    double sign=1.0;
    char *p;
    char tmp[SZ];

    p=s;
    if (*p == '-') { sign = -1.0; *p++; }

    for (i=0; i < SZ; i++) {
	if (*p == '.') {
	    point = i ;
	}
	else if (*p == '\0' || *p == '\n' || *p == ' ') { 
	    if (point<0) point = i;
	    break;
	}
	else tmp[i] = *p;
	p++;
    }
    tmp[i] = '\0';
    while (point--) {
	r *= 10.0;
	r +=(double)tmp[j]-'0';
	j++, i--;
    }
    if (i) {
	i--;
	if (!i) goto exit_it;
	while (i--) {
	    mul /=10.0;
	    r += (double)(tmp[++j]-'0') * mul;
	}
    }
  exit_it:
    return (r* sign);   
}
#endif /* MAC */

struct font_list *init_font_list(int num, const char *name, double scale)
{
    font_list *f;

    f = new font_list;

    f->next = NULL;
    f->name = (char *) malloc (BL );
    strcpy (f->name, name);
    f->num = num;
    if (num > max_font_num) max_font_num = num;
    f->fnt = new tfm_font(name, scale);
    f->real_name = NULL;
    f->size = 10.0 * scale;
    return f;
}

struct font_list  *delete_font_list(struct font_list *f)
{
    free(f->name);
    delete(f->fnt);
    delete(f);
    return(NULL);
}

int str_to_dvi(const char *string)
{
    char *s, str[32];

    mystrncpy(str, string, (int)sizeof(str)-1);

    s = strstr(str, "in");
    if (s) {			             /* inches */
	*s = 0;
	return (inch_to_dvi(atof(str)));
    }
    else if ((s = strstr(str, "pt"))) {          /* points */
	*s = 0;
	return (pt_to_dvi(atof(str)));
    }
    else if ((s = strstr(str, "mm"))) {          /* points */
	*s = 0;
	return (mm_to_dvi(atof(str)));
    }
    dbg1(Error,
	 "tab: str_to_dvi: unsupported dimension %s\n", (void *)str);

    return (0);
}

void
mystrncpy(char *s1, const char *s2, int n)	/* not as robust as the real thing */
                 			/* but quite a bit faster! */
{
    while (n) {
	*s1 = *s2;
	s1++;
	s2++;
	if (!*s2) break;
	n--;
    }
    *s1=0;
}

int inch_to_dvi(const double inch)
{
    return ((int)(red * (inch * 254000.0) * ( (double)DENOM / (double)NUM)));
}

int mm_to_dvi(double mm)
{
    return ((int)(
	    (red * (mm * 254000) * ( (double)DENOM / (double)NUM)) / 25.4));
}

int pt_to_dvi(double pt)
{
    return((int) (
	   (red * 
	    (pt * 254000) * 
	    ( 
	     (double)DENOM / (double)NUM
	     )
	    ) / 72.27)
	   );
}

double dvi_to_inch(int dvi)
{
    return ((double) dvi * ((double)NUM / (double)DENOM) / (254000.0 * red) );
}

double tfm_dvi_to_inch(int dvi)
{
    return ((double) dvi * ((double)NUM / (double)DENOM) / (254000.0) );
}
double dvi_to_pt(int dvi)
{
    return ((72.27 / red ) * ( (double)NUM / (double)DENOM) 
	    * (double) dvi / 254000 );
}
double dvi_to_mm(int dvi)
{
  return (dvi_to_inch(dvi) * 25.4);
}

double str_to_inch(const char *string)
{
    char *s, str[85];
    double val;

    mystrncpy(str, string, sizeof(str));
    s = strstr(str, "in");
    if (s) {			/* inches */
	*s = 0;
	val = atof(str);
	return (val);
    }
    else if ((s = strstr(str, "pt"))) {          /* points */
	*s = 0;
	val = (atof(str) / 72.27 );
	return (val);
    }
    else if ((s = strstr(str, "mm"))) {          /* points */
	*s = 0;
	return (atof(str) / 25.4);
    }
    dbg2(Warning, "tab: str_to_inch: unsupported dimen %s  [ %s ]\n",
	   (void *)string, (void *)str);
    return(0);
}


struct font_list *add_font(char *name, double scale) 
{
    font_list *f;

    f = new font_list;

    f->next = NULL;
    f->name = (char *) malloc (BL );
				/* look for name in list */
    strcpy (f->name, name);
    f->num = ++max_font_num;
    f->fnt = new tfm_font(name, scale);
    return f;
}

#include "tree.h"

char *
get_real_name(const char *short_name, int dump)
{
  static char *c = NULL;
  struct tuple pfonts[] = {
    {"pbkd", (void *)"/Bookman-Demi"},
    {"pbkdi", (void *)"/Bookman-DemiItalic"},
    {"pbkl", (void *)"/Bookman-Light"},
    {"pbkli", (void *)"/Bookman-LightItalic"},
    {"pcrb", (void *)"/Courier-Bold"},
    {"pcrbo", (void *)"/Courier-BoldOblique"},
    {"pcrro", (void *)"/Courier-Oblique"},
    {"pcrr", (void *)"/Courier"},
    {"phvb", (void *)"/Helvetica-Bold"},
    {"phvbo", (void *)"/Helvetica-BoldOblique"},
    {"phvro", (void *)"/Helvetica-Oblique"},
    {"phvr", (void *)"/Helvetica"},
    {"phvbrn", (void *)"/Helvetica-Narrow-Bold"},
    {"phvbon", (void *)"/Helvetica-Narrow-BoldOblique"},
    {"phvron", (void *)"/Helvetica-Narrow-Oblique"},
    {"phvrrn", (void *)"/Helvetica-Narrow"},
    {"pncb", (void *)"/NewCenturySchlbk-Bold"},
    {"pncbi", (void *)"/NewCenturySchlbk-BoldItalic"},
    {"pncri", (void *)"/NewCenturySchlbk-Italic"},
    {"pncr", (void *)"/NewCenturySchlbk-Roman"},
    {"pplb", (void *)"/Palatino-Bold"},
    {"pplbi", (void *)"/Palatino-BoldItalic"},
    //    {"pplbu", (void *)"/Palatino-BoldUnslanted"},
    //    {"pplrrn", (void *)"/Palatino-Narrow"},
    //    {"pplrre", (void *)"/Palatino-Expanded"},
    {"pplri", (void *)"/Palatino-Italic"},
    {"pplr", (void *)"/Palatino-Roman"},
    {"ptmb", (void *)"/Times-Bold"},
    {"ptmbi", (void *)"/Times-BoldItalic"},
    {"ptmri", (void *)"/Times-Italic"},
    {"ptmr", (void *)"/Times-Roman"},
    {"pzcmi", (void *)"/ZapfChancery-MediumItalic"},
    {"  ", (void *)"  "},
    {0,0}
  };
  static tree ft(pfonts);

  if (dump) ft.dumpval();
  c = (char *)ft.get(short_name);
  return (c);
}

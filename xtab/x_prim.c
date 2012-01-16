/*
 *  primitaves for X windows
 *
 */

#include <stdio.h>
#include <string.h>
#include "../tab.h"

extern int dvi_h, dvi_v;
void x_command();
int x_lute_font;

unsigned int
d_to_x(n)
     int n;
{
/* dvi to inch -> dvi * ((NUM / DENOM) / 254000) */
/* dvi to inch -> dvi * ((12700000 / 473628672) / 254000) = 0.000000105 */
/* times 118 = 0.000012390 */

    return ((unsigned) ((double) n * 0.000012390)); 
}

x_to_d(n) 
     int n;
{
    return ((int)(((double)n) / 0.000012390));
}

x_init_hv()
{
    dvi_h = inch_to_dvi(0.75);
    dvi_v = inch_to_dvi(1.0);
}

x_put_rule(h, v)
     int h, v;
{
    x_command(RULE, h, v, 0, 0);
}

x_put_line(bx, by, ex, ey)
     int bx, by, ex, ey;
{
    x_command(LINE, bx, by, ex, ey);
}

x_moveh(h)
    int h;
{
    dvi_h += h;
}

x_movev(v)
    int v;	
{
    dvi_v += v;
}

x_movehv(h, v)
     int h, v;
{
    dvi_h += h;
    dvi_v += v;
}

x_put_char(c)			/* only works for fonts with no x width */
unsigned char c;
{
    x_command(CHAR, (int)c, 0,0 ,0);
}

x_set_char(c) 
     char c; 
{ 
    x_put_char((unsigned char)c);
    dvi_h += inch_to_dvi(0.11);
}

x_get_width(c)			/* bogus */
{
    return (inch_to_dvi(0.11));
}

x_setfont() { 
    x_lute_font=0;
}
x_setLute() {
    x_lute_font++;
}

static int last_com = OTHER;

void 
  x_command(com, h_n, v_n, hh_n, vv_n)
int com, h_n, v_n, hh_n, vv_n;
{
    static int h, v, hh, vv;
    static int dvi_oh, dvi_ov;

    last_com = com;
    h = h_n; v = v_n; 
    hh = hh_n; vv = vv_n;
    dvi_oh = dvi_h; dvi_ov = dvi_v;

/*
    if (com == last_com) {
	switch (com) {
	  case MOVEH:
	    h += h_n;
	    return;
	  case MOVEV:
	    v += v_n;
	    return;
	}
    }
*/

    switch (last_com) {
      case RULE:
	xp_put_rule( d_to_x(dvi_oh), d_to_x(dvi_ov - v), 
		    d_to_x(h), d_to_x(v));
	break;
      case GLP:
	break;
      case LINE:
	xp_put_line( d_to_x(h), d_to_x(v), d_to_x(hh), d_to_x(vv));
	break;
      case CHAR:
	if (x_lute_font)
	  xp_put_char (h, d_to_x(dvi_oh), d_to_x(dvi_ov));
	else xp_put_a_char (h, d_to_x(dvi_oh), d_to_x(dvi_ov));
	break;
      case OTHER:
	break;
      default:
	printf ("tab: x_command: unknown command %d\n", last_com);
	break;
    }
    
    return;
}

make_x(f)			/* top level x routine */
     struct file_info *f;
{
    main_x(f);
}

do_x_page()
{
    extern struct file_info *ff;

    reset_inter(ff);		/* rewinds input file */
    x_clear_page();		/* clear the whole screen */
    x_init_hv();		/* reset the zero point */
    return (format_page(ff));	/* print the page */
}


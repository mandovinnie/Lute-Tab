/*
 *
 *
 */

#include "tab.h"
#include "print.h"
#include "dviprint.h"
#include "dvi.h"
#include "i_buf.h"

print::print()
{
    dvi_h = dvi_v = 0;
    pr_out = new i_buf;
    sp=15;
    highlight=Off;
}

print::~print()
{
    pr_out->Seek(0, rew);
    delete pr_out;
}



void print::glp(int reg,int h[], int v[])
{
    if (reg >= REGS) 
      dbg1 (Error, "tab: getloc: illegal register %d\n", (void *)reg);

    h_diff = h[reg] - dvi_h;
    v_diff = v[reg] - dvi_v;
    dvi_h = h[reg];
    dvi_v = v[reg];

    dbg5(Stack, "print: glp: reg %d v_diff %d h_diff %d dvi_v %d dvi_h %d\n", 
	 (void *)reg, 
	 (void *)v_diff, (void *)h_diff, 
	 (void *)dvi_v, (void *)dvi_h );

    if (v_diff) {/* dvi_print::dvi_command(DOWN4, v_diff, 0); */
	dvi_v -= v_diff;
	p_movev( v_diff);
    }
    if (h_diff) {/* dvi_print::dvi_command(RIGHT4, h_diff, 0); */
	dvi_h -= h_diff;
	p_moveh( h_diff);
    }
}

void print::slp(int reg,int h[], int v[])
{ 
    if (reg >= REGS) 
      dbg1 (Error, "tab: slp: illegal register %d\n", (void *)reg);
    h[reg] = dvi_h;
    v[reg] = dvi_v;
    dbg3(Stack, "print: slp: reg %d dvi_v %d dvi_h %d\n", 
	   (void *)reg, (void *)dvi_v, (void *)dvi_h);

} 



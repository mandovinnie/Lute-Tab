/*


 */

#include <stdio.h>
#include "tab.h"
#include <time.h>

#ifdef POSTSCRIPT

char of[] = "out.ps";
char title[] = " Title";
extern FILE *op, *tp;
char tmpout[] = "tabXXXX";
char vaxTmpout[12];

char *ps_font[6];

make_ps(outFile, f)
char *outFile;
struct file_info *f;
{
    int npages=0;
#ifdef sgi
    signed char c;		/* for sgi */
#else
    char c;
#endif
    int i;
    int more = 1;
    extern int ps;		/* a flag for primatives */
    ps++;
    op = fopen(outFile, "w");
    if (op == NULL) {
	printf ("tab: make_ps: can't open output file %s  -\n", outFile);
	exit (-1);
    }
    mktemp(tmpout);
    tp = fopen(tmpout, "w+");
    if (tp == NULL) {
	printf ("tab: make_ps: can't open temporary output file %s  -\n", tmpout);
	exit (-1);
    }

/* we need tfm font widths for lute font */

    if (red == 1.0) {
	open_tfm_file(0, "lute9.tfm", f);
	if (f->flags & DPI600) open_pk_file("lute9.600pk", f);
	else open_pk_file("lute9.300pk", f);
    }
    else if (red == 0.88880) {
	open_tfm_file(0, "lute8.tfm", f);
	if (f->flags & DPI600) open_pk_file("lute8.600pk", f);
	else open_pk_file("lute8.300pk", f);
    }
    else if (red == 0.777770) {
	open_tfm_file(0, "lute7.tfm", f);
	if (f->flags & DPI600) open_pk_file("lute7.600pk", f);
	else open_pk_file("lute7.300pk", f);
    }
    else {
	open_tfm_file(0, "lute6.tfm", f);
	if (f->flags & DPI600) open_pk_file("lute6.600pk", f);
	else open_pk_file("lute6.300pk", f);
    }

/* we are using tfm files for ps fonts ha ha */

    for (i=1; i <= 5; i++) ps_text_tfm(f, i);

    for (i=0; i < 256 ; i++) ps_used[i] = 0;

/* Page */
    while (more) {
	pagenum++;
	more = ps_page(f);
	npages++;
    }
/* flush buffer */
    ps_command(OTHER, 0, 0, 0, 0 );
    fprintf(tp, "%%%%Trailer\n");
    fprintf(tp, "%%%%EOF\n");

/* now we print the header and fonts */

    read_pk_file();

    ps_header(op, npages, f);

    fseek(tp, 0L, 0);
    while ((SIGNED_C)(c = getc(tp)) != EOF) {
	putc (c, op);
    }
 
    fclose (tp);
    fclose (op);
#ifdef VAX
    strcpy (vaxTmpout, tmpout);
    strcat (vaxTmpout, ".;1");
#ifndef ultrix
    if (delete (vaxTmpout)) {
	printf ("delete failed for $s\n", vaxTmpout);
	perror ("delete ");
    }
#endif  /* ultrix */
#else
    unlink(tmpout);
#endif /* VAX */
}

do_ps_page(f)
     struct file_info *f;
{
    return (format_page(f));
}

ps_page(f)
     struct file_info *f;
{
    int more;
    fprintf(tp, "%%%%Page: %s\n", title);
    fprintf(tp, "%%%%BeginPageSetup\n");
    fprintf(tp, "/pgsave save def\n");
    fprintf(tp, "%%%%EndPageSetup\n");
    if (f->flags & LSA_FORM )
      fprintf(tp, "72 -42 translate\n"); /* was 52 */
    else if (f->flags & ROTATE)
      /* for fun try [ 0 1 1 0 -250 72 ] concat */
      fprintf(tp, "[0 1 -1 0  818 72 ] concat\n"); /* was 835 822 is half way*/
    else
      fprintf(tp, "72 -35 translate\n");

    fprintf(tp, "/LuteFont findfont setfont\n"); /* in case there is no */
						 /* default font */
    ps_init_hv();
    more = do_ps_page(f);
	
    fprintf(tp, "pgsave restore\n");
    fprintf(tp, "showpage\n");
    return (more);
}

ps_header(o, npages, f)
     FILE *o;
     int npages;
     struct file_info *f;
{
    extern int dvi_v;
    time_t x;
    double dvi_to_inch(), bot;

    fprintf(o, "%%!PS-Adobe-3.0 EPSF-3.0\n");

    bot = ( dvi_to_inch(dvi_v) -2.75) * 72.27;

    if (npages == 1 &&		/* this doesn't work yet */
	! (f->flags & ROTATE))
      fprintf(o, "%%%%BoundingBox:  52 %d 550 732\n", (int)bot); 
    else 
      fprintf(o, "%%%%BoundingBox:  50 65 555 740\n"); 
    /* 0 0 612 792 */
    fprintf(o, "%%%%Creator: tab, version 3.0-a Wayne Cripps\n");
    fprintf(o, "%%%%Title: %s\n", f->file);
    x = time(0);
    fprintf(o, "%%%%Creation Date: %s", ctime(&x));
    fprintf(o, "%%%%Pages: %d\n", npages);
    fprintf(o, "%%%%EndComments\n");
    fprintf(o, "%%%%BeginProlog\n");
    fprintf(o, "%%%%EndProlog\n");
    fprintf(o, "%%%%BeginSetup\n");
    if (f->flags & DPI600) 
      fprintf(o, "%%%%Feature: *Resolution 600\n");
    else fprintf(o, "%%%%Feature: *Resolution 300\n");
    make_ps_font(o, f);
    fprintf(o, "%%%%EndSetup\n");
}

init_ps_font_names() 
{
    int i;
    for (i = 1; i < 6; i++) 
      ps_font[i] = (char *) my_malloc(80);

    strcpy (ps_font[1], PS_FONT_1);
    strcpy (ps_font[2], PS_FONT_2);
    strcpy (ps_font[3], PS_FONT_3);
    strcpy (ps_font[4], PS_FONT_4);
    strcpy (ps_font[5], PS_FONT_5);
}


ps_text_tfm(f, fontnum) 
     struct file_info *f;
     int fontnum;
{
    char tfm_f[120];

/*    printf ("tab: ps_text_tfm: opening font %d as %s\n",
	    fontnum, ps_font[fontnum]);
 */
 
    strcpy (tfm_f, ps_font[fontnum]); 
    strcat (tfm_f, ".tfm");
    open_tfm_file(fontnum, tfm_f, f );
}
#endif /* POSTSCRIPT */

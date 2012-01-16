/*

 */
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/XawInit.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Viewport.h>

#include "../pk_bit.h"

Widget box;
Window w;
Display *dis;
GC gc;
Window ww;
Widget art;
struct file_info *ff;


void
quit_proc(widget, closure, data) 
Widget widget;
XtPointer closure, data;
{ 
    printf ("quit called\n");
    XtDestroyWidget((Widget)closure);
    exit(0); 
}

void
doit_proc(widget, closure, data) 
Widget widget;
XtPointer closure;
XtPointer data;
{ 
    struct file_info *f;
    do_x_page(f);
}

void 
get_position(widget, closure, data)
Widget widget;
XtPointer closure;
XEvent *data;
{ 
    extern double dvi_to_inch();

    get_char_position( data->xmotion.x, data->xmotion.y);

}

main_x(f, argc, argv)
     struct file_info *f;
     int argc;
     char **argv;
{
    XtAppContext context;
    Widget toplevel;
    Widget xtab;
    Widget quit;
    Widget doit;
    Arg arglist[10];
    Cardinal n_args = 0;
    int xargc=0;
    char **xxargv;
    char xargv[3];
    long event_mask;
    extern int x;

    x++;			/* tell the primatives we are */
    ff = f;			/* are using x                */

    toplevel = XtAppInitialize(&context, "Xtab",
			       NULL,0,
			       &xargc,xargv,
			       NULL,NULL,0);

    dis = XtDisplay(toplevel);
    w = XtWindow(toplevel);
    gc = XDefaultGC(dis, XDefaultScreen(dis));
    XtSetArg(arglist[n_args], XtNwidth,  500); n_args++;
    XtSetArg(arglist[n_args], XtNheight, 500); n_args++;
    xtab = XtCreateManagedWidget("xtab", panedWidgetClass,toplevel,
				 arglist, n_args);

    n_args = 0;
    XtSetArg(arglist[n_args], XtNlabel, "Quit!"); n_args++;
    quit = XtCreateManagedWidget("Quit", commandWidgetClass, xtab,
				 arglist, n_args);
    XtAddCallback(quit, XtNcallback, quit_proc, (XtPointer)toplevel);

    n_args = 0;
    XtSetArg(arglist[n_args], XtNlabel, "DoIt"); n_args++;
    doit = XtCreateManagedWidget("DoIt", commandWidgetClass, xtab,
				 arglist, n_args);
    XtAddCallback(doit, XtNcallback, doit_proc, (XtPointer)toplevel);

    n_args = 0;
    XtSetArg(arglist[n_args], XtNallowHoriz, True); n_args++;
    XtSetArg(arglist[n_args], XtNallowVert, True); n_args++;
    XtSetArg(arglist[n_args], XtNforceBars, True); n_args++;
    box = XtCreateManagedWidget("Port", viewportWidgetClass, xtab,
				 arglist, n_args);

    n_args = 0;
    XtSetArg(arglist[n_args], XtNwidth,  (int)(8.5 * 118)); n_args++;
    XtSetArg(arglist[n_args], XtNheight, 11 * 118); n_args++;
    art = XtCreateManagedWidget("Simple", simpleWidgetClass, box,
				 arglist, n_args); 

/* set the display painting function to or so glyphs can overwrite */

/*    XSetFunction(dis, gc, GXor );
 */
    XSetFunction(dis, gc, GXcopy );
    XSetLineAttributes(dis, gc, 1, LineSolid, CapButt, JoinMiter);

/*
    if (BlackPixelOfScreen(XDefaultScreenOfDisplay(dis)) == 0)
      XSetForeground(dis, gc, 0 );
    if (WhitePixelOfScreen(XDefaultScreenOfDisplay(dis)) == 1)
      XSetForeground(dis, gc, 1 );
 */
/*
    if (BlackPixelOfScreen(XDefaultScreenOfDisplay(dis)) == 0)
      XSetFunction(dis, gc, GXorReverse );
 */
/*
    printf("BlackPixelOfScreen %d\n", 
	   BlackPixelOfScreen(XDefaultScreenOfDisplay(dis)));
    printf("WhitePixelOfScreen %d\n", 
	   WhitePixelOfScreen(XDefaultScreenOfDisplay(dis)));
    printf("CellsOfScreen %d\n", 
	   CellsOfScreen(XDefaultScreenOfDisplay(dis)));
    printf("WhitePixel %d\n", 
	   WhitePixel(dis, XDefaultScreen(dis)));
    printf("BlackPixel %d\n", 
	   BlackPixel(dis, XDefaultScreen(dis)));

    printf("XServerVendor %s\n", XServerVendor(dis));
    printf("XProtocolRevision %d\n", 
	   XProtocolRevision(dis));
    printf("BitmapUnit %d\n", 
	   BitmapUnit(dis));
    printf("BitmapBitOrder %d\n", 
	   BitmapBitOrder(dis));
    printf("BitmapPad %d\n", 
	   BitmapPad(dis));
 */
/* get the fonts */

    get_font();

    XtRealizeWidget(toplevel);

    XtAddCallback(box, XtNreportCallback, doit_proc, (XtPointer)toplevel);

    event_mask = ButtonPressMask;
    XtAddEventHandler(art, event_mask, True, get_position, (XtPointer)art);

    XtAppMainLoop(context);
}


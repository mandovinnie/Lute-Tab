// Macintosh Main.c
//


#include "tab.h"
#include "mac.h"

int my_main(int flags);
void mac_read_input(int filetype);
void my_close();
short my_create(const char *name, int m_mode);
void my_put(char c);
void c2p(const char * name, Str255 fname);
void mac_open_font_file(char *tfm_file, struct file_info *ff);
void mac_close_font_file();
int printf(const char *fmt, ...) ;

int printf(const char *fmt, ...) { return (0);}
int my_sprintf(char *string, const char *fmt, ...);


#ifdef __MWERKS__
void main(void)
#else
void foomain(void)
#endif
{
    char argument[] = "-P";
    WindowPtr myWindow;
    WindowPtr theWindow;
    MenuHandle menu, menu2;
    int menuId = 130,menuId2 = 129;
    short ans;
    long mAns;
    int menuHi, menuLo;
    int flags = PS;
    EventRecord theEvent;
    Boolean t;
    unsigned long sleep = 100;
    RgnHandle mouseRgn;
    Point startPt;
    
    MaxApplZone();
    MoreMasters();
    InitGraf(&qd.thePort);
    InitFonts();
    InitMenus();
    FlushEvents(everyEvent, 0);
    InitWindows();
    InitDialogs(NULL);
    
    menu = NewMenu(menuId, "\pOutput");
    AppendMenu (menu, "\pPostScript");
    AppendMenu (menu, "\pdvi");
    AppendMenu (menu, "\pHP");
    InsertMenu(menu, 0);
    menu2 = NewMenu(menuId2, "\pFile");
    AppendMenu (menu2, "\pOpen");
    AppendMenu (menu2, "\pQuit");
    InsertMenu(menu2, menuId);
    DrawMenuBar();
    DisableItem(menu, 3);
    myWindow= GetNewWindow(128, NULL, (WindowPtr)(-1L));
    SetWTitle(myWindow, "\pTab Errors");
    SetPort(myWindow);
    MoveTo(10,10);

    FlushEvents(everyEvent, 0);
    t=0;
    mouseRgn = NewRgn();
    while (1) {
	WaitNextEvent(10, &theEvent, sleep, mouseRgn);
	if (theEvent.what == mouseDown) {
	    ans = FindWindow(theEvent.where, &theWindow);
	    if (ans == inMenuBar) {
		mAns = MenuSelect(theEvent.where);
		menuLo = mAns & 0xffff;
		menuHi = (mAns & 0xffff0000)>>16;
		HiliteMenu(0);
		switch (menuHi) {
		  case 129:	// file
		    switch (menuLo){
		      case 1:
			my_printf("doit");
			my_main(flags);
			goto end;
			break;
		      case 2:
			goto end;
			break;
		    }
		    break;
		  case 130: 	// output
		    switch (menuLo){
		      case 1:  //PostScript
			my_printf("output ps");
			flags = PS;
			break;
		      case 2:  //dvi
			my_printf("output dvi");
			flags = DVI_O;
			break;
		    }
		}
		
	    }
	} 		
	if (theEvent.what == keyDown)
	  break;
    }
  end:
  	my_printf("Click to exit program");
    while (!Button())
      ;
    DisposeWindow (myWindow); 
}

#include <stdarg.h>

void
my_printf(const char *fmt, ...)
{
    DialogPtr myDialog;
    WindowPtr behind = (WindowPtr) -1;
    short itemHit;
    pascal Str255 foo = " test case";
    int i, j;
    Str255 text;
    char *p, *sval, ttt;
    va_list ap;
    int ival;
    void *dStorage = NULL;
    short dialogID=128;
    Point pt;
    int v, h;
    int init=0;

    va_start(ap, fmt);
	
    for (i=0,j=1; i < 254 && fmt[i] != '\0'; i++,j++) {
    ttt = fmt[i];
	if (fmt[i] == NEWLINE || fmt[i] == '/n') text[j] = ' ';
	else if (fmt[i] == '%') {
	    i++;
	    if (fmt[i] == 's') {
		i++;
		for (sval = va_arg(ap, char *); *sval; sval++) {
		    text[j] = *sval;
		    j++;
		}
	    }
	    else if (fmt[i] == 'd') {
	    int sign = 1;
//		i++;
		ival = va_arg(ap, int);
		if (ival < 0 ) {
			ival = -ival;
			sign = -1;
		    text[j] = '-';
		}
		else text[j] = ' ';
		text[j+10] = '\0';
		text[j+9] = (ival % 10) + 48;
		ival /= 10;
		text[j+8] = (ival % 10) + 48;
		ival /= 10;
		text[j+7] = (ival % 10) + 48;
		ival /= 10;
		text[j+6] = (ival % 10) + 48;
		ival /= 10;
		text[j+5] = (ival % 10) + 48;
		ival /= 10;
		text[j+4] = (ival % 10) + 48;
		ival /= 10;
		text[j+3] = (ival % 10) + 48;
		ival /= 10;
		text[j+2] = (ival % 10) + 48;
		ival /= 10;
		text[j+1] = (ival % 10) + 48;
		j+=10;
	    }
	    else if ( fmt[i] == 'x') {
	    	ival = va_arg(ap, int);
	    	text [j+3] = (ival & 0x000f) + 48;
	    	text [j+2] = ((ival & 0x00f0) >> 4) + 48;
	    	text [j+1] = ((ival & 0x0f00) >> 8) + 48;
	    	text [j] =   ((ival & 0xf000) >>12) + 48;
	    	j+=4;
	    }
	    else if ( fmt[i] == 'X') {
	    	ival = va_arg(ap, long);
	    	text [j+7] =  (ival & 0x0000000f) + 48;
	    	text [j+6] = ((ival & 0x000000f0) >> 4) + 48;
	    	text [j+5] = ((ival & 0x00000f00) >> 4) + 48;
	    	text [j+4] = ((ival & 0x0000f000) >> 4) + 48;
	    	text [j+3] = ((ival & 0x000f0000) >> 4) + 48;
	  	    text [j+2] = ((ival & 0x00f00000) >> 4) + 48;
	    	text [j+1] = ((ival & 0x0f000000) >> 8) + 48;
	    	text [j] =   ((ival & 0xf0000000) >>12) + 48;
	    	j+=8;
	    }	    else if ( fmt[i] == 'c' ) {
		text[j] = va_arg(ap, char);
		j++;
	    }
	}
	else text[j] = fmt[i];
    }
    
    va_end(ap);
    
    text[0]=j-1;
    
    GetPen(&pt);
    if (! init && pt.v > 500) {
    	pt.h += 200;
    	pt.v = 0;
    	init++;
    }
    DrawString (text);
    MoveTo (pt.h, pt.v+12 );

    
//    ParamText(text, NULL, NULL, NULL);
//    myDialog = GetNewDialog(dialogID, dStorage, behind);
//    ModalDialog (NULL, &itemHit);
//    if (myDialog == NULL) return; 
//    DisposDialog (myDialog); 
}

int
my_sprintf(char *string, const char *fmt, ...)
{
    DialogPtr myDialog;
    WindowPtr behind = (WindowPtr) -1;
    short itemHit;
    pascal Str255 foo = " test case";
    int i, j;
    Str255 text;
    char *p, *sval, ttt;
    va_list ap;
    int ival;
    void *dStorage = NULL;
    short dialogID=128;
    Point pt;
    int v, h;
    int init=0;

    va_start(ap, fmt);
	
    for (i=0,j=1; i < 254 && fmt[i] != '\0'; i++,j++) {
    ttt = fmt[i];
	if (fmt[i] == NEWLINE || fmt[i] == '/n') text[j] = ' ';
	else if (fmt[i] == '%') {
	    i++;
	    if (fmt[i] == 's') {
		i++;
		for (sval = va_arg(ap, char *); *sval; sval++) {
		    text[j] = *sval;
		    j++;
		}
	    }
	    else if (fmt[i] == 'd') {
	    int sign = 1;
		ival = va_arg(ap, int);
		if (ival < 0 ) {
			ival = -ival;
			sign = -1;
		    text[j] = '-';
		}
		else text[j] = ' ';
		text[j+10] = '\0';
		text[j+9] = (ival % 10) + 48;
		ival /= 10;
		text[j+8] = (ival % 10) + 48;
		ival /= 10;
		text[j+7] = (ival % 10) + 48;
		ival /= 10;
		text[j+6] = (ival % 10) + 48;
		ival /= 10;
		text[j+5] = (ival % 10) + 48;
		ival /= 10;
		text[j+4] = (ival % 10) + 48;
		ival /= 10;
		text[j+3] = (ival % 10) + 48;
		ival /= 10;
		text[j+2] = (ival % 10) + 48;
		ival /= 10;
		text[j+1] = (ival % 10) + 48;
		j+=10;
	    }
	    else if ( fmt[i] == 'x') {
	    	ival = va_arg(ap, int);
	    	text [j+3] = (ival & 0x000f) + 48;
	    	text [j+2] = ((ival & 0x00f0) >> 4) + 48;
	    	text [j+1] = ((ival & 0x0f00) >> 8) + 48;
	    	text [j] =   ((ival & 0xf000) >>12) + 48;
	    	j+=4;
	    }
	    else if ( fmt[i] == 'X') {
	    	ival = va_arg(ap, long);
	    	text [j+7] =  (ival & 0x0000000f) + 48;
	    	text [j+6] = ((ival & 0x000000f0) >> 4) + 48;
	    	text [j+5] = ((ival & 0x00000f00) >> 4) + 48;
	    	text [j+4] = ((ival & 0x0000f000) >> 4) + 48;
	    	text [j+3] = ((ival & 0x000f0000) >> 4) + 48;
	  	    text [j+2] = ((ival & 0x00f00000) >> 4) + 48;
	    	text [j+1] = ((ival & 0x0f000000) >> 8) + 48;
	    	text [j] =   ((ival & 0xf0000000) >>12) + 48;
	    	j+=8;
	    }	    else if ( fmt[i] == 'c' ) {
		text[j] = va_arg(ap, char);
		j++;
	    }
	}
	else text[j] = fmt[i];
    }
    
    va_end(ap);
    
    text[0]=j-1;
    
    for (i=1; i < j; i++) {
	string[i-1] == text[i];
    }
    string[i-1] = '\0';
    return (1);
}


short curVolume;
short curRef;

short
my_open()
{

    SFReply fileReply;
    Point where;
    Str255 prompt="Select TAB File";
    FileFilterUPP fileFilter=NULL;
    short numTypes;
    SFTypeList typeList;
    DlgHookUPP dlgHook=NULL;
    SFReply reply;
    OSType type1, type2, type3;
    short refNum;
    OSErr error;
    
    type1 = sigTeachText;
    type2 = sigWord;
    type3 = ftGenericDocumentPC;
    numTypes=1;
    typeList[0] = type3;
    typeList[1] = type2;
    typeList[2] = type1;
    
    SFGetFile(
	      where, prompt, 
	      fileFilter, numTypes, 
	      typeList, dlgHook, 
	      &reply);
    if (reply.good ==  FALSE)
    	dbg0(Error, "exiting");
    	
    curVolume = reply.vRefNum;
    
    error = FSOpen(
		   reply.fName, /* fileName */
		   reply.vRefNum, /* vRefNum */
		   &refNum);
    
    if (error != noErr) {
    			switch (error) {
    				case fnfErr:
    					dbg0(Warning, "File not found");
    				break;
    				case bdNamErr:
    					dbg0(Warning, "BadFile Name");
    				break;    				
    				case mFulErr:
    					dbg0(Warning, "Memory Full");
    				break;
    				case opWrErr:
    					dbg0(Warning, "File already opened for writing");
    				break;    				
    				case tmfoErr:
    					dbg0(Warning, "Too many files already opened");
    				break;       				
    				default:
    					dbg0(Warning, "other error");
    				break;
    			}
			    dbg0(Error, "can't open input file\n");
			}
    return refNum;
	
}

void c2p(const char * name, Str255 fname)
{
    char *pp;\
      int len=1;
	
    pp = (char *)name;	
    while (*pp) {
		    fname[len] = *pp;
		    pp++;
		    len++;
		}
    fname[0] = len-1;	
}

short my_create(const char *name, int m_mode /* 1 creat 2 app */)
{
    short refNum;
    OSErr error;
    Str255 fname;
    char *p, *pp;
    OSType creator = ftPostScriptMac;
    OSType filetype = ftPostScriptMac;

    c2p(name, fname);
	
	if (m_mode == 1) {	
    	error = FSDelete(fname, curVolume);
    	error = Create( fname, curVolume, 
		   creator, filetype);
    	if (error != noErr) {
			    my_printf("can't create output file\n");
		}
	}
    
    error = FSOpen ( fname, curVolume,
		    &refNum);
    if (error != noErr) {
		my_printf("can't open output file\n");
	}    	
    
    if (m_mode == 2 ) {
    	error = SetFPos(refNum, 2, 0);
    }
    curRef = refNum;
    return refNum;
}

void my_close()
{
    OSErr error;
    error = FSClose(curRef);
}
// put to currently open out file
void my_put(char c)
{
    OSErr error;
    long int count =1;
    char  buf;
	
    buf = c;
	
    error = FSWrite(curRef, &count, &buf);
}

short fontRef;

void mac_open_font_file(char *tfm_file, struct file_info *ff)
{
    OSErr error;
    Str255 volName;
    Str255 fname;
    short vRefNum;

    c2p(tfm_file, fname);

    error = GetVol(volName, &vRefNum);
    error = FSOpen (fname, vRefNum, &fontRef );
}

void mac_close_font_file()
{
    OSErr error;
    error = FSClose(fontRef);
}

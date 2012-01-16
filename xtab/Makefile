
CC=cc
CFLAGS= -g -UPOSTSCRIPT -DX_WIN

all:	xtab

OBJS = xtab.o util.o fonts.o x_prim.o tab_doc.o
TOBJS = pk_input.o pk_font.o tfm_file.o externs.o \
   main.o tab_page.o prim.o args.o draw.o \
   dvi_format.o getsystem.o score.o \
   title.o underline.o dvi.o pass1.o pass2.o file.o 
XOBJS = XPutImage.o XImUtil.o
SOURCES = xtab.c util.c fonts.c x_prim.c \
   ../pk_input.c ../pk_font.c ../tfm_file.c ../externs.c \
   ../main.c ../tab_page.c ../prim.c ../args.c ../draw.c \
   ../dvi_format.c ../getsystem.c ../score.c \
   ../title.c ../underline.c ../dvi.c ../pass1.c ../pass2.c ../file.c 
LIBS = -lXaw -lXmu -lXt -lXext -lX11

xtab:	${OBJS} ${TOBJS}
	${CC} ${CFLAGS} -o xtab ${OBJS} ${TOBJS} ${LIBS}

pk_input.o:	../pk_input.c ../tab.h
	$(CC) $(CFLAGS) -c  ../pk_input.c

pk_font.o:	../pk_font.c ../tab.h
	$(CC) $(CFLAGS) -c  ../pk_font.c

tfm_file.o:	../tfm_file.c ../tab.h
	$(CC) $(CFLAGS) -c  ../tfm_file.c

externs.o:	../externs.c ../tab.h
	$(CC) $(CFLAGS) -c  ../externs.c

main.o:	../main.c ../tab.h
	$(CC) $(CFLAGS) -c  ../main.c

tab_page.o:	../tab_page.c ../tab.h
	$(CC) $(CFLAGS) -c  ../tab_page.c

args.o:	../args.c ../tab.h
	$(CC) $(CFLAGS) -c  ../args.c

draw.o:	../draw.c ../tab.h
	$(CC) $(CFLAGS) -c  ../draw.c

dvi_format.o:	../dvi_format.c ../tab.h
	$(CC) $(CFLAGS) -c  ../dvi_format.c

getsystem.o:	../getsystem.c ../tab.h
	$(CC) $(CFLAGS) -c  ../getsystem.c

score.o:	../score.c ../tab.h
	$(CC) $(CFLAGS) -c  ../score.c

title.o:	../title.c ../tab.h
	$(CC) $(CFLAGS) -c  ../title.c

underline.o:	../underline.c ../tab.h
	$(CC) $(CFLAGS) -c  ../underline.c

dvi.o:	../dvi.c ../tab.h
	$(CC) $(CFLAGS) -c  ../dvi.c

pass1.o:	../pass1.c ../tab.h
	$(CC) $(CFLAGS) -c  ../pass1.c

pass2.o:	../pass2.c ../tab.h
	$(CC) $(CFLAGS) -c  ../pass2.c

file.o:	../file.c ../tab.h
	$(CC) $(CFLAGS) -c  ../file.c

prim.o:	../prim.c ../tab.h
	${CC} ${CFLAGS} -c -DX_WIN ../prim.c 

depend:
	mkdep ${SOURCES}


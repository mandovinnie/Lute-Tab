#
#  if you are installing the fonts and tfm file
#  locally because you do not have TeX and metafont on 
#  your system, change the TFM_PATH to be the directory
#  there the lute9.tfm file resides
#
CC = cc
#
LIBS =
#
# use this line if the fonts are in this directory
#
# TPATH = '"./"'
#
# use this line if the fonts are in the TeX library
# each path name *must* end with a /
#
TPATH = '"/usr/sienna/wbc/src/lute/:/usr/lib/tex/fonts/tfm/:/usr/lib/tex/fonts/pk300/:."'
#
CFLAGS = -g  -DTFM_PATH=${TPATH} -UX_WIN
#

OBJS = main.o dvi.o prim.o draw.o getsys.o title.o file.o \
	tfm.o score.o strstr.o tab_p.o args.o \
 	uline.o pass1.o pass2.o dvi_f.o ext.o \
	ps.o ps_font.o pk_font.o pk_in.o map.o notes.o \
	include.o

SOURCES = main.c dvi.c prim.c draw.c getsys.c title.c file.c \
	tfm.c score.c strstr.c tab_p.c args.c \
	uline.c pass1.c pass2.c dvi_f.c ext.c \
	 ps.c ps_font.c pk_font.c pk_in.c map.c notes.c \
	include.c

HEADERS = dvi.h tab.h pk_input.h pkfile.h pk_bit.h

all:	tab 

conv:	conv.o
	$(CC) $(CFLAGS) -o conv conv.o


tab:	${OBJS}  dvi.h 
	$(CC) $(CFLAGS) -o tab $(OBJS) $(LIBS) 

clean:
	rm -f *~ *.dvi *.ps *.log
	rm -f *.o
	rm -f *.bdf *gf

pk_font.o:	pk_input.h pkfile.h

depend:
	mkdep ${SOURCES}

veryclean:	clean
	rm -f tab

distrib:	lute_tab3-0p.tar

lute_tab3-0p.tar:	 \
	Makefile README INSTALL MANUAL \
	${HEADERS} \
	${SOURCES} main.c \
	makelutefont  mk_font_local music/Mathias.tab \
	lute9.mf lute.mf lute8.mf lute7.mf lute6.mf \
	music/Lothe.tab music/sellinger.tab music/joan.tab \
	sample.tab music/allemande_D.tab music/R_Johnson_Alman_VII.tab \
	cmr12.tfm cmr10.tfm \
	lute9.300pk lute9.tfm \
	lute8.300pk lute8.tfm \
	lute7.300pk lute7.tfm \
	lute6.300pk lute6.tfm \
	Bookman-Light.tfm \
	xtab/Makefile xtab/tab_doc.c \
	xtab/x_prim.c xtab/xtab.c xtab/util.c xtab/fonts.c \
	mkdep TAB_ON_PC ListPrinterFonts.ps.orig make.com \
	paper.tab
	tar cvf lute_tab3-0p.tar \
	Makefile README INSTALL MANUAL \
	${HEADERS} \
	${SOURCES} main.c \
	makelutefont mk_font_local music/Mathias.tab \
	lute9.mf lute.mf lute8.mf lute7.mf lute6.mf \
	music/Lothe.tab music/sellinger.tab music/joan.tab \
	music/Milan_Villancico_castellano_1.tab \
	music/Lachrime.tab \
	sample.tab music/allemande_D.tab music/R_Johnson_Alman_VII.tab \
	cmr12.tfm cmr10.tfm \
	lute9.300pk lute9.tfm \
	lute8.300pk lute8.tfm \
	lute7.300pk lute7.tfm \
	lute6.300pk lute6.tfm \
	Bookman-Light.tfm \
	Times-Roman.tfm \
	Palatino-Bold.tfm Palatino-BoldItalic.tfm \
	xtab/Makefile xtab/tab_doc.c \
	xtab/x_prim.c xtab/xtab.c xtab/util.c xtab/fonts.c \
	TAB_ON_PC mkdep ListPrinterFonts.ps.orig make.com \
	paper.tab


cmr12.tfm:	
	cp -p /usr/lib/tex/fonts/tfm/cmr12.tfm .

cmr12.300pk:
	cp -p /usr/lib/tex/fonts/pk300/cmr12.300pk .

cmr10.300pk:
	cp -p /usr/lib/tex/fonts/pk300/cmr10.300pk .

cmr10.tfm:	
	cp -p /usr/lib/tex/fonts/tfm/cmr10.tfm .
 
lute9.tfm:	lute9.mf lute.mf
	mk_font_local

lute9.300pk:	lute9.mf lute.mf
	mk_font_local

lute8.300pk:	lute8.mf lute.mf
	mk_font_local

lute8.tfm:	lute8.mf lute.mf
	mk_font_local

lute7.300pk:	lute7.mf lute.mf
	mk_font_local

lute7.tfm:	lute7.mf lute.mf
	mk_font_local

lute6.tfm:	lute6.mf lute.mf
	mk_font_local

lute6.300pk:	lute6.mf lute.mf
	mk_font_local

Bookman-Light.tfm:	/usr/lib/tex/fonts/tfm/Bookman-Light.tfm
	cp /usr/lib/tex/fonts/tfm/Bookman-Light.tfm .

Times-Roman.tfm:	/usr/lib/tex/fonts/tfm/Times-Roman.tfm
	cp /usr/lib/tex/fonts/tfm/Times-Roman.tfm .

Palatino-Bold.tfm:	/usr/lib/tex/fonts/tfm/Palatino-Bold.tfm
	cp /usr/lib/tex/fonts/tfm/Palatino-Bold.tfm .

Palatino-BoldItalic.tfm:	/usr/lib/tex/fonts/tfm/Palatino-BoldItalic.tfm
	cp /usr/lib/tex/fonts/tfm/Palatino-BoldItalic.tfm .


# It is OK to Delete the following lines #
# DO NOT DELETE THIS LINE -- mkdep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

main.o:	main.c /usr/include/stdio.h ./tab.h /usr/include/setjmp.h
main.o:	/usr/include/sgidefs.h /usr/include/stdio.h
dvi.o:	dvi.c /usr/include/stdio.h ./dvi.h ./tab.h /usr/include/setjmp.h
dvi.o:	/usr/include/sgidefs.h /usr/include/stdio.h /usr/include/errno.h
dvi.o:	/usr/include/sys/errno.h /usr/include/string.h
prim.o:	prim.c /usr/include/stdio.h /usr/include/string.h ./dvi.h ./tab.h
prim.o:	/usr/include/setjmp.h /usr/include/sgidefs.h /usr/include/stdio.h
prim.o:	/usr/include/time.h
draw.o:	draw.c /usr/include/stdio.h /usr/include/setjmp.h
draw.o:	/usr/include/sgidefs.h ./tab.h /usr/include/setjmp.h
draw.o:	/usr/include/stdio.h
getsys.o:	getsys.c /usr/include/stdio.h /usr/include/setjmp.h
getsys.o:	/usr/include/sgidefs.h ./tab.h /usr/include/setjmp.h
getsys.o:	/usr/include/stdio.h
title.o:	title.c /usr/include/stdio.h ./tab.h /usr/include/setjmp.h
title.o:	/usr/include/sgidefs.h /usr/include/stdio.h
file.o:	file.c /usr/include/stdio.h /usr/include/setjmp.h
file.o:	/usr/include/sgidefs.h ./tab.h /usr/include/setjmp.h
file.o:	/usr/include/stdio.h
tfm.o:	tfm.c /usr/include/stdio.h /usr/include/string.h ./tab.h
tfm.o:	/usr/include/setjmp.h /usr/include/sgidefs.h /usr/include/stdio.h
tfm.o:	./dvi.h
score.o:	score.c /usr/include/stdio.h /usr/include/ctype.h ./tab.h
score.o:	/usr/include/setjmp.h /usr/include/sgidefs.h /usr/include/stdio.h
strstr.o:	strstr.c
tab_p.o:	tab_p.c /usr/include/stdio.h /usr/include/setjmp.h
tab_p.o:	/usr/include/sgidefs.h ./tab.h /usr/include/setjmp.h
tab_p.o:	/usr/include/stdio.h
args.o:	args.c /usr/include/stdio.h ./tab.h /usr/include/setjmp.h
args.o:	/usr/include/sgidefs.h /usr/include/stdio.h
uline.o:	uline.c /usr/include/stdio.h
pass1.o:	pass1.c ./tab.h /usr/include/setjmp.h /usr/include/sgidefs.h
pass1.o:	/usr/include/stdio.h /usr/include/stdio.h /usr/include/string.h
pass2.o:	pass2.c /usr/include/stdio.h ./tab.h /usr/include/setjmp.h
pass2.o:	/usr/include/sgidefs.h /usr/include/stdio.h
dvi_f.o:	dvi_f.c /usr/include/stdio.h /usr/include/setjmp.h
dvi_f.o:	/usr/include/sgidefs.h ./tab.h /usr/include/setjmp.h
dvi_f.o:	/usr/include/stdio.h
ext.o:	ext.c /usr/include/setjmp.h /usr/include/sgidefs.h /usr/include/stdio.h
ext.o:	./dvi.h
ps.o:	ps.c /usr/include/stdio.h ./tab.h /usr/include/setjmp.h
ps.o:	/usr/include/sgidefs.h /usr/include/stdio.h /usr/include/time.h
ps_font.o:	ps_font.c /usr/include/stdio.h ./tab.h /usr/include/setjmp.h
ps_font.o:	/usr/include/sgidefs.h /usr/include/stdio.h ./pk_bit.h
pk_font.o:	pk_font.c /usr/include/stdio.h /usr/include/string.h ./tab.h
pk_font.o:	/usr/include/setjmp.h /usr/include/sgidefs.h /usr/include/stdio.h
pk_font.o:	./pk_bit.h ./pk_input.h ./pkfile.h
pk_in.o:	pk_in.c /usr/include/stdio.h ./pkfile.h ./tab.h /usr/include/setjmp.h
pk_in.o:	/usr/include/sgidefs.h /usr/include/stdio.h
map.o:	map.c ./tab.h /usr/include/setjmp.h /usr/include/sgidefs.h
map.o:	/usr/include/stdio.h
notes.o:	notes.c ./tab.h /usr/include/setjmp.h /usr/include/sgidefs.h
notes.o:	/usr/include/stdio.h
include.o:	include.c /usr/include/stdio.h ./tab.h /usr/include/setjmp.h
include.o:	/usr/include/sgidefs.h /usr/include/stdio.h

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY

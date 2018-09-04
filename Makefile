#
#  You may need to use gnu make on this file!
#
#
#  if you are installing the fonts and tfm file
#  locally because you do not have TeX and metafont on 
#  your system, change the TFM_PATH to be the directory
#  where the lute9.tfm file resides
#
# OPTIM = -O2
OPTIM = -g -O0 -Wno-int-to-pointer-cast -Wno-unused-value
CC = g++ -wuninitilized
LD = g++
# LD = g++ -m64
# LD = g++ -m32
LDFLAGS = ${OPTIM}  
CXX = g++
# CXX = g++ -m64
# CXX = g++ -m32
#
#
LIBS =
#
# use this line if the fonts are in working directory
#
TLOC='"."'
#
# use this line if the fonts are in the TeX library
# or your personal library.
#
# TLOC = '"/Users/wbc/nnt"'
# TLOC = '"/net/wbc/src/nnt"'
# TLOC = '"/net/ifs-users/wbc/src/nnt"'
# TLOC = '"/net/quimby/wbc/src/nnt"'
# TLOC = '"/net/wbc/src/old-nnt"'
# TLOC = '"/net/tahoe4/wbc/src/nnt"'
# TLOC = '"/usr/aeolus1/wbc/src/nnt"'
# TLOC = '"/home/wbc/nnt"'
#
#
#
WARNS = -Wtraditional -Wpointer-arith -Wcast-align -Wconversion -Waggregate-return  -Wstrict-prototypes  -Wmissing-declarations   -Wnested-externs -Woverloaded-virtual -Winline -Wunused
CXXFLAGS = ${OPTIM} -DTFM_PATH=${TLOC}  
#

OBJS = args.o buffer.o dbg.o draw.o dvi.o dvi_f.o dviprint.o \
	file_in.o get_t.o getsys.o i_buf.o incl.o main.o \
	map.o notes.o pass1.o pass2.o pk_font.o pk_in.o \
	print.o ps_print.o score.o sizes.o tab_p.o tfm.o \
	title.o tree.o uline.o sound.o beam.o raw_snd.o midi_snd.o \
	pdf_print.o ascii.o nmidi.o do_nmidi.o


SOURCES = main.cc file_in.cc tfm.cc dvi.cc buffer.cc i_buf.cc \
	getsys.cc get_t.cc args.cc incl.cc print.cc tab_p.cc sizes.cc \
	title.cc draw.cc pass1.cc pass2.cc dvi_f.cc map.cc notes.cc \
	dviprint.cc score.cc uline.cc ps_print.cc \
	pk_in.cc pk_font.cc dbg.cc tree.cc sound.cc beam.cc raw_snd.cc \
	midi_snd.cc pdf_print.cc ascii.cc nmidi.cc do_nmidi.cc


HEADERS = buffer.h dbg.h file_in.h print.h system.h tfm.h pkfile.h\
	dvi.h i_buf.h sizes.h tab.h dviprint.h ps.h pk_bit.h pk_input.h \
	ps_print.h mac.h tree.h sound.h beam.h version.h raw_snd.h \
	midi_snd.h pdf_print.h pdf.h ascii.h nmidi.h

all:	tab


tt:	tree.o tt.o dbg.o
	$(LD)  $(LDFLAGS) -o tt tree.o tt.o dbg.o

tab:	${OBJS} 
	$(LD)  $(LDFLAGS) -o tab ${OBJS} -lm

clean:
	rm -f *~ *.dvi *.ps *.log
	rm -f *.o
	rm -f *.bdf *gf

pk_font.o:	pk_input.h pkfile.h

depend:
	mkdep ${SOURCES}

#depend:
#	./makedepend ${SOURCES}

veryclean:	clean
	rm -f tab

MISC = README blute.mf blute9.mf blute8.mf blute85.mf \
	blute6.mf blute6.tfm blute6.300pk  blute6.600pk \
	blute7.mf blute7.tfm blute7.300pk blute7.600pk \
	lute.mf lute9.mf lute8.mf lute7.mf lute6.mf \
	blute9.300pk blute8.300pk blute85.300pk \
	lute9.300pk lute8.300pk lute7.300pk lute6.300pk \
	blute9.600pk blute8.600pk blute85.600pk \
	lute9.600pk lute8.600pk lute7.600pk lute6.600pk \
	lute9.tfm lute8.tfm lute7.tfm lute6.tfm \
	blute9.tfm blute8.tfm blute85.tfm \
	blute6.2400pk blute8.2400pk  blute9.2400pk lute7.2400pk lute85.2400pk \
	blute7.2400pk blute85.2400pk lute6.2400pk  lute8.2400pk lute9.2400pk \
	cmr10.tfm cmr12.tfm cmti10.tfm cmti12.tfm \
	ptmr.tfm ptmri.tfm psyr.tfm \
	pbkd.tfm  pbkdi.tfm pbkl.tfm pbkli.tfm \
	pncb.tfm    pncbi.tfm   pncr.tfm    pncri.tfm  \
	ptmb.tfm     ptmbo.tfm    ptmrc.tfm    ptmro.tfm    ptmrrn.tfm \
	ptmbi.tfm    ptmrre.tfm \
	pzcmi.tfm \
	pplr.tfm pplri.tfm pplb.tfm pplbi.tfm \
	mk_font_local mk_test mk_300 mk_600 mk_1200 mk_2400 \
	sample.tab demo.tab c.tab t.tab AboutTab.txt  mkdep \
	version.pl makedepend CHANGELOG \
	midi-docs/midi-dump.c midi-docs/Makefile \
	midi-docs/tftopl.web \
	tlute.mf tlute9.mf tlute8.mf tlute7.mf tlute6.mf mk_600_t \
	tlute9.300pk tlute8.300pk tlute7.300pk tlute6.300pk \
	tlute9.600pk tlute8.600pk tlute7.600pk tlute6.600pk \
	simple.tab 

DISTFILE = lute_tab4.3.90.tar

distrib:	 ${DISTFILE}

${DISTFILE}:	 \
	Makefile \
	${HEADERS} \
	${SOURCES} ${MISC}
	cp Makefile Makefile.bak;\
	cat Makefile.bak | \
	sed '/^# DO NOT DELETE/,/# IF YOU PUT ANYTH/d' | \
	sed 's/^TLOC = /# TLOC = /' \
	> Makefile; \
	tar cvf ${DISTFILE} \
	Makefile \
	${HEADERS} \
	${SOURCES} ${MISC}

cmr12.tfm:	
	cp -p /usr/lib/tex/fonts/tfm/cmr12.tfm .

cmr12.300pk:
	cp -p /usr/lib/tex/fonts/pk300/cmr12.300pk .

cmr10.300pk:
	cp -p /usr/lib/tex/fonts/pk300/cmr10.300pk .

cmr10.tfm:	
	cp -p /usr/lib/tex/fonts/tfm/cmr10.tfm .
 
# lute9.tfm:	lute9.mf lute.mf
#	mk_font_local
#
#lute9.300pk:	lute9.mf lute.mf
#	mk_font_local
#
#lute8.300pk:	lute8.mf lute.mf
#	mk_font_local
#
#lute8.tfm:	lute8.mf lute.mf
#	mk_font_local
#
#lute7.300pk:	lute7.mf lute.mf
#	mk_font_local
#
#lute7.tfm:	lute7.mf lute.mf
#	mk_font_local
#
#lute6.tfm:	lute6.mf lute.mf
#	mk_font_local
#
#lute6.300pk:	lute6.mf lute.mf
#	mk_font_local

Bookman-Light.tfm:	/usr/lib/tex/fonts/tfm/Bookman-Light.tfm
	cp /usr/lib/tex/fonts/tfm/Bookman-Light.tfm .

Times-Roman.tfm:	/usr/lib/tex/fonts/tfm/Times-Roman.tfm
	cp /usr/lib/tex/fonts/tfm/Times-Roman.tfm .

Palatino-Bold.tfm:	/usr/lib/tex/fonts/tfm/Palatino-Bold.tfm
	cp /usr/lib/tex/fonts/tfm/Palatino-Bold.tfm .

Palatino-BoldItalic.tfm:	/usr/lib/tex/fonts/tfm/Palatino-BoldItalic.tfm
	cp /usr/lib/tex/fonts/tfm/Palatino-BoldItalic.tfm .

win:	${HEADERS} ${SOURCES} c.tab sample.tab
	mkdir -p win
	rm -rf win/*
	cp ${HEADERS} ${SOURCES} win
	cp c.tab sample.tab README AboutTab.txt win

	for f in win/*.cc ;do \
	 ff=`echo $$f| awk '{print substr($$0, 1, index($$0, ".")-1)}'`; \
	 echo '#include <io.h>' >> $$ff.cpp; \
	 echo '#define _CRT_SECURE_NO_DEPRECATE' >> $$ff.cpp; \
	 cat $$f >>  $$ff.cpp; rm $$f; \
	done

	cp blute[6789].300pk win
	cp blute[6789].600pk win
	cp blute[6789].tfm  win
	cp blute85.300pk    win
	cp blute85.tfm      win
	cp lute[6789].tfm   win
	cp lute[6789].300pk win
	cp lute[6789].600pk win
	cp cmr1[02].tfm cmti1[02].tfm win
	cp pncb.tfm pncbi.tfm pncr.tfm pncri.tfm psyr.tfm ptmr.tfm win
#	cp tab.rsrc.bin win

version:
	./version.pl


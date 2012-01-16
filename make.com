$ set on
$ set verify
$ TAB == "$ USER2:[WAYNE]TAB.EXE"
$ PURGE
$ CC   args.c
$ CC   draw.c,dvi.c
$ CC   dvi_f.c
$ CC   ext.c
$ CC   file.c
$ CC   getsys.c
$ CC   include.c
$ CC   main.c
$ CC   map.c
$ CC   notes.c
$ CC   pass1.c
$ CC   pass2.c
$ CC   pk_font.c
$ CC   pk_in.c
$ CC   prim.c
$ CC   ps.c
$ CC   ps_font.c
$ CC   score.c
$ CC   strstr.c
$ CC   tab_p.c
$ CC   tfm.c
$ CC   map.c
$ CC   title.c
$ CC   uline.c
$ DEFINE  LNK$LIBRARY  SYS$LIBRARY:VAXCRTL
$ LINK/EX=tab args.obj,draw.obj,dvi.obj,dvi_f.obj,ext.obj,file.obj,getsys.obj,main.obj,pass1.obj,pass2.obj,pk_font.obj,pk_in.obj,prim.obj,ps.obj,ps_font.obj,score.obj,strstr.obj,tab_p.obj,tfm.obj,title.obj,uline.obj,map.obj,notes.obj,include.obj
$ DELETE *.OBJ;*

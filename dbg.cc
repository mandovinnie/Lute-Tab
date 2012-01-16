
#include <stdlib.h>
#include "dbg.h"
#include <setjmp.h>
#include <stdio.h>

int debug_flag=0;
jmp_buf b_env;

void dbg_set(const dbg_type type)
{
    debug_flag |= type;
}

void dbg5(const int type, const char *fmt, 
	  void *a, void *b, void *c, void *d, void *e)
{
    if (type & debug_flag) {
#ifdef MAC
	my_printf ( fmt, a, b, c, d, e);
#else
	printf ( fmt, a, b, c, d, e);
#endif
    }
    if (type == Error) exit(-1);//longjmp(b_env, 5);
}
void dbg4(const int type, const char *fmt, 
	  void *a, void *b, void *c, void *d)
{
    if (type & debug_flag) {
#ifdef MAC
	my_printf (fmt, a, b, c, d);
#else
	printf ( fmt, a, b, c, d);
#endif
    }
    if (type == Error) exit(-1);//longjmp(b_env, 5);
}
void dbg3(const int type, const char *fmt, void *a, void *b, void *c)
{
    if (type & debug_flag) {
#ifdef MAC
	my_printf (fmt, a, b, c);
#else
	printf ( fmt, a, b, c);
#endif
    }
    if (type == Error) exit(-1);//longjmp(b_env, 5);
}
void dbg2(const int type, const char *fmt, void *a, void *b)
{
    if (type & debug_flag) {
#ifdef MAC
	my_printf (fmt, a, b);
#else
	printf ( fmt, a, b);
#endif
    }
    if (type == Error) exit(-1);//longjmp(b_env, 5);
}
void dbg1(const int type, const char *fmt, void *a)
{
    if (type & debug_flag) {
#ifdef MAC
	my_printf (fmt, a);
#else
	printf ( fmt, a);
#endif
    }
    if (type == Error) exit(-1);//longjmp(b_env, 5);
}
void dbg0(const int type, const char *fmt)
{
    if (type & debug_flag) {
#ifdef MAC
	my_printf (fmt);
#else
	printf (fmt);
#endif
    }
    if (type == Error) exit(-1);//longjmp(b_env, 5);
}

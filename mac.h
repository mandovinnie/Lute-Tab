#pragma once
/* mac.h */


//#include <stdlib.h>
#include <Stdio.h>


//#define BUFSIZ 1024


#include <ctype.h>
//char toupper (char c);
//char tolower (char c);
//int isdigit (int c);
//#define EOF -1

#include <string.h>
//char *strchr( const char *s, int c);
//int strlen (const char * s);
//int strncmp(const char *s1, const char *s2, int n);
//char * strcpy(char *s1, const char *s2);
//char * strncpy(char *s1, const char *s2, int n);
//char *strstr (const char *s1, const char *s2);
//char *strcat(char *s1, const char *s2);



#include <stdlib.h>
//int atoi(char * s);
//double atof(char *s);
//void * memset(void *s, int c, int n);
//memcpy(void *s1, const void *s2, int n);
//void * malloc (int size);
//void free(void * pointer);

int my_sprintf(char *string, const char *format, ...);
/* int */
void my_printf(const char *format, ...);
short my_open();


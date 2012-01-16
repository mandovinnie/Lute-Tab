/*
   This program is copyright 1991 by Wayne Cripps,
   P.O. Box 677 Hanover N.H. 03755.
   All rights reserved.  It is supplied "as is" 
   without express or implied warranty.

   Permission is granted to use, copy, modify and distribute
   this software without fee, provided that this notice appears
   in all copies, and that a copy of this notice is provided to
   anyone who recieves a binary copy without sources.

   This software may not be used for commercial purposes
   without explicit, prior written permission.

   Please mail bug reports, suggestions, and improvements
   to wbc@sunapee.dartmouth.edu.

 */

char *
strstr( s1, s2)
char *s1, *s2;
{
   char *cp1, *cp2, *cpp1, *cpp2;

   if (!s1 || !s2) return (0);
   if (!*s1 || !*s2) return (0);
   cp1 = s1;
   cp2 = s2;
   while (*cp1) {
       if ( *cp1 == *cp2 ) {
	   cpp1 = cp1; 
	   cpp2 = cp2;
	   while (*cpp1 && *cpp2) {
	       if ( *cpp1 != *cpp2 ) 
		 return ((char *)0);
	       cpp1++; cpp2++;
	   } 
	   return ( cp1 );
       }
       cp1++;
   }
   return ((char *)0);
}


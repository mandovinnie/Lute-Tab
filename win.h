
/* this includes some things to help compile
 * on windows
 * April 2019 wbc
 */

/* the following quiets warnings about using
   the posix string functions that in Windows can 
   allow buffer overrun attacks.  It must be first in 
   every file that uses string functions
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define BUILD_FOR_WINDOWS
#endif

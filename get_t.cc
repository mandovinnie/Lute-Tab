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
#include "win.h"
#include "tab.h"
#include "file_in.h"
#include "i_buf.h"
#include "system.h"

void include(i_buf *ib, struct file_info *f, char file []);
int getsystem(file_in *fi, i_buf *ib, struct file_info *f, char buf[]);
void args_from_string(char *buf, struct file_info *f);
void get_tab_file(file_in *fi, i_buf *ib, struct file_info *f);

int  setflag(file_info *f, char * string, pass pass);

int badchar(unsigned char p) {
  if ( p == 255 )
    return (0);
  else if ((int)p > 127)
    return (1);
  else if ( p == '\n' )
    return (0);
  else if (strchr("{%PLJWw0123456xYyQqeMSkbBiIp8vV .-\n\r*$[F#CcRAT!", p))
    return(0);
  return(1);
}

/* static struct file_info *ff; */

void get_tab_file(file_in *fi, i_buf *ib, struct file_info *f)
{
  char *p, *pp;
  int r;
  char buf[BUFSIZ];
  int square_bracket=0;

  /*     ff = f; */

  while (1) {
    fi->GetLine(buf, sizeof(buf));
    dbg1 (Flow, "tab: get_tab_file: %s", (void *)buf);

    p = buf;

    // test to see if this is valid tab file stuff

    // printf("buf %s", buf );

    if ( badchar((unsigned char)*p) ) {
      dbg2(Error, "Bad character-%c-%x, this may not be a valid tab file\n",
	   (void *)*p, (void *)*p);
    }
    if (! strncmp(buf, "Content-Type: text/plain", 24 )) {
      printf("get_t: get_tab_file: Content-Type: text/plain\n"/* , buf */);
      continue;
    }
    if (! strncmp(buf, "Content-Type: text/html", 22 )) {
      // we should not get text/html stuff
      // we should exit here
      printf("get_t: get_tab_file: Content-Type: text/html\n");
      ib->PutByte(NEWLINE);
      return;
    }
    if (! strncmp(buf, "Content-Transfer-Encoding: quoted-printable", 43 )) {
      printf(
        "get_t: get_tab_file: Content-Transfer-Encoding: quoted-printable\n" /*, buf */ );
      continue;
    }
    switch (*p) {
    case '-':		/* these flags are set before inter file */
      switch (buf[1]) {	/* these flags are set for the whole */
      case 'a':
	//     printf("get_t: buf %s\n", &buf[1]);
	if (!strncmp(&buf[1], "autoKey", 7)) {
	  goto end;
	}
	else if (!strncmp(&buf[1], "allDsup", 7)) {
	  //    printf("allDsup\n");
	  goto end;
	}
	else if (!strncmp(&buf[1], "allDsdown", 9)) {
	  //	   printf("allDsdown\n");
	  goto end;
	}

	do {
	  ib->PutByte(*p);
	  p++;
	} while (*p != NEWLINE);
	ib->PutByte(NEWLINE);
	break;
      case 't':
	if (!strncmp(&buf[1], "twostaff", 8) ||
	    !strncmp(&buf[1], "twelvedots", 10) ||
	    !strncmp(&buf[1], "tuning", 6) ||
	    !strncmp(&buf[1], "thin", 4)) {
	  goto end;
	}
	break;
      case 'e':		/* printout! */
      case 'b':		/* baroque uses different font files */
      case 'E':
      case 'G':			/* copyright */
      case 'h': 		/* flakey behavior */
      case 'H': 		/* h cant be turned off */
      case 'j':
      case '6':
      case 'R':
      case 'p':
      case 'w':
      case 'W':
      case 'D':
      end:
	args_from_string(buf, f);
	//		printf ("tab: setting flag %s\n",  buf);
	p = buf;
	break;
      default:
	do {
	  ib->PutByte(*p);
	  p++;
	} while (*p != NEWLINE);
	ib->PutByte(NEWLINE);
	break;
      }
      break;
    case 'V':		/* pass this whole */
    case 'F':
      do {
	ib->PutByte(*p);
	p++;
      } while (*p != NEWLINE );
      ib->PutByte(NEWLINE);
      break;
    case '[':
      square_bracket=1;
    case '{':
      do {
	ib->PutByte(*p);
	if (*p == NEWLINE) {
	  fi->GetLine(buf, sizeof(buf));
	  if ((unsigned)buf[0] ==  (unsigned)EOF) {
	    // surprise end of file
	    dbg0(Error, "Tab: Error: End of file within title\n");
	  }
	  p = buf;
	}
	else if ( *p == '\\') {
	  p++;
	  ib->PutByte(*p);
	  p++;
	}
	else
	  p++;
      } while (*p != '}' && ! (square_bracket && *p==']') );
      square_bracket=0;
      ib->PutByte('}');
      ib->PutByte(NEWLINE);
      break;
    case '\0':
    case 'e':
      if (!(f->flags & INCLUDE))
	ib->PutByte('e');
      ib->PutByte(NEWLINE);
      return;
    case 'p':
      ib->PutByte('p'); ib->PutByte(NEWLINE);
      break;
    case NEWLINE:
    case '\t':
    case ' ':
      break;
    case '%':
      break;
    case 'I':	                      	        /* include file */
      {
	p++;		/* remove I */
	while ( *p == ' ' )
	  p++;
	pp = p;
	while ( *pp != NEWLINE )
	  pp ++;
	*pp = '\0';
	include(/* fi, */ ib, f, p);
      }
      break;
    case 'K':
      ib->PutByte('K'); ib->PutByte(NEWLINE);
      break;
    case '$':
      // code ripped out of getsystem.cc
      {
	int i, c, j=0;
	char buffer[80];
	char *bp;
	bp = &buffer[0];
	for (i=1; (c = buf[i]) != NEWLINE; i++) {
	  *bp=c;
	  bp++;
	}

	buffer[--i] = 0;
	if (setflag(f, buffer, first))
	  break;            // return if we set the flag
	ib->PutByte('$');
	while (j < i && buffer[j] != '\0') {
	  ib->PutByte(buffer[j++]);
	}
	ib->PutByte(NEWLINE);
      }

      break;
    default:		/* assume that it must be lines of chords */
      r = getsystem(fi, ib, f, buf);	/* get line tab */
      f->cur_system++;
      if (!r) {
	return;
      }
      ib->PutByte(NEWLINE);
    }
  }
}


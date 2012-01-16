/*
 map a character to that required for the "font"
 and put it

 */

#include "tab.h"

char flag_indent[] = "0.02 in";	               /* hor. offset of flag staff */
char i_flag_indent[] = "-0.009 in";  /* hor. offset of italian flag staff */

mapflag(c, f)			/* print a flag */
     char c;
     struct file_info *f;
{
    int flags = f->flag_flag;

/*    printf("mapflag: flags %X   c %c\n", flags, c);  */

    switch (c) {
      case 'L':
      case 'J':
	put_a_char(c);
	break;
      case 'W':
	switch (flags) {
	  case CONTEMP_FLAGS:
	    moveh(i_flag_indent);
	    put_a_char(c);	/* a modern whole note */
	    break;
	  case BOARD_FLAGS:
	  case STAND_FLAGS:
	    moveh(i_flag_indent);
	    put_a_char(c); 
	    break;
	  case ITAL_FLAGS:
	  case S_ITAL_FLAGS:
	  case THIN_FLAGS:
	  case CAP_FLAGS:
	    moveh(i_flag_indent);
	    put_a_char(216);	/* an italian whole note */
	    break;
	  default:
	    moveh(i_flag_indent);
	    put_a_char(c); 
	    break;
	}
	break;
      case 'w':
	switch (flags) {
	  case CONTEMP_FLAGS:
	    moveh(i_flag_indent);
	    put_a_char(c); 
	    break;
	  case BOARD_FLAGS:
	  case STAND_FLAGS:
	    moveh(i_flag_indent);
	    put_a_char(c); 
	    break;
	  case ITAL_FLAGS:
	  case THIN_FLAGS:
	    moveh(i_flag_indent);
	    put_a_char(217);
	    break;
	  case CAP_FLAGS:
	  case S_ITAL_FLAGS:
	    moveh(i_flag_indent);
	    put_a_char(231);
	    break;
	  default:
	    moveh(i_flag_indent);
	    put_a_char(c); 
	    break;
	}
	break;
      case '0':
	switch (flags) {
	  case ITAL_FLAGS:
	    moveh(i_flag_indent);
	    put_a_char(218);
	    break;
	  case CAP_FLAGS:
	  case S_ITAL_FLAGS:
	    moveh(i_flag_indent);
	    put_a_char(232);	    
	    break;
	  case THIN_FLAGS:
	    moveh(i_flag_indent);
	    put_a_char(242); 
	    break;
	  case CONTEMP_FLAGS:
	    moveh(i_flag_indent);
	    put_a_char(c - '0' + 202); 
	    break;
	  case BOARD_FLAGS:
	  case STAND_FLAGS:
	    moveh(flag_indent);
	    put_a_char(c +  146);
	    break;
	default:
	    moveh(flag_indent);
	    put_a_char(c + 146);
	    break;
	}	
	break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
	switch (flags) {
	  case ITAL_FLAGS:
	    moveh(i_flag_indent);
	    put_a_char(c - '0' + 218);
	    break;
	  case S_ITAL_FLAGS:
	    moveh(i_flag_indent);
	    put_a_char( c - '0' + 232);	    
	    break;
	  case THIN_FLAGS:
	  case CAP_FLAGS:
	    moveh(i_flag_indent);
	    put_a_char(c - '0' + 242); 
	    break;
	  case CONTEMP_FLAGS:
	    moveh(i_flag_indent);
	    put_a_char(c - '0' + 202); 
	    break;
	  case BOARD_FLAGS:
	    moveh(flag_indent);
	    put_a_char(c - '0' + 0354);
	    break;
	  case STAND_FLAGS:
	    moveh(flag_indent);
	    put_a_char(c + 146);
	    break;
	  default:
	    moveh(flag_indent);
	    put_a_char(c + 146);
	    break;
	}	
	break;
      case 'x':
	break;
      default:
	put_a_char(c);	
    }
}

mapchar(c, f)
     unsigned char c;
     struct file_info *f;
{
    extern char italian_offset[];

    if (f->line_flag == ON_LINE )       /* shift here in case we have  */
	if (c != 'Z') movev(italian_offset);    /* italian and mace, etc */

    if (c >= 150 && c <=  159) {
	switch (f->num_flag) {

	  case STAND_NUM:
	    put_a_char (c);
	    break;
	  case ITAL_NUM:
	    if (p_get_w(0, c - 20)) {
		fmoveh((get_width(0, 133) - get_width(0, c - 20))/2.0);
		put_a_char(c - 20); /* map to italian  */
	    }
	    else
	      put_a_char(c);
	    break;
	  default:
	    printf("tab: mapchar: invalid number flag %X\n",
		   (unsigned int)f->num_flag);
	    break;
	}
    }
    else if (c >= 220) {
	printf ("number over 9  ");
	c -= 220;
	printf ("c is %d  ", c/10 + 0202);
	set_a_char( c/10 + 0202 );
	put_a_char( c%10 + 0202 );

    }
    else {
	switch (f->char_flag) {
	    
	  case STAND_CHAR:
	    put_a_char(c);
	    break;
	  case MACE_CHAR:
	    if (p_get_w(0, tolower(c) - 'a' + 160) > 0)
	      put_a_char(tolower(c) - 'a' + 160);
	    else put_a_char (c);
	    break;
	  case ROB_CHAR:
	    if (p_get_w(0, tolower(c) - 'a' + 175) > 0)
	      put_a_char(tolower(c) - 'a' + 175);
	    else put_a_char (c);
	    break;
	  case BOARD_CHAR:
	    switch (c) {
	      case 'f':
	      case 'h':
	      case 'i':
		put_a_char(toupper(c));
		break;
	      case 'G':
		put_a_char(tolower(c));
		break;
	      default:
		put_a_char(c);
	    }
	    break;
	  default:
	    printf ("mapchar: unknown or duplicate character flag %X\n ",
		    (unsigned int)f->char_flag);
	    break;
	}
    }
}

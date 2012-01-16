/*
 map a character to that required for the "font"
 and put it

 */

#include "tab.h"
#include "sizes.h"
#include "tfm.h"
#include "print.h"
#include "dviprint.h"

extern char flag_indent[];
extern char i_flag_indent[];
extern char interspace[];
void mapflag(print *p, font_list *f_a[], char c, struct file_info *f);
void mapchar(print *p, font_list *f_a[], unsigned char c, struct file_info *f);


void mapflag(print *p, font_list *f_a[], char c, struct file_info *f)
{
    int flags = f->flag_flag;

/*    printf("mapflag: flags %X   c %c\n", flags, c);  */

/*
  if (f->line_flag == ON_LINE) 
  p->movev (-.5 * str_to_inch(interspace));
  */
    switch (c) {
    case 'L':
    case 'J':
	p->put_a_char(c);
	break;
    case '$':
    {
	p->use_font(1);
	p->set_a_char('S');
	p->use_font(0);
	break;
    }
    case 'W':
	switch (flags) {
	case CONTEMP_FLAGS:
	    p->moveh(i_flag_indent);
	    p->put_a_char(c);	/* a modern whole note */
	    break;
	case BOARD_FLAGS:
	case STAND_FLAGS:
	    p->moveh(i_flag_indent);
	    p->put_a_char(c); 
	    break;
	case ITAL_FLAGS:
	case S_ITAL_FLAGS:
	case THIN_FLAGS:
	case CAP_FLAGS:
	    p->moveh(i_flag_indent);
	    p->put_a_char(216);	/* an italian whole note */
	    break;
	default:
	    p->moveh(i_flag_indent);
	    p->put_a_char(c); 
	    break;
	}
	break;
    case 'w':
	switch (flags) {
	case CONTEMP_FLAGS:
	    p->moveh(i_flag_indent);
	    p->put_a_char(c); 
	    break;
	case BOARD_FLAGS:
	case STAND_FLAGS:
	    p->moveh(i_flag_indent);
	    p->put_a_char(c); 
	    break;
	case ITAL_FLAGS:
	case THIN_FLAGS:
	    p->moveh(i_flag_indent);
	    p->put_a_char(217);
	    break;
	case CAP_FLAGS:
	case S_ITAL_FLAGS:
	    p->moveh(i_flag_indent);
	    p->put_a_char(231);
	    break;
	default:
	    p->moveh(i_flag_indent);
	    p->put_a_char(c); 
	    break;
	}
	break;
    case '0':
	switch (flags) {
	case ITAL_FLAGS:
	    p->moveh(i_flag_indent);
	    p->put_a_char(218);
	    break;
	case CAP_FLAGS:
	case S_ITAL_FLAGS:
	    p->moveh(i_flag_indent);
	    p->put_a_char(232);	    
	    break;
	case THIN_FLAGS:
	    p->moveh(i_flag_indent);
	    p->put_a_char(242); 
	    break;
	case CONTEMP_FLAGS:
	    p->moveh(i_flag_indent);
	    p->put_a_char(c - '0' + 202); 
	    break;
	case BOARD_FLAGS:
	case STAND_FLAGS:
	    p->moveh(flag_indent);
	    p->put_a_char(c +  146);
	    break;
	default:
	    p->moveh(flag_indent);
	    p->put_a_char(c + 146);
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
	    p->moveh(i_flag_indent);
	    p->put_a_char(c - '0' + 218);
	    break;
	case S_ITAL_FLAGS:
	    p->moveh(i_flag_indent);
	    p->put_a_char( c - '0' + 232);	    
	    break;
	case THIN_FLAGS:
	case CAP_FLAGS:
	    p->moveh(i_flag_indent);
	    p->put_a_char(c - '0' + 242); 
	    break;
	case CONTEMP_FLAGS:
	    p->moveh(i_flag_indent);
	    p->put_a_char(c - '0' + 202); 
	    break;
	case BOARD_FLAGS:
	    p->moveh(flag_indent);
	    p->put_a_char(c - '0' + 0354);
	    break;
	case STAND_FLAGS:
	    p->moveh(flag_indent);
	    p->put_a_char(c + 146);
	    break;
	default:
	    p->moveh(flag_indent);
	    p->put_a_char(c + 146);
	    break;
	}	
	break;
    case '~':
	p->put_a_char(067);
	break;
    case '6':
    case 'x':
	break;
    default:
	p->put_a_char(c);	
    }
}



void mapchar(print *p, font_list *f_a[], unsigned char c, struct file_info *f)
{
    extern char italian_offset[];

    if (f->line_flag == ON_LINE )       /* shift here in case we have  */
	if (c != 'Z') p->movev(italian_offset);    /* italian and mace, etc */

    if (c >= 150 && c <=  159) {
	switch (f->num_flag) {

	  case STAND_NUM:
	    p->put_a_char (c);
	    break;
	  case ITAL_NUM:
	    if (f_a[0]->fnt->is_defined(c - 20)) {
		p->moveh((f_a[0]->fnt->get_width(133) 
			  - f_a[0]->fnt->get_width(c - 20))/2.0);
		p->put_a_char(c - 20); /* map to italian  */
	    }
	    else
	      p->put_a_char(c);
	    break;
	case ADOBE_NUM:
	  p->use_font(2);
	  p->movev(0.017);
	  p->put_a_char(c-150+48);
	  p->use_font(0);
	  break;
	default:
	  dbg1(Warning, "tab: mapchar: invalid number flag %X\n",
	       (void *)f->num_flag);
	  break;
	}
    }
    else if (c >= 220) {
	if ( c == 251) {
	    p->put_a_char(c);	/* the "zap" character  */
	}
	else if ( c == 254) {
	    p->movev(0.15);
	    p->put_a_char(c);	/* the del signe character  */
	}
	else {
	    c -= 220;
	    //	    dbg1 (Warning, "map: number over 9 is %d  \n", (void *)c);
	    //  this should work from 10 to 34, 230 to 254
	    if (f->num_flag == ITAL_NUM) {
	      if (c == 10 )
		p->moveh(-0.021);
	      if (c == 11 ) 
		p->moveh(-0.009);
	      if (c == 12 || c == 13) 
		p->moveh(-0.018);
	      p->set_a_char( c/10 + 0202 );
	      p->moveh(0.011);
	      p->put_a_char( c%10 + 0202 );
	    }
	    else {
		p->set_a_char( c/10 + 0226 );
		p->put_a_char( c%10 + 0226 );
	    }
	}
    }
    else if (( c >= 'A' && c <= 'P') || ( c >= 'a' && c <= 'p')) {
	switch (f->char_flag) {
	    
	case STAND_CHAR:
	    p->put_a_char(c);
	    break;
	case MACE_CHAR:
	    if (f_a[0]->fnt->is_defined(tolower((char)c) - 'a' + 160))
		p->put_a_char(tolower((char)c) - 'a' + 160);
	    else p->put_a_char (c);
	    break;
	case ROB_CHAR:
	    if (f_a[0]->fnt->is_defined(tolower((char)c) - 'a' + 175)) {
		// printf("char %d\n", tolower((char)c) - 'a' + 175);
		// printf("widths %d\n", f_a[0]->fnt->f_widths[(int)cc]);
		// printf("width %f\n", f_a[0]->fnt->get_width(c));
		// printf("mapping %c\n", c);
		p->put_a_char(tolower((char)c) - 'a' + 175);
	    }
	    else p->put_a_char (c);
	    break;
	case BOARD_CHAR:
	    switch (c) {
	    case 'E':
		p->put_a_char(tolower((char)c) - 'a' + 175);
		break;
	    case 'f':
	    case 'h':
	    case 'i':
		p->put_a_char(toupper((char)c));
		break;
	    case 'G':
		p->put_a_char(tolower((char)c));
		break;
	    default:
		p->put_a_char(c);
	    }
	    break;
	default:
	    dbg1 (Warning, "mapchar: unknown or duplicate character flag %X\n ",
		  (void *)f->char_flag);
	    break;
	}
    }
    else if ( c == 'x' && f->num_flag ==  ITAL_NUM ) {
      //           printf("here in map flag\n");
	   p->moveh((f_a[0]->fnt->get_width(133) 
		     - f_a[0]->fnt->get_width('X'))/2.0); 
	   p->put_a_char('X');
    }
    else {
      p->put_a_char(c);
    }
}







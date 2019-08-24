#pragma once
/*
 *
 * ascii output
 *
 */

#ifndef __ASCII__
#define __ASCII__

#define LINES 7 + 3 + 2		/* 7 for notes 3 for flags */

class ascii {
 protected:
  char *lines[LINES];
  int n_lines;
  int n_char;
  int print_n_char;
  int n_staves;			/* number of lines on staff */
  int ascii_h;
  int ascii_v;
 public:
  ascii();
  ~ascii();
  void put_a_char (unsigned char c);
  void put_a_char_next (unsigned char c);
  void put_a_char_before (unsigned char c);
  void put_barline ();
  void increment_h();
  void decrement_h();
  void increment_v();
  void reset_h();
  void reset_v();
  void set_print_width(const int w);
};


#endif  /*__ASCII__ */


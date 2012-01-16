/*
 * tree.cc
 * by Wayne Cripps
 * Jan 22 1997
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "tab.h"

node::node()
{
  root = this;
  p = l = r = 0;
  k = (char *)malloc(6);
  strcpy (k, "null");
  v = 0;
  //  fprintf (stderr, "init root %X\n", root);
}
node::node(const char *key, void *val, node *top)
{
  node *pp;

  root = top;
  //  fprintf (stderr, "new node, root is %X\n", root);
  if (root == 0) {
    *root = node();
  }
  //  fprintf (stderr, "new node, root is %X\n", root);

  k = (char *)malloc(strlen(key)+2);
  //  k = new char(strlen(key)+2); malloc is better for windows
  strcpy (k, key);
  v = val;
  r = 0;
  l = 0;
  pp = root;
  while (pp) {
    if ( strcmp ( key, pp->k) > 0 )
      if (pp->l)
	pp = pp->l;
      else {
	pp->l = this;
	break;
      }
    else
      if (pp->r)
	pp = pp->r;
      else {
	pp->r = this;
	break;
      }
  }
  // fprintf (stderr,"new node %s  %X root %X\n", k, this, root);
}

node::~node()
{
  if (k) free(k);
  k = NULL;
}

void *
node::get_val(const char  *key) 
{
  node * pp;

  pp = root;
  while (pp) {
    if ( strcmp ( key, pp->k) > 0 )
      if (pp->l) {
	pp = pp->l;
      }
      else {
	//	printf ("get_val: no match\n");
	break;
      }
    else if ( strcmp ( key, pp->k) < 0 )
      if (pp->r) {
	pp = pp->r;
      }
      else {
	//	printf ("get_val: no match\n");
	break;
      }
    else {
      return (pp->v);
    }
  }
  return (0);
}

char *
node::get_key ()
{
  return (this->k);
}

void *
node::get_val ()
{
  return (this->v);
}

// node *node::getleft();
node *node::getleft()
{
  if (this->l)
    return (this->l);
  else return (0);
}
// node *node::getright();
node *node::getright()
{
  if (this->r)
    return (this->r);
  else return (0);
}
tree::tree(struct tuple data[])
{
  top = new node();
  initkeys (top, data);
}

tree::~tree()
{
}


void 
tree::initkeys(node * top, struct tuple data[])
{
  /*  node * a; */
  int i;

  for (i=0; data[i].key; i++) {
    //    printf ("%d %s\n", i, data[i].key);
    /* a = */ 
    new node(data[i].key, data[i].val, top );
  }

}

void *
tree::get(const char * key)
{
  //  fprintf (stderr, "get: top %X\n", top); 
  return top->get_val(key);
}

node *
tree::first() 
{
  node *n;

  if (top) {
    if (n = top->getleft()) 
      return(first(n));
    else return (top);
  }
  return (0);
}

node *
tree::first(node * n)
{
 node * nn;

 if (n) {
    if (nn = n->getleft())
      //      return (nn);
      return (first(nn));
    else return (n);
 }
 else 
   return (0);
}
char * tree::getkey(node *n)
{
  char *cp;
  cp = (char *)n->get_key();
  return (cp);
}

char * tree::getval(node *n)
{
  char *cp;
  cp = (char *)n->get_val();
  return (cp);
}

void tree::dump_t(node * n)
{
  char *key;
  node * nn;
  if (nn=n->getright())
    dump_t(nn);
  key=getkey(n);
  dbg1 (Warning, "tab: tree: key: %s  ", (void *)key);
  key = getval(n);
  dbg1 (Warning, "tab: tree: val: %s\n", (void *)key);
  if (nn=n->getleft())
    dump_t(nn);
}
void tree::dumpval(node * n)
{
  char *key;
  node * nn;
  if (nn=n->getright())
    dump_t(nn);
  key=getkey(n);
  dbg1 (Warning, "tab: %s", (void *)key);
  if (key)
    dbg1 (Warning, "  %s\n", (void *)get(key));
  if (nn=n->getleft())
    dump_t(nn);
}
/****************************************
 *
 *  tab specific code here
 *  this is not c++
 *
 ****************************************/

void set_lutefont(const char *value, struct file_info *f)
{
  f->font_names[0] = (char *) malloc (strlen (value));
  strcpy (f->font_names[0], value);
  //  if ( ! (f->m_flags & QUIET))
  //    dbg1 (Warning, "tab: lutefont = %s\n", (void *)value);
}
void set_charstyle(const char *value, struct file_info *f)
{
  switch (value[0]) {
  case 'b':
    //board
    f->char_flag = BOARD_CHAR;
    break;
  case 'd':
    //board
    f->char_flag = DOWLAND_CHAR;
    break;
  case 'm':
    //mace
    f->char_flag = MACE_CHAR;
    break;
  case 's':
    //standard
    f->char_flag = STAND_CHAR;
    break;
  case 'r':
    //robinson
    f->char_flag = ROB_CHAR;
    break;
  }
  //  if ( ! (f->m_flags & QUIET))
  //    dbg1 (Warning, "tab: set_charstyle %s\n", (void *)value);
  
}
void set_numstyle(const char *value, struct file_info *f)
{
  switch (value[0]) {
  case 's':
    //standard numbers
    f->num_flag = STAND_NUM;
    break;
  case 'i':
    //italian numbers
    f->num_flag = ITAL_NUM;
    break;
  case 'a':
    // use the printer fonts
    f->num_flag = ADOBE_NUM;
    break;
  }
}

void set_flagname(const char *value, struct file_info *f)
{
  //  if ( ! (f->m_flags & QUIET))
  //    dbg1 (Warning, "tab: set_flagname: flagstyle = %s\n", (void *)value);
  switch (value[0]){
  case 'b':
    //  "board"
    f->flag_flag = BOARD_FLAGS;	
    break;
  case 'c':
    //  "capirola"
    if (value[1] == 'a')
      f->flag_flag = CAP_FLAGS;
    //  "contemp"
    if (value[1] == 'o')
      f->flag_flag = CONTEMP_FLAGS;
    break;
  case 'i':
  //  "italian"
    f->flag_flag = ITAL_FLAGS;
    break;
  case 's':
    //  "smallitalian"
    if (value[1] == 'm')
      f->flag_flag = S_ITAL_FLAGS;
    //  "standard"
    if (value[1] == 't')
      f->flag_flag = STAND_FLAGS;
    break;
  case 't':
    //  "thin"
    f->flag_flag = THIN_FLAGS;
    break;
  default:
    dbg1 (Warning, "tab: unknown flagstyle = %s\n", (void *)value);
    break;
  }
}
void set_line(const char *value, struct file_info *f)
{
    if (value[0] == 'o')
	f->line_flag = ON_LINE;
    else if (value[0] == 'b')
	f->line_flag = BETWEEN_LINE;
    else   
	dbg1 (Warning, "tab: unknown line = %s\n", (void *)value);

    //    dbg1 (Warning, "tab: set_line: %s\n", (void *)value);
}

void set_textfont(const char *value, struct file_info *f)
{
    f->font_names[1] = (char *) malloc (strlen (value));
    strcpy (f->font_names[1], value);
    //    dbg1 (Warning, "tab: set_textfont %s\n", (void *)value);
}
void set_alt_textfont(const char *value, struct file_info *f)
{
    f->font_names[5] = (char *) malloc (strlen (value));
    strcpy (f->font_names[5], value);
    //    dbg1 (Warning, "tab: set_textfont %s\n", (void *)value);
}

void set_dvitextfont(const char *value, struct file_info *f)
{
    if (f->flags & DVI_O ){
	f->font_names[1] = (char *) malloc (strlen (value));
	strcpy (f->font_names[1], value);
	//    dbg1 (Warning, "tab: set_textfont %s\n", (void *)value);
    }
}
void set_dvialttextfont(const char *value, struct file_info *f)
{
    if (f->flags & DVI_O ){
	f->font_names[5] = (char *) malloc (strlen (value));
	strcpy (f->font_names[5], value);
	//    dbg1 (Warning, "tab: set_textfont %s\n", (void *)value);
    }
}

void set_pstextfont(const char *value, struct file_info *f)
{
    if (f->flags & PS ){
	f->font_names[1] = (char *) malloc (strlen (value));
	strcpy (f->font_names[1], value);
	//    dbg1 (Warning, "tab: set_textfont %s\n", (void *)value);
    }
}
void set_psalttextfont(const char *value, struct file_info *f)
{
    if (f->flags & PS ){
	f->font_names[5] = (char *) malloc (strlen (value));
	strcpy (f->font_names[5], value);
	//    dbg1 (Warning, "tab: set_textfont %s\n", (void *)value);
    }
}

void set_textsize(const char *value, struct file_info *f)
{
  if (! (f->m_flags & QUIET )) {
    dbg1 (Warning, "tab: set_textsize %s\n", (void *)value);
  }
  if (f->flags & PS) {
    f->font_sizes[1] = atof(value); 
  }
}

void set_alttextsize(const char *value, struct file_info *f)
{
  if (! (f->m_flags & QUIET )) {
	dbg1 (Warning, "tab: set_altextsize %s\n", (void *)value);
  }
  if (f->flags & PS) {
    f->font_sizes[5] = atof(value); 
  }
}
void set_titlefont(const char *value, struct file_info *f)
{
    f->font_names[2] = (char *)malloc (strlen (value));
    strcpy (f->font_names[2], value);
}
void set_alt_titlefont(const char *value, struct file_info *f)
{
    f->font_names[3] = (char *)malloc (strlen (value));
    strcpy (f->font_names[3], value);
    if ( ! (f->m_flags & QUIET))
      dbg1 (Warning, "tab: set_alt_titlefont: %s\n", (void *)value);
}
void set_dvititlefont(const char *value, struct file_info *f)
{
    if (f->flags & DVI_O) {
	f->font_names[2] = (char *)malloc (strlen (value));
	strcpy (f->font_names[2], value);
    }
}
void set_dvialttitlefont(const char *value, struct file_info *f)
{
    if (f->flags & DVI_O) {
	f->font_names[3] = (char *)malloc (strlen (value));
	strcpy (f->font_names[3], value);
    }
}
void set_pstitlefont(const char *value, struct file_info *f)
{
    if (f->flags & PS) {
	f->font_names[2] = (char *)malloc (strlen (value));
	strcpy (f->font_names[2], value);
    }
}
void set_psalttitlefont(const char *value, struct file_info *f)
{
    if (f->flags & PS) {
	f->font_names[3] = (char *)malloc (strlen (value));
	strcpy (f->font_names[3], value);
	if ( ! (f->m_flags & QUIET))
	  dbg1 (Warning, "tab: set_psalttitlefont: %s\n", (void *)value);
    }
}
void set_titlesize(const char *value, struct file_info *f)
{
  //    dbg1 (Warning, "set_titlesize %s\n", (void *)value);
    if (f->flags & PS) {
	f->font_sizes[2] = atof(value); 
    }
}

void set_alttitlesize(const char *value, struct file_info *f)
{
    if ( ! (f->m_flags & QUIET))
      dbg1 (Warning, "set_alttitlesize %s\n", (void *)value);
    if (f->flags & PS) {
	f->font_sizes[3] = atof(value); 
    }
}

void set_val(const char *value, struct file_info *f)
{
    if ( ! (f->m_flags & QUIET))
      dbg1 (Warning, "set_val %s\n", (void *)value);
}

void set_description(const char *value, struct file_info *f)
{
    if ( ! (f->m_flags & QUIET))
      dbg0 (Warning, "set_description\n");
}
void set_tempo(const char *value, struct file_info *f)
{
  extern double conv;
  conv = atof(value);
  //   fprintf(stderr, "in set tempo %f\n", conv);
}

void set_noteconv(const char *value, struct file_info *f)
{
        f->note_conv = *value-'0'; 
	if (f->note_conv < 0 || f->note_conv > 12 ) {
	  dbg1(Warning, "strange note-conversion value %c ",
	       (void *)(int) *value);
	  dbg0(Warning, "should be between 0 and 3\n");
	}
	//	printf ("tree conv %f\n", f->note_conv);
}


void set_sys_skip(const char *value, struct file_info *f)
{
  f->sys_skip = atof(value);
}

void set_midi_patch(const char *value, struct file_info *f)
{
  f->midi_patch = atoi(value);
}
void set_start_system(const char *value, struct file_info *f)
{
  int i = atoi(value);
  f->start_system = i;
}
void set_transpose(const char *value, struct file_info *f)
{
  int i = atoi(value);
  f->transpose = i;
}
void set_scribe(const char *value, struct file_info *f)
{
  if (f->scribe != 0 ) {
    free (f->scribe);
    f->scribe = 0;
    dbg0(Warning, "set scribe - re setting scribe\n");
  }
  f->scribe = (char * )malloc (strlen(value)+1);
  strcat (f->scribe, value);
  f->scribe[strlen(value)] = 0;
}
void set_left_margin(const char *value, struct file_info *f)
{
  int i = atoi(value);
  f->left_margin = i;
}


/*
void setit(void *r, const char *flag, struct file_info *f)
{
  if ( !r )
    return;
  r(flag, f);// was *r wbc oct 2001
}
*/

int
set_string(const char *arg, const char *val, struct file_info *f, pass pass) 
{
  void ** res();
  void (*r)(const char *, struct file_info *);

  struct tuple data[] = {
    {(char*)"lutefont",   (void *)set_lutefont},
    {(char*)"charstyle",  (void *)set_charstyle},
    {(char*)"numstyle",   (void *)set_numstyle},
    {(char*)"flagstyle",  (void *)set_flagname},
    {(char*)"textfont",   (void *)set_textfont},
    {(char*)"textsize",   (void *)set_textsize},
    {(char*)"titlefont",  (void *)set_titlefont},
    {(char*)"titlesize",  (void *)set_titlesize},
    {(char*)"alttitlefont",    (void *)set_alt_titlefont},
    {(char*)"alttextfont",     (void *)set_alt_textfont},
    {(char*)"alttextsize",     (void *)set_alttextsize},
    {(char*)"alttitlesize",    (void *)set_alttitlesize},
    {(char*)"pstextfont",      (void *)set_pstextfont  },
    {(char*)"dvitextfont",     (void *)set_dvitextfont},
    {(char*)"psalttextfont",   (void *)set_psalttextfont},
    {(char*)"dvialttextfont",  (void *)set_dvialttextfont},
    {(char*)"pstitlefont",     (void *)set_pstitlefont  },
    {(char*)"dvititlefont",    (void *)set_dvititlefont},
    {(char*)"psalttitlefont",  (void *)set_psalttitlefont},
    {(char*)"dvialttitlefont", (void *)set_dvialttitlefont},
    {(char*)"val", (void *)set_val},
    {(char*)"description", (void *)set_description},
    {(char*)"line",       (void *)set_line},
    {(char*)"tempo",      (void *)set_tempo},
    {(char*)"note-conversion", (void *)set_noteconv},
    {(char*)"sys-skip",   (void *)set_sys_skip},
    {(char*)"midi-patch",   (void *)set_midi_patch},
    {(char*)"start-system", (void *)set_start_system},
    {(char*)"transpose",    (void *)set_transpose},
    {(char*)"scribe",       (void *)set_scribe},
    {(char*)"left-margin", (void*)set_left_margin},
    {0,0}
  };

  struct tuple passnum[] = {
    {(char*)"lutefont",   (void *)first},
    {(char*)"charstyle",  (void *)second},
    {(char*)"numstyle",   (void *)second},
    {(char*)"flagstyle",  (void *)second},
    {(char*)"textfont",   (void *)first},
    {(char*)"textsize",   (void *)first},
    {(char*)"alttextsize",(void *)first},
    {(char*)"titlefont",  (void *)first},
    {(char*)"titlesize",  (void *)first},
    {(char*)"alttitlefont", (void *)first},
    {(char*)"alttextfont",  (void *)first},
    {(char*)"alttitlesize", (void *)first},
    {(char*)"pstextfont",   (void *)first},
    {(char*)"dvitextfont",   (void *)first},
    {(char*)"psalttextfont",  (void *)first},
    {(char*)"dvialttextfont",  (void *)first},
    {(char*)"description", (void *)first},
    {(char*)"val",  (void *)first},
    {(char*)"line",       (void *)second},
    {(char*)"tempo",      (void *)second},
    {(char*)"note-conversion", (void *)first},
    {(char*)"sys-skip",   (void *)first},
    {(char*)"midi-patch",   (void *)first},
    {(char*)"start-system",   (void *)first},
    {(char*)"transpose",    (void *)first},
    {(char*)"scribe",       (void *)first},
    {(char*)"left-margin", (void*)first},
    {0,0}
  };
  
  static tree pt(passnum);
  static tree tt(data);

  r = (void(*)(const char*, file_info*))tt.get(arg);
  if ((void *)pass == pt.get(arg)) {
    if ( !r ) {
      dbg1(Warning, "tab: set_string: bad $ flag, %s ignored\n", (void *)arg);
    }
    else {
      //      if ( ! (f->m_flags & QUIET))
      //setit(r, val, f);
      (*r)(val, f);
    }
    return (1);
  }
  return (0);
}

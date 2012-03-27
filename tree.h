/*
 * tree.h
 * by Wayne Cripps
 * Jan 22 1997
 */

struct tuple {
  char * key;
  /*  void * val; changed to make the g++ happy */
  void * val;
};

class node {
  char * k;
  void * v;
  node * p;
  node * l;
  node * r;
  node * root;
public:
  node();			// for root
  node(const char *key, void *val, node *top); // create node and set val 
  ~node();
  void * get_val(const char *key); /* return val for key */
  void * get_val(); /* return val for current  key */
  char * get_key();
  node * getleft();
  node * getright();
};

class tree {
private:
  node * top;
  void initkeys(node * top, struct tuple data[]);
  node * first();
  node * first(node * n);
public:
  tree(struct tuple data[]);
  ~tree();
  void * get(const char *key);
  void dump_t() { dump_t(top); };
  void dump_t(node * n);
  void dumpval() { dumpval(top); };
  void dumpval(node * n);
  char * getkey(node *n);
  char * getval(node *n);
};

void setit(void *r(), const char *flag, struct file_info *f);

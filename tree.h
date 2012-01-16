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
  void * getval(const char *key); /* return val for key */
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
  void dump() { dump(top); };
  void dump(node * n);
  void dumpval() { dumpval(top); };
  void dumpval(node * n);
  char * getkey(node *n);
};



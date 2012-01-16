
/*
 *
 *  read and write to an intermediate buffer
 *
 *
 */
 
#ifndef __BUFFER__
#define __BUFFER__

/* #define BUFSIZ 512 */

enum mode {Creat, Append};

class buffer {
  protected:
    char * bytes;
    int read_ptr;
    int num_bytes;		// bytes put in buffer
    int size;			// total size of buffer
  public:
    virtual void PutByte(const char c);
    void PutLine(const char * l);	/* newline on end */
    void PutString(const char * l);	/* no newline */
    void PutChar(const char c) {PutByte(c);}		/* ascii */
    void Put10(const int num);        /* format decimal */
    void Put16(const int num);        /* format hexidecimal */
    void PutF(const double num, const int res);      /* format double */
    virtual unsigned char GetByte();
    virtual unsigned char unGet(const char c);
    char * GetLine(char *buf, int buflen);
    int Tell();
    virtual int Seek(const long offset, const int how);
    virtual void dump(const char *fname, const mode mode);
};

enum How { rew =0, cur=1, end=2};  // for Seek 


#endif /* __BUFFER__ */

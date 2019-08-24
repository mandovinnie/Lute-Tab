#pragma once

/*
 * a tmp io buffer
 */

#ifndef __I_BUF__
#define __I_BUF__

#include "buffer.h"

struct link {
    char * bytes;
    struct link * next;
    struct link * prev;
};

class i_buf : public buffer {
    link *start;
    void more_buffer();
    struct link *buf_l;
  public:
    i_buf();
    ~i_buf();
    void dump(const char *fname, const mode mode);
    void PutByte(const char c);
    void SignedPair(unsigned short c);
    void SignedTrio(int c);
    void SignedQuad(int c);
    unsigned char GetByte();
    unsigned char unGet(const char c);
    char * GetLine(char *buf, int buflen);
    int Seek(const long offset, const int how);
};

#endif /* __I_BUF__ */

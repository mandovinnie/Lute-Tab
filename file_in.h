//
//  to input a file to memory, and hand out a byte at a time
//

#ifndef FILE_IN_H_INCLUDED
#define FILE_IN_H_INCLUDED

#include "buffer.h"

class file_in : public buffer {
    char * fn;
    void _file_in(const char *fname, const char *mode);
  public:
    unsigned char GetByte();
    unsigned char unGet(const char c);
    char * GetLine(char * buf , int buflen);

    file_in();
    file_in(const char *fname, const char *mode);
    ~file_in();
    int Get2Bytes();
    int Get3Bytes();
    int Get4Bytes();
    int curLine;
    void Skip(const long offset);
    int Seek(const long offset, const int how);
};


#endif // FILE_IN_H_INCLUDED


#include "tab.h"
#include "i_buf.h"
int my_min(int a, int b);
short my_create(const char *name, int m_mode);

i_buf::i_buf()
{
    dbg0(Proceedure, "starting i_buf");

    start = new link;
    if ( start == NULL)
      dbg0(Error, "i_buf, new failed\n");
    start->prev = NULL;
    start->next = NULL;
    start->bytes = bytes = /* new char [BUFSIZ] */ (char *)malloc(BUFSIZ);

    buf_l = start;

    read_ptr = 0;
    size = BUFSIZ;
    num_bytes = 0;

    dbg2(Proceedure, "ptr %d size %d\n", 
	 (void *)read_ptr, (void *)size);
    //    dbg3(Warning, "starting i_buf ptr %d size %d bufsiz %d\n", 
    //	 (void *)read_ptr, (void *)size, (void *)BUFSIZ);

}

void i_buf::more_buffer()
{
    dbg0(Flow, "more_buffer: \n"); 
    // dbg1(Warning, "more_buffer: size %d", (void *) size); 

    buf_l->next = new link;
    if (buf_l->next  == NULL)
      dbg0(Error, "more, new failed\n");
    //    dbg2(Warning, "more_buffer: size %d, addr %X", 
    //	 (void *) size, (void *)buf_l->next); 
    buf_l->next->prev = buf_l;
    buf_l->next->next = NULL;
    buf_l->next->bytes = (char *) malloc(BUFSIZ);
    if (buf_l->next->bytes  == NULL)
      dbg0(Error, "more, new failed\n");
    buf_l = buf_l->next;
    bytes = buf_l->bytes;
    size +=BUFSIZ;
    //    dbg1(Warning, "new size %d\n", (void *) size); 
}

i_buf::~i_buf()
{
    dbg2(Proceedure, "ending i_buf ptr %d size %d\n", 
	 (void *)read_ptr, (void *)size);
    //    dbg2(Warning, "ending i_buf ptr %d size %d\n", 
    //	 (void *)read_ptr, (void *)size);
}

void i_buf::PutByte(const char c)
{
    if (read_ptr >= size) {
	dbg1(Flow, "PutByte: size %d\n", (void *)read_ptr);
	more_buffer();
    }

    bytes[read_ptr%BUFSIZ] = c;

    //        dbg2(Warning, "put_byte: size %d %c\n", 
    //	     (void *)read_ptr, (void *)c);
    
    read_ptr++;
    num_bytes++;
    return;
}

unsigned char i_buf::GetByte()
{
    unsigned char c;

    //    dbg2(Warning, "i_buf: GetByte: read_ptr %d  size %d",
    //	 (void *)read_ptr,
    //	 (void *)size);
	
    if (read_ptr >= num_bytes) {
	/* eof jump here */
	return((unsigned char )EOF);
    }

    if (read_ptr >= size /* wbc july 96 - 1 */) {
	dbg1(Error, "i_buf: GetByte: overflow %d\n", (void *)read_ptr);
	return (0);
    }
    buf_l = start;

    if (read_ptr >= BUFSIZ) {
	int j = read_ptr/BUFSIZ;
	while (j-- && buf_l->next) {
	    buf_l = buf_l->next;
	}
    }

    c = buf_l->bytes[read_ptr%BUFSIZ];
    //    dbg2(Warning, " c %c %d\n", (void *)c, (void *)c);
    read_ptr++;

    return (c);
}

unsigned char i_buf::unGet(const char c)
{
    read_ptr--;
    buf_l = start;
//    fprintf(stderr, "unGet %d\n", read_ptr);
    if (read_ptr >= BUFSIZ) {
	int j = read_ptr/BUFSIZ;
	while (j-- && buf_l->next) {
	    buf_l = buf_l->next;
	}
    }
    buf_l->bytes[read_ptr%BUFSIZ] = c;
    return (c);
}
 

char * i_buf::GetLine(char *buf, int buflen)
{
    return (buffer::GetLine(buf, buflen));
}


//void i_buf::PutLine(char * l)
//{
//    buffer::PutLine(l);
//}

void i_buf::SignedQuad(int c)
{
    PutByte((unsigned)((c & 0xff000000) >> 24 & 0xff));
    PutByte((unsigned)((c & 0xff0000) >> 16 & 0xff));
    PutByte((unsigned)((c & 0xff00) >> 8 & 0xff));
    PutByte((unsigned)((c & 0xff) & 0xff));
}

void i_buf::SignedTrio(int c)
{
    PutByte((unsigned)(c & 0xff0000) >> 16 & 0xff);
    PutByte((unsigned)(c & 0xff00) >> 8 & 0xff);
    PutByte((unsigned)(c & 0xff) & 0xff);
}

void i_buf::SignedPair(unsigned short c)
{
    PutByte((unsigned)(c & 0xff00) >> 8 & 0xff);
    PutByte((unsigned)(c & 0xff) & 0xff);
}

int i_buf::Seek(const long offset, const int how)
{

    switch (how) {
      case rew:
	if (offset > BUFSIZ)
	  dbg2(Error, "Seek cant do rew offset %u how %d\n", 
	 (void *)offset, (void *)how);
	buf_l = start;
	bytes = buf_l->bytes;
	read_ptr = offset;
	break;
      case cur:
	dbg2(Error, "Seek cant do cur offset %u how %d\n", 
	 (void *)offset, (void *)how);
	read_ptr += offset;
	break;
      case end:
	dbg2(Error, "Seek cant do endoffset %u how %d\n", 
	 (void *)offset, (void *)how);
	read_ptr = size - offset;
	break;
    }
    if (read_ptr > size) return (0);
    if (read_ptr < 0)    return (0);
    return (1);
}


int my_min(int a, int b)
{
    return( a < b ? a:b);
}

void i_buf::dump(const char *fname, const mode mode)	// dump to a file
{
    dbg2(Flow, "Dump; writing %s  size %d\n", 
	 (void *)fname, (void *)num_bytes);
	 
#ifdef MAC
    short refNum;
    OSErr error;
    int n_written, file_bytes;
    long int wsize;
    int m_mode;
       
    Seek(rew, 0);
    if (mode == Creat) m_mode = 1;
    else if (mode == Append) m_mode = 2;
    file_bytes = num_bytes;
    refNum = my_create(fname, m_mode);
    dbg1(Flow, "i_buf: dump: %s", (void *)fname);
    while (file_bytes > 0) {
    	wsize = my_min(file_bytes, BUFSIZ);
    	error = FSWrite(refNum, &wsize, buf_l->bytes);
		if (wsize == BUFSIZ && buf_l->next) buf_l = buf_l->next;
		file_bytes -= wsize;
    }	
	error = FSClose(refNum);
    dbg1(Warning, "i_buff::dump: done%c", (void *)NEWLINE);
#else /* not  MAC */
    static FILE *fp;
    int n_written, file_bytes;


    Seek(rew, 0);

    if (!strcmp(fname, "stdout")) {
      if (mode == Creat) 
	fp = fdopen(1, "wb");
      //      else if (mode == Append) {
      //	fp = fdopen(1, "ab");
      //      }
    }
    else {
      if (mode == Creat) 
	fp = fopen(fname, "wb");
      else if (mode == Append)
	fp = fopen(fname, "ab");
    }

    if (fp == NULL) {
	dbg1(Error, "tab: dump: can't open %s for output\n", (void *)fname);
    }

    file_bytes = num_bytes;
    while (file_bytes > 0) {
	n_written = fwrite(buf_l->bytes, 1, my_min(file_bytes, BUFSIZ), fp);
	if (buf_l->next) buf_l = buf_l->next;
	file_bytes -= n_written;
	dbg1(Flow, "%u bytes left\n", (void *)file_bytes);
    }
    if (strcmp(fname, "stdout"))
      fclose(fp);
    else if (mode == Append)fclose(fp);
#endif
}



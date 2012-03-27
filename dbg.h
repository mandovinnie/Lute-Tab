enum dbg_type {File = 1, Path = 2, TFM = 4, Inter = 8,
         Flow=16, Warning=32, Error = 64, Stack=128,
         Proceedure=256, Fonts=512, Bug=1024, Widths=2048,
         z=4098, xx=8192, yy = 16384, Null =32768 };

void dbg_set(const dbg_type type);

void dbg5(const int type, const char *fmt, 
	  void *a, void *b, void *c, void *d, void *e);
void dbg4(const int type, const char *fmt, 
	  void *a, void *b, void *c, void *d);
void dbg3(const int type, const char *fmt, 
	  void *a, void *b, void *c);
void dbg2(const int type, const char *fmt, 
	  void *a, void *b);
void dbg1(const int type, const char *fmt, 
	  void *a);
void dbg0(const int type, const char *fmt);


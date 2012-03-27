/*

 */

typedef struct {
    int bm_char;		/* char number in font */
    int bm_w;			/* width in pixels */
    int bm_h;			/* height in pixels */
    int bm_h_off;		/* hor offset to origin */
    int bm_v_off;		/* vert offset to origin */
    char *bm_bits;		/* bitmap l to r,  bot to top */
} PKBit;


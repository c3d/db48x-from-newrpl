#ifndef _GGL_H
#define _GGL_H

#define LCD_H 80
#define LCD_W 160

// internal buffer for hblt routines

#define HBLT_BUFFER 64        // default 64 words = 512 pixels



// data types/structures


// surface used for bitblt operations
// notes:
// a surface is infinite in both width and height, there are no memory limits
// .addr must be a word aligned address
// .width is used only to find a new scanline, and can be changed at will
//        the width is given in pixels and it can be arbitrary (no alignement needed)
// for normal drawing primitives, (0,0) is the word-aligned address pointed by .addr,
// disregarding of the values in .x and .y
// for bitblt operations, .x and .y give the origin (top-left corner) of the region to use
// the surface is nibble-aligned, so a 1 pixel wide surface will contain 8
// rows of pixels per word

typedef struct {
    int *addr;  //! Word-aligned address of the surface buffer
    int width;  //! Width (in pixels) of the buffer
    int x,y;    //! Offset coordinates within the buffer
    int clipx,clipx2,clipy,clipy2;

} gglsurface;

typedef unsigned int (*gglfilter)(unsigned int pixels,int param);

typedef unsigned int (*ggloperator)(unsigned int dest,unsigned int source,int param);


// inline routines

// THIS IS PLATFORM INDEPENDENT ROTATION
// IN ARM, A<<B WITH B>=32 = ZERO
// IN X86, A<<B WITH B>=32 = A<<(B&31)

#define ROT_LEFT(a,b) ( ((b)>=32)? 0:((a)<<(b)))
#define ROT_RIGHT(a,b) ( ((b)>=32)? 0:((a)>>(b)))



#define ggl_leftmask(cx) ((ROT_LEFT(1,(( (cx)&7)<<2))-1))    // create mask
#define ggl_rightmask(cx) (ROT_LEFT((-1),((((cx)&7)+1)<<2)))    // create mask


void ggl_initscr(gglsurface *surface);

// drawing primitives
// general pixel set/read routines

void ggl_pltnib(int *buff,int off,int color);    // poke a pixel (off in nibbles)
int  ggl_getnib(int *buff,int off);                // peek a pixel (off in nibbles)

// general drawing primitives

// note: the argument color is a 32-bit value containing a different
//       color for each pixel. For solid colors, set color to contain the same value
//       on every nibble (for color 8, color=0x88888888)
//       or call ggl_mkcolor for that purpose

void ggl_hline(gglsurface *srf,int y,int xl,int xr, int color); // fast low-level horizontal line
void ggl_vline(gglsurface *srf,int x,int yt,int yb, int color); // fast low-level vertical line
void ggl_rect(gglsurface *srf,int x1,int y1,int x2,int y2,int color); // low-level rectangle
void ggl_cliprect(gglsurface *srf,int x1,int y1,int x2,int y2,int color); // low-level rectangle

void ggl_rectp(gglsurface *srf,int x1,int y1,int x2,int y2,int *color); // low-level rectangle with 8x8 pattern

// bit-blit functions

// LOW-LEVEL row copy functions
// ggl_ll_hblt is a general nibble-aligned memcpy
// WARNING: npixels is limited to 512!!
//          if you need more than that, increase the constant HBLT_BUFFER above
//          and RE-COMPILE the ggl library
// dest and src must be word-aligned
// destoff and srcoff are offsets in nibbles from the word-aligned pointers
// npixels is the number of nibbles to copy
// note: hblt will behave well even if the zones overlap, no need for moveup/movedown

void ggl_hblt(int *dest,int destoff,int *src,int srcoff, int npixels); // copy a row of pixels

// same behavior as hblt but specifying a transparent color
// every pixel in *src with the transparent color will not affect the
// corresponding pixel in *dest
void ggl_hbltmask(int *dest,int destoff,int *src,int srcoff, int npixels, int tcol); // copy a row of pixels w/mask

// rectangle blt
// note: see gglsurface above for complete understanding of the behavior of these routines
// ggl_bitblt loops from top to bottom
void ggl_bitblt(gglsurface *dest,gglsurface *src,int width, int height); // copy a rectangular region
// ggl_revblt loops from bottom to top, for overlapping zones
void ggl_revblt(gglsurface *dest,gglsurface *src,int width, int height); // copy a rectangular region, reverse loop
// ggl_ovlblt chooses to use normal/reverse loop based on the addresses
// use it when the direcction of movement is unknown
void ggl_ovlblt(gglsurface *dest,gglsurface *src,int width, int height); // copy overlapped regions
// ggl_bitbltmask behaves exactly as ggl_bitblt but using tcol as a transparent color
#define ggl_bitbltmask(dest,src,width,height,tcol)  ggl_bitbltoper(dest,src,width,height,tcol,&ggl_opmask)


// rectangle scrolling routines
// dest contains the surface to scroll, and width and height define the rectangle
// the area that needs to be redrawn after the scroll is not erased or modified by these routines 
void ggl_scrollup(gglsurface *dest,int width, int height, int npixels); // scroll npixels up
void ggl_scrolldn(gglsurface *dest,int width, int height, int npixels); // scroll npixels dn
void ggl_scrolllf(gglsurface *dest,int width, int height, int npixels); // scroll npixels left
void ggl_scrollrt(gglsurface *dest,int width, int height, int npixels); // scroll npixels right

// custom filters and operators

// low-level row filtering routine
void ggl_hbltfilter(int *dest,int destoff, int npixels, int param,gglfilter filterfunc);
// bitmap filtering routine
void ggl_filter(gglsurface *dest,int width, int height, int param, gglfilter filterfunc);

// low-level row operator routine
void ggl_hbltoper(int *dest,int destoff,int *src,int srcoff, int npixels, int param,ggloperator foperator);
// bitblt operator routine
void ggl_bitbltoper(gglsurface *dest,gglsurface *src,int width, int height,int param,ggloperator fop);

// predefined filters and operators

// filters (unary operators)
// ligthens an image by subtracting param from all pixels
unsigned ggl_fltlighten(unsigned word,int param);
// darkens an image by adding param to all pixels
unsigned ggl_fltdarken(unsigned word,int param);

// operators (between two surfaces)
// standard mask, tcolor in src is considered transparent
unsigned ggl_opmask(unsigned dest,unsigned src,int tcolor);
// transparency blend, weight is 0 = src is opaque, 16 = src is fully transparent
unsigned ggl_optransp(unsigned dest,unsigned src,int weight);
// standard mask, tcolor in src is considered transparent, black color in src is AND with newcolor
unsigned ggl_opmaskcol(unsigned dest,unsigned src,int tcolor, int newcolor);



// miscellaneous

// ggl_mkcolor repeats the same color on every nibble
// ggl_mkcolor(2) will return 0x22222222
int ggl_mkcolor(int color); // solid color generator

// ggl_mkcolor32 creates virtual 32-colors by using 8x8 patterns
// col32 is a value from 0 to 30, being 30=black, 0=white
// note: the user is responsible to provide a valid int[8] buffer in the
// pattern argument
void ggl_mkcolor32(int col32, int *pattern);    // 50% dither pattern generator for 31 colors
    
#endif

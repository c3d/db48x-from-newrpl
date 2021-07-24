/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef _CGL_H
#define _CGL_H

#define LCD_H SCREEN_H
#define LCD_W SCREEN_W

// internal buffer for hblt routines

#define HBLT_BUFFER 64  // default 64 words = 512 pixels

// The CGL library is a close drop-in replacement to replace the GGL (Gray Graphics Library)
// which was fixed to 4-bits per pixel
// CGL library has cgl_xxx functions that are fully compatible with GGL and will map the 16 grays
// to a proper color. Code designed to run on GGL should work correctly with the screen in color mode.
// cgl_xxx functions are equivalent but work with full color.

// Data structures and API are meant to be compatible with GGL, so they need to be very similar for
// ease of porting.

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
// the surface is PIXEL-aligned, so there's multiple pixels per word, and a scanline
// may start misaligned. Use a proper .width if each scanline needs to be word aligned

typedef struct
{
    int *addr;  //! Word-aligned address of the surface buffer
    int width;  //! Width (in pixels) of the buffer
    int x, y;   //! Offset coordinates within the buffer
    int clipx, clipx2, clipy, clipy2;
    int actbuffer;   //! Active buffer: 0 or 1
} gglsurface;

typedef unsigned int (*gglfilter)(unsigned int pixels, int param);

typedef unsigned int (*ggloperator)(unsigned int dest, unsigned int source,
        int param);

// inline routines

// THIS IS PLATFORM INDEPENDENT ROTATION
// IN ARM, A<<B WITH B>=32 = ZERO
// IN X86, A<<B WITH B>=32 = A<<(B&31)

#define ROT_LEFT(a,b) ( ((b)>=32)? 0:(((unsigned)a)<<(b)))
#define ROT_RIGHT(a,b) ( ((b)>=32)? 0:((a)>>(b)))

#define cgl_leftmask(cx) ((ROT_LEFT(1,(( (cx)&7)<<2))-1))       // create mask
#define cgl_rightmask(cx) (ROT_LEFT((-1),((((cx)&7)+1)<<2)))    // create mask

#define ggl_initscr cgl_initscr
void cgl_initscr(gglsurface * surface);

// drawing primitives
// general pixel set/read routines

void ggl_pltnib(int *buff, int off, int color); // poke a pixel (off in nibbles)
int ggl_getnib(int *buff, int off);     // peek a pixel (off in nibbles)
int ggl_getmonopix(char *buf, int off); // peek a pixel in monochrome bitmap (off in pixels)

// general drawing primitives

// note: the argument color is a 32-bit value containing a different
//       color for each pixel. For solid colors, set color to contain the same value
//       on every nibble (for color 8, color=0x88888888)
//       or call cgl_mkcolor for that purpose

#define ggl_hline cgl_hline
void cgl_hline(gglsurface * srf, int y, int xl, int xr, int color);     // fast low-level horizontal line
#define ggl_cliphline cgl_cliphline
void cgl_cliphline(gglsurface * srf, int y, int xl, int xr, int color);
#define ggl_vline cgl_vline
void cgl_vline(gglsurface * srf, int x, int yt, int yb, int color);     // fast low-level vertical line
#define ggl_clipvline cgl_clipvline
void cgl_clipvline(gglsurface * srf, int x, int yt, int yb, int color);
#define ggl_rect  cgl_rect
void cgl_rect(gglsurface * srf, int x1, int y1, int x2, int y2, int color);     // low-level rectangle
#define ggl_cliprect cgl_cliprect
void cgl_cliprect(gglsurface * srf, int x1, int y1, int x2, int y2, int color); // low-level rectangle

void cgl_rectp(gglsurface * srf, int x1, int y1, int x2, int y2, int *color);   // low-level rectangle with 8x8 pattern

// bit-blit functions

// LOW-LEVEL row copy functions
// cgl_ll_hblt is a general nibble-aligned memcpyb
// WARNING: npixels is limited to 512!!
//          if you need more than that, increase the constant HBLT_BUFFER above
//          and RE-COMPILE the ggl library
// dest and src must be word-aligned
// destoff and srcoff are offsets in nibbles from the word-aligned pointers
// npixels is the number of nibbles to copy
// note: hblt will behave well even if the zones overlap, no need for moveup/movedown

void cgl_hblt(int *dest, int destoff, int *src, int srcoff, int npixels);       // copy a row of pixels

// same behavior as hblt but specifying a transparent color
// every pixel in *src with the transparent color will not affect the
// corresponding pixel in *dest
void cgl_hbltmask(int *dest, int destoff, int *src, int srcoff, int npixels, int tcol); // copy a row of pixels w/mask

// rectangle blt
// note: see gglsurface above for complete understanding of the behavior of these routines
// cgl_bitblt loops from top to bottom
#define ggl_bitblt cgl_bitblt
void cgl_bitblt(gglsurface * dest, gglsurface * src, int width, int height);    // copy a rectangular region
// cgl_revblt loops from bottom to top, for overlapping zones
void cgl_revblt(gglsurface * dest, gglsurface * src, int width, int height);    // copy a rectangular region, reverse loop
// cgl_ovlblt chooses to use normal/reverse loop based on the addresses
// use it when the direcction of movement is unknown
void cgl_ovlblt(gglsurface * dest, gglsurface * src, int width, int height);    // copy overlapped regions
// cgl_bitbltmask behaves exactly as cgl_bitblt but using tcol as a transparent color
#define cgl_bitbltmask(dest,src,width,height,tcol)  cgl_bitbltoper(dest,src,width,height,tcol,(ggloperator)&cgl_opmask)
#define ggl_monobitbltmask cgl_monobitbltmask
#define cgl_monobitbltmask(dest,src,width,height,tcol)  cgl_monobitbltoper(dest,src,width,height,tcol,(ggloperator)&cgl_opmask)

#define ggl_bitbltclip cgl_bitbltclip
void cgl_bitbltclip(gglsurface * dest, gglsurface * src, int width, int height);        // copy a rectangular region, clipped within dest

// rectangle scrolling routines
// dest contains the surface to scroll, and width and height define the rectangle
// the area that needs to be redrawn after the scroll is not erased or modified by these routines
void cgl_scrollup(gglsurface * dest, int width, int height, int npixels);       // scroll npixels up
void cgl_scrolldn(gglsurface * dest, int width, int height, int npixels);       // scroll npixels dn
void cgl_scrolllf(gglsurface * dest, int width, int height, int npixels);       // scroll npixels left
void cgl_scrollrt(gglsurface * dest, int width, int height, int npixels);       // scroll npixels right

// custom filters and operators

// low-level row filtering routine
void cgl_hbltfilter(int *dest, int destoff, int npixels, int param,
        gglfilter filterfunc);
// bitmap filtering routine
#define ggl_filter cgl_filter
void cgl_filter(gglsurface * dest, int width, int height, int param,
        gglfilter filterfunc);

// low-level row operator routine
void cgl_hbltoper(int *dest, int destoff, int *src, int srcoff, int npixels,
        int param, ggloperator foperator);
// low-level row operator routine for monochrome bitmaps
void cgl_monohbltoper(int *dest, int destoff, unsigned char *src, int srcoff,
        int npixels, int param, ggloperator foperator);
// bitblt operator routine
void cgl_bitbltoper(gglsurface * dest, gglsurface * src, int width, int height,
        int param, ggloperator fop);
// bitblt operator routine for monochrome bitmaps
#define ggl_monobitbltoper cgl_monobitbltoper
void cgl_monobitbltoper(gglsurface * dest, gglsurface * src, int width,
        int height, int param, ggloperator fop);

// predefined filters and operators

// filters (unary operators)
// ligthens an image by subtracting param from all pixels
#define ggl_fltlighten cgl_fltlighten
unsigned cgl_fltlighten(unsigned word, int param);
// darkens an image by adding param to all pixels
unsigned cgl_fltdarken(unsigned word, int param);
// invert the colors on all pixels
#define ggl_fltinvert cgl_fltinvert
unsigned cgl_fltinvert(unsigned word, int param);
// replace a color with another
#define ggl_fltreplace cgl_fltreplace
unsigned cgl_fltreplace(unsigned word, int param);

// operators (between two surfaces)
// standard mask, tcolor in src is considered transparent
unsigned cgl_opmask(unsigned dest, unsigned src, unsigned tcolor);
// transparency blend, weight is 0 = src is opaque, 16 = src is fully transparent
unsigned cgl_optransp(unsigned dest, unsigned src, int weight);
// standard mask, tcolor in src is considered transparent, white color in src is replaced with newcolor
#define ggl_opmaskcol cgl_opmaskcol
unsigned cgl_opmaskcol(unsigned dest, unsigned src, unsigned tcolor, unsigned newcolor);

// ggl_mkcolor repeats the same color on every nibble
// ggl_mkcolor(2) will return 0x22222222

// cgl_getcolor takes a system palette index color and expands to an actual RGB16 color
// for values 0-15 the system palette must match grayscale levels for compatibility
#define ggl_mkcolor cgl_mkcolor
#define cgl_mkcolor(color) (cgl_palette[(color)&PALETTEMASK])


// Set a palette index entry
#define cgl_setpalette(index,color) { cgl_palette[(index)&PALETTEMASK]=(color); }

// Return the actual color to draw, either the given color or a palette color
#define cgl_getcolor(color) (((color)&IS_PALETTE_COLOR)? cgl_palette[(color)&PALETTEMASK]: (color))


#endif

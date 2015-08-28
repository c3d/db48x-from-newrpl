/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include <newrpl.h>
#include <ui.h>


void __keyb_waitrelease();
int __keyb_getkey(int wait);

extern const unsigned int Font_6A[];
extern unsigned int RPLLastOpcode;

void __ex_print(int x,int y,char *str)
{
    DRAWSURFACE dr;
    dr.addr=(int *)MEM_PHYS_EXSCREEN;
    dr.width=LCD_W;
    dr.x=dr.y=0;
    dr.clipx=dr.clipy=0;
    dr.clipx2=SCREEN_WIDTH;
    dr.clipy2=SCREEN_HEIGHT;

    DrawTextMono(x,y,str,(UNIFONT *)Font_6A,1,&dr);
}

void __ex_clrscreen()
{
    int *ptr=(int *)MEM_PHYS_EXSCREEN,*end=ptr+400;
    while(ptr!=end) *ptr++=0;
}

void __ex_hline(int y)
{
    int *yptr=((int *)MEM_PHYS_EXSCREEN)+5*y;
    yptr[0]=yptr[1]=yptr[2]=yptr[3]=yptr[4]=0xaaaaaaaa;
}

int __ex_width(char *string) { return StringWidth(string,(UNIFONT *)Font_6A); }

// MAIN EXCEPTION PROCESSOR

#define __EX_CONT 1		// SHOW CONTINUE OPTION
#define __EX_EXIT 2		// SHOW EXIT OPTION
#define __EX_WARM 4		// SHOW WARMSTART OPTION
#define __EX_RESET 8	// SHOW RESET OPTION
#define __EX_NOREG 16	// DON'T SHOW REGISTERS
#define __EX_WIPEOUT 32	// FULL MEMORY WIPEOUT AND WARMSTART
#define __EX_RPLREGS 64 // SHOW RPL REGISTERS INSTEAD


int __exception_handler(char *exstr, unsigned int *registers,int options)
{
UNUSED_ARGUMENT(registers);

unsigned int lcd_buffer[17];
char a[10];

int f,j;


lcd_save(lcd_buffer);
lcd_setmode(0,(unsigned int *)MEM_PHYS_EXSCREEN);

doitagain:

__ex_clrscreen();

if(options&__EX_NOREG) {

    __ex_print(36,12,"-- EXCEPTION --");
    __ex_hline(8);
    __ex_hline(20);
	
    __ex_print(65-(__ex_width(exstr)>>1),30,exstr);


}
else {
    __ex_print(0,0,"Exception: ");
    __ex_print(44,0,exstr);

    // ALWAYS SHOW RPL CORE INSTEAD OF REGISTERS IN PC TARGET
        __ex_hline(8);
    // SHOW RPL CORE INFORMATION INSTEAD
        __ex_print(0,12,"IP: ");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)IPtr)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(16,12,a);

        __ex_print(0,18,"OPC:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)RPLLastOpcode)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(16,18,a);

        __ex_print(0,24,"TOe:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)TempObEnd)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(16,24,a);

        __ex_print(0,30,"TOs:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)TempObSize)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(16,30,a);

        __ex_print(0,36,"TBe:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)TempBlocksEnd)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(16,36,a);

        __ex_print(0,42,"TBs:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)TempBlocksSize)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(16,42,a);

        __ex_print(0,48,"RSe:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)RSTop)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(16,48,a);

        __ex_print(0,54,"RSs:");

        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)RStkSize)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(16,54,a);

        __ex_print(0,60,"DSe:");

        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)DSTop)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(16,60,a);


    __ex_hline(70);
		
}

if(options&__EX_CONT) {
        int *pnewb=(int *)MEM_PHYS_EXSCREEN;
		// DRAW BUTTON 1
        __ex_print(0,12*6,"Cont");
		//pnewb[70*5]|=0x10000;
		for(f=0;f<8;++f) pnewb[71*5+5*f]|=0x20000;
		pnewb[78*5]|=0x3ffff;
		pnewb[79*5]|=0x3fffc;
}

if(options&__EX_EXIT) {
        int *pnewb=(int *)MEM_PHYS_EXSCREEN;

		// DRAW BUTTON 2
        __ex_print(5*4,12*6,"Exit");
		for(f=0;f<8;++f) pnewb[71*5+1+5*f]|=0x20;
		pnewb[78*5]|=0xfff80000;
		pnewb[78*5+1]|=0x3f;
		pnewb[79*5]|=0xffe00000;
		pnewb[79*5+1]|=0x3f;
}

if(options&__EX_WARM) {
        int *pnewb=(int *)MEM_PHYS_EXSCREEN;

		// DRAW BUTTON 3
        if(options&__EX_WIPEOUT) __ex_print(11*4,12*6,"*Clear Mem*");
            else __ex_print(11*4,12*6,"*Warmstart*");
		for(f=0;f<8;++f) pnewb[71*5+2+5*f]|=0x2000000;
		pnewb[78*5+2]|=0x3ffffff;
		pnewb[78*5+1]|=0xfffff000;
		pnewb[79*5+2]|=0x3ffffff;
		pnewb[79*5+1]|=0xffffc000;
}

if(options&__EX_RESET) {
        int *pnewb=(int *)MEM_PHYS_EXSCREEN;
		// DRAW BUTTON 4
        __ex_print(23*4,12*6,"**Reset**");
		for(f=0;f<9;++f) pnewb[71*5+4+5*f]|=0x1;
		pnewb[78*5+3]|=0xffffffff;
		pnewb[78*5+2]|=0xf0000000;
		pnewb[79*5+3]|=0xffffffff;
		pnewb[79*5+2]|=0xc0000000;
}
		
// WARMSTART AND RESET REQUIRE SIMULTANEOUS SHIFT OR ALPHA KEYPRESS

// WAIT FOR ALL KEYS TO BE RELEASED TO AVOID ACCIDENTAL KEYPRESSES

__keyb_waitrelease();


do {
f=__keyb_getkey(1);

if(options&__EX_CONT) {
j=__EX_CONT;
if( KEYVALUE(f)==KB_A )	break;
}
if(options&__EX_EXIT) {
j=__EX_EXIT;
if( KEYVALUE(f)==KB_B )	break;
}
if( KEYVALUE(f)==KB_L) {
    options^=__EX_RPLREGS;
    options&=~__EX_NOREG;
    goto doitagain;
}
// FORCE A SHIFTED KEY PRESS
if(!KEYSHIFT(f)) continue;


if(options&(__EX_WARM|__EX_WIPEOUT)) {
    if((options&__EX_WIPEOUT) && (KEYSHIFT(f)==(SHIFT_ALPHA|SHIFT_ALPHAHOLD|SHIFT_RS|SHIFT_RSHOLD|SHIFT_LS|SHIFT_LSHOLD))) j=__EX_WIPEOUT;
    else j=__EX_WARM;
if( KEYVALUE(f)==KB_C )	break;
if( KEYVALUE(f)==KB_D )	break;
}

if(options&__EX_RESET) {
j=__EX_RESET;
if( KEYVALUE(f)==KB_E )	break;
if( KEYVALUE(f)==KB_F )	break;
}
} while(1);

__keyb_waitrelease();
lcd_restore(lcd_buffer);

return j;

}

void __exception_install()
{

}

void __attribute__ ((noinline)) throw_exception(char * message, unsigned int options)
{
    int value;

    value=__exception_handler((char *)message,NULL,options | __EX_NOREG);

    if(value==__EX_RESET) {
        // TODO: RESET ON THE PC TARGET

    }
    if(value==__EX_WARM) {
        // TODO: WARMSTART ON THE PC TARGET
    }
    if(value==__EX_WIPEOUT) {
        // TODO: WIPEOUT ON THE PC TARGET


    }
}

void __attribute__ ((noinline)) throw_dbgexception(char * message, unsigned int options)
{
    int value;

    value=__exception_handler((char *)message,NULL,options);
    if(value==__EX_RESET) {
        // TODO: RESET ON THE PC TARGET

    }
    if(value==__EX_WARM) {
        // TODO: WARMSTART ON THE PC TARGET
    }
    if(value==__EX_WIPEOUT) {
        // TODO: WIPEOUT ON THE PC TARGET


    }

}

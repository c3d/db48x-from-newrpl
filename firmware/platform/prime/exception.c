/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <hal_api.h>
#include <ui.h>
#include "recorder.h"

RECORDER(exceptions, 16, "System exceptions");

void keyb_irq_waitrelease();
int keyb_irq_getkey(int wait);

extern const unsigned int Font_10A[];
extern unsigned int RPLLastOpcode;

// EXCEPTIONS SCREEN AREA RIGHT AFTER THE SCREEN, RESERVE SPACE FOR 16-BIT COLOR MODE
#define MEM_SCREENSIZE ((((*VIDTCON2)>>11)+1)*(((*VIDW00ADD2B0)&0x1fff)+((*VIDW00ADD2B0)>>13)))
#define MEM_PHYS_EXSCREEN (MEM_PHYS_SCREEN+2*MEM_SCREENSIZE)
#define FNT_HEIGHT 12
#define FNT_AVGW   7
#define BTN_WIDTH  52

void ex_print(int x,int y,char *str)
{
    DRAWSURFACE dr;
    dr.addr=(int *)MEM_PHYS_EXSCREEN;
    dr.width=LCD_W;
    dr.x=dr.y=0;
    dr.clipx=dr.clipy=0;
    dr.clipx2=SCREEN_WIDTH;
    dr.clipy2=SCREEN_HEIGHT;

    DrawText(x,y,str,(UNIFONT *)Font_10A,cgl_mkcolor(PAL_GRAY15),&dr);
}

void ex_clrscreen()
{
    DRAWSURFACE dr;
    dr.addr=(int *)MEM_PHYS_EXSCREEN;
    dr.width=LCD_W;
    dr.x=dr.y=0;
    dr.clipx=dr.clipy=0;
    dr.clipx2=SCREEN_WIDTH;
    dr.clipy2=SCREEN_HEIGHT;
    cgl_rect(&dr,dr.x,dr.y,dr.clipx2-1,dr.clipy2-1,cgl_mkcolor(PAL_GRAY0));
}

void ex_hline(int y)
{
    DRAWSURFACE dr;
    dr.addr=(int *)MEM_PHYS_EXSCREEN;
    dr.width=LCD_W;
    dr.x=dr.y=0;
    dr.clipx=dr.clipy=0;
    dr.clipx2=SCREEN_WIDTH;
    dr.clipy2=SCREEN_HEIGHT;
    cgl_hline(&dr,y,dr.x,dr.clipx2-1,cgl_mkcolor(PAL_GRAY8));
}

inline int ex_width(char *string) { return StringWidth(string,(UNIFONT *)Font_10A); }

// GET HIGH REGISTERS R8 TO R14 + CPSR (8 WORDS)

ARM_MODE void ex_gethireg(unsigned int *hi_reg)
{
register unsigned int tmp asm ("r3");
register unsigned int tmp2 asm ("r2");

asm volatile ("mrs %0,cpsr" : "=r" (tmp));
asm volatile ("mrs %0,spsr" : "=r" (tmp2));
asm volatile ("bic %0,%1,#0x1f" : "=r" (tmp) : "r" (tmp));		// CLEAN STATE BITS
asm volatile ("and %0,%1,#0x1f" : "=r" (tmp2) : "r" (tmp2));	// ISOLATE ORIGINAL STATUS
asm volatile ("orr %0,%1,%2" : "=r" (tmp) : "r" (tmp), "r" (tmp2) );		// APPLY ORIGINAL STATUS
asm volatile ("cmp %0,#0x10" : "=r" (tmp2));					// CHECK FOR USER MODE
asm volatile ("orreq %0,%1,#0x1f" : "=r" (tmp) : "r" (tmp) );	// IF USER MODE, SWITCH TO SYSTEM MODE
asm volatile ("mrs %0,cpsr" : "=r" (tmp2));
asm volatile ("msr cpsr,%0" : "=r" (tmp));
asm volatile ("stmia r0,{r8,r9,r10,r11,r12,r13,r14}");
asm volatile ("msr cpsr,%0" : "=r" (tmp2));

asm volatile ("mrs %0,spsr" : "=r" (tmp));

hi_reg[7]=tmp;

return;
}


/*
// MAIN EXCEPTION PROCESSOR

#define EX_CONT 1		// SHOW CONTINUE OPTION
#define EX_EXIT 2		// SHOW EXIT OPTION
#define EX_WARM 4		// SHOW WARMSTART OPTION
#define EX_RESET 8	// SHOW RESET OPTION
#define EX_NOREG 16	// DON'T SHOW REGISTERS
#define EX_WIPEOUT 32	// FULL MEMORY WIPEOUT AND WARMSTART
#define EX_RPLREGS 64 // SHOW RPL REGISTERS INSTEAD
#define EX_RPLEXIT 128 // SHOW EXIT OPTION, IT RESUMES EXECUTION AFTER SETTING Exception=EX_EXITRPL
*/
int exception_handler(char *exstr, unsigned int *registers,int options)
{
unsigned int lcd_buffer[17];
unsigned int hi_reg[8];
char a[10];

int f,j;


lcd_save(lcd_buffer);

*WINCON0&=~0x800001;   // Window 0 off, switch to buffer 0
// Move Window 0 to the exception buffer
*VIDW00ADD0B0=MEM_PHYS_EXSCREEN;
*VIDW00ADD1B0=(MEM_PHYS_EXSCREEN+MEM_SCREENSIZE)&0x00ffffff;
*WINCON0|=1;   // Window 0 on


doitagain:

ex_clrscreen();

if(options&EX_NOREG) {

    ex_print(SCREEN_WIDTH/2-7*FNT_AVGW,FNT_HEIGHT*2,"-- EXCEPTION --");
    ex_hline(FNT_HEIGHT*2-4);
    ex_hline(FNT_HEIGHT*3+4);

    ex_print(SCREEN_WIDTH/2-(ex_width(exstr)>>1),FNT_HEIGHT*5,exstr);


}
else {
    if(!(options&EX_MEMDUMP)) {
    ex_print(0,0,"Exception: ");
    ex_print(11*FNT_AVGW,0,exstr);
    }

    if(options&EX_MEMDUMP) {
        ex_print(0,0,"Exception: MEMORY DUMP REQUESTED");
        ex_hline(FNT_HEIGHT*2-4);
    // SHOW MEMORY DUMP INSTEAD
        int i;
        for(i=0;i<10;++i) {
        a[8]=':';
        a[9]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)(exstr+i*8))>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }

        // PRINT OFFSET OF MEMORY DUMP
        ex_print(0,FNT_HEIGHT*2+i*FNT_HEIGHT,a);

        for(f=0;f<8;++f) {
        a[2]=0;
        for(j=1;j>=0;j--)
         {
         a[1-j]=(((exstr[f+i*8])>>(j<<2))&0xf)+48;
          if(a[1-j]>'9') a[1-j]+=7;
         }
        ex_print(10*FNT_AVGW+f*3*FNT_AVGW,FNT_HEIGHT*2+i*FNT_HEIGHT,a);
        }

        }



    }
    else if(options&EX_RPLREGS) {
        ex_hline(FNT_HEIGHT*2-4);
    // SHOW RPL CORE INFORMATION INSTEAD
        ex_print(0,FNT_HEIGHT*2,"IP: ");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)IPtr)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(4*FNT_AVGW,FNT_HEIGHT*2,a);

        ex_print(0,FNT_HEIGHT*3,"OPC:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)RPLLastOpcode)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(4*FNT_AVGW,FNT_HEIGHT*3,a);

        ex_print(0,FNT_HEIGHT*4,"TOe:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)TempObEnd)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(4*FNT_AVGW,FNT_HEIGHT*4,a);

        ex_print(0,FNT_HEIGHT*5,"TOs:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)TempObSize)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(4*FNT_AVGW,FNT_HEIGHT*5,a);

        ex_print(0,FNT_HEIGHT*6,"TBe:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)TempBlocksEnd)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(4*FNT_AVGW,FNT_HEIGHT*6,a);

        ex_print(0,FNT_HEIGHT*7,"TBs:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)TempBlocksSize)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(4*FNT_AVGW,FNT_HEIGHT*7,a);

        ex_print(0,FNT_HEIGHT*8,"RSe:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)RSTop)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(4*FNT_AVGW,FNT_HEIGHT*8,a);

        ex_print(0,FNT_HEIGHT*9,"RSs:");

        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)RStkSize)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(4*FNT_AVGW,FNT_HEIGHT*9,a);

        ex_print(0,FNT_HEIGHT*10,"DSe:");

        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)DSTop)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(4*FNT_AVGW,FNT_HEIGHT*10,a);

        // RIGHT COLUMN

        ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*2,"DSs: ");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)DStkSize)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*2,a);

        ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*3,"DIe:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)DirsTop)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*3,a);

        ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*4,"DIs:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)DirSize)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*4,a);

        ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*5,"LAe:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)LAMTop)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*5,a);

        ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*6,"LAs:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)LAMSize)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*6,a);

        ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*7,"Exc:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)Exceptions)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*7,a);

        ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*8,"Err:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)ErrorCode)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*8,a);


        ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*9,"Um:");

        {
        WORD total=halGetTotalPages();
        WORD freemem=halGetFreePages();
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=(((freemem)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*9,a);


        ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*10,"Tm:");

        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=(((total)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*10,a);
        }

     }
    else {
        // PRINT CPU REGISTERS
	a[2]=0;
	for(f=0;f<8;++f) {
    ex_print(0,f*FNT_HEIGHT+2*FNT_HEIGHT,"R  :");
	a[1]=0;
	a[0]=f+48;
    ex_print(1*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,a);
    ex_print(SCREEN_WIDTH/2,f*FNT_HEIGHT+2*FNT_HEIGHT,"R   :");
	if(f<2) a[0]=f+8+48;
	else { a[0]='1'; a[1]=f-2+48; }
    ex_print(SCREEN_WIDTH/2+1*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,a);
	}

    ex_hline(2*FNT_HEIGHT-4);

	a[8]=0;

	// DISPLAY REGISTERS

	for(f=0;f<8;++f)
	{
		for(j=7;j>=0;j--)
		{
		a[7-j]=((registers[f+1]>>(j<<2))&0xf)+48;
		if(a[7-j]>'9') a[7-j]+=7;
		}
        ex_print(4*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,a);

	}


	// DISPLAY BANKED REGISTERS
	// GET BANKED REGISTERS
	ex_gethireg(hi_reg);

	for(f=0;f<7;++f)
	{

		for(j=7;j>=0;j--)
		{
		a[7-j]=((hi_reg[f]>>(j<<2))&0xf)+48;
		if(a[7-j]>'9') a[7-j]+=7;
		}
        ex_print(SCREEN_WIDTH/2+5*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,a);

	}

		// PC
		for(j=7;j>=0;j--)
		{
		a[7-j]=((registers[0]>>(j<<2))&0xf)+48;
		if(a[7-j]>'9') a[7-j]+=7;
		}
        ex_print(SCREEN_WIDTH/2+5*FNT_AVGW,9*FNT_HEIGHT,a);

		// FLAGS
        if(hi_reg[7]&0x80000000) { ex_print(SCREEN_WIDTH/2+19*FNT_AVGW,2*FNT_HEIGHT,"N");  ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,2*FNT_HEIGHT,"MI");} else ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,2*FNT_HEIGHT,"PL");
        if(hi_reg[7]&0x40000000) { ex_print(SCREEN_WIDTH/2+19*FNT_AVGW,3*FNT_HEIGHT,"Z"); ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,3*FNT_HEIGHT,"EQ"); } else ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,3*FNT_HEIGHT,"NE");
        if(hi_reg[7]&0x20000000) { ex_print(SCREEN_WIDTH/2+19*FNT_AVGW,4*FNT_HEIGHT,"C"); ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,4*FNT_HEIGHT,"CS"); } else ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,4*FNT_HEIGHT,"CC");
        if(hi_reg[7]&0x10000000) { ex_print(SCREEN_WIDTH/2+19*FNT_AVGW,5*FNT_HEIGHT,"V");  ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,5*FNT_HEIGHT,"VS");} else ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,5*FNT_HEIGHT,"VC");
        if( (hi_reg[7]&0x60000000)==0x20000000) { ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,6*FNT_HEIGHT,"HI"); } else ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,6*FNT_HEIGHT,"LS");
		if(  (hi_reg[7] ^ (hi_reg[7]>>3) )&0x10000000) {
            ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,7*FNT_HEIGHT,"LT");
			}
        else { if(!(hi_reg[7]&0x40000000)) ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,7*FNT_HEIGHT,"GT"); }

		if((hi_reg[7]&0x40)) {
        ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,8*FNT_HEIGHT,"F" );
		}
		if((hi_reg[7]&0x80)) {
        ex_print(SCREEN_WIDTH/2+19*FNT_AVGW,8*FNT_HEIGHT,"I" );
		}


		if((hi_reg[7]&0x1f)==0x10) {
        ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,9*FNT_HEIGHT,"USER" );
		}
		if((hi_reg[7]&0x1f)==0x11) {
        ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,9*FNT_HEIGHT,"FIQ" );
		for(f=0;f<7;++f)
		{
        ex_print(SCREEN_WIDTH/2+12*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,"/B" );
		}
		}
		if((hi_reg[7]&0x1f)==0x12) {
        ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,9*FNT_HEIGHT,"IRQ" );
		for(f=5;f<7;++f)
		{
        ex_print(SCREEN_WIDTH/2+12*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,"/B" );
		}
		}
		if((hi_reg[7]&0x1f)==0x13) {
        ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,9*FNT_HEIGHT,"SUP" );
		for(f=5;f<7;++f)
		{
        ex_print(SCREEN_WIDTH/2+12*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,"/B" );
		}
		}
		if((hi_reg[7]&0x1f)==0x17) {
        ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,9*FNT_HEIGHT,"ABT" );
		for(f=5;f<7;++f)
		{
        ex_print(SCREEN_WIDTH/2+12*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,"/B" );
		}
		}
		if((hi_reg[7]&0x1f)==0x1B) {
        ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,9*FNT_HEIGHT,"UND" );
		for(f=5;f<7;++f)
		{
        ex_print(SCREEN_WIDTH/2+12*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,"/B" );
		}
		}
		if((hi_reg[7]&0x1f)==0x1f) {
        ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,9*FNT_HEIGHT,"SYS" );
		}


        if(hi_reg[7]&0x20) ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,10*FNT_HEIGHT,"Thumb"); else  ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,10*FNT_HEIGHT,"ARM");
}
        ex_hline(11*FNT_HEIGHT+4*FNT_AVGW);

}

if(options&EX_CONT) {
        int *pnewb=(int *)MEM_PHYS_EXSCREEN;
		// DRAW BUTTON 1
        ex_print(1*BTN_WIDTH+4,(SCREEN_HEIGHT-3*FNT_HEIGHT),"Cont");
		//pnewb[70*5]|=0x10000;
        //for(f=0;f<8;++f) pnewb[(SCREEN_HEIGHT-9)*5+5*f]|=0x20000;
        //pnewb[(SCREEN_HEIGHT-2)*5]|=0x3ffff;
        //pnewb[(SCREEN_HEIGHT-1)*5]|=0x3fffc;
}

if(options&(EX_EXIT|EX_RPLEXIT)) {
        int *pnewb=(int *)MEM_PHYS_EXSCREEN;

		// DRAW BUTTON 2
        ex_print(1*BTN_WIDTH+4,(SCREEN_HEIGHT-2*FNT_HEIGHT),"Exit");
        //for(f=0;f<8;++f) pnewb[(SCREEN_HEIGHT-9)*5+1+5*f]|=0x20;
        //pnewb[(SCREEN_HEIGHT-2)*5]|=0xfff80000;
        //pnewb[(SCREEN_HEIGHT-2)*5+1]|=0x3f;
        //pnewb[(SCREEN_HEIGHT-1)*5]|=0xffe00000;
        //pnewb[(SCREEN_HEIGHT-1)*5+1]|=0x3f;
}

if(options&EX_WARM) {
        int *pnewb=(int *)MEM_PHYS_EXSCREEN;

		// DRAW BUTTON 3
        if(options&EX_WIPEOUT) ex_print(1*BTN_WIDTH+4,(SCREEN_HEIGHT-FNT_HEIGHT),"*Clear Mem*");
            else ex_print(1*BTN_WIDTH+4,(SCREEN_HEIGHT-FNT_HEIGHT),"*Warmstart*");
        //for(f=0;f<8;++f) pnewb[(SCREEN_HEIGHT-9)*5+2+5*f]|=0x2000000;
        //pnewb[(SCREEN_HEIGHT-2)*5+2]|=0x3ffffff;
        //pnewb[(SCREEN_HEIGHT-2)*5+1]|=0xfffff000;
        //pnewb[(SCREEN_HEIGHT-1)*5+2]|=0x3ffffff;
        //pnewb[(SCREEN_HEIGHT-1)*5+1]|=0xffffc000;
}

if(options&EX_RESET) {
        int *pnewb=(int *)MEM_PHYS_EXSCREEN;
		// DRAW BUTTON 4
        ex_print(4*BTN_WIDTH+4,(SCREEN_HEIGHT-3*FNT_HEIGHT),"**Reset**");
        //for(f=0;f<9;++f) pnewb[(SCREEN_HEIGHT-9)*5+4+5*f]|=0x1;
        //pnewb[(SCREEN_HEIGHT-2)*5+3]|=0xffffffff;
        //pnewb[(SCREEN_HEIGHT-2)*5+2]|=0xf0000000;
        //pnewb[(SCREEN_HEIGHT-1)*5+3]|=0xffffffff;
        //pnewb[(SCREEN_HEIGHT-1)*5+2]|=0xc0000000;
}

// WARMSTART AND RESET REQUIRE SIMULTANEOUS SHIFT OR ALPHA KEYPRESS

// WAIT FOR ALL KEYS TO BE RELEASED TO AVOID ACCIDENTAL KEYPRESSES

keyb_irq_waitrelease();


do {
f=keyb_irq_getkey(1);

if(options&EX_CONT) {
j=EX_CONT;
if( KEYVALUE(f)==KB_SYM )	break;
}
if(options&(EX_EXIT|EX_RPLEXIT)) {
j=options&(EX_EXIT|EX_RPLEXIT);
if( KEYVALUE(f)==KB_PLT )	break;
}
if( KEYVALUE(f)==KB_MEN) {
    options^=EX_RPLREGS;
    options&=~EX_NOREG;
    goto doitagain;
}
// FORCE A SHIFTED KEY PRESS
if(!KEYSHIFT(f)) continue;


if(options&(EX_WARM|EX_WIPEOUT)) {
    if((options&EX_WIPEOUT) && (KEYSHIFT(f)==(SHIFT_ALPHA|SHIFT_ALPHAHOLD|SHIFT_RS|SHIFT_RSHOLD|SHIFT_LS|SHIFT_LSHOLD))) j=EX_WIPEOUT;
    else j=EX_WARM;
if( KEYVALUE(f)==KB_NUM )	break;
}

if(options&EX_RESET) {
j=EX_RESET;
if( KEYVALUE(f)==KB_HLP )	break;
}
} while(1);

keyb_irq_waitrelease();

lcd_restore(lcd_buffer);

return j;

}


ARM_MODE void handler_dabort(void) __attribute__ ((naked));
ARM_MODE void handler_dabort(void)
{
	// STORE ALL REGISTERS INTO THE SAFE STACK
    asm volatile ("stmfd sp!, {r0-r12, r14}");
    asm volatile ("mrs r0,cpsr");
    asm volatile ("orr r0,r0,#0xc0");			// DISABLE ALL FIQ AND IRQ INTERRUPTS
    asm volatile ("msr cpsr,r0");

    asm volatile ("sub r14,r14,#8");			// CHANGE FOR ALL DIFFERENT EXCEPTIONS TO POINT TO THE RIGHT PC
    asm volatile ("str r14,[sp,#-4]!");

	register unsigned int *stackptr asm("sp");
	// CALL CUSTOM HANDLER
	register int f=exception_handler("Data abort",stackptr,EX_CONT | EX_EXIT | EX_WARM | EX_RESET);

	if(f==EX_CONT) {
	// RESTORE ALL REGISTERS
	asm volatile ("add sp,sp,#4");
    asm volatile ("ldmfd sp!, {r0-r12, r14}");

	// RETURN TO THE NEXT INSTRUCTION AFTER DATA FAILED (DON'T RETRY)
    asm volatile ("subs pc, r14,#4");

	}

	asm volatile (".Ldohandlerexit:");

    /*
	if(f==EX_EXIT) {

	asm volatile ("ldr lr, .Lexit");
	asm volatile ("b .Lretexit");

	}
    */

	if(f==EX_WARM) {
	asm volatile ("ldr lr, .Lexitwarm");
	asm volatile ("b .Lretexit");
	}

    if(f==EX_WIPEOUT) {
    asm volatile ("ldr lr, .Lexitwipeout");
    asm volatile ("b .Lretexit");
    }

    // DEFAULT = EXIT W/RESET

	asm volatile ("ldr lr, .Lexitreset");

	asm volatile (".Lretexit:");

    // MASK ALL INTERRUPTS AND CLEAR ALL PENDING REQUESTS
    *INTMSK1=0xffffffff;
    *INTMSK2=0xffffffff;
    *SRCPND1=0xffffffff;
    *SRCPND2=0xffffffff;
    *INTPND1=0xffffffff;
    *INTPND2=0xffffffff;

    register unsigned int tmp asm ("r0");

	asm volatile ("mrs %0,spsr" : "=r" (tmp));
	asm volatile ("bic %0,%1,#0xff" : "=r" (tmp) : "r" (tmp));		// CLEAN STATE BITS
    asm volatile ("orr %0,%1, #0x1f" : "=r" (tmp) : "r" (tmp));		// SET SYSTEM CPU MODE W/IRQ AND FIQ ENABLED, ARM MODE
	asm volatile ("msr spsr,%0" : "=r" (tmp));						// SAVE USER MODE STATUS


	// RESTORE ALL REGISTERS
	asm volatile ("add sp,sp,#4");
    asm volatile ("ldmfd sp!, {r0-r12}");
	asm volatile ("add sp,sp,#4");

    asm volatile ("mov r0,#-1"); 	// EXIT WITH CODE -1

	// RETURN DIRECTLY INTO THE exit FUNCTION
    asm volatile ("movs pc, r14");

    asm volatile (".Lexitwarm: .word halWarmStart");
    asm volatile (".Lexitreset: .word halReset");
    asm volatile (".Lexitwipeout: .word halWipeoutWarmStart");

}




ARM_MODE void handler_iabort(void) __attribute__ ((naked));
ARM_MODE void handler_iabort(void)
{
register unsigned int *stackptr asm("sp");

	// STORE ALL REGISTERS INTO THE SAFE STACK
    asm volatile ("stmfd sp!, {r0-r12, r14}");
    asm volatile ("mrs r0,cpsr");
    asm volatile ("orr r0,r0,#0xc0");			// DISABLE ALL FIQ AND IRQ INTERRUPTS
    asm volatile ("msr cpsr,r0");
    asm volatile ("sub r14,r14,#4");			// CHANGE FOR ALL DIFFERENT EXCEPTIONS TO POINT TO THE RIGHT PC
    asm volatile ("str r14,[sp,#-4]!");

	// CALL CUSTOM HANDLER
    register int f=exception_handler("Prefetch abort",stackptr,EX_CONT | EX_WARM | EX_RESET);

   	if(f==EX_CONT) {
	// RESTORE ALL REGISTERS
	asm volatile ("add sp,sp,#4");
    asm volatile ("ldmfd sp!, {r0-r12, r14}");

	// RETURN TO THE NEXT INSTRUCTION AFTER DATA FAILED (DON'T RETRY)
    asm volatile ("movs pc, r14");

	}

	asm volatile ("b .Ldohandlerexit");

}

ARM_MODE void handler_und(void) __attribute__ ((naked));
ARM_MODE void handler_und(void)
{
register unsigned int *stackptr asm("sp");
register unsigned int value asm("r0");

	// STORE ALL REGISTERS INTO THE SAFE STACK
    asm volatile ("stmfd sp!, {r0-r12, r14}");
    asm volatile ("sub r14,r14,#4");			// CHANGE FOR ALL DIFFERENT EXCEPTIONS TO POINT TO THE RIGHT PC
    asm volatile ("mrs r0,cpsr");
    asm volatile ("orr r0,r0,#0xc0");			// DISABLE ALL FIQ AND IRQ INTERRUPTS
    asm volatile ("msr cpsr,r0");
    asm volatile ("mrs r0,spsr");
    asm volatile ("ands r0,r0,#0x20");
    asm volatile ("add r14,r14,r0,LSR #4");		// ADD 2 IF IN THUMB MODE (PC+2)


    asm volatile ("str r14,[sp,#-4]!");

    // DO NOT LOAD ANY VALUE IF IN THUMB MODE
    asm volatile ("ldreq %0,[r14]" : "=r" (value) : "r" (value));				// GET OFFENDING INSTRUCTION

	if(value==0xe6cccc10)
	{

	value=exception_handler((char *) (stackptr[1]),stackptr,stackptr[2] | EX_NOREG);

	}
	else {
	// CALL CUSTOM HANDLER
	if(value==0xe6dddd10) 	value=exception_handler((char *) (stackptr[1]),stackptr,stackptr[2]);
 	else
        value=exception_handler("Undefined instruction",stackptr,EX_CONT | EX_WARM | EX_RESET);
	}

    if(value==EX_RPLEXIT) {
        // RAISE AN RPL EXCEPTION AND ISSUE A CONTINUE
        Exceptions|=EX_EXITRPL;
        ExceptionPointer=IPtr;
        value=EX_CONT;
    }

   	if(value==EX_CONT) {
	// RESTORE ALL REGISTERS
	asm volatile ("add sp,sp,#4");
    asm volatile ("ldmfd sp!, {r0-r12, r14}");

	// RETURN TO THE NEXT INSTRUCTION AFTER DATA FAILED (DON'T RETRY)
    asm volatile ("movs pc, r14");

	}

	asm volatile ("b .Ldohandlerexit");


}

extern void startup(void);
void exception_install()
{
    unsigned *handler_addr=(unsigned int *)0x31ffff00L;
    handler_addr[0]=(unsigned int)(&startup); // RESET EXCEPTION!!
    handler_addr[1]=(unsigned int)(&handler_und); // UNDEFINED instruction
    handler_addr[2]=(unsigned int)(&handler_und); // SWI service handler
    handler_addr[3]=(unsigned int)(&handler_iabort); // PREFETCH abort
    handler_addr[4]=(unsigned int)(&handler_dabort); // DATA abort
    handler_addr[5]=(unsigned int)(&handler_und); // RESERVED
    handler_addr[6]=(unsigned int)(&handler_und); // IRQ handler - will be overwritten by the ISR
    handler_addr[7]=(unsigned int)(&handler_und); // FIQ handler - will be overwritten by the ISR


    handler_addr[62]=0xe51ff100; // This goes at 0x31fffff8: asm volatile ("ldr pc, .Lhdlr_reset");
    handler_addr[61]=0xe51ff0f8;
    handler_addr[60]=0xe51ff0f0;
    handler_addr[59]=0xe51ff0e8;
    handler_addr[58]=0xe51ff0e0;
    handler_addr[57]=0xe51ff0d8;
    handler_addr[56]=0xe51ff0d0;
    handler_addr[55]=0xe51ff0c8;
    handler_addr[54]=0xe51ff0c0;

    // Relocation of exception vectors from SRAM to DRAM (why is this needed? we could keep the table at 0x20):

    handler_addr=0;
    handler_addr[0]=0xe26ff432; // rsb pc,pc,#0x32000000 --> pc=0x32000000 - (pc+8) = 0x31fffff8 (reset handler)
    handler_addr[1]=0xe26ff432; // rsb pc,pc,#0x32000000 --> pc=0x32000000 - (pc+8) = 0x31fffff4 (reset handler)
    handler_addr[2]=0xe26ff432; // rsb pc,pc,#0x32000000 --> pc=0x32000000 - (pc+8) = 0x31fffff0 (reset handler)
    handler_addr[3]=0xe26ff432; // rsb pc,pc,#0x32000000 --> pc=0x32000000 - (pc+8) = 0x31ffffec (reset handler)
    handler_addr[4]=0xe26ff432; // rsb pc,pc,#0x32000000 --> pc=0x32000000 - (pc+8) = 0x31ffffe8 (reset handler)
    handler_addr[5]=0xe26ff432; // rsb pc,pc,#0x32000000 --> pc=0x32000000 - (pc+8) = 0x31ffffe4 (reset handler)
    handler_addr[6]=0xe26ff432; // rsb pc,pc,#0x32000000 --> pc=0x32000000 - (pc+8) = 0x31ffffe0 (reset handler)
    handler_addr[7]=0xe26ff432; // rsb pc,pc,#0x32000000 --> pc=0x32000000 - (pc+8) = 0x31ffffdc (reset handler)

	irq_install();

}

void __attribute__ ((noinline)) throw_exception(char * message, unsigned int options)
{
 asm volatile (".word 0xE6CCCC10");
}

void __attribute__ ((noinline)) throw_dbgexception(char * message, unsigned int options)
{
 asm volatile (".word 0xE6DDDD10");
}


#ifndef NDEBUG
void exit(int rc)
{
    record(exceptions, "exit(%d), reseting system", rc);
    while(1)
        halReset();
}
#endif // NDEBUG

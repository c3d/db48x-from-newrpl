/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <hal_api.h>

void __keyb_waitrelease();
int __keyb_getkey(int wait);

extern const unsigned int Font_10A[];
extern unsigned int RPLLastOpcode;

// EXCEPTIONS SCREEN AREA RIGHT AFTER THE SCREEN, RESERVE SPACE FOR 16-BIT COLOR MODE
#define MEM_PHYS_EXSCREEN (*VIDW00ADD0B1);
#define FNT_HEIGHT 12
#define FNT_AVGW   7
#define BTN_WIDTH  52

void __ex_print(int x,int y,char *str)
{
    DRAWSURFACE dr;
    dr.addr=(int *)MEM_PHYS_EXSCREEN;
    dr.width=LCD_W;
    dr.x=dr.y=0;
    dr.clipx=dr.clipy=0;
    dr.clipx2=SCREEN_WIDTH;
    dr.clipy2=SCREEN_HEIGHT;

    DrawText(x,y,str,(UNIFONT *)Font_10A,0xf,&dr);
}

void __ex_clrscreen()
{
    DRAWSURFACE dr;
    dr.addr=(int *)MEM_PHYS_EXSCREEN;
    dr.width=LCD_W;
    dr.x=dr.y=0;
    dr.clipx=dr.clipy=0;
    dr.clipx2=SCREEN_WIDTH;
    dr.clipy2=SCREEN_HEIGHT;
    ggl_rect(&dr,dr.x,dr.y,dr.clipx2-1,dr.clipy2-1,0x00000000);
}

void __ex_hline(int y)
{
    DRAWSURFACE dr;
    dr.addr=(int *)MEM_PHYS_EXSCREEN;
    dr.width=LCD_W;
    dr.x=dr.y=0;
    dr.clipx=dr.clipy=0;
    dr.clipx2=SCREEN_WIDTH;
    dr.clipy2=SCREEN_HEIGHT;
    ggl_hline(&dr,y,dr.x,dr.clipx2-1,0xf0f0f0f0);
}

inline int __ex_width(char *string) { return StringWidth(string,(UNIFONT *)Font_10A); }

// GET HIGH REGISTERS R8 TO R14 + CPSR (8 WORDS)

__ARM_MODE__ void __ex_gethireg(unsigned int *hi_reg)
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

#define __EX_CONT 1		// SHOW CONTINUE OPTION
#define __EX_EXIT 2		// SHOW EXIT OPTION
#define __EX_WARM 4		// SHOW WARMSTART OPTION
#define __EX_RESET 8	// SHOW RESET OPTION
#define __EX_NOREG 16	// DON'T SHOW REGISTERS
#define __EX_WIPEOUT 32	// FULL MEMORY WIPEOUT AND WARMSTART
#define __EX_RPLREGS 64 // SHOW RPL REGISTERS INSTEAD
#define __EX_RPLEXIT 128 // SHOW EXIT OPTION, IT RESUMES EXECUTION AFTER SETTING Exception=EX_EXITRPL
*/
int __exception_handler(char *exstr, unsigned int *registers,int options)
{
unsigned int lcd_buffer[17];
unsigned int hi_reg[8];
char a[10];

int f,j;


lcd_save(lcd_buffer);
*WINCON0|=(1<<23);                        // AND START DISPLAYING BUFFER 1
//lcd_setmode(0,(int *)MEM_PHYS_EXSCREEN);

doitagain:

__ex_clrscreen();

if(options&__EX_NOREG) {

    __ex_print(SCREEN_WIDTH/2-7*FNT_AVGW,FNT_HEIGHT*2,"-- EXCEPTION --");
    __ex_hline(FNT_HEIGHT*2-4);
    __ex_hline(FNT_HEIGHT*3+4);
	
    __ex_print(SCREEN_WIDTH/2-(__ex_width(exstr)>>1),FNT_HEIGHT*5,exstr);


}
else {
    __ex_print(0,0,"Exception: ");
    __ex_print(11*FNT_AVGW,0,exstr);

    if(options&__EX_RPLREGS) {
        __ex_hline(FNT_HEIGHT*2-4);
    // SHOW RPL CORE INFORMATION INSTEAD
        __ex_print(0,FNT_HEIGHT*2,"IP: ");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)IPtr)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(4*FNT_AVGW,FNT_HEIGHT*2,a);

        __ex_print(0,FNT_HEIGHT*3,"OPC:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)RPLLastOpcode)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(4*FNT_AVGW,FNT_HEIGHT*3,a);

        __ex_print(0,FNT_HEIGHT*4,"TOe:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)TempObEnd)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(4*FNT_AVGW,FNT_HEIGHT*4,a);

        __ex_print(0,FNT_HEIGHT*5,"TOs:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)TempObSize)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(4*FNT_AVGW,FNT_HEIGHT*5,a);

        __ex_print(0,FNT_HEIGHT*6,"TBe:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)TempBlocksEnd)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(4*FNT_AVGW,FNT_HEIGHT*6,a);

        __ex_print(0,FNT_HEIGHT*7,"TBs:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)TempBlocksSize)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(4*FNT_AVGW,FNT_HEIGHT*7,a);

        __ex_print(0,FNT_HEIGHT*8,"RSe:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)RSTop)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(4*FNT_AVGW,FNT_HEIGHT*8,a);

        __ex_print(0,FNT_HEIGHT*9,"RSs:");

        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)RStkSize)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(4*FNT_AVGW,FNT_HEIGHT*9,a);

        __ex_print(0,FNT_HEIGHT*10,"DSe:");

        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)DSTop)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(4*FNT_AVGW,FNT_HEIGHT*10,a);

        // RIGHT COLUMN

        __ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*2,"DSs: ");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)DStkSize)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*2,a);

        __ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*3,"DIe:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)DirsTop)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*3,a);

        __ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*4,"DIs:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)DirSize)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*4,a);

        __ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*5,"LAe:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)LAMTop)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*5,a);

        __ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*6,"LAs:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)LAMSize)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*6,a);

        __ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*7,"Exc:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)Exceptions)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*7,a);

        __ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*8,"Err:");
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=((((WORD)ErrorCode)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*8,a);


        __ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*9,"Um:");

        {
        WORD total=halGetTotalPages();
        WORD freemem=halGetFreePages();
        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=(((freemem)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*9,a);


        __ex_print(SCREEN_WIDTH/2,FNT_HEIGHT*10,"Tm:");

        a[8]=0;
        for(j=7;j>=0;j--)
         {
         a[7-j]=(((total)>>(j<<2))&0xf)+48;
          if(a[7-j]>'9') a[7-j]+=7;
         }
        __ex_print(SCREEN_WIDTH/2+4*FNT_AVGW,FNT_HEIGHT*10,a);
        }

     }
    else {
        // PRINT CPU REGISTERS
	a[2]=0;
	for(f=0;f<8;++f) {
    __ex_print(0,f*FNT_HEIGHT+2*FNT_HEIGHT,"R  :");
	a[1]=0;
	a[0]=f+48;
    __ex_print(1*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,a);
    __ex_print(SCREEN_WIDTH/2,f*FNT_HEIGHT+2*FNT_HEIGHT,"R   :");
	if(f<2) a[0]=f+8+48;
	else { a[0]='1'; a[1]=f-2+48; }
    __ex_print(SCREEN_WIDTH/2+1*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,a);
	}
	
    __ex_hline(2*FNT_HEIGHT-4);
	
	a[8]=0;
	
	// DISPLAY REGISTERS
	
	for(f=0;f<8;++f)
	{
		for(j=7;j>=0;j--)
		{
		a[7-j]=((registers[f+1]>>(j<<2))&0xf)+48;
		if(a[7-j]>'9') a[7-j]+=7;
		}
        __ex_print(4*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,a);
		
	}


	// DISPLAY BANKED REGISTERS
	// GET BANKED REGISTERS	
	__ex_gethireg(hi_reg);
	
	for(f=0;f<7;++f)
	{

		for(j=7;j>=0;j--)
		{
		a[7-j]=((hi_reg[f]>>(j<<2))&0xf)+48;
		if(a[7-j]>'9') a[7-j]+=7;
		}
        __ex_print(SCREEN_WIDTH/2+5*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,a);
	
	}

		// PC
		for(j=7;j>=0;j--)
		{
		a[7-j]=((registers[0]>>(j<<2))&0xf)+48;
		if(a[7-j]>'9') a[7-j]+=7;
		}
        __ex_print(SCREEN_WIDTH/2+5*FNT_AVGW,9*FNT_HEIGHT,a);
		
		// FLAGS
        if(hi_reg[7]&0x80000000) { __ex_print(SCREEN_WIDTH/2+19*FNT_AVGW,2*FNT_HEIGHT,"N");  __ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,2*FNT_HEIGHT,"MI");} else __ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,2*FNT_HEIGHT,"PL");
        if(hi_reg[7]&0x40000000) { __ex_print(SCREEN_WIDTH/2+19*FNT_AVGW,3*FNT_HEIGHT,"Z"); __ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,3*FNT_HEIGHT,"EQ"); } else __ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,3*FNT_HEIGHT,"NE");
        if(hi_reg[7]&0x20000000) { __ex_print(SCREEN_WIDTH/2+19*FNT_AVGW,4*FNT_HEIGHT,"C"); __ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,4*FNT_HEIGHT,"CS"); } else __ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,4*FNT_HEIGHT,"CC");
        if(hi_reg[7]&0x10000000) { __ex_print(SCREEN_WIDTH/2+19*FNT_AVGW,5*FNT_HEIGHT,"V");  __ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,5*FNT_HEIGHT,"VS");} else __ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,5*FNT_HEIGHT,"VC");
        if( (hi_reg[7]&0x60000000)==0x20000000) { __ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,6*FNT_HEIGHT,"HI"); } else __ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,6*FNT_HEIGHT,"LS");
		if(  (hi_reg[7] ^ (hi_reg[7]>>3) )&0x10000000) { 
            __ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,7*FNT_HEIGHT,"LT");
			}
        else { if(!(hi_reg[7]&0x40000000)) __ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,7*FNT_HEIGHT,"GT"); }
		
		if((hi_reg[7]&0x40)) {
        __ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,8*FNT_HEIGHT,"F" );
		}
		if((hi_reg[7]&0x80)) {
        __ex_print(SCREEN_WIDTH/2+19*FNT_AVGW,8*FNT_HEIGHT,"I" );
		}
		
		
		if((hi_reg[7]&0x1f)==0x10) {
        __ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,9*FNT_HEIGHT,"USER" );
		}
		if((hi_reg[7]&0x1f)==0x11) {
        __ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,9*FNT_HEIGHT,"FIQ" );
		for(f=0;f<7;++f)
		{
        __ex_print(SCREEN_WIDTH/2+14*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,"/B" );
		}
		}
		if((hi_reg[7]&0x1f)==0x12) {
        __ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,9*FNT_HEIGHT,"IRQ" );
		for(f=5;f<7;++f)
		{
        __ex_print(SCREEN_WIDTH/2+14*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,"/B" );
		}
		}
		if((hi_reg[7]&0x1f)==0x13) {
        __ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,9*FNT_HEIGHT,"SUP" );
		for(f=5;f<7;++f)
		{
        __ex_print(SCREEN_WIDTH/2+14*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,"/B" );
		}
		}
		if((hi_reg[7]&0x1f)==0x17) {
        __ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,9*FNT_HEIGHT,"ABT" );
		for(f=5;f<7;++f)
		{
        __ex_print(SCREEN_WIDTH/2+14*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,"/B" );
		}
		}
		if((hi_reg[7]&0x1f)==0x1B) {
        __ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,9*FNT_HEIGHT,"UND" );
		for(f=5;f<7;++f)
		{
        __ex_print(SCREEN_WIDTH/2+14*FNT_AVGW,f*FNT_HEIGHT+2*FNT_HEIGHT,"/B" );
		}
		}
		if((hi_reg[7]&0x1f)==0x1f) {
        __ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,9*FNT_HEIGHT,"SYS" );
		}


        if(hi_reg[7]&0x20) __ex_print(SCREEN_WIDTH/2+15*FNT_AVGW,10*FNT_HEIGHT,"Thumb"); else  __ex_print(SCREEN_WIDTH/2+16*FNT_AVGW,10*FNT_HEIGHT,"ARM");
}
        __ex_hline(11*FNT_HEIGHT+4*FNT_AVGW);
		
}

if(options&__EX_CONT) {
        int *pnewb=(int *)MEM_PHYS_EXSCREEN;
		// DRAW BUTTON 1
        __ex_print(0*BTN_WIDTH+4,(SCREEN_HEIGHT-8),"Cont");
		//pnewb[70*5]|=0x10000;
        //for(f=0;f<8;++f) pnewb[(SCREEN_HEIGHT-9)*5+5*f]|=0x20000;
        //pnewb[(SCREEN_HEIGHT-2)*5]|=0x3ffff;
        //pnewb[(SCREEN_HEIGHT-1)*5]|=0x3fffc;
}

if(options&(__EX_EXIT|__EX_RPLEXIT)) {
        int *pnewb=(int *)MEM_PHYS_EXSCREEN;

		// DRAW BUTTON 2
        __ex_print(1*BTN_WIDTH+4,(SCREEN_HEIGHT-8),"Exit");
        //for(f=0;f<8;++f) pnewb[(SCREEN_HEIGHT-9)*5+1+5*f]|=0x20;
        //pnewb[(SCREEN_HEIGHT-2)*5]|=0xfff80000;
        //pnewb[(SCREEN_HEIGHT-2)*5+1]|=0x3f;
        //pnewb[(SCREEN_HEIGHT-1)*5]|=0xffe00000;
        //pnewb[(SCREEN_HEIGHT-1)*5+1]|=0x3f;
}

if(options&__EX_WARM) {
        int *pnewb=(int *)MEM_PHYS_EXSCREEN;

		// DRAW BUTTON 3
        if(options&__EX_WIPEOUT) __ex_print(2*BTN_WIDTH+4,(SCREEN_HEIGHT-8),"*Clear Mem*");
            else __ex_print(2*BTN_WIDTH+4,(SCREEN_HEIGHT-8),"*Warmstart*");
        //for(f=0;f<8;++f) pnewb[(SCREEN_HEIGHT-9)*5+2+5*f]|=0x2000000;
        //pnewb[(SCREEN_HEIGHT-2)*5+2]|=0x3ffffff;
        //pnewb[(SCREEN_HEIGHT-2)*5+1]|=0xfffff000;
        //pnewb[(SCREEN_HEIGHT-1)*5+2]|=0x3ffffff;
        //pnewb[(SCREEN_HEIGHT-1)*5+1]|=0xffffc000;
}

if(options&__EX_RESET) {
        int *pnewb=(int *)MEM_PHYS_EXSCREEN;
		// DRAW BUTTON 4
        __ex_print(4*BTN_WIDTH+4,(SCREEN_HEIGHT-8),"**Reset**");
        //for(f=0;f<9;++f) pnewb[(SCREEN_HEIGHT-9)*5+4+5*f]|=0x1;
        //pnewb[(SCREEN_HEIGHT-2)*5+3]|=0xffffffff;
        //pnewb[(SCREEN_HEIGHT-2)*5+2]|=0xf0000000;
        //pnewb[(SCREEN_HEIGHT-1)*5+3]|=0xffffffff;
        //pnewb[(SCREEN_HEIGHT-1)*5+2]|=0xc0000000;
}
		
// WARMSTART AND RESET REQUIRE SIMULTANEOUS SHIFT OR ALPHA KEYPRESS

// WAIT FOR ALL KEYS TO BE RELEASED TO AVOID ACCIDENTAL KEYPRESSES
while(1);

__keyb_waitrelease();


do {
f=__keyb_getkey(1);

if(options&__EX_CONT) {
j=__EX_CONT;
if( KEYVALUE(f)==KB_A )	break;
}
if(options&(__EX_EXIT|__EX_RPLEXIT)) {
j=options&(__EX_EXIT|__EX_RPLEXIT);
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
*WINCON0&=~(1U<<23);                        // AND START DISPLAYING BUFFER 0
lcd_restore(lcd_buffer);

return j;

}


__ARM_MODE__ void __handler_dabort(void) __attribute__ ((naked));
__ARM_MODE__ void __handler_dabort(void)
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
	register int f=__exception_handler("Data abort",stackptr,__EX_CONT | __EX_EXIT | __EX_WARM | __EX_RESET);

	if(f==__EX_CONT) {
	// RESTORE ALL REGISTERS
	asm volatile ("add sp,sp,#4");
    asm volatile ("ldmfd sp!, {r0-r12, r14}");

	// RETURN TO THE NEXT INSTRUCTION AFTER DATA FAILED (DON'T RETRY)
    asm volatile ("subs pc, r14,#4");
    
	}
	
	asm volatile (".Ldohandlerexit:");

    /*
	if(f==__EX_EXIT) {
		
	asm volatile ("ldr lr, .Lexit");
	asm volatile ("b .Lretexit");
    
	}
    */

	if(f==__EX_WARM) {
	asm volatile ("ldr lr, .Lexitwarm");
	asm volatile ("b .Lretexit");
	}

    if(f==__EX_WIPEOUT) {
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




__ARM_MODE__ void __handler_iabort(void) __attribute__ ((naked));
__ARM_MODE__ void __handler_iabort(void)
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
    register int f=__exception_handler("Prefetch abort",stackptr,__EX_CONT | __EX_WARM | __EX_RESET);

   	if(f==__EX_CONT) {
	// RESTORE ALL REGISTERS
	asm volatile ("add sp,sp,#4");
    asm volatile ("ldmfd sp!, {r0-r12, r14}");

	// RETURN TO THE NEXT INSTRUCTION AFTER DATA FAILED (DON'T RETRY)
    asm volatile ("movs pc, r14");
    
	}

	asm volatile ("b .Ldohandlerexit");
     
}

__ARM_MODE__ void __handler_und(void) __attribute__ ((naked));
__ARM_MODE__ void __handler_und(void)
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

	value=__exception_handler((char *) (stackptr[1]),stackptr,stackptr[2] | __EX_NOREG);

	}
	else {
	// CALL CUSTOM HANDLER
	if(value==0xe6dddd10) 	value=__exception_handler((char *) (stackptr[1]),stackptr,stackptr[2]);
 	else
        value=__exception_handler("Undefined instruction",stackptr,__EX_CONT | __EX_WARM | __EX_RESET);
	}

    if(value==__EX_RPLEXIT) {
        // RAISE AN RPL EXCEPTION AND ISSUE A CONTINUE
        Exceptions|=EX_EXITRPL;
        ExceptionPointer=IPtr;
        value=__EX_CONT;
    }

   	if(value==__EX_CONT) {
	// RESTORE ALL REGISTERS
	asm volatile ("add sp,sp,#4");
    asm volatile ("ldmfd sp!, {r0-r12, r14}");

	// RETURN TO THE NEXT INSTRUCTION AFTER DATA FAILED (DON'T RETRY)
    asm volatile ("movs pc, r14");
    
	}
	
	asm volatile ("b .Ldohandlerexit");
	

}



void __exception_install()
{
    unsigned *handler_addr=(unsigned int *)0x31ffff00L;
	handler_addr[1]=(unsigned int)(&__handler_und);
	handler_addr[3]=(unsigned int)(&__handler_iabort);
	handler_addr[4]=(unsigned int)(&__handler_dabort);

	__irq_install();
}

void __attribute__ ((noinline)) throw_exception(char * message, unsigned int options)
{
 asm volatile (".word 0xE6CCCC10");
}

void __attribute__ ((noinline)) throw_dbgexception(char * message, unsigned int options)
{
 asm volatile (".word 0xE6DDDD10");
}

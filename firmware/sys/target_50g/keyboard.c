#include <ui.h>

extern unsigned int __cpu_intoff();
extern void __cpu_inton(unsigned int);
extern void __tmr_eventreschedule();



#define DEBOUNCE  16  // 10 SEEMS TO BE ADEQUATE EVEN AT 75 MHz


// KEYBOARD, LOW LEVEL GLOBAL VARIABLES
unsigned short int __keyb_buffer[KEYB_BUFFER] __attribute__((section (".system_globals")));
volatile int __keyb_lock __attribute__((section (".system_globals")));
int __keyflags __attribute__((section (".system_globals")));
int __kused __attribute__((section (".system_globals"))),__kcurrent __attribute__((section (".system_globals")));
keymatrix __kmat __attribute__((section (".system_globals")));
int __keyplane __attribute__((section (".system_globals")));
int __keynumber __attribute__((section (".system_globals"))),__keycount __attribute__((section (".system_globals")));
int __keyb_repeattime,__keyb_longpresstime __attribute__((section (".system_globals"))),__keyb_debounce __attribute__((section (".system_globals")));












// LOW-LEVEL ROUTINE TO BE USED BY THE IRQ HANDLERS AND EXCEPTION
// HANDLERS ONLY

keymatrix __keyb_getmatrix()
{


    unsigned int volatile * GPGDAT = (unsigned int*) (IO_REGS+0x64); //data
    unsigned int volatile * GPGCON = (unsigned int*) (IO_REGS+0x60); //control

    unsigned int tmp[DEBOUNCE],f,k;


    unsigned int lo=0,hi=0;

    int col;
    unsigned int control;

    for(col=7;col>=4;--col)
    {

    control = 1<<((col+8)*2);
    control = control | 0x1;
    *GPGCON = control;
    // GPGDAT WAS SET TO ZERO, SO THE SELECTED COLUMN IS DRIVEN LOW


    // DEBOUNCE TECHNIQUE

    // FILL DEBOUNCE BUFFER
    for(f=0;f<DEBOUNCE;++f) tmp[f]=f;
    // DO CIRCULAR BUFFER, CHECKING FOR NOISE ON EVERY READ
    k=0;
    do {
        tmp[k]=*GPGDAT & 0xfe;
    for(f=1;f<DEBOUNCE;++f) {
        if(tmp[f]!=tmp[f-1]) break;
    }
    ++k;
    if(k>=DEBOUNCE) k=0;
    } while(f<DEBOUNCE);


    hi=(hi<<8) | ((~(tmp[0]))&0xfe);

    }

    for(;col>=0;--col)
    {

    control = 1<<((col+8)*2);
    control = control | 0x1;
    *GPGCON = control;
    // GPGDAT WAS SET TO ZERO, SO THE SELECTED COLUMN IS DRIVEN LOW
    for(f=0;f<DEBOUNCE;++f) tmp[f]=f;
    // DO CIRCULAR BUFFER, CHECKING FOR NOISE ON EVERY READ
    k=0;
    do {
        tmp[k]=*GPGDAT & 0xfe;
    for(f=1;f<DEBOUNCE;++f) {
        if(tmp[f]!=tmp[f-1]) break;
    }
    ++k;
    if(k>=DEBOUNCE) k=0;
    } while(f<DEBOUNCE);


    lo=(lo<<8) | ((~(tmp[0]))&0xfe);

    }

    *GPGCON = 0x5555AAA9; // SET TO TRIGGER INTERRUPTS ON ANY KEY

    unsigned int volatile *GPFDAT = ((unsigned int*) (IO_REGS+0x54));


    for(f=0;f<DEBOUNCE;++f) tmp[f]=f;
    // DO CIRCULAR BUFFER, CHECKING FOR NOISE ON EVERY READ
    k=0;
    do {
        tmp[k]=*GPFDAT& 0x71;
    for(f=1;f<DEBOUNCE;++f) {
        if(tmp[f]!=tmp[f-1]) break;
    }
    ++k;
    if(k>=DEBOUNCE) k=0;
    } while(f<DEBOUNCE);



    hi |=  (tmp[k]&0x70)<<24;
    hi |=  tmp[k]<<31;

    return ((keymatrix)lo) | (((keymatrix)hi)<<32);
}


// WRAPPER TO DISABLE INTERRUPTS WHILE READING THE KEYBOARD
// NEEDED ONLY WHEN CALLED FROM WITHIN AN EXCEPTION HANDLER



keymatrix __keyb_getmatrixEX()
{
    unsigned int saved=__cpu_intoff();
    keymatrix m=__keyb_getmatrix();
    __cpu_inton(saved);
    return m;
}






// LOW-LEVEL FUNCTION TO BE USED BY THE
// EXCEPTION SUBSYSTEM ONLY


const unsigned short const __keyb_shiftconvert[8]={
0,
SHIFT_ALPHA|SHIFT_ALPHAHOLD,
SHIFT_LS|SHIFT_LSHOLD,
SHIFT_ALPHA|SHIFT_ALPHAHOLD|SHIFT_LS|SHIFT_LSHOLD,
SHIFT_RS|SHIFT_RSHOLD,
SHIFT_ALPHA|SHIFT_ALPHAHOLD|SHIFT_RS|SHIFT_RSHOLD,
SHIFT_LS|SHIFT_LSHOLD|SHIFT_RS|SHIFT_RSHOLD,
SHIFT_LS|SHIFT_LSHOLD|SHIFT_ALPHA|SHIFT_ALPHAHOLD|SHIFT_RS|SHIFT_RSHOLD
};

int __keyb_getkey(int wait)
{

    keymatrix m;
    m=__keyb_getmatrixEX();

    if(wait) {
        // wait for a non-shift key to be pressed
        while( (m&0x8fffffffffffffffLL )==0LL ) m=__keyb_getmatrixEX();
    }

    int kcode,shft=(m>>60)&0x7;
    unsigned char *mbytes=(unsigned char *)&m;
    int k;
    for(k=0,kcode=0;k<8;++mbytes,++k,kcode+=8)
    {
        if(*mbytes!=0) {
            k=*mbytes;
            while( !(k&1) ) {
                k>>=1;
                ++kcode;
                }
        break;
        }
    }


    if(wait) {
        while( (m&0x8fffffffffffffffLL )!=0 ) m=__keyb_getmatrixEX();
    if(kcode>=60) kcode=63;
    }

    if(kcode<60) return kcode | __keyb_shiftconvert[shft];
    if(kcode<64) return kcode;
    return 0;

}


void __keyb_waitrelease()
{
    keymatrix m=1;
    while(m!=0LL) { m=__keyb_getmatrixEX(); }
}



#define LONG_KEYPRESSTIME (__keyb_longpresstime)
#define REPEAT_KEYTIME (__keyb_repeattime)
#define BOUNCE_KEYTIME (__keyb_debounce)

#define KF_RUNNING   1
#define KF_ALPHALOCK 2
#define KF_NOREPEAT  4



#define GPGCON ((unsigned int *)(IO_REGS+0x60))
#define GPGDAT ((unsigned int *)(IO_REGS+0x64))
#define GPGUP ((unsigned int *)(IO_REGS+0x68))
#define GPFCON ((unsigned int *)(IO_REGS+0x50))
#define EXTINT0 ((unsigned int *)(IO_REGS+0x88))
#define EXTINT1 ((unsigned int *)(IO_REGS+0x8c))
#define EINTMASK ((unsigned int *)(IO_REGS+0xa4))
#define EINTPEND ((unsigned int *)(IO_REGS+0xa8))
#define INTMSK   ((unsigned int *)(INT_REGS+0x8))
#define INTPND   ((unsigned int *)(INT_REGS+0x10))
#define SRCPND   ((unsigned int *)(INT_REGS+0x0))





void __keyb_postmsg(unsigned int msg)
{

__keyb_buffer[__kcurrent]=msg;
__kcurrent=(__kcurrent+1)&(KEYB_BUFFER-1);
// CHECK FOR BUFFER OVERRUN
if(__kcurrent==__kused) {
    // BUFFER OVERRUN, DROP LAST KEY
    __kcurrent=(__kcurrent-1)&(KEYB_BUFFER-1);
}

}


void keyb_postmsg(unsigned int msg)
{
    // WARNING: PROBLEMS MAY ARISE IF THE INTERRUPT SERVICE WANTS
    // TO POST A MESSAGE WHILE THE USER IS POSTING ONE.
    while(cpu_getlock(1,&__keyb_lock));

    __keyb_postmsg(msg);
    __keyb_lock=0;

}



unsigned int keyb_getmsg()
{
if(__kused==__kcurrent) return 0;
unsigned int msg=__keyb_buffer[__kused];
__kused=(__kused+1)&(KEYB_BUFFER-1);
return msg;
}

// CHECK IF ANY AVAILABLE KEYSTROKES
int keyb_anymsg()
{
    if(__kused==__kcurrent) return 0;
    return 1;
}

// FLUSH KEYBOARD BUFFER
void keyb_flush()
{
    while(keyb_getmatrix()!=0LL);
    __kused=__kcurrent;
}

// RETURNS THE CURRENT WORKING MATRIX INSTEAD OF
// MESSING WITH THE HARDWARE, BUT ONLY IF KEYBOARD HANDLERS WERE STARTED
keymatrix keyb_getmatrix()
{
    if(__keyflags&KF_RUNNING)	return __kmat;
    else return __keyb_getmatrix();
}

// ANALYZE CHANGES IN THE KEYBOARD STATUS AND POST MESSAGES ACCORDINGLY

void __keyb_update()
{

    if(cpu_getlock(1,&__keyb_lock)) return;

    keymatrix a,b;

doupdate:

    a=__keyb_getmatrix();
    b=a^__kmat;
    __kmat=a;

    // ANALYZE CHANGES
    if(b!=0) {
    // POST MESSAGE
    int key=0;
    while(b!=0) {
    if(b&1) {
    if(a&1) {
        // POST KEYDN MESSAGE
        if(__keynumber==-key) {
            // DISREGARD SPURIOUS KEYPRESS
            __kmat&=~(1LL<<key);	// CLEAR THE KEY
            __keynumber=0;
            __keycount=0;
        }
        else {
        __keyb_postmsg(KM_KEYDN + key);
        if(key<60 || ( (key==KB_ALPHA)&&(__keyplane&(SHIFT_RS|SHIFT_LS)) )) {	// TREAT SHIFT-ALPHA LIKE REGULAR KEYPRESS
        __keyb_postmsg(KM_PRESS + key + (__keyplane&SHIFT_ANY));
        __keynumber=key;
        __keycount=0;

        } else {
            // TODO: ADD SHIFT PLANE SWITCHING HERE
            if(key==KB_LSHIFT) {
                __keyplane|=SHIFT_LSHOLD|SHIFT_LS;
                __keyplane&=~(SHIFT_RSHOLD|SHIFT_RS | (SHIFT_RS<<16));
                __keyplane^=SHIFT_LS<<16;
            }
            if(key==KB_RSHIFT) {
                __keyplane|=SHIFT_RSHOLD|SHIFT_RS;
                __keyplane&=~(SHIFT_LSHOLD|SHIFT_LS| (SHIFT_LS<<16));
                __keyplane^=SHIFT_RS<<16;
            }
            if(key==KB_ALPHA) {
                if( __keyflags&KF_ALPHALOCK) {
                    if(__keyplane&SHIFT_ALPHA)  {
                        // OTHER KEYS WERE PRESSED, SO END ALPHA MODE
                        __keyplane&=~((SHIFT_ALPHA<<17)|(SHIFT_ALPHA<<16)); // UNLOCK ALPHA

                    } else {
                            __keyplane|=SHIFT_ALPHA<<17; // LOCK ALPHA
                            __keyplane&=~(SHIFT_ALPHA<<16);
                        }

                }
                else {
                if(__keyplane&SHIFT_ALPHA)  {
                    if(__keyplane&(SHIFT_ALPHA<<16)) {
                        // DOUBLE ALPHA KEYPRESS
                        __keyplane|=SHIFT_ALPHA<<17; // LOCK ALPHA
                        __keyplane&=~(SHIFT_ALPHA<<16);
                    }
                    else {
                        // OTHER KEYS WERE PRESSED, SO END ALPHA MODE
                        __keyplane&=~((SHIFT_ALPHA<<17)|(SHIFT_ALPHA<<16)); // UNLOCK ALPHA
                    }
                }
                else __keyplane^=SHIFT_ALPHA<<16;
                }
                __keyplane|=SHIFT_ALPHAHOLD|SHIFT_ALPHA;


            }
            if(key==KB_ON) {
                __keyplane|=SHIFT_ONHOLD;
            }

            __keyb_postmsg(KM_SHIFT | (__keyplane&SHIFT_ANY));

        }
        }
    }
    else {
        __keyb_postmsg(KM_KEYUP + key);

        if(key<60 || (__keynumber==KB_ALPHA)) {
        if(__keynumber>0) __keynumber=-__keynumber;
        __keycount=-BOUNCE_KEYTIME;
        __keyplane&=~((SHIFT_LS|SHIFT_RS|SHIFT_ALPHA)<<16);

        if(!(__keyplane& (SHIFT_LSHOLD | SHIFT_RSHOLD | SHIFT_ALPHAHOLD | SHIFT_ONHOLD))) {
            int oldkeyplane=__keyplane;
            __keyplane&=~(SHIFT_LS|SHIFT_RS|SHIFT_ALPHA); // KILL ALL SHIFT PLANES
            __keyplane|=(__keyplane>>17)&SHIFT_ALPHA; // KEEP ALPHA IF LOCKED
            if(oldkeyplane!=__keyplane)	__keyb_postmsg(KM_SHIFT | (__keyplane&SHIFT_ANY));
        }
        }
        else {
            // TODO: ADD SHIFT PLANE SWITCHING HERE
            if(key==KB_LSHIFT) {
                __keyplane&=~((SHIFT_LSHOLD|SHIFT_LS)^((__keyplane>>16)&SHIFT_LS));
                __keyplane&=~((SHIFT_ALPHA)^(((__keyplane>>16)|(__keyplane>>17))&SHIFT_ALPHA));

            }
            if(key==KB_RSHIFT) {
                __keyplane&=~((SHIFT_RSHOLD|SHIFT_RS)^((__keyplane>>16)&SHIFT_RS));
                __keyplane&=~((SHIFT_ALPHA)^(((__keyplane>>16)|(__keyplane>>17))&SHIFT_ALPHA));

            }
            if(key==KB_ALPHA) {
                __keyplane&=~((SHIFT_ALPHAHOLD|SHIFT_ALPHA)^(((__keyplane>>16)|(__keyplane>>17))&SHIFT_ALPHA));

            }
            if(key==KB_ON) {
                __keyplane&=~SHIFT_ONHOLD;
            }
            __keyb_postmsg(KM_SHIFT | (__keyplane&SHIFT_ANY));

            __keynumber=-key;
            __keycount=-BOUNCE_KEYTIME;
        }

    }
    }
    b>>=1;
    a>>=1;
    ++key;
    }
    }

    // ANALYZE STATUS OF CURRENT KEYPRESS
    if(__keynumber>=0) {
    if(__kmat & (1LL<<__keynumber)) {
        // KEY STILL PRESSED, INCREASE COUNTER
        ++__keycount;
        if( (__keycount>LONG_KEYPRESSTIME) )
        {
            if(!(__keyflags&KF_NOREPEAT)) {
            __keyb_postmsg(KM_PRESS | __keynumber | (__keyplane&SHIFT_ANY));
            __keycount=-REPEAT_KEYTIME;
            }
            else {
                // NOKEYBOARD REPEAT, ISSUE A LONG KEYPRESS
                __keyb_postmsg(KM_LPRESS | __keynumber | (__keyplane&SHIFT_ANY));
                __keynumber=-__keynumber; // THERE WILL BE NO REPETITIONS OF LONG KEYPRESS
            }
        }

        if(!__keycount) {
            __keyb_postmsg(KM_PRESS | __keynumber | (__keyplane&SHIFT_ANY));
            __keycount=-REPEAT_KEYTIME;
        }

    }
    }


    // REPEATER
    if(__kmat==0) {
        if(__keycount>=0) { tmr_events[0].status=0; __keynumber=0; }
        else { ++__keycount; }
    } else {

        if(!(tmr_events[0].status&1)) {
            // ACTIVATE THE TIMER EVENT IF NOT ALREADY RUNNING
            tmr_events[0].ticks=tmr_ticks()+tmr_events[0].delay;
            tmr_events[0].status=3;
            __tmr_eventreschedule();
    }
    }


    // On-C and On-A-F handling

    if(__kmat== ((1LL<<KB_ON) | (1LL<<KB_A)  | (1LL<<KB_F)))
    {
        // ON-A-F pressed, offer the option to stop the program

        __keyb_lock=0;


        throw_exception("User BREAK requested",__EX_CONT | __EX_WARM | __EX_RESET );

        //  AFTER RETURNING FROM THE EXCEPTION HANDLER, ALL KEYS ARE GUARANTEED TO BE RELEASED
        //  DO AN UPDATE TO SEND KEY_UP MESSAGES TO THE APPLICATION AND CORRECT SHIFT PLANES
        goto doupdate;

    }

    __keyb_lock=0;

}

void __keyb_int_handler()
{

*EINTMASK|=0xfe70;

    __keyb_update();

*EINTPEND|=0xfe70;
*EINTMASK&=~0xfe70;


}


void __keyb_init()
{
__keyflags=KF_RUNNING;
__keyplane=0;
__kused=__kcurrent=0;
__keynumber=0;
__kmat=0LL;
__keyb_repeattime=80/KEYB_SCANSPEED;
__keyb_longpresstime=1000/KEYB_SCANSPEED;
__keyb_debounce=20/KEYB_SCANSPEED;
__keyb_lock=0;
// INITIALIZE TIMER EVENT 0

tmr_events[0].eventhandler=__keyb_update;
tmr_events[0].delay=(KEYB_SCANSPEED*tmr_getsysfreq())/1000;

tmr_events[0].status=0;

// MASK ALL EXTINT UNTIL THEY ARE PROPERLY PROGRAMMED
*INTMSK|=0x31;


*GPGCON=0x5555AAA9; // DRIVE ALL COLUMNS TO OUTPUT, ROWS TO EINT
*GPGDAT=0;      // DRIVE OUTPUTS LOW
*GPGUP=0x1;    // ENABLE PULLUPS ON ALL INPUT LINES, DISABLE ON ALL OUTPUTS
//keysave[1]=*GPFCON;
*GPFCON=(*GPFCON&0xffffc0fc)|0x2a02;    // SET ALL SHIFTS TO GENERATE INTERRUPTS

__irq_addhook(5,&__keyb_int_handler);
__irq_addhook(4,&__keyb_int_handler);	// SHIFTS
__irq_addhook(0,&__keyb_int_handler); // ON

*EXTINT0=(*EXTINT0&0xf000fff0)|0x06660006;      // ALL SHIFTS TRIGGER ON BOTH EDGES
*EXTINT1=0x66666666;                            // ALL OTHER KEYS TRIGGER ON BOTH EDGES
*EINTMASK=(*EINTMASK&0x00ff018f);               // UNMASK 4,5,6 AND 9-15
*EINTPEND=0xffffffff;                           // CLEAR ALL PENDING INTERRUPTS
*INTMSK=*INTMSK&0xffffffce;                     // UNMASK EXTERNAL INTERRUPTS
*SRCPND|=0x31;
*INTPND|=0x31;                     // UNMASK EXTERNAL INTERRUPTS




}


void __keyb_stop(unsigned int *keysave)
{

tmr_events[0].status=0;

// DISABLE INTERRUPTS, STATUS WILL BE FULLY RESTORED ON EXIT
*INTMSK|=0x31;
*EINTMASK|=0xFE70;
__irq_releasehook(5);
__irq_releasehook(4);
__irq_releasehook(0);

// RESTORE IO PORT CONFIGURATION
*GPGCON=keysave[0];
*GPFCON=keysave[1];
__keyflags&=~1;
}


void keyb_settiming(int repeat,int longpress,int debounce)
{
    __keyb_repeattime=(repeat+KEYB_SCANSPEED-1)/KEYB_SCANSPEED;
    __keyb_longpresstime=(longpress+KEYB_SCANSPEED-1)/KEYB_SCANSPEED;
    __keyb_debounce=(debounce+KEYB_SCANSPEED-1)/KEYB_SCANSPEED;
}

void keyb_setrepeat(int repeat)
{
    if(!repeat) __keyflags|=KF_NOREPEAT;
    else __keyflags&=~KF_NOREPEAT;
}

void keyb_setalphalock(int single_alpha_lock)
{
    if(single_alpha_lock) __keyflags|=KF_ALPHALOCK;
    else __keyflags&=~KF_ALPHALOCK;

}

void keyb_setshiftplane(int leftshift,int rightshift,int alpha,int alphalock)
{
    while(keyb_getmatrix()!=0LL) ;		// WAIT UNTIL NO MORE KEYS ARE PRESSED TO UPDATE SHIFT STATE
    if(leftshift) __keyplane|=SHIFT_LS|(SHIFT_LS<<16);
    else __keyplane&=~(SHIFT_LS|(SHIFT_LS<<16));
    if(rightshift) __keyplane|=SHIFT_RS|(SHIFT_RS<<16);
    else __keyplane&=~(SHIFT_RS|(SHIFT_RS<<16));
    if(alpha||alphalock) __keyplane|=SHIFT_ALPHA|(SHIFT_ALPHA<<16);
    else __keyplane&=~(SHIFT_ALPHA|(SHIFT_ALPHA<<16));
    if(alphalock) {
    __keyplane|=SHIFT_ALPHA<<17; // LOCK ALPHA
    __keyplane&=~(SHIFT_ALPHA<<16);
    }
    else {
        __keyplane&=~(SHIFT_ALPHA<<17);
    }
    keyb_postmsg(KM_SHIFT | (__keyplane&SHIFT_ANY));

}



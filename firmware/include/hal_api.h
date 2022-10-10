/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef _HAL_API_H
#define _HAL_API_H

#include "usb.h"
#include "fontlist.h"
#include "ggl.h"
#include "newrpl.h"
#include "recorder.h"
#include "unifont.h"
#include "keyboard.h"

#ifndef EXTERN
#  define EXTERN extern
#endif

#ifndef NULL
#  define NULL 0
#endif

//! Type to use with all timer routines: ticks and frequencies are 64-bit
typedef long long tmr_t;
//! Handle to refer to a timed event.
typedef int       HEVENT;

// COMPACT TIME STRUCTURE, SIMILAR TO STANDARD tm BUT USING ONLY 64 BITS.
struct compact_tm
{
    unsigned tm_sec : 6, // seconds after the minute    0-60*
        tm_min      : 6, // minutes after the hour      0-59
        tm_hour     : 5, // hours since midnight        0-23
        tm_mday     : 5, // day of the month            1-31
        tm_mon      : 4, // months since January        0-11
        tm_wday     : 3, // days since Sunday           0-6
        tm_isdst    : 1;
    int tm_year; // int     years since 1900
};

enum halFlagsEnum
{
    HAL_FASTMODE       = 1,
    HAL_SLOWLOCK       = 2,
    HAL_HOURGLASS      = 4,
    HAL_AUTOOFFTIME1   = 8,
    HAL_AUTOOFFTIME2   = 16,
    HAL_AUTOOFFTIME3   = 32,
    HAL_NOCLOCKCHANGE  = 64,
    HAL_SKIPNEXTALARM  = 128,
    HAL_HALTED         = 256,
    HAL_TIMEOUT        = 512,
    HAL_AUTORESUME     = 1024,
    HAL_FASTAUTORESUME = 2048,
    HAL_POWEROFF       = 4096,
    HAL_RESET          = 8192,
    HAL_HWRESET        = 16384,
    HAL_QUICKRESPONSE  = 32768
    // ADD MORE BITS HERE
    // STARTING WITH 2^16 = 65536 ARE NOTIFICATION FLAGS
};

#define HAL_AUTOOFFTIME           (HAL_AUTOOFFTIME1 | HAL_AUTOOFFTIME2 | HAL_AUTOOFFTIME3)
#define GET_AUTOOFFTIME(flags)    (((flags) &HAL_AUTOOFFTIME) / HAL_AUTOOFFTIME1)
#define SET_AUTOOFFTIME(ntimes15) ((ntimes15 * HAL_AUTOOFFTIME1) & HAL_AUTOOFFTIME)

// ANNUNCIATORS

/*
 * (131, 0) - Remote
 * (131, 1) - Left Shift
 * (131, 2) - Right Shift
 * (131, 3) - Alpha
 * (131, 4) - Low Battery
 * (131, 5) - Wait
 */
enum halNotification
{
    N_CONNECTION = 0,
    N_LEFT_SHIFT,
    N_RIGHT_SHIFT,
    N_ALPHA,
    N_LOWBATTERY,
    N_HOURGLASS,
    N_DATARECVD,
    N_ALARM,
};

enum halFonts
{
    FONT_INDEX_STACK = 0,
    FONT_INDEX_STACK_LEVEL1,
    FONT_INDEX_STACK_INDEX,
    FONT_INDEX_CMDLINE,
    FONT_INDEX_CURSOR,
    FONT_INDEX_MENU,
    FONT_INDEX_STATUS,
    FONT_INDEX_PLOT,
    FONT_INDEX_FORMS,
    FONT_INDEX_ERRORS,
    FONT_INDEX_HELP_TEXT,
    FONT_INDEX_HELP_TITLE,
    FONT_INDEX_HELP_BOLD,
    FONT_INDEX_HELP_ITALIC,
    FONT_INDEX_HELP_CODE,
    FONT_INDEX_BATTERY,
    FONTS_NUM
};

#define FONT(F)                 (halScreen.FontArray[FONT_INDEX_##F])
#define FONT_STACK              FONT(STACK)
#define FONT_STACK_LEVEL1       FONT(STACK_LEVEL1)
#define FONT_CMDLINE            FONT(CMDLINE)
#define FONT_CURSOR             FONT(CURSOR)
#define FONT_MENU               FONT(MENU)
#define FONT_STATUS             FONT(STATUS)
#define FONT_PLOT               FONT(PLOT)
#define FONT_FORMS              FONT(FORMS)
#define FONT_ERRORS             FONT(ERRORS)
#define FONT_HELP_TEXT          FONT(HELP_TEXT)
#define FONT_HELP_TITLE         FONT(HELP_TITLE)
#define FONT_HELP_BOLD          FONT(HELP_BOLD)
#define FONT_HELP_ITALIC        FONT(HELP_ITALIC)
#define FONT_HELP_CODE          FONT(HELP_CODE)
#define FONT_BATTERY            FONT(BATTERY)
#define FONT_HEIGHT(F)          ((F)->BitmapHeight)

#define FORM_DIRTY              1
#define STACK_DIRTY             2
#define CMDLINE_DIRTY           4
#define CMDLINE_LINE_DIRTY       8
#define CMDLINE_CURSOR_DIRTY    16
#define CMDLINE_ALL_DIRTY       (4 + 8 + 16)
#define MENU1_DIRTY             32
#define MENU2_DIRTY             64
#define MENU_DIRTY              (MENU1_DIRTY | MENU2_DIRTY)
#define STATUS_DIRTY            128
#define HELP_DIRTY              256
#define ERROR_DIRTY             512
#define BACKGROUND_DIRTY        1024
#define ALL_DIRTY               ~0U
#define BUFFER_LOCK             16384
#define BUFFER_ALT              32768


// NUMBER OF ENTRIES IN THE RENDER CACHE

#define MAX_RENDERCACHE_ENTRIES 32

// STRUCT TO CONTAIN THE HEIGHT IN PIXELS OF SCREEN AREAS (0=INVISIBLE)
typedef struct
{
    int            Form;
    int            Stack;
    int            CmdLine;
    int            Menu1;
    int            Menu2;
    utf8_p         HelpMessage;
    unsigned       HelpLine;
    unsigned       HelpMaxLine;
    utf8_p         ShortHelpMessage;
    utf8_p         ErrorMessage;
    unsigned       ErrorMessageLength;
    int            DirtyFlag;
    HEVENT         CursorTimer;
    UNIFONT const *FontArray[FONTS_NUM]; // POINTERS TO FONTS
    WORD           FontHash[FONTS_NUM];  // HASH TO DETECT FONT CHANGES
    // VARIABLES FOR THE TEXT EDITOR / COMMAND LINE
    int            LineVisible, LineCurrent, LineIsModified;
    int            NumLinesVisible; // HEIGHT OF COMMAND LINE AREA IN LINES OF TEXT
    int            CursorState;     // Lowercase, Uppercase, Token, VISIBLE OR INVISIBLE
    int            CursorPosition;  // OFFSET FROM START OF CURRENT LINE
    int            CursorX, XVisible;
    int            SelStart, SelEnd;         // CURRENT SELECTION START/END OFFSET WITHIN THE LINE
    int            SelStartLine, SelEndLine; // CURRENT SELECTION START/END LINE (-1 IF NO SELECTION)
    int            CmdLineState;             // STATUS FLAGS FOR THE COMMAND LINE
    int            CmdLineIndent;            // INDENT SPACING
    int            ACTokenStart;             // START OF TOKEN FOR AUTO COMPLETE, OFFSET FROM START OF LINE
    WORD           ACSuggestion;             // CURRENT SUGGESTED OPCODE

    // VARIABLES FOR USER INTERFACE
    int            StkUndolevels, StkCurrentLevel;
    unsigned       KeyContext;

    // INTERACTIVE STACK VARIABLES
    int            StkPointer, StkVisibleLvl, StkSelStart, StkSelEnd;
    int            StkVisibleOffset, StkSelStatus;

    // DEFERRED IDLE PROCESSES
    void (*Processes[3])(void);

} HALSCREEN;

extern HALSCREEN halScreen;

typedef enum keyboard_context
// ----------------------------------------------------------------------------
// Keyboard context
// ----------------------------------------------------------------------------
{
    CONTEXT_ANY               = 0,        // Any context
    CONTEXT_EDITOR            = (1 << 0), // Command line is active
    CONTEXT_STACK             = (1 << 1), // Editing
    CONTEXT_INTERACTIVE_STACK = (1 << 2), // Interactive stack manipulation
    CONTEXT_PLOT              = (1 << 3),
    CONTEXT_PICT              = (1 << 4),
    CONTEXT_FORM              = (1 << 5),
    CONTEXT_SYSTEM            = (1 << 6), // Some other system context
    CONTEXT_USER              = 1024, // User contexts are above CONTEXT_USER
    CONTEXT_SYSTEM_MASK       = CONTEXT_USER - 1,
} keyboard_context_t;


#define CMDSTATE_OPEN       0x100
#define CMDSTATE_FULLSCREEN 0x200
#define CMDSTATE_SELBEGIN   0x400
#define CMDSTATE_SELEND     0x800
#define CMDSTATE_SELECTION  0xC00
#define CMDSTATE_OVERWRITE  0x1000
#define CMDSTATE_ACACTIVE   0x2000
#define CMDSTATE_ACUPDATE   0x4000

// Type definition for interrupt handler functions

typedef void (*tmr_event_fn)(void);

enum
{
    MEM_AREA_RSTK = 0,
    MEM_AREA_DSTK,
    MEM_AREA_DIR,
    MEM_AREA_LAM,
    MEM_AREA_TEMPOB,
    MEM_AREA_TEMPBLOCKS
};


// MAIN EXCEPTION PROCESSOR

#define EX_CONT    1   // SHOW CONTINUE OPTION
#define EX_EXIT    2   // SHOW EXIT OPTION
#define EX_WARM    4   // SHOW WARMSTART OPTION
#define EX_RESET   8   // SHOW RESET OPTION
#define EX_NOREG   16  // DON'T SHOW REGISTERS
#define EX_WIPEOUT 32  // FULL MEMORY WIPEOUT AND WARMSTART
#define EX_RPLREGS 64  // SHOW RPL REGISTERS INSTEAD
#define EX_RPLEXIT 128 // SHOW EXIT OPTION, IT RESUMES EXECUTION AFTER SETTING Exception=EX_EXITRPL
#define EX_MEMDUMP 256 // SHOW A MEMORY DUMP OF THE ADDRESS PASSED UP AS THE EXCEPTION MESSAGE

/*!
    \brief Throw a user exception
    Cause a user exception to be thrown. It displays the requested message and offer the user several
    options to treat the exception.

    \param message The string that will be displayed.
    \param options One or more of the following constants:
    \li EX_CONT Display the "Continue" option, allowing the program to continue.
    \li EX_EXIT Display the "Exit" option, which immediately exits the program.
    \li EX_WARM Display the "Warmstart" option, which exits the program and causes a
                  restart of the calculator, similar to On-C.
    \li EX_RESET Display the "Reset" option, which exits the program and then reset the calculator
                   in a way equivalent to a paperclip. It is similar to a Warmstart but also restarts
                   all the ARM hardware.
    \note If the options parameter is passed as 0, the calculator will display the exception message
          and show no options, staying in an infinite loop until the user resets the calc using a
          paperclip. Only use this option if it is not safe to attempt to exit the program.

    \return The function does not return a value, and it may not return at all depending on the
            user's choice to handle the exception. If the user chooses to exit, warmstart or reset
            the program will exit first as if the exit() function was called.
*/
void throw_exception(cstring message, unsigned int options);

void throw_dbgexception(cstring message, unsigned int options);

// *****************************************************
// **************  IRQ MACHINERY ***********************
// *****************************************************

/*!
    \brief Install an IRQ handler
    Set a routine to service interrupt requests from a specific device. It does not change the
    state of the interrupt controller, so the user needs to manually unmask the interrupt and
    configure the rest of the hardware to generate the proper IRQ request.

    \param service_number Identifies the device that is causing the interrupt. It's a
                          number from 0 to 31 according to the list below (see Samsung S3C2410X manual).
    \param serv_routine   Address of a service routine. The service routine has no special requirements, other
                          than returning as fast as possible to allow other IRQ's to be processed.
    \note The interrupt service number is as follows:
\liINT_ADC  = [31]
\liINT_RTC  = [30]
\liINT_SPI1 = [29]
\liINT_UART0= [28]
\liINT_IIC  = [27]
\liINT_USBH = [26]
\liINT_USBD = [25]
\liReserved = [24] Not used
\liINT_UART1= [23]
\liINT_SPI0 = [22]
\liINT_SDI  = [21]
\liINT_DMA3 = [20]
\liINT_DMA2 = [19]
\liINT_DMA1 = [18]
\liINT_DMA0 = [17]
\liINT_LCD  = [16]
\liINT_UART2= [15]
\li\liINT_TIMER4=[14]
\liINT_TIMER3=[13]
\liINT_TIMER2=[12] [Used for sound routines, do not use]
\liINT_TIMER1=[11] [Used for timed events, do not use]
\liINT_TIMER0=[10] [Used for system timer, do not use]
\liINT_WDT  = [9]
\liINT_TICK = [8]
\linBATT_FLT= [7]
\liReserved = [6] Not used
\liEINT8_23 = [5] [Used for keyboard routines, do not use]
\liEINT4_7  = [4] [Used for keyboard routines, do not use]
\liEINT3    = [3] [Used for SD Card insertion detect, do not use]
\liEINT2    = [2]
\liEINT1    = [1]
\liEINT0    = [0] [Used for exception handler, do not use]

 \sa irq_releasehook
*/

void irq_add_hook(int service_number, tmr_event_fn serv_routine);

/*!
    \brief Uninstall an IRQ handler
    Removes a service routine that handles interrupt requests from a specific device. It does not change the
    state of the interrupt controller, so the user needs to manually mask the interrupt and
    configure the rest of the hardware to stop generating IRQ requests. If an IRQ is generated
    after this routine is called, it will be serviced by a do-nothing routine.

    \param service_number Identifies the device that is causing the interrupt. It's a
                          number from 0 to 31 according to the list below (see Samsung S3C2410X manual).
    \note See irq_add_hook for a list of interrupt service numbers

 \sa irq_add_hook
*/

void irq_releasehook(int service_number);

void irq_mask(int service_number);
void irq_unmask(int service_number);
void irq_clrpending(int service_number);

// Saves usb_drvstatus
void usb_mutex_lock(void);
void usb_mutex_unlock(void);




#define NUM_EVENTS 5 // NUMBER OF SIMULTANEOUS TIMED EVENTS

// INTERNAL USE ONLY
typedef struct
{
    tmr_event_fn eventhandler;
    long long     ticks;
    unsigned int  delay;
    unsigned int  status; // 1=ACTIVE, 2=AUTORELOAD, 4=PAUSED, NOT FINISHED
} timed_event;

// LOW-LEVEL TIMER STRUCTURE
extern timed_event tmr_events[NUM_EVENTS];

//! Save all timers state to a buffer (13-word)
void               tmr_save(unsigned int *tmrbuffer);
//! Restore saved timers state from buffer
void               tmr_restore(unsigned int *tmrbuffer);
//! Setup system timers and event scheduler - automatically called at startup
void               tmr_setup();

//! Get the frequency of the system timer in ticks per second, normally 100 KHz or 125KHz
tmr_t              tmr_getsysfreq();

//! Get system timer count in ticks since program started.
tmr_t              tmr_ticks();
//! Do not return until the specified time has passed
void               tmr_delayms(int milliseconds);
//! Do not return until the specified time has passed (in microseconds). Accuracy is +/- 10 microseconds.
void               tmr_delayus(int microseconds);

//! Calculate elapsed time in milliseconds between before and after (both given in ticks)
int                tmr_ticks2ms(tmr_t before, tmr_t after);
//! Calculate elapsed time in microseconds between before and after (both given in ticks)
int                tmr_ticks2us(tmr_t before, tmr_t after);
//! Add/subtract an interval in milliseconds to the given time in ticks
tmr_t              tmr_addms(tmr_t time, int ms);
//! Add/subtract an interval in microseconds to the given time in ticks
tmr_t              tmr_addus(tmr_t time, int us);
//! Wait until the system timer reaches the given time in ticks
void               tmr_waituntil(tmr_t time);

//! Macro to convert milliseconds to ticks
#define tmr_ms2ticks(a) tmr_addms((tmr_t) 0, (a))

//! Macro to convert milliseconds to ticks
#define tmr_us2ticks(a) tmr_addus((tmr_t) 0, (a))

/*!
    \brief Create a timed event.
    Create a new timed event, specifying a callback function that will be called after
    the given time has passed. The autorepeat feature allows the event to be automatically
    rescheduled for another interval of time. Autorepeated events need to be manually removed
    by tmr_event_kill. One-shot events will remove themselves after completion (no need to explicitly
    call tmr_event_kill).

    \param handler  The function that will be called back on every interval
    \param ms       Time interval in milliseconds after which the handler will be called
    \param autorepeat If TRUE, the event will be repeated every 'ms' milliseconds, if FALSE
                      the event will be executed only once after 'ms' milliseconds have passed
                      since the event was created.

    \return An event handler, or -1 if there are no more handles available (see notes).

    \note A maximum of NUM_EVENTS (typically 5) can be created simultaneously.

 \sa tmr_event_kill
*/

HEVENT      tmr_event_create(tmr_event_fn handler, unsigned int ms, int autorepeat);

/*!
    \brief Kill a timed event.
    Stops an autoreloading event.

    \param event The event handler obtained from tmr_event_create

 \sa tmr_event_create
*/

void         tmr_event_kill(HEVENT event);

// Battery level measurement API
void         battery_setup();
void         battery_handler();
void         battery_read();
int          battery_level();    // Normalized 0% - 100%
int          battery_charging(); // Indicate if charging (e.g. USB)
int          battery_low();      // Returns 1 for low, 2 for critical

// VARIABLE WHERE THE BATTERY STATUS IS STORED
extern WORD battery;

// const unsigned int System5Font[];
// const unsigned int System6Font[];
// const unsigned int System7Font[];
// const unsigned int MiniFont[];

coord        DrawText(gglsurface    *drawsurf,
                      coord          x,
                      coord          y,
                      utf8_p         Text,
                      UNIFONT const *Font,
                      pattern_t      color);
coord        DrawTextN(gglsurface    *drawsurf,
                       coord          x,
                       coord          y,
                       utf8_p         Text,
                       utf8_p         End,
                       UNIFONT const *Font,
                       pattern_t      color);
coord        DrawTextBk(gglsurface    *drawsurf,
                        coord          x,
                        coord          y,
                        utf8_p         Text,
                        UNIFONT const *Font,
                        pattern_t      color,
                        pattern_t      bkcolor);
coord        DrawTextBkN(gglsurface    *drawsurf,
                         coord          x,
                         coord          y,
                         utf8_p         Text,
                         utf8_p         End,
                         UNIFONT const *Font,
                         pattern_t      color,
                         pattern_t      bkcolor);
coord        DrawTextMono(gglsurface    *drawsurf,
                          coord          x,
                          coord          y,
                          utf8_p         Text,
                          UNIFONT const *Font,
                          pattern_t      colors);

static inline utf8_p StringEnd(utf8_p str)
{
    while (*str)
        str++;
    return str;
}

size        StringWidth(utf8_p Text, UNIFONT const *Font);
size        StringWidthN(utf8_p Text, utf8_p End, UNIFONT const *Font);
utf8_p      StringCoordToPointer(utf8_p         Text,
                                 utf8_p         End,
                                 UNIFONT const *Font,
                                 int           *xcoord);

int         cpu_get_lock(int lockvar, volatile int *lock_ptr);
int         cpu_setspeed(int);
void        cpu_wait_for_interrupt();
void        cpu_off();
void        cpu_intoff();
void        cpu_inton();

// LCD LOW-LEVEL HARDWARE API

extern int  lcd_contrast;
void        lcd_sync();
void        lcd_fix();
void        lcd_off();
void        lcd_on();
void        lcd_poweron();
void        lcd_set_contrast(int level);
int         lcd_setmode(int mode, unsigned int *physbuf);
void        lcd_save(unsigned int *buf);
void        lcd_restore(unsigned int *buf);
int         lcd_scanline();
void        lcd_setactivebuffer(int buffer);
int         lcd_getactivebuffer();

// BASIC LOW-LEVEL INTERRUPT HANDLERS
void        exception_install();
void        irq_install();
void        keyb_irq_init();

// LOW-LEVEL MEMORY MANAGEMENT
void        create_mmu_tables();
void        enable_mmu();
void        set_stackall();

// LOW-LEVEL HARDWARE DRIVERS - POWER
void        cpu_off_prepare();
void        cpu_off_die();
void        cpu_flushwritebuffers();
void        cpu_flushTLB();
void        cpu_flushicache(void);

// LOW-LEVEL COMMON USB API
void        usb_sendcontrolpacket(int packet_type);
void        usb_receivecontrolpacket();
int         usb_waitforreport();
USB_PACKET *usb_getreport();
void        usb_releasereport();

// USER-LEVEL COMMON USB API
int         usb_isconnected();
int         usb_isconfigured();
int         usb_hasdata();
int         usb_waitfordata(int nbytes);

int         usb_txfileopen(int file_type);
int         usb_filewrite(int fileid, byte_p data, int nbytes);
int         usb_txfileclose(int fileid);

int         usb_rxfileopen();
int         usb_rxbytesready(int fileid);
int         usb_eof(int fileid);
int         usb_fileread(int fileid, byte_p dest, int nbytes);
int         usb_rxfileclose(int fileid);
// file_type = 'O','B','W', OR 'D', SEE SPECS
#define usb_filetype(fileid) ((fileid) >> 8)

// LOW-LEVEL HARDWARE DRIVERS - KEYBOARD
void        keyb_irq_wait_release();

// LOW-LEVEL HARDWARE DRIVERS - FLASH MEMORY
void        flash_CFIRead(unsigned short *ptr);
void        flash_prepareforwriting();
void        flash_donewriting();
void        ram_startfwupdate();

// LOW-LEVEL MEMORY SUBALLOCATOR FOR FILE SYSTEM
void        init_simpalloc();

// LOW-LEVEL DRIVER FOR REAL TIME CLOCK
void        rtc_getdatetime(struct date *dt, struct time *tm);
int         rtc_setdatetime(struct date dt, struct time tm);
struct date rtc_getdate();
struct time rtc_gettime();
int         rtc_settime(struct time tm);
int         rtc_setdate(struct date dt);
void        rtc_getalarm(struct date *dt, struct time *tm, int *enabled);
int         rtc_setalarm(struct date dt, struct time tm, int enabled);
int         rtc_chkalrm();
void        rtc_setaie(int enabled);
void        rtc_poweron();
void        rtc_poweroff();
void        rtc_reset();

// LOW-LEVEL HARDWARE DRIVERS - USB
void        usb_init(int force);
void        usb_shutdown();
int         usb_isconnected();
int         usb_isconfigured();

// HIGHER LEVEL MEMORY MANAGEMENT

word_p    *halGrowMemory(int32_t zone, word_p *base, int32_t newsize);
int         halGetFreePages();
int         halGetTotalPages();
int         halCheckMemoryMap();
int         halCountUsedPages(int zone);
int         halCheckRplMemory();
void        halInitMemoryMap();

// HIGHER LEVEL GLOBAL VARIABLES

extern int32_t halFlags;
extern void (*halProcesses[3])(void);
extern HEVENT halBusyEvent, halTimeoutEvent;
extern int32_t   halLongKeyPending;
extern int32_t   halKeyMenuSwitch;

// HIGHER LEVEL HAL FUNCTIONS

// CPU AND POWER MANAGEMENT FUNCTIONS
void          halSetBusyHandler();
void          halInitBusyHandler();
void          halEnterPowerOff();

void          halPreparePowerOff();
void          halWakeUp();
void          halCPUSlowMode();
void          halCPUFastMode();
void          halReset();

// TIMER FUNCTIONS
int64_t        halTicks();

// SOTWARE ALARM FUNCTIONS
void          halTriggerAlarm();

// HARDWARE CLOCK AND ALARM FUNCTIONS
struct date   halGetSystemDate();
int           halSetSystemDate(struct date dt);
struct time   halGetSystemTime();
int           halSetSystemTime(struct time tm);
void          halGetSystemAlarm(struct date *dt, struct time *tm, int *enabled);
int           halSetSystemAlarm(struct date dt, struct time tm, int enabled);
void          halGetSystemDateTime(struct date *dt, struct time *tm);
void          halDisableSystemAlarm();
int           halCheckSystemAlarm();

// Screen functions
void          halInitScreen();
void          halSetupTheme(color16_t *palette);
void          halSetNotification(enum halNotification type, unsigned color);
unsigned      halGetNotification(enum halNotification type);
void          halShowMsg(utf8_p Text);
void          halShowMsgN(utf8_p Text, utf8_p End);
void          halSetCmdLineHeight(int h);
void          halStatusAreaPopup();
void          halUpdateFonts();
void          halRedrawAll(gglsurface *scr);
void          halRedrawCmdLine(gglsurface *scr);
void          halUpdateStatus();
void          halSetStackHeight(int h);
void          halSetFormHeight(int h);
void          halSetMenu1Height(int h);
void          halSetMenu2Height(int h);

static inline void halRefresh(unsigned area)
{
    halScreen.DirtyFlag |= area;
}
static inline void halRepainted(unsigned area)
{
    halScreen.DirtyFlag &= ~area;
}

static inline int halDirty(unsigned area)
{
    return (halScreen.DirtyFlag & area) != 0;
}

// Error reporting and messages
word_p             halGetCommandName(word_p NameObject);

// Higher level UI
keyboard_context_t halContext(keyboard_context_t mask);
void               halSetContext(keyboard_context_t KeyContext);
void               halExitContext(keyboard_context_t KeyContext);
void               halSetCmdLineMode(BYTE mode);
BYTE               halGetCmdLineMode();
void               halForceAlphaModeOn();
void               halForceAlphaModeOff();

// Forms, plots and other screens
void               halSwitch2Form();
void               halSwitch2Stack();

// OUTER LOOP FLAGS
#define OL_NOEXIT        1   // DON'T POLL EXTERNAL EXIT FUNCTION DURING OUTER POL
#define OL_NOAUTOOFF     2   // DON'T ALLOW AUTO-OFF FEATURE WHILE WAITING
#define OL_NOALARM       4   // DON'T ALLOW EXECUTION OF ALARMS AND TIMERS
#define OL_NOSDFLUSH     8   // DON'T FLUSH SD CACHES AFTER 3 SECONDS
#define OL_LONGPRESS     16  // DETECT LONG PRESS MESSAGE ON ALL KEYS
#define OL_NOCUSTOMKEYS  32  // DON'T DO USER-DEFINED ACTIONS
#define OL_NODEFAULTKEYS 64  // DON'T DO DEFAUL KEY ACTIONS
#define OL_NOCOMMS       128 // DON'T AUTOMATICALLY RECEIVE DATA OVER USB OR SERIAL
#define OL_EXITONERROR   256 // EXIT THE POL IF THERE ARE ANY EXCEPTIONS

#define HAL_KEY_WAKEUP  ((keyb_msg_t)  0)
#define HAL_KEY_TIMEOUT ((keyb_msg_t) -1)

// Outer loop
void          halOuterLoop(int32_t timeoutms,
                           int (*dokey)(WORD),
                           int (*doidle)(WORD),
                           int32_t flags);

//  If this function returns true, terminate the outer loop
int           halExitOuterLoop();

// Keyboard functions
void          halInitKeyboard();
keyb_msg_t    halWaitForKey();
keyb_msg_t    halWaitForKeyTimeout(int32_t timeoutms);
void          halPostKeyboardMessage(keyb_msg_t keymsg);
int           halDoDefaultKey(keyb_msg_t keymsg);
int           halDoCustomKey(keyb_msg_t keymsg);

// Idle processes
void          halDeferProcess(void (*function)(void));


// Render cache external DATA
extern word_p halCacheContents[3 * MAX_RENDERCACHE_ENTRIES];
extern WORD   halCacheEntry;


// Render
void          uiClearRenderCache();
void          uiAddCacheEntry(word_p object,
                              word_p bitmap,
                              const UNIFONT *font);
void          uiUpdateOrAddCacheEntry(word_p object,
                                      word_p bitmap,
                                      const UNIFONT *font);
word_p        uiFindCacheEntry(word_p object, const UNIFONT *font);
void          uiDrawObject(gglsurface    *scr,
                           coord          x,
                           coord          y,
                           word_p         object,
                           const UNIFONT *font);
word_p        uiRenderObject(word_p object, const UNIFONT *font);
void          uiDrawBitmap(gglsurface *scr, coord x, coord y, word_p bmp);


RECORDER_DECLARE(hal_api);

#endif

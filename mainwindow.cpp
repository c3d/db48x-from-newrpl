/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <QtGui>
#include <QtCore>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QKeyEvent>
#include <QStandardPaths>

#include "hidapi.h"
#include "usbselector.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qpaletteeditor.h"

// ONLY REQUIRED UNDER MINGW
#ifdef DrawText
#undef DrawText
#endif
#define WORD _WORD

extern "C"
{
#include "ui.h"
#include "firmware.h"
}
#define takemax(a,b) (((a)>(b))? (a):(b))

MainWindow *myMainWindow;

// CAN'T INCLUDE THE HEADERS DIRECTLY DUE TO CONFLICTING TYPES ON WINDOWS ONLY...

extern unsigned long long __pckeymatrix;
extern int __pc_terminate;
extern int __memmap_intact;
extern volatile int __cpu_idle;
extern hid_device *__usb_curdevice;
extern char __usb_devicepath[8192];
extern volatile int __usb_paused;

extern "C" void usb_irqservice();
extern "C" void usb_irqdisconnect();
extern "C" void usb_irqconnect();
extern "C" int usb_isconnected();

extern "C" void __keyb_update();
// BACKUP/RESTORE
extern "C" int rplBackup(int (*writefunc)(unsigned int, void *), void *);
extern "C" int rplRestoreBackup(int, unsigned int (*readfunc)(void *), void *);
extern "C" int rplRestoreBackupMessedup(unsigned int (*readfunc)(void *), void *);      // DEBUG ONLY
extern "C" void __SD_irqeventinsert();

extern int __sd_inserted;
extern int __sd_nsectors;       // TOTAL SIZE OF SD CARD IN 512-BYTE SECTORS
extern int __sd_RCA;
extern unsigned char *__sd_buffer;      // BUFFER WITH THE ENTIRE CONTENTS OF THE SD CARD

extern "C" int usbremotearchivestart();
extern "C" int usbreceivearchive(uint32_t * buffer, int bufsize);
extern "C" int usbremoterestorestart();
extern "C" int usbsendarchive(uint32_t * buffer, int bufsize);
extern "C" int change_autorcv(int newfl);

extern "C" void setExceptionPoweroff();

MainWindow::MainWindow(QWidget * parent):
QMainWindow(parent), rpl(this), usbdriver(this), ui(new Ui::MainWindow), themeEdit(this)
{

    QCoreApplication::setOrganizationName("newRPL");
    QCoreApplication::setApplicationName("newRPL Desktop");

    myMainWindow = this;

    ui->setupUi(this);

    ui->KeybImage->setAttribute(Qt::WA_AcceptTouchEvents);
    ui->KeybImage->installEventFilter(this);
    ui->EmuScreen->setAttribute(Qt::WA_AcceptTouchEvents);
    ui->EmuScreen->installEventFilter(this);

    nousbupdate = true; // DON'T UPDATE THE ON-SCREEN USB CONNECTION STATUS
    __usb_devicepath[0] = 0;    // NULL PATH
    __usb_curdevice = 0;        // USB IS INITIALLY DISCONNECTED
    __usb_paused = 1;   // PAUSE THE USB THREAD
    currentusb.clear();
    currentusbpath.clear();

    usbdriver.start();  // LAUNCH THE USB DRIVER THREAD

    ui->USBDockSelect->setVisible(false);
    screentmr = new QTimer(this);
    ui->EmuScreen->setTimer(screentmr);
    ui->EmuScreen->connect(screentmr, SIGNAL(timeout()), ui->EmuScreen,
            SLOT(update()));
    connect(screentmr, SIGNAL(timeout()), this, SLOT(usbupdate()));
    __memmap_intact = 0;
    __sd_inserted = 0;
    __sd_RCA = 0;
    __sd_nsectors = 0;
    __sd_buffer = NULL;
    ui->actionEject_SD_Card_Image->setEnabled(false);
    ui->actionInsert_SD_Card_Image->setEnabled(true);

    //rpl.start();
    //maintmr->start(1);
    //screentmr->start(20);
    setWindowTitle("newRPL - [Unnamed]");

    QSettings settings;

    QString startfile = settings.value("CurrentFile", QString("")).toString();

    if(!OpenFile(startfile)) {
        rpl.start();
        screentmr->setSingleShot(true);
        screentmr->start(20);
    }

}

MainWindow::~MainWindow()
{
    delete screentmr;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    on_actionExit_triggered();

    event->accept();

}

void MainWindow::resizeEvent(QResizeEvent * event)
{
    int w, h;
    qreal scale;

    //qreal realdpi=qApp->primaryScreen()->logicalDotsPerInch();
    qreal dpratio = qApp->primaryScreen()->devicePixelRatio();
    w = ui->EmuScreen->screen_width;
    h = ui->EmuScreen->screen_height + 5;
    if(!h)
        h = SCREEN_HEIGHT+5;
    if(!w)
        w = SCREEN_WIDTH;
    qreal dpwidth = event->size().width();
    qreal realwidth = dpwidth * dpratio;
    scale = realwidth / w;
    if((int)scale < 1)
        scale = 1.0;
    else
        scale = (int)scale;
    if(event->size().height() * 0.38 * dpratio < scale * h) {
        scale = event->size().height() * 0.38 * dpratio / h;
        if((int)scale < 1)
            scale = 1.0;
        else
            scale = (int)scale;
    }
    // NOW CONVERT BACK TO dp SCALE
    ui->EmuScreen->setScale(scale / dpratio);

}

void MainWindow::on_EmuScreen_destroyed()
{
    if(rpl.isRunning())
        rpl.terminate();
    QMainWindow::close();
}

extern volatile long long __pcsystmr;
int __tmr_singleshot_running = 0;
volatile unsigned long long __tmr1_msec;

extern "C" void __tmr_newirqeventsvc();

extern "C" void stop_singleshot()
{
    __tmr_singleshot_running = 0;
}

extern "C" void timer_singleshot(int msec)
{
    __tmr1_msec = msec;
    __tmr_singleshot_running = 1;
}


#ifndef TARGET_PC_PRIMEG1

const int keyMap[] = {
    /*

       KEYBOARD BIT MAP
       ----------------
       This is the bit number in the 64-bit keymatrix.
       Bit set means key is pressed.

       A]-+  B]-+  C]-+  D]-+  E]-+  F]-+
       |41|  |42|  |43|  |44|  |45|  |46|
       +--+  +--+  +--+  +--+  +--+  +--+

       G]-+  H]-+  I]-+        UP]+
       |47|  |53|  |54|        |49|
       +--+  +--+  +--+  LF]+  +--+  RT]+
       |50|  DN]+  |52|
       J]-+  K]-+  L]-+  +--+  |51|  +--+
       |55|  |57|  |58|        +--+
       +--+  +--+  +--+

       M]--+  N]--+  O]--+  P]--+  BKS]+
       | 33|  | 25|  | 17|  | 09|  | 01|
       +---+  +---+  +---+  +---+  +---+

       Q]--+  R]--+  S]--+  T]--+  U]--+
       | 34|  | 26|  | 18|  | 10|  | 02|
       +---+  +---+  +---+  +---+  +---+

       V]--+  W]--+  X]--+  Y]--+  /]--+
       | 35|  | 27|  | 19|  | 11|  | 03|
       +---+  +---+  +---+  +---+  +---+

       AL]-+  7]--+  8]--+  9]--+  *]--+
       | 60|  | 28|  | 20|  | 12|  | 04|
       +---+  +---+  +---+  +---+  +---+

       LS]-+  4]--+  5]--+  6]--+  -]--+
       | 61|  | 29|  | 21|  | 13|  | 05|
       +---+  +---+  +---+  +---+  +---+

       RS]-+  1]--+  2]--+  3]--+  +]--+
       | 62|  | 30|  | 22|  | 14|  | 06|
       +---+  +---+  +---+  +---+  +---+

       ON]-+  0]--+  .]--+  SP]-+  EN]-+
       | 63|  | 31|  | 23|  | 15|  | 07|
       +---+  +---+  +---+  +---+  +---+

     */
    Qt::Key_Backspace, 1,
    Qt::Key_U, 2,
    Qt::Key_Slash, 3,
    Qt::Key_Z, 3,
    Qt::Key_Asterisk, 4,
    Qt::Key_Minus, 5,
    Qt::Key_Plus, 6,
    Qt::Key_Return, 7,
    Qt::Key_Enter, 7,
    Qt::Key_P, 9,
    Qt::Key_T, 10,
    Qt::Key_Y, 11,
    Qt::Key_9, 12,
    Qt::Key_6, 13,
    Qt::Key_3, 14,
    Qt::Key_Space, 15,
    Qt::Key_O, 17,
    Qt::Key_S, 18,
    Qt::Key_X, 19,
    Qt::Key_8, 20,
    Qt::Key_5, 21,
    Qt::Key_2, 22,
    Qt::Key_Period, 23,
    Qt::Key_N, 25,
    Qt::Key_R, 26,
    Qt::Key_W, 27,
    Qt::Key_7, 28,
    Qt::Key_4, 29,
    Qt::Key_1, 30,
    Qt::Key_0, 31,
    Qt::Key_M, 33,
    Qt::Key_Q, 34,
    Qt::Key_V, 35,
    Qt::Key_A, 41,
    Qt::Key_F1, 41,
    Qt::Key_B, 42,
    Qt::Key_F2, 42,
    Qt::Key_C, 43,
    Qt::Key_F3, 43,
    Qt::Key_D, 44,
    Qt::Key_F4, 44,
    Qt::Key_E, 45,
    Qt::Key_F5, 45,
    Qt::Key_F, 46,
    Qt::Key_F6, 46,
    Qt::Key_G, 47,
    Qt::Key_Up, 49,
    Qt::Key_Left, 50,
    Qt::Key_Down, 51,
    Qt::Key_Right, 52,
    Qt::Key_H, 53,
    Qt::Key_I, 54,
    Qt::Key_J, 55,
    Qt::Key_K, 57,
    Qt::Key_L, 58,
    Qt::Key_Tab, 60,
    Qt::Key_CapsLock, 61,
    Qt::Key_Control, 62,
    Qt::Key_Escape, 63,
    Qt::Key_Home, 63
            // ADD MORE KEYS HERE
    , 0, 0
};

struct mousemap
{
    int key, keynum;
    qreal left, right, top, bot;
} mouseMap[] = {
    {Qt::Key_Backspace, 1, 0.842672, 0.982759, 0.335196, 0.383613},
    {Qt::Key_U, 2, 0.842672, 0.982759, 0.428305, 0.482309},
    {Qt::Key_Z, 3, 0.842672, 0.982759, 0.527002, 0.575419},
    {Qt::Key_Asterisk, 4, 0.842672, 0.982759, 0.616387, 0.683426},
    {Qt::Key_Minus, 5, 0.842672, 0.982759, 0.716946, 0.783985},
    {Qt::Key_Plus, 6, 0.842672, 0.982759, 0.817505, 0.886406},
    {Qt::Key_Enter, 7, 0.842672, 0.982759, 0.918063, 0.979516},

    {Qt::Key_P, 9, 0.637931, 0.778017, 0.335196, 0.383613},
    {Qt::Key_T, 10, 0.637931, 0.778017, 0.428305, 0.482309},
    {Qt::Key_Y, 11, 0.637931, 0.778017, 0.527002, 0.575419},
    {Qt::Key_9, 12, 0.637931, 0.778017, 0.616387, 0.683426},
    {Qt::Key_6, 13, 0.637931, 0.778017, 0.716946, 0.783985},
    {Qt::Key_3, 14, 0.637931, 0.778017, 0.817505, 0.886406},
    {Qt::Key_Space, 15, 0.637931, 0.778017, 0.918063, 0.979516},

    {Qt::Key_O, 17, 0.428879, 0.573276, 0.335196, 0.383613},
    {Qt::Key_S, 18, 0.428879, 0.573276, 0.428305, 0.482309},
    {Qt::Key_X, 19, 0.428879, 0.573276, 0.527002, 0.575419},
    {Qt::Key_8, 20, 0.428879, 0.573276, 0.616387, 0.683426},
    {Qt::Key_5, 21, 0.428879, 0.573276, 0.716946, 0.783985},
    {Qt::Key_2, 22, 0.428879, 0.573276, 0.817505, 0.886406},
    {Qt::Key_Period, 23, 0.428879, 0.573276, 0.918063, 0.979516},

    {Qt::Key_N, 25, 0.217672, 0.364224, 0.335196, 0.383613},
    {Qt::Key_R, 26, 0.217672, 0.364224, 0.428305, 0.482309},
    {Qt::Key_W, 27, 0.217672, 0.364224, 0.527002, 0.575419},
    {Qt::Key_7, 28, 0.217672, 0.364224, 0.616387, 0.683426},
    {Qt::Key_4, 29, 0.217672, 0.364224, 0.716946, 0.783985},
    {Qt::Key_1, 30, 0.217672, 0.364224, 0.817505, 0.886406},
    {Qt::Key_0, 31, 0.217672, 0.364224, 0.918063, 0.979516},

    {Qt::Key_M, 33, 0.00862069, 0.153017, 0.335196, 0.383613},
    {Qt::Key_Q, 34, 0.00862069, 0.153017, 0.428305, 0.482309},
    {Qt::Key_V, 35, 0.00862069, 0.153017, 0.527002, 0.575419},

    {Qt::Key_F1, 41, 0.00862069, 0.118534, 0.00931099, 0.0521415},
    {Qt::Key_F2, 42, 0.181034, 0.295259, 0.00931099, 0.0521415},
    {Qt::Key_F3, 43, 0.357759, 0.467672, 0.00931099, 0.0521415},
    {Qt::Key_F4, 44, 0.530172, 0.642241, 0.00931099, 0.0521415},
    {Qt::Key_F5, 45, 0.706897, 0.814655, 0.00931099, 0.0521415},
    {Qt::Key_F6, 46, 0.872845, 0.987069, 0.00931099, 0.0521415},
    {Qt::Key_G, 47, 0.00862069, 0.118534, 0.108007, 0.163873},
    {Qt::Key_Up, 49, 0.706897, 0.803879, 0.0893855, 0.156425},
    {Qt::Key_Left, 50, 0.581897, 0.678879, 0.150838, 0.230912},
    {Qt::Key_Down, 51, 0.706897, 0.803879, 0.219739, 0.292365},
    {Qt::Key_Right, 52, 0.838362, 0.937500, 0.150838, 0.230912},
    {Qt::Key_H, 53, 0.181034, 0.295259, 0.108007, 0.163873},
    {Qt::Key_I, 54, 0.357759, 0.467672, 0.108007, 0.163873},
    {Qt::Key_J, 55, 0.00862069, 0.118534, 0.221601, 0.271881},
    {Qt::Key_K, 57, 0.181034, 0.295259, 0.221601, 0.271881},
    {Qt::Key_L, 58, 0.357759, 0.467672, 0.221601, 0.271881},

    {Qt::Key_Tab, 60, 0.00862069, 0.161638, 0.616387, 0.683426},
    {Qt::Key_CapsLock, 61, 0.00862069, 0.161638, 0.716946, 0.783985},
    {Qt::Key_Control, 62, 0.00862069, 0.161638, 0.817505, 0.886406},
    {Qt::Key_Home, 63, 0.00862069, 0.118534, 0.918063, 0.979516},

    {Qt::Key_F10, 64, 0.872845, 0.987069, 0.108007, 0.163873},

// ADD MORE KEYS HERE

    {0, 0, 0.0, 0.0, 0.0, 0.0}

};

#else



const int keyMap[] = {
    /*

    KEYBOARD BIT MAP
    ----------------
    This is the bit number in the 64-bit keymatrix.
    Bit set means key is pressed.

        AP]+  SY]+                   HL]+  ES]+
        |36|  |20|                   |61|  |52|
        +--+  +--+                   +--+  +--+

        HM]+  PL]+        UP]+       VW]+  CA]+
        |28|  |12|        |37|       |29|  |13|
        +--+  +--+  LF]+  +--+  RT]+ +--+  +--+
                    |57|  DN]+  |15|
              NM]+  +--+  |44|  +--+ ME]+
              |04|        +--+       |21|
              +--+                   +--+

        A]--+  B]--+  C]--+  D]--+  E]--+  BKS]+
        | 42|  | 58|  | 18|  | 10|  | 34|  | 02|
        +---+  +---+  +---+  +---+  +---+  +---+

        F]--+  G]--+  H]--+  I]--+  J]--+  K]--+
        | 59|  | 50|  | 43|  | 35|  | 27|  | 19|
        +---+  +---+  +---+  +---+  +---+  +---+

        L]--+  M]--+  N]--+  O]--+  ENTER]-----+
        | 11|  | 03|  | 60|  | 06|  |    07    |
        +---+  +---+  +---+  +---+  +----------+

        P]--+  7]---+  8]---+  9]---+  /]--+
        | 01|  | 22 |  | 14 |  | 05 |  | 17|
        +---+  +----+  +----+  +----+  +---+

        AL]-+  4]---+  5]---+  6]---+  *]--+
        | 26|  | 46 |  | 38 |  | 30 |  | 25|
        +---+  +----+  +----+  +----+  +---+

        RS]-+  1]---+  2]---+  3]---+  -]--+
        | 51|  | 45 |  | 62 |  | 54 |  | 33|
        +---+  +----+  +----+  +----+  +---+

        ON]-+  0]---+  .]---+  SP]--+  +]--+
        | 63|  | 09 |  | 53 |  | 49 |  | 41|
        +---+  +----+  +----+  +----+  +---+

    */

    Qt::Key_Backspace, 2,
    Qt::Key_U, 36,
    Qt::Key_Slash, 17,
    Qt::Key_Z, 17,
    Qt::Key_Asterisk, 25,
    Qt::Key_Minus, 33,
    Qt::Key_Plus, 41,
    Qt::Key_Return, 7,
    Qt::Key_Enter, 7,
    Qt::Key_P, 1,
    Qt::Key_T, 13,
    Qt::Key_Y, 61,
    Qt::Key_9, 5,
    Qt::Key_6, 30,
    Qt::Key_3, 54,
    Qt::Key_Space, 49,
    Qt::Key_O, 6,
    Qt::Key_S, 21,
    Qt::Key_X, 20,
    Qt::Key_8, 14,
    Qt::Key_5, 38,
    Qt::Key_2, 62,
    Qt::Key_Period, 53,
    Qt::Key_N, 60,
    Qt::Key_R, 4,
    Qt::Key_W, 29,
    Qt::Key_7, 22,
    Qt::Key_4, 46,
    Qt::Key_1, 45,
    Qt::Key_0, 9,
    Qt::Key_M, 3,
    Qt::Key_Q, 28,
    Qt::Key_V, 12,
    Qt::Key_A, 42,
    Qt::Key_F1, 20,
    Qt::Key_B, 58,
    Qt::Key_F2, 12,
    Qt::Key_C, 18,
    Qt::Key_F3, 4,
    Qt::Key_D, 10,
    Qt::Key_F4, 61,
    Qt::Key_E, 34,
    Qt::Key_F5, 29,
    Qt::Key_F, 59,
    Qt::Key_F6, 21,
    Qt::Key_G, 50,
    Qt::Key_Up, 37,
    Qt::Key_Left, 57,
    Qt::Key_Down, 44,
    Qt::Key_Right, 15,
    Qt::Key_H, 43,
    Qt::Key_I, 35,
    Qt::Key_J, 27,
    Qt::Key_K, 19,
    Qt::Key_L, 11,
    Qt::Key_Tab, 26,
    Qt::Key_CapsLock, 51,
    Qt::Key_Control, 63,
    Qt::Key_Escape, 52,
    Qt::Key_Home, 28
            // ADD MORE KEYS HERE
    , 0, 0
};

struct mousemap
{
    int key, keynum;
    qreal left, right, top, bot;
} mouseMap[] = {
    {Qt::Key_Escape, 52, 0.836036, 0.960360, 0.030631, 0.104286},
    {Qt::Key_T, 13, 0.836036, 0.960360, 0.137143, 0.214286},

    {Qt::Key_Backspace, 2, 0.836036, 0.960360, 0.270000, 0.345714},
    {Qt::Key_K, 19, 0.836036, 0.960360, 0.372857, 0.448571},
    {Qt::Key_Enter, 7, 0.675676, 0.960360, 0.477143, 0.557143},

    {Qt::Key_Slash, 17, 0.836036, 0.960360, 0.580000, 0.660000},
    {Qt::Key_Asterisk, 25, 0.836036, 0.960360, 0.687143, 0.762857},
    {Qt::Key_Minus, 33, 0.836036, 0.960360, 0.791429, 0.867143},
    {Qt::Key_Plus, 41, 0.836036, 0.960360, 0.894286, 0.972857},

    {Qt::Key_Y, 61, 0.677477, 0.798198, 0.015714, 0.067143},
    {Qt::Key_W, 29, 0.677477, 0.798198, 0.094286, 0.145714},
    {Qt::Key_S, 21, 0.677477, 0.798198, 0.172857, 0.222857},

    {Qt::Key_E, 34, 0.677477, 0.798198, 0.270000, 0.345714},
    {Qt::Key_J, 27, 0.677477, 0.798198, 0.372857, 0.448571},

    {Qt::Key_9, 5, 0.625225, 0.803604, 0.580000, 0.660000},
    {Qt::Key_6, 30, 0.625225, 0.803604, 0.687143, 0.762857},
    {Qt::Key_3, 54, 0.625225, 0.803604, 0.791429, 0.867143},
    {Qt::Key_Space, 49, 0.625225, 0.803604, 0.894286, 0.972857},

    {Qt::Key_D, 10, 0.517117, 0.636036, 0.270000, 0.345714},
    {Qt::Key_I, 35, 0.517117, 0.636036, 0.372857, 0.448571},
    {Qt::Key_O, 6,  0.517117, 0.636036, 0.477143, 0.557143},

    {Qt::Key_C, 18, 0.356757, 0.477477, 0.270000, 0.345714},
    {Qt::Key_H, 43, 0.356757, 0.477477, 0.372857, 0.448571},
    {Qt::Key_N, 60,  0.356757, 0.477477, 0.477143, 0.557143},

    {Qt::Key_8, 14, 0.410811, 0.585586, 0.580000, 0.660000},
    {Qt::Key_5, 38, 0.410811, 0.585586, 0.687143, 0.762857},
    {Qt::Key_2, 62, 0.410811, 0.585586, 0.791429, 0.867143},
    {Qt::Key_Period, 53, 0.410811, 0.585586, 0.894286, 0.972857},

    {Qt::Key_X, 20, 0.192793, 0.318919, 0.015714, 0.067143},
    {Qt::Key_V, 12, 0.192793, 0.318919, 0.094286, 0.145714},
    {Qt::Key_R, 04, 0.192793, 0.318919, 0.172857, 0.222857},

    {Qt::Key_B, 58, 0.192793, 0.318919, 0.270000, 0.345714},
    {Qt::Key_G, 50, 0.192793, 0.318919, 0.372857, 0.448571},
    {Qt::Key_M, 3,  0.192793, 0.318919, 0.477143, 0.557143},

    {Qt::Key_7, 22, 0.192793, 0.374775, 0.580000, 0.660000},
    {Qt::Key_4, 46, 0.192793, 0.374775, 0.687143, 0.762857},
    {Qt::Key_1, 45, 0.192793, 0.374775, 0.791429, 0.867143},
    {Qt::Key_0, 9, 0.192793, 0.374775, 0.894286, 0.972857},

    {Qt::Key_U, 36, 0.034234, 0.158559, 0.030631, 0.104286},
    {Qt::Key_Q, 28, 0.034234, 0.158559, 0.137143, 0.214286},

    {Qt::Key_A, 42, 0.034234, 0.158559, 0.270000, 0.345714},
    {Qt::Key_F, 59, 0.034234, 0.158559, 0.372857, 0.448571},
    {Qt::Key_L, 11,  0.034234, 0.158559, 0.477143, 0.557143},
    {Qt::Key_P, 1,  0.034234, 0.158559, 0.580000, 0.660000},

    {Qt::Key_Tab, 26, 0.034234, 0.158559, 0.687143, 0.762857},
    {Qt::Key_CapsLock, 51, 0.034234, 0.158559, 0.791429, 0.867143},
    {Qt::Key_Control , 63, 0.034234, 0.158559, 0.894286, 0.972857},

    {Qt::Key_Up, 37, 0.410811, 0.585586, 0.030631, 0.104286},
    {Qt::Key_Down, 44, 0.410811, 0.585586,  0.149202, 0.222857},
    {Qt::Key_Left, 57, 0.356757, 0.477477, 0.104286, 0.149202},
    {Qt::Key_Right, 15, 0.517117, 0.636036, 0.104286, 0.149202},


    // DONE UP TO HERE


//    {Qt::Key_F10, 64, 0.872845, 0.987069, 0.108007, 0.163873},

// ADD MORE KEYS HERE

    {0, 0, 0.0, 0.0, 0.0, 0.0}

};


#endif




void MainWindow::keyPressEvent(QKeyEvent * ev)
{

    int i;

    if(ev->isAutoRepeat()) {
        ev->accept();
        return;
    }

    if(ev->key() == Qt::Key_F12) {
        __pckeymatrix = (1ULL << 63) | (1ULL << 41) | (1ULL << 43);
        __keyb_update();
        ev->accept();
        return;
    }

    for(i = 0; keyMap[i] != 0; i += 2) {
        if(ev->key() == keyMap[i]) {
            __pckeymatrix |= 1ULL << (keyMap[i + 1]);
            __keyb_update();
            ev->accept();
            return;
        }
    }

    QMainWindow::keyPressEvent(ev);
}

void MainWindow::keyReleaseEvent(QKeyEvent * ev)
{

    int i;
    if(ev->isAutoRepeat()) {
        ev->accept();
        return;
    }

    if(ev->key() == Qt::Key_F12) {
        __pckeymatrix &= ~((1ULL << 63) | (1ULL << 41) | (1ULL << 43));
        __keyb_update();
        ev->accept();
        return;
    }

    int mykey = ev->key();

    for(i = 0; keyMap[i] != 0; i += 2) {
        if(mykey == keyMap[i]) {
            __pckeymatrix &= ~(1ULL << (keyMap[i + 1]));
            __keyb_update();
            ev->accept();
            return;
        }
    }

    QMainWindow::keyReleaseEvent(ev);
}

extern "C" void thread_processevents()
{
    QCoreApplication::processEvents();
    QThread::yieldCurrentThread();
}

void MainWindow::on_actionExit_triggered()
{

    // CLEANUP SD CARD EMULATION
    if(__sd_inserted) {
        // STOP RPL ENGINE
        screentmr->stop();
        if(rpl.isRunning()) {
            __cpu_idle = 0;
            __pc_terminate = 1;
            __pckeymatrix ^= (1ULL << 63);
            __keyb_update();
            while(rpl.isRunning()) {
                __pc_terminate = 1;
            }

        }
        on_actionEject_SD_Card_Image_triggered();
    }

    // SAVE CURRENT FILE
    if(currentfile.isEmpty()) {
        QMessageBox a(QMessageBox::Warning, "Work not saved",
                "Do you want to save before exit?",
                QMessageBox::Yes | QMessageBox::No, this);
        if(a.exec() == QMessageBox::Yes)
            on_actionSave_triggered();
    }
    else
        on_actionSave_triggered();

    // STOP RPL ENGINE
    screentmr->stop();
    if(rpl.isRunning()) {
        __cpu_idle = 0;
        __pc_terminate = 1;
        __pckeymatrix ^= (1ULL << 63);
        __keyb_update();
        while(rpl.isRunning()) {
            __pc_terminate = 1;
        }
    }

    // STOP THE USB DRIVER THREAD
    __usb_paused = 2;
    while(usbdriver.isRunning() && (__usb_paused >= 0));

    QSettings settings;

    settings.setValue(QString("CurrentFile"), QVariant(currentfile));

}

int MainWindow::WriteWord(unsigned int word)
{
    if(myMainWindow->fileptr->write((const char *)&word, 4) != 4)
        return 0;
    return 1;
}

unsigned int MainWindow::ReadWord()
{
    unsigned int w;
    myMainWindow->fileptr->read((char *)&w, 4);
    return w;
}

extern "C" int write_data(unsigned int word, void *opaque)
{
    (void)opaque;
    return MainWindow::WriteWord(word);
}

extern "C" unsigned int read_data(void *opaque)
{
    (void)opaque;
    return MainWindow::ReadWord();
}

void MainWindow::on_actionSave_triggered()
{
    QString fname;
    QString path = getDocumentsLocation();

    if(currentfile.isEmpty()) {
        fname = QFileDialog::getSaveFileName(this,
                "Select file name to Save as", path,
                "newRPL Backups (*.nrpb *.* *)");
        if(!fname.isEmpty()) {
            currentfile = fname;
            setWindowTitle(QString("newRPL - [") + fname + QString("]"));
        }
        else
            return;

    }
    else
        fname = currentfile;

    SaveFile(fname);

}

void MainWindow::on_actionOpen_triggered()
{
    QString path = getDocumentsLocation();

    QString fname = QFileDialog::getOpenFileName(this, "Open File Name", path,
            "newRPL Backups (*.nrpb *.* *)",nullptr,QFileDialog::DontUseNativeDialog);

    if(!OpenFile(fname)) {
        if(!rpl.isRunning()) {
            // RESTART RPL ENGINE
            __pc_terminate = 0;
            __pckeymatrix = 0;

            rpl.start();
            screentmr->setSingleShot(true);
            screentmr->start(20);

        }
    }
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString path = getDocumentsLocation();

    QString fname =
            QFileDialog::getSaveFileName(this, "Select file name to Save as",
            path, "newRPL Backups (*.nrpb *.* *)");

    if(!fname.isEmpty()) {
        currentfile = fname;
        setWindowTitle(QString("newRPL - [") + fname + QString("]"));
    }
    else
        return;

    SaveFile(fname);
}

void MainWindow::on_actionNew_triggered()
{

    // STOP RPL ENGINE
    screentmr->stop();
    if(rpl.isRunning()) {
        __cpu_idle = 0;
        __pc_terminate = 1;
        __pckeymatrix ^= (1ULL << 63);
        __keyb_update();
        while(rpl.isRunning()) {
            usbupdate();
            __pc_terminate = 1;
        }
    }

    currentfile.clear();
    setWindowTitle("newRPL - [Unnamed]");
    __memmap_intact = 0;

    // RESTART RPL ENGINE
    __pc_terminate = 0;
    __pckeymatrix = 0;

    rpl.start();

    screentmr->setSingleShot(true);
    screentmr->start(20);

}

void MainWindow::on_actionInsert_SD_Card_Image_triggered()
{
    QString path = getDocumentsLocation();

    QString fname =
            QFileDialog::getOpenFileName(this, "Open SD Card Image", path,
            "*.img");
    if(!fname.isEmpty()) {
        sdcard.setFileName(fname);

        if(!sdcard.open(QIODevice::ReadOnly)) {
            QMessageBox a(QMessageBox::Warning, "Error while opening",
                    "Cannot open file " + fname, QMessageBox::Ok, this);
            a.exec();
            return;
        }

        __sd_inserted = 0;
        __sd_RCA = 0;
        __sd_nsectors = 0;
        if(__sd_buffer != NULL)
            free(__sd_buffer);

        // FILE IS OPEN AND READY FOR READING
        __sd_buffer = (unsigned char *)malloc(sdcard.size());
        if(__sd_buffer == NULL) {
            QMessageBox a(QMessageBox::Warning, "Error while opening",
                    "Not enough memory to read SD Image", QMessageBox::Ok,
                    this);
            a.exec();
            return;
        }

        if(sdcard.read((char *)__sd_buffer, sdcard.size()) != sdcard.size()) {
            QMessageBox a(QMessageBox::Warning, "Error while opening",
                    "Can't read SD Image", QMessageBox::Ok, this);
            a.exec();
            return;
        }

        __sd_nsectors = sdcard.size() / 512;
        __sd_inserted = 1;
        sdcard.close();
        // SIMULATE AN IRQ
        __SD_irqeventinsert();

        ui->actionEject_SD_Card_Image->setEnabled(true);
        ui->actionInsert_SD_Card_Image->setEnabled(false);
        return;
    }

    // NOTHING TO MOUNT, KEEP THE PREVIOUS STATUS

}

void MainWindow::on_actionEject_SD_Card_Image_triggered()
{
    if(__sd_inserted) {
        // SAVE THE CONTENTS BACK BEFORE EJECTING
        if(!sdcard.open(QIODevice::WriteOnly)) {
            QMessageBox a(QMessageBox::Warning,
                    "Error while saving SD Card contents",
                    "Cannot open file " + sdcard.fileName(), QMessageBox::Ok,
                    this);
            a.exec();
        }
        else {
            sdcard.write((char *)__sd_buffer, (qint64) __sd_nsectors * 512LL);
            sdcard.close();
        }
    }
    __sd_inserted = 0;
    __sd_RCA = 0;
    __sd_nsectors = 0;
    if(__sd_buffer != NULL) {
        free(__sd_buffer);
        __sd_buffer = NULL;
    }

    // SIMULATE AN IRQ
    __SD_irqeventinsert();

    ui->actionEject_SD_Card_Image->setEnabled(false);
    ui->actionInsert_SD_Card_Image->setEnabled(true);

}

void MainWindow::on_actionPower_ON_triggered()
{

    // STOP RPL ENGINE
    screentmr->stop();
    if(rpl.isRunning()) {
        __cpu_idle = 0;
        __pc_terminate = 1;
        __pckeymatrix ^= (1ULL << 63);
        __keyb_update();
        while(rpl.isRunning()) {
            __pc_terminate = 1;
        }
    }

    if(__pc_terminate == 2) {
        // IT WAS POWERED OFF
        __memmap_intact = 2;
    }
    else
        __memmap_intact = 1;

    // RESTART RPL ENGINE
    __pc_terminate = 0;
    __pckeymatrix = 0;

    rpl.start();
    screentmr->setSingleShot(true);
    screentmr->start(20);

}

void MainWindow::on_actionSimulate_Alarm_triggered()
{

}

void MainWindow::on_actionTake_Screenshot_triggered()
{
    QString path = getDocumentsLocation();

    QString fname =
            QFileDialog::getSaveFileName(this, "Save screenshot as...", path,
            "*.png");
    if(!fname.isEmpty()) {

        QRectF r = ui->EmuScreen->sceneRect();
        QImage image(r.width() * 4, r.height() * 4, QImage::Format_ARGB32);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        ui->EmuScreen->scene()->render(&painter);
        image.save(fname);
    }
}

extern void Stack2Clipboard(int level, int dropit);
extern void Clipboard2Stack();
extern void Clipboard2StackCompile();
extern int SaveRPLObject(QString & filename, int level);
int LoadRPLObject(QString & filename);

void MainWindow::on_actionCopy_Level_1_triggered()
{
    if(!rpl.isRunning())
        return; // DO NOTHING

    while(!__cpu_idle)
        QThread::msleep(1);     // BLOCK UNTIL RPL IS IDLE

    __cpu_idle = 2;     // BLOCK REQUEST

    // NOW WORK ON THE RPL ENGINE WHILE THE THREAD IS BLOCKED
    Stack2Clipboard(1, 0);

    __cpu_idle = 0;     // LET GO THE SIMULATOR
}

void MainWindow::on_actionPaste_to_Level_1_triggered()
{
    if(!rpl.isRunning())
        return; // DO NOTHING

    while(!__cpu_idle)
        QThread::msleep(1);     // BLOCK UNTIL RPL IS IDLE

    __cpu_idle = 2;     // BLOCK REQUEST

    // NOW WORK ON THE RPL ENGINE WHILE THE THREAD IS BLOCKED
    Clipboard2Stack();

    __cpu_idle = 0;     // LET GO THE SIMULATOR

}

void MainWindow::on_actionCut_Level_1_triggered()
{
    if(!rpl.isRunning())
        return; // DO NOTHING

    while(!__cpu_idle)
        QThread::msleep(1);     // BLOCK UNTIL RPL IS IDLE

    __cpu_idle = 2;     // BLOCK REQUEST

    // NOW WORK ON THE RPL ENGINE WHILE THE THREAD IS BLOCKED
    Stack2Clipboard(1, 1);

    __cpu_idle = 0;     // LET GO THE SIMULATOR
    halScreenUpdated();
}

void MainWindow::on_actionSave_Level_1_As_triggered()
{
    QString path = getDocumentsLocation();

    QString fname = QFileDialog::getSaveFileName(this,
            "Select file name to store object", path,
            "newRPL objects (*.nrpl *.* *)");

    if(!fname.isEmpty()) {
        // GOT A NAME, APPEND EXTENSION IF NOT GIVEN

        //if(!fname.endsWith(".nrpl")) fname+=".nrpl";

        if(!SaveRPLObject(fname, 1)) {
            QMessageBox a(QMessageBox::Warning, "Error while saving",
                    "Cannot write to file " + fname, QMessageBox::Ok, this);
            a.exec();
            return;
        }
    }
}

void MainWindow::on_actionOpen_file_to_Level_1_triggered()
{
    QString path = getDocumentsLocation();

    QString fname = QFileDialog::getOpenFileName(this, "Select File Name", path,
            "newRPL objects (*.nrpl *.* *)",nullptr,QFileDialog::DontUseNativeDialog);

    if(!fname.isEmpty()) {
        if(!rpl.isRunning())
            return;     // DO NOTHING

        while(!__cpu_idle)
            QThread::msleep(1); // BLOCK UNTIL RPL IS IDLE

        __cpu_idle = 2; // BLOCK REQUEST

        // NOW WORK ON THE RPL ENGINE WHILE THE THREAD IS BLOCKED
        if(!LoadRPLObject(fname)) {
            QMessageBox a(QMessageBox::Warning, "Error while opening",
                    "Cannot read file. Corrupted data?\n" + fname,
                    QMessageBox::Ok, this);
            a.exec();
            return;
        }

    }

    __cpu_idle = 0;     // LET GO THE SIMULATOR
    halScreenUpdated();

}

void MainWindow::on_actionConnect_to_calc_triggered()
{
    ui->USBDockSelect->setVisible(true);
    halScreenUpdated();
}

void MainWindow::usbupdate()
{
    if(nousbupdate)
        return;

    if(!usb_isconnected()) {
        if(!currentusb.isEmpty()) {
            // ATTEMPT TO RECONNECT WITH THE DEVICE
            if(ui->usbconnectButton->text().endsWith("[ Click to reconnect ]")) {
                halScreenUpdated();
                // ATTEMPT TO RECONNECT
                __usb_paused = 1;
                while(__usb_paused >= 0);
                usb_shutdown();
                if(safe_stringcpy(__usb_devicepath, 8192,
                            currentusbpath.toUtf8().constData()))
                    __usb_devicepath[0] = 0;
                usb_init(0);
                if(!usb_isconnected()) {
                    return;
                }
                else {
                    ui->usbconnectButton->setText(currentusb);
                    __usb_paused = 0;   // AND RESUME THE DRIVER
                    return;
                }
            }
            else
                ui->usbconnectButton->setText(currentusb +
                        QString(" [ Click to reconnect ]"));
        }

    }

}

void MainWindow::on_usbconnectButton_clicked()
{
    nousbupdate = true;

    // PAUSE THE USB DRIVER
    __usb_paused = 1;
    while(__usb_paused >= 0);

    if(ui->usbconnectButton->text().endsWith("[ Click to reconnect ]")) {
        // ATTEMPT TO RECONNECT
        usb_shutdown();
        if(safe_stringcpy(__usb_devicepath, 8192,
                    currentusbpath.toUtf8().constData()))
            __usb_devicepath[0] = 0;
        usb_init(0);
        if(!usb_isconnected()) {
            currentusb.clear();
            currentusbpath.clear();
        }
        else {
            ui->usbconnectButton->setText(currentusb);
            __usb_paused = 0;   // AND RESUME THE DRIVER
            halScreenUpdated();
            nousbupdate = false;
            return;
        }
    }

    if(!usb_isconnected()) {
        currentusb.clear();
        currentusbpath.clear();
    }
    else {
        // DISCONNECT
        ui->usbconnectButton->setText(" [ Select a USB Device ] ");
        usb_shutdown();
        currentusb.clear();
        currentusbpath.clear();
        return;
    }

    int oldflag;
    if(rpl.isRunning())
        while(!__cpu_idle)
            QThread::msleep(1); // BLOCK UNTIL RPL IS IDLE

    __cpu_idle = 2;     // PAUSE RPL ENGINE UNTIL WE ARE DONE CONNECTING

    oldflag = change_autorcv(1);

    __cpu_idle = 0;
    if(rpl.isRunning())
        while(!__cpu_idle)
            QThread::msleep(1); // BLOCK UNTIL RPL IS IDLE

    USBSelector seldlg;

    if(seldlg.exec() == QDialog::Accepted) {
        if(!seldlg.getSelectedDevicePath().isEmpty()) {
            usb_shutdown();
            if(safe_stringcpy(__usb_devicepath, 8192,
                        seldlg.getSelectedDevicePath().toUtf8().constData()))
                __usb_devicepath[0] = 0;
            usb_init(0);
            if(usb_isconnected()) {
                currentusbpath = seldlg.getSelectedDevicePath();
                currentusbpath.detach();
                currentusb = seldlg.getSelectedDeviceName();
                currentusb.detach();
            }
        }

    }

    if(currentusb.isEmpty())
        ui->usbconnectButton->setText(" [ Select a USB Device ] ");
    else {
        ui->usbconnectButton->setText(currentusb);
        nousbupdate = false;
        __usb_paused = 0;
    }

    if(rpl.isRunning()) {

        while(!__cpu_idle)
            QThread::msleep(1); // BLOCK UNTIL RPL IS IDLE

        __cpu_idle = 2; // PAUSE RPL ENGINE UNTIL WE ARE DONE CONNECTING

    }
    change_autorcv(oldflag);

    __cpu_idle = 0;

    halScreenUpdated();

}

void MainWindow::on_actionUSB_Remote_ARCHIVE_to_file_triggered()
{

    if(!usb_isconnected()) {
        QMessageBox a(QMessageBox::Warning, "USB not connected",
                "Need to establish a connection to a device first.",
                QMessageBox::Ok, this);
        a.exec();
        return;
    }

    QString path = getDocumentsLocation();

    QString fname =
            QFileDialog::getSaveFileName(this, "Select file name to Save as",
            path, "newRPL Backups (*.nrpb *.* *)");

    if(!fname.isEmpty()) {
        // GOT A NAME, APPEND EXTENSION IF NOT GIVEN

        //if(!fname.endsWith(".nrpb")) fname+=".nrpb";

        QFile file(fname);

        if(!file.open(QIODevice::WriteOnly)) {
            QMessageBox a(QMessageBox::Warning, "Error while saving",
                    "Cannot write to file " + fname, QMessageBox::Ok, this);
            a.exec();
            return;
        }

        // FILE IS OPEN AND READY FOR WRITING

        if(!usbremotearchivestart()) {
            QMessageBox a(QMessageBox::Warning, "Error while saving",
                    "Failed to send remote commands to " + currentusb,
                    QMessageBox::Ok, this);
            a.exec();
            file.close();
            return;
        }

#define USBARCHIVE_MAX_SIZE_WORDS 1024*1024

        uint32_t *buffer = new uint32_t[USBARCHIVE_MAX_SIZE_WORDS];     // 4 MB EXPECTED MAXIMUM SIZE OF AN ARCHIVE
        if(!buffer) {
            file.close();       // RETURN - THIS WILL NEVER HAPPEN FOR JUST 4 MB
            return;
        }

        int oldflag;
        if(rpl.isRunning())
            while(!__cpu_idle)
                QThread::msleep(1);     // BLOCK UNTIL RPL IS IDLE

        __cpu_idle = 2; // PAUSE RPL ENGINE UNTIL WE ARE DONE CONNECTING

        oldflag = change_autorcv(1);    // STOP THE SIMULATOR FROM RECEIVING THR TRANSMISSION

        __cpu_idle = 0;

        int nwords = usbreceivearchive(buffer, USBARCHIVE_MAX_SIZE_WORDS);

        if(rpl.isRunning())
            while(!__cpu_idle)
                QThread::msleep(1);     // BLOCK UNTIL RPL IS IDLE

        __cpu_idle = 2; // PAUSE RPL ENGINE UNTIL WE ARE DONE CONNECTING

        change_autorcv(oldflag);        // RESTORE THE SIMULATOR FLAG

        __cpu_idle = 0;

        if(nwords == -1) {
            file.close();
            QMessageBox a(QMessageBox::Warning, "Error while saving",
                    "USB communication error", QMessageBox::Ok, this);
            a.exec();
            delete[]buffer;
            return;
        }

        file.write((const char *)buffer, nwords * sizeof(uint32_t));

        file.close();

        delete[]buffer;

    }
}

void MainWindow::on_actionRemote_USBRESTORE_from_file_triggered()
{
    if(!usb_isconnected()) {
        QMessageBox a(QMessageBox::Warning, "USB not connected",
                "Need to establish a connection to a device first.",
                QMessageBox::Ok, this);
        a.exec();
        return;
    }

    QString path = getDocumentsLocation();

    QString fname = QFileDialog::getOpenFileName(this, "Open File Name", path,
            "newRPL Backups (*.nrpb *.* *)",nullptr,QFileDialog::DontUseNativeDialog);

    if(!fname.isEmpty()) {
        QFile file(fname);

        if(!file.open(QIODevice::ReadOnly)) {
            QMessageBox a(QMessageBox::Warning, "Error while opening",
                    "Cannot open file " + fname, QMessageBox::Ok, this);
            a.exec();
            return;
        }

        // FILE IS OPEN AND READY FOR READING

        QByteArray filedata;

        filedata = file.readAll();

        file.close();

        QMessageBox warn(QMessageBox::Warning, "Remote USBRESTORE",
                "USBRESTORE will completely replace *ALL DATA* on the connected device with no way to undo the operation. OK to proceed?",
                QMessageBox::Yes | QMessageBox::No, this);

        if(warn.exec() == QMessageBox::Yes) {

            if(!usbremoterestorestart()) {
                QMessageBox a(QMessageBox::Warning, "Error while restoring",
                        "Failed to send remote commands to " + currentusb,
                        QMessageBox::Ok, this);
                a.exec();
                return;
            }

            int oldflag;

            if(rpl.isRunning())
                while(!__cpu_idle)
                    QThread::msleep(1); // BLOCK UNTIL RPL IS IDLE
            __cpu_idle = 2;     // PAUSE RPL ENGINE UNTIL WE ARE DONE CONNECTING

            oldflag = change_autorcv(1);        // STOP THE SIMULATOR FROM RECEIVING THR TRANSMISSION

            __cpu_idle = 0;

            int nwords =
                    usbsendarchive((uint32_t *) filedata.constData(),
                    (filedata.size() + 3) >> 2);

            if(rpl.isRunning())
                while(!__cpu_idle)
                    QThread::msleep(1); // BLOCK UNTIL RPL IS IDLE

            __cpu_idle = 2;     // PAUSE RPL ENGINE UNTIL WE ARE DONE CONNECTING

            change_autorcv(oldflag);    // RESTORE THE SIMULATOR FLAG

            __cpu_idle = 0;

            halScreenUpdated();

            if(nwords == -1) {
                file.close();
                QMessageBox a(QMessageBox::Warning, "Error while restoring",
                        "USB communication error", QMessageBox::Ok, this);
                a.exec();
                return;
            }

        }

    }
}

int MainWindow::OpenFile(QString fname)
{
    if(!fname.isEmpty()) {
        QFile file(fname);

        if(!file.open(QIODevice::ReadOnly)) {
            QMessageBox a(QMessageBox::Warning, "Error while opening",
                    "Cannot open file " + fname, QMessageBox::Ok, this);
            a.exec();
            return 0;
        }

        // FILE IS OPEN AND READY FOR READING

        // STOP RPL ENGINE
        screentmr->stop();
        if(rpl.isRunning()) {
            __cpu_idle = 0;
            __pc_terminate = 1;
            __pckeymatrix ^= (1ULL << 63);
            __keyb_update();
            while(rpl.isRunning()) {
                usbupdate();
                __pc_terminate = 1;
            }
        }

        // PERFORM RESTORE PROCEDURE
        myMainWindow = this;
        fileptr = &file;
        int result = rplRestoreBackup(1, &read_data, (void *)fileptr);

        file.close();

        switch (result) {
        case -1:
        {
            QMessageBox a(QMessageBox::Warning, "Error while opening",
                    "File " + fname +
                    " is corrupt or incompatible.\nCan't recover and memory was destroyed.",
                    QMessageBox::Ok, this);
            a.exec();
            currentfile.clear();
            setWindowTitle("newRPL - [Unnamed]");
            __memmap_intact = 0;
            return 0;
        }
        case 0:
        {
            QMessageBox a(QMessageBox::Warning, "Error while opening",
                    "File " + fname +
                    " is corrupt or incompatible.\nCan't recover but memory was left intact.",
                    QMessageBox::Ok, this);
            a.exec();
            __memmap_intact = 1;
            break;
        }
        case 1:
        {
            currentfile = fname;
            QString nameonly =
                    currentfile.right(currentfile.length() - 1 -
                    takemax(currentfile.lastIndexOf("/"),
                        currentfile.lastIndexOf("\\")));
            setWindowTitle("newRPL - [" + nameonly + "]");

            __memmap_intact = 2;

            break;
        }
        case 2:
        {
            QMessageBox a(QMessageBox::Warning, "Recovery success",
                    "File " + fname +
                    " was recovered with minor errors.\nRun MEMFIX to correct them.",
                    QMessageBox::Ok, this);
            a.exec();
            currentfile = fname;
            QString nameonly =
                    currentfile.right(currentfile.length() - 1 -
                    takemax(currentfile.lastIndexOf("/"),
                        currentfile.lastIndexOf("\\")));
            setWindowTitle("newRPL - [" + nameonly + "]");

            __memmap_intact = 1;
            break;
        }

        }

        // RESTART RPL ENGINE
        __pc_terminate = 0;
        __pckeymatrix = 0;

        rpl.start();
        screentmr->setSingleShot(true);
        screentmr->start(20);

        return 1;
    }

    return 0;
}

void MainWindow::SaveFile(QString fname)
{

    if(!fname.isEmpty()) {
        // GOT A NAME, APPEND EXTENSION IF NOT GIVEN

        //if(!fname.endsWith(".nrpb")) fname+=".nrpb";

        QFile file(fname);

        if(!file.open(QIODevice::WriteOnly)) {
            QMessageBox a(QMessageBox::Warning, "Error while saving",
                    "Cannot write to file " + fname, QMessageBox::Ok, this);
            a.exec();
            return;
        }

        // FILE IS OPEN AND READY FOR WRITING

        // STOP RPL ENGINE
        screentmr->stop();
        if(rpl.isRunning()) {

            setExceptionPoweroff();
            __cpu_idle = 0;
            __pc_terminate = 1;
            __pckeymatrix ^= (1ULL << 63);
            __keyb_update();
            while(rpl.isRunning()) {
                usbupdate();
                __pc_terminate = 1;
            }
        }

        // PERFORM BACKUP
        myMainWindow = this;
        fileptr = &file;
        rplBackup(&write_data, (void *)fileptr);

        file.close();

        __memmap_intact = 2;
        // RESTART RPL ENGINE
        __pc_terminate = 0;
        __pckeymatrix = 0;
        rpl.start();
        screentmr->setSingleShot(true);
        screentmr->start(20);

    }

}

void MainWindow::on_actionShow_LCD_grid_toggled(bool arg1)
{
    ui->EmuScreen->BkgndPen.setStyle((arg1) ? Qt::SolidLine : Qt::NoPen);

    halScreenUpdated();

}

bool MainWindow::eventFilter(QObject * obj, QEvent * ev)
{
    if(obj == ui->KeybImage) {
        if((ev->type() == QEvent::TouchBegin)
                || (ev->type() == QEvent::TouchUpdate)
                || (ev->type() == QEvent::TouchEnd)
                || (ev->type() == QEvent::TouchCancel)) {
            // ACCEPT THE TOUCH
            QTouchEvent *me = static_cast < QTouchEvent * >(ev);
            int npoints, k, pressed;
            npoints = me->touchPoints().count();
            for(k = 0; k < npoints; ++k) {
                QPointF coordinates = me->touchPoints().at(k).startPos();
                qreal relx, rely;

                if(me->touchPoints().at(k).state() & Qt::TouchPointPressed)
                    pressed = 1;
                else if(me->touchPoints().at(k).
                        state() & Qt::TouchPointReleased)
                    pressed = 0;
                else
                    continue;   // NOT INTERESTED IN DRAGGING

                relx = coordinates.x() / (qreal) ui->KeybImage->width();
                rely = coordinates.y() / (qreal) ui->KeybImage->height();

                //qDebug() << "PRESS x=" << relx << ", y=" << rely ;

                struct mousemap *ptr = mouseMap;

                while(ptr->key != 0) {
                    if((relx >= ptr->left) && (relx <= ptr->right)
                            && (rely >= ptr->top) && (rely <= ptr->bot)) {
                        // CLICKED INSIDE A KEY

                        if(ptr->keynum == 64) {
                            // PRESSED THE SIMULATED MAIN MENU KEY
                            //menuBar()->activateWindow();
                        }
                        else {
                            //TODO: HIGHLIGHT IT FOR VISUAL EFFECT
                            if(pressed) {
                                __pckeymatrix |= 1ULL << (ptr->keynum);
                                //qDebug() << "PRESS x=" << relx << ", y=" << rely << ", key=" << ptr->keynum;
                                if(ptr->keynum == 63) {
                                    // CHECK IF ON WAS PRESSED AND THE CALCULATOR WAS OFF
                                    if(!rpl.isRunning())
                                        on_actionPower_ON_triggered();
                                }
                            }
                            else {
                                __pckeymatrix &= ~(1ULL << (ptr->keynum));
                                //qDebug() << "RELEA x=" << relx << ", y=" << rely << ", key=" << ptr->keynum;
                            }

                            __keyb_update();
                        }
                    }
                    ptr++;
                }
            }

            return true;

        }

        if(ev->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast < QMouseEvent * >(ev);
            //QPoint coordinates = me->pos();
            qreal relx, rely;

            /*if(coordinates.y()<30) {
               // TOUCHED THE TOP BAR, SHOW THE MENU
               contextMenuEvent(NULL);
               return true;
               } */

            relx = (qreal) me->x() / (qreal) ui->KeybImage->width();
            rely = (qreal) me->y() / (qreal) ui->KeybImage->height();

            //qDebug() << "PRESS x=" << relx << ", y=" << rely ;

            struct mousemap *ptr = mouseMap;

            while(ptr->key != 0) {
                if((relx >= ptr->left) && (relx <= ptr->right)
                        && (rely >= ptr->top) && (rely <= ptr->bot)) {
                    // CLICKED INSIDE A KEY

                    if(ptr->keynum == 64) {
                        // PRESSED THE SIMULATED MAIN MENU KEY
                        //menuBar()->activateWindow();
                    }
                    else {
                        //TODO: HIGHLIGHT IT FOR VISUAL EFFECT
                        __pckeymatrix |= 1ULL << (ptr->keynum);
                        __keyb_update();
                        if(ptr->keynum == 63) {
                            // CHECK IF ON WAS PRESSED AND THE CALCULATOR WAS OFF
                            if(!rpl.isRunning())
                                on_actionPower_ON_triggered();
                        }

                    }
                }
                ptr++;
            }

            return true;
        }
        if(ev->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *me = static_cast < QMouseEvent * >(ev);
            //QPoint coordinates = me->pos();
            qreal relx, rely;

            relx = (qreal) me->x() / (qreal) ui->KeybImage->width();
            rely = (qreal) me->y() / (qreal) ui->KeybImage->height();

            //qDebug() << "RELEASE x=" << relx << ", y=" << rely ;

            struct mousemap *ptr = mouseMap;

            while(ptr->key != 0) {
                if((relx >= ptr->left) && (relx <= ptr->right)
                        && (rely >= ptr->top) && (rely <= ptr->bot)) {
                    // CLICKED INSIDE A KEY

                    //TODO: HIGHLIGHT IT FOR VISUAL EFFECT
                    __pckeymatrix &= ~(1ULL << (ptr->keynum));
                    __keyb_update();
                }
                ptr++;
            }

            return true;
        }

        return false;
    }

    if(obj == ui->EmuScreen) {
        if((ev->type() == QEvent::TouchBegin)
                || (ev->type() == QEvent::TouchUpdate)
                || (ev->type() == QEvent::TouchEnd)
                || (ev->type() == QEvent::TouchCancel)) {
            // ACCEPT THE TOUCH
            QTouchEvent *me = static_cast < QTouchEvent * >(ev);
            int npoints, k, pressed;
            npoints = me->touchPoints().count();
            for(k = 0; k < npoints; ++k) {
                QPointF coordinates = me->touchPoints().at(k).startPos();

                if(me->touchPoints().at(k).state() & Qt::TouchPointPressed)
                    pressed = 1;
                else if(me->touchPoints().at(k).
                        state() & Qt::TouchPointReleased)
                    pressed = 0;
                else
                    continue;   // NOT INTERESTED IN DRAGGING

                if(pressed && (coordinates.y() < 30)) {
                    contextMenuEvent(NULL);
                    return true;
                }
            }

        }

        if(ev->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast < QMouseEvent * >(ev);
            QPoint coordinates = me->pos();
            //qreal relx,rely;

            if(coordinates.y() < 30) {
                // TOUCHED THE TOP BAR, SHOW THE MENU
                contextMenuEvent(NULL);
                return true;
            }

        }

    }

    return false;
}

void MainWindow::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu popup;
    popup.addMenu(ui->menuFile);
    popup.addMenu(ui->menuStack);
    popup.addMenu(ui->menuHardware);
    popup.setStyleSheet("font-size: 16px;");
    QString menufilestyle = ui->menuFile->styleSheet();
    ui->menuFile->setStyleSheet("font-size: 16px;");
    QString menustkstyle = ui->menuStack->styleSheet();
    ui->menuStack->setStyleSheet("font-size: 16px;");
    QString menuhardstyle = ui->menuHardware->styleSheet();
    ui->menuHardware->setStyleSheet("font-size: 16px;");
    popup.exec(ui->centralWidget->mapToGlobal(ui->EmuScreen->pos()));

    ui->menuFile->setStyleSheet(menufilestyle);
    ui->menuStack->setStyleSheet(menustkstyle);
    ui->menuHardware->setStyleSheet(menuhardstyle);
    if(event)
        event->accept();
}

void MainWindow::on_actionPaste_and_compile_triggered()
{
    if(!rpl.isRunning())
        return; // DO NOTHING

    while(!__cpu_idle)
        QThread::msleep(1);     // BLOCK UNTIL RPL IS IDLE

    __cpu_idle = 2;     // BLOCK REQUEST

    // NOW WORK ON THE RPL ENGINE WHILE THE THREAD IS BLOCKED
    Clipboard2StackCompile();

    __cpu_idle = 0;     // LET GO THE SIMULATOR

    halScreenUpdated();

}

// ****************************************** USB DRIVER ON A SEPARATE THREAD

USBThread::USBThread(QObject * parent)
:      QThread(parent)
{
}

USBThread::~USBThread()
{
}

// RUNNING THREAD FOR USB COMMS
// __usb_paused=0 MEANS COMMS ARE ACTIVE
// __usb_paused==1 MEANS COMMS ARE TEMPORARILY HALTED
// __usb_paused==2 MEANS EXIT THE THREAD

void USBThread::run()
{
    QElapsedTimer timer;
    timer.start();
    __pcsystmr = timer.elapsed() * 100; // INITIALIZE TICK COUNTER
    __usb_timeout = 5000;       // DEFAULT

    while((__usb_paused != 2) && (__usb_paused != -2)) {
        if(__usb_curdevice && !(__usb_drvstatus & USB_STATUS_CONNECTED))
            usb_irqdisconnect();
        if(__usb_paused == 0)
            usb_irqservice();
        else if(__usb_paused > 0)
            __usb_paused = -__usb_paused;       // SIGNAL THAT THE PAUSE WAS ACKNOWLEDGED BY MAKING IT NEGATIVE

        __pcsystmr = timer.elapsed() * 100;

        if(__tmr_singleshot_running) {
            if(__tmr1_msec)
                __tmr1_msec--;
            if(!__tmr1_msec) {
                __tmr_singleshot_running = 0;
                __tmr_newirqeventsvc();

            }
        }
        if(!(__usb_drvstatus & USB_STATUS_NOWAIT))
            msleep(1);
    }
    if(__usb_paused == 2) {
        // EXIT WAS REQUESTED, RELEASE ALL HANDLES
        usb_irqdisconnect();

        __usb_paused = -__usb_paused;   // MAKE SURE WE END THE THREAD WITH A NEGATIVE NUMBER
    }
}

QString MainWindow::getDocumentsLocation()
{
    QString path;

    if(currentfile.isEmpty()) {
        path = QStandardPaths::locate(QStandardPaths::DocumentsLocation,
                "newRPL", QStandardPaths::LocateDirectory);
        if(path.isEmpty())
            path = QStandardPaths::writableLocation(QStandardPaths::
                    DocumentsLocation);
    }
    else {
        QFileInfo info(currentfile);
        path = info.path();
    }

    return path;
}

extern "C"
{
    void usb_mutex_lock_implementation(void);
    void usb_mutex_unlock_implementation(void);
}

static QMutex usb_mutex;

void usb_mutex_lock_implementation(void)
{
    usb_mutex.lock();
}

void usb_mutex_unlock_implementation(void)
{
    usb_mutex.unlock();
}

void MainWindow::on_actionColor_Theme_Editor_triggered()
{
    themeEdit.setModal(false);
    themeEdit.ReadPalette();
    themeEdit.show();
}

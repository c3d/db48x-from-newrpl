#include <QtGui>
#include <QtCore>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow *myMainWindow;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    rpl(this),
    ui(new Ui::MainWindow)
{
    myMainWindow=this;

    ui->setupUi(this);
    screentmr=new QTimer(this);
    ui->EmuScreen->connect(screentmr,SIGNAL(timeout()),ui->EmuScreen,SLOT(update()));
    screentmr->start(50);
    maintmr=new QTimer(this);
    connect(maintmr,SIGNAL(timeout()),this,SLOT(domaintimer()));
    maintmr->start(1);

    rpl.start();

}

MainWindow::~MainWindow()
{
    screentmr->stop();
    delete screentmr;
    delete ui;
}

void MainWindow::on_EmuScreen_destroyed()
{
    if(rpl.isRunning()) rpl.terminate();
    QMainWindow::close();
}

extern volatile long long __pcsystmr;
int __tmr_singleshot_running=0;
volatile unsigned long long __tmr1_msec;

extern "C" void __tmr_newirqeventsvc();


void MainWindow::domaintimer()
{
// THE CLOCK IS FIXED AT 100 KHZ, SO ADD 1 msec
__pcsystmr+=100;
if(__tmr_singleshot_running) {
    __tmr1_msec--;
    if(!__tmr1_msec) {
        __tmr_singleshot_running=0;
        __tmr_newirqeventsvc();
    }
}
}

extern "C" void stop_singleshot()
{
    __tmr_singleshot_running=0;
}


extern "C" void timer_singleshot(int msec)
{
    __tmr1_msec=msec;
    __tmr_singleshot_running=1;
}





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
    Qt::Key_Backspace,  1,
    Qt::Key_U,          2,
    Qt::Key_Slash,      3,
    Qt::Key_Asterisk,   4,
    Qt::Key_Minus,      5,
    Qt::Key_Plus,       6,
    Qt::Key_Return,     7,
    Qt::Key_Enter,      7,
    Qt::Key_P,          9,
    Qt::Key_T,          10,
    Qt::Key_Y,          11,
    Qt::Key_9,          12,
    Qt::Key_6,          13,
    Qt::Key_3,          14,
    Qt::Key_Space,      15,
    Qt::Key_O,          17,
    Qt::Key_S,          18,
    Qt::Key_X,          19,
    Qt::Key_8,          20,
    Qt::Key_5,          21,
    Qt::Key_2,          22,
    Qt::Key_Period,     23,
    Qt::Key_N,          25,
    Qt::Key_R,          26,
    Qt::Key_W,          27,
    Qt::Key_7,          28,
    Qt::Key_4,          29,
    Qt::Key_1,          30,
    Qt::Key_0,          31,
    Qt::Key_M,          33,
    Qt::Key_Q,          34,
    Qt::Key_V,          35,
    Qt::Key_A,          41,
    Qt::Key_F1,         41,
    Qt::Key_B,          42,
    Qt::Key_F2,         42,
    Qt::Key_C,          43,
    Qt::Key_F3,         43,
    Qt::Key_D,          44,
    Qt::Key_F4,         44,
    Qt::Key_E,          45,
    Qt::Key_F5,         45,
    Qt::Key_F,          46,
    Qt::Key_F6,         46,
    Qt::Key_G,          47,
    Qt::Key_Up,         49,
    Qt::Key_Left,       50,
    Qt::Key_Down,       51,
    Qt::Key_Right,      52,
    Qt::Key_H,          53,
    Qt::Key_I,          54,
    Qt::Key_J,          55,
    Qt::Key_K,          57,
    Qt::Key_L,          58,
    Qt::Key_Tab,        60,
    Qt::Key_CapsLock,   61,
    Qt::Key_Control,    62,
    Qt::Key_Escape,     63

    // ADD MORE KEYS HERE

    ,0,0
};








extern unsigned long long __pckeymatrix;
extern "C" void __keyb_update();


void MainWindow::keyPressEvent(QKeyEvent *ev)
{

    int i;

    if(ev->isAutoRepeat()) { ev->accept(); return; }

    for(i=0;keyMap[i]!=0;i+=2) {
        if(ev->key()==keyMap[i]) {
            __pckeymatrix|=1ULL<<(keyMap[i+1]);
            __keyb_update();
            ev->accept();
            return;
        }
    }

    QMainWindow::keyPressEvent(ev);
}

void MainWindow::keyReleaseEvent(QKeyEvent *ev)
{

    int i;
    if(ev->isAutoRepeat()) { ev->accept(); return; }

    for(i=0;keyMap[i]!=0;i+=2) {
        if(ev->key()==keyMap[i]) {
            __pckeymatrix&=~(1ULL<<(keyMap[i+1]));
            __keyb_update();
            ev->accept();
            return;
        }
    }


    QMainWindow::keyReleaseEvent(ev);
}

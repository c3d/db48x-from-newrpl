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
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hidapi.h"
#define takemax(a,b) (((a)>(b))? (a):(b))

MainWindow *myMainWindow;


// CAN'T INCLUDE THE HEADERS DIRECTLY DUE TO CONFLICTING TYPES ON WINDOWS ONLY...

extern unsigned long long __pckeymatrix;
extern int __pc_terminate;
extern int __memmap_intact;
extern volatile int __cpu_idle;

extern "C" void __keyb_update();
// BACKUP/RESTORE
extern "C" int rplBackup(void (*writefunc)(unsigned int,void *),void *);
extern "C" int rplRestoreBackup(unsigned int (*readfunc)(void *),void *);
extern "C" int rplRestoreBackupMessedup(unsigned int (*readfunc)(void *),void *);    // DEBUG ONLY
extern "C" void __SD_irqeventinsert();


extern int __sd_inserted;
extern int __sd_nsectors;             // TOTAL SIZE OF SD CARD IN 512-BYTE SECTORS
extern int __sd_RCA;
extern unsigned char *__sd_buffer;    // BUFFER WITH THE ENTIRE CONTENTS OF THE SD CARD


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    rpl(this),
    ui(new Ui::MainWindow)
{
    myMainWindow=this;

    ui->setupUi(this);
    screentmr=new QTimer(this);
    ui->EmuScreen->connect(screentmr,SIGNAL(timeout()),ui->EmuScreen,SLOT(update()));
    maintmr=new QTimer(this);
    connect(maintmr,SIGNAL(timeout()),this,SLOT(domaintimer()));
    __memmap_intact=0;
    __sd_inserted=0;
    __sd_RCA=0;
    __sd_nsectors=0;
    __sd_buffer=NULL;
    ui->actionEject_SD_Card_Image->setEnabled(false);
    ui->actionInsert_SD_Card_Image->setEnabled(true);

    rpl.start();
    maintmr->start(1);
    screentmr->start(50);
    setWindowTitle("newRPL - [Unnamed]");

}

MainWindow::~MainWindow()
{
    delete maintmr;
    delete screentmr;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    on_actionExit_triggered();

    event->accept();

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
    Qt::Key_Z,          3,
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
    Qt::Key_Escape,     63,
    Qt::Key_Home,       63

    // ADD MORE KEYS HERE

    ,0,0
};









void MainWindow::keyPressEvent(QKeyEvent *ev)
{

    int i;

    if(ev->isAutoRepeat()) { ev->accept(); return; }

    if(ev->key()==Qt::Key_F12) {
        __pckeymatrix=(1ULL<<63) | (1ULL<<41) | (1ULL<<43);
        __keyb_update();
        ev->accept();
        return;
    }


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

    if(ev->key()==Qt::Key_F12) {
        __pckeymatrix&=~((1ULL<<63) | (1ULL<<41) | (1ULL<<43));
        __keyb_update();
        ev->accept();
        return;
    }



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

extern "C" void thread_processevents()
{
    QCoreApplication::processEvents();
}

void MainWindow::on_actionExit_triggered()
{
    maintmr->stop();
    screentmr->stop();
    if(rpl.isRunning()) {
        __cpu_idle=0;
        __pc_terminate=1;
        __pckeymatrix^=(1ULL<<63);
        __keyb_update();
    while(rpl.isRunning());
    }

    // CLEANUP SD CARD EMULATION
    if(__sd_inserted) on_actionEject_SD_Card_Image_triggered();

}

void MainWindow::WriteWord(unsigned int word)
{
    myMainWindow->fileptr->write((const char *)&word,4);
}

unsigned int MainWindow::ReadWord()
{
    unsigned int w;
    myMainWindow->fileptr->read((char *)&w,4);
    return w;
}


extern "C" void write_data(unsigned int word,void *opaque)
{
    (void)opaque;
    MainWindow::WriteWord(word);
}
extern "C" unsigned int read_data(void *opaque)
{
    (void)opaque;
    return MainWindow::ReadWord();
}


void MainWindow::on_actionSave_triggered()
{
    QString fname;

    if(currentfile.isEmpty()) fname=QFileDialog::getSaveFileName(this,"Select File Name",QString(),"*.nrpl");
    else fname=currentfile;
    if(!fname.isEmpty()) {
        // GOT A NAME, APPEND EXTENSION IF NOT GIVEN

        if(!fname.endsWith(".nrpl")) fname+=".nrpl";

        QFile file(fname);

        if(!file.open(QIODevice::WriteOnly)) {
            QMessageBox a(QMessageBox::Warning,"Error while saving","Cannot write to file "+ fname,QMessageBox::Ok,this);
            a.exec();
            return;
        }

        // FILE IS OPEN AND READY FOR WRITING

        // STOP RPL ENGINE
        maintmr->stop();
        screentmr->stop();
        if(rpl.isRunning()) {
            __cpu_idle=0;
            __pc_terminate=1;
            __pckeymatrix^=(1ULL<<63);
            __keyb_update();
        while(rpl.isRunning());
        }

        // PERFORM BACKUP
        myMainWindow=this;
        fileptr=&file;
        rplBackup(&write_data,(void *)fileptr);

        file.close();

        __memmap_intact=1;
        // RESTART RPL ENGINE
        __pc_terminate=0;
        __pckeymatrix=0;
        rpl.start();
        maintmr->start(1);
        screentmr->start(50);
        }

}

void MainWindow::on_actionOpen_triggered()
{
    QString fname=QFileDialog::getOpenFileName(this,"Open File Name",QString(),"*.nrpl");

    if(!fname.isEmpty()) {
        QFile file(fname);

        if(!file.open(QIODevice::ReadOnly)) {
            QMessageBox a(QMessageBox::Warning,"Error while opening","Cannot open file "+ fname,QMessageBox::Ok,this);
            a.exec();
            return;
        }

        // FILE IS OPEN AND READY FOR READING

        // STOP RPL ENGINE
        maintmr->stop();
        screentmr->stop();
        if(rpl.isRunning()) {
            __cpu_idle=0;
            __pc_terminate=1;
            __pckeymatrix^=(1ULL<<63);
            __keyb_update();
        while(rpl.isRunning());
        }

        // PERFORM RESTORE PROCEDURE
        myMainWindow=this;
        fileptr=&file;
        int result=rplRestoreBackup(&read_data,(void *)fileptr);

        file.close();


        switch(result)
        {
        case -1:
        {
            QMessageBox a(QMessageBox::Warning,"Error while opening","File "+ fname + " is corrupt or incompatible.\nCan't recover and memory was destroyed.",QMessageBox::Ok,this);
            a.exec();
            currentfile.clear();
            setWindowTitle("newRPL - [Unnamed]");
            __memmap_intact=0;
            break;
        }
        case 0:
        {
            QMessageBox a(QMessageBox::Warning,"Error while opening","File "+ fname + " is corrupt or incompatible.\nCan't recover but memory was left intact.",QMessageBox::Ok,this);
            a.exec();
            __memmap_intact=1;
            break;
        }
        case 1:
        {
            QMessageBox a(QMessageBox::Warning,"Recovery success","File "+ fname + " was sucessfully recovered.",QMessageBox::Ok,this);
            a.exec();
            currentfile=fname;
            QString nameonly=currentfile.right(currentfile.length()-1-takemax(currentfile.lastIndexOf("/"),currentfile.lastIndexOf("\\")));
            setWindowTitle("newRPL - ["+ nameonly + "]");

            __memmap_intact=1;

            break;
        }
        case 2:
        {
            QMessageBox a(QMessageBox::Warning,"Recovery success","File "+ fname + " was recovered with minor errors.\nRun MEMFIX to correct them.",QMessageBox::Ok,this);
            a.exec();
            currentfile=fname;
            QString nameonly=currentfile.right(currentfile.length()-1-takemax(currentfile.lastIndexOf("/"),currentfile.lastIndexOf("\\")));
            setWindowTitle("newRPL - ["+ nameonly + "]");

            __memmap_intact=1;
            break;
        }

        }

        // RESTART RPL ENGINE
        __pc_terminate=0;
        __pckeymatrix=0;

        rpl.start();
        maintmr->start(1);
        screentmr->start(50);
        }


}

void MainWindow::on_actionSaveAs_triggered()
{
    QString fname=QFileDialog::getSaveFileName(this,"Select File Name",QString(),"*.nrpl");

    if(!fname.isEmpty()) {
        // GOT A NAME, APPEND EXTENSION IF NOT GIVEN

        if(!fname.endsWith(".nrpl")) fname+=".nrpl";

        QFile file(fname);

        if(!file.open(QIODevice::WriteOnly)) {
            QMessageBox a(QMessageBox::Warning,"Error while saving","Cannot write to file "+ fname,QMessageBox::Ok,this);
            a.exec();
            return;
        }

        // FILE IS OPEN AND READY FOR WRITING

        // STOP RPL ENGINE
        maintmr->stop();
        screentmr->stop();
        if(rpl.isRunning()) {
            __cpu_idle=0;
            __pc_terminate=1;
            __pckeymatrix^=(1ULL<<63);
            __keyb_update();
        while(rpl.isRunning());
        }

        // PERFORM BACKUP
        myMainWindow=this;
        fileptr=&file;
        rplBackup(&write_data,(void *)fileptr);

        file.close();

        // RESTART RPL ENGINE
        __memmap_intact=1;
        __pc_terminate=0;
        __pckeymatrix=0;
        rpl.start();
        maintmr->start(1);
        screentmr->start(50);
        }

}

void MainWindow::on_actionNew_triggered()
{


    // STOP RPL ENGINE
    maintmr->stop();
    screentmr->stop();
    if(rpl.isRunning()) {
        __cpu_idle=0;
        __pc_terminate=1;
        __pckeymatrix^=(1ULL<<63);
        __keyb_update();
    while(rpl.isRunning());
    }

    currentfile.clear();
    setWindowTitle("newRPL - [Unnamed]");
    __memmap_intact=0;

    // RESTART RPL ENGINE
    __pc_terminate=0;
    __pckeymatrix=0;

    rpl.start();
    maintmr->start(1);
    screentmr->start(50);


}

void MainWindow::on_actionInsert_SD_Card_Image_triggered()
{
    QString fname=QFileDialog::getOpenFileName(this,"Open SD Card Image",QString(),"*.img");
    if(!fname.isEmpty()) {
        sdcard.setFileName(fname);

        if(!sdcard.open(QIODevice::ReadOnly)) {
            QMessageBox a(QMessageBox::Warning,"Error while opening","Cannot open file "+ fname,QMessageBox::Ok,this);
            a.exec();
            return;
        }

        __sd_inserted=0;
        __sd_RCA=0;
        __sd_nsectors=0;
        if(__sd_buffer!=NULL) free(__sd_buffer);


        // FILE IS OPEN AND READY FOR READING
        __sd_buffer=(unsigned char *)malloc(sdcard.size());
        if(__sd_buffer==NULL) {
            QMessageBox a(QMessageBox::Warning,"Error while opening","Not enough memory to read SD Image",QMessageBox::Ok,this);
            a.exec();
            return;
        }

        if(sdcard.read((char *)__sd_buffer,sdcard.size())!=sdcard.size()) {
            QMessageBox a(QMessageBox::Warning,"Error while opening","Can't read SD Image",QMessageBox::Ok,this);
            a.exec();
            return;
        }

        __sd_nsectors=sdcard.size()/512;
        __sd_inserted=1;
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
                QMessageBox a(QMessageBox::Warning,"Error while saving SD Card contents","Cannot open file "+ sdcard.fileName(),QMessageBox::Ok,this);
                a.exec();
            }
            else {
                sdcard.write((char *)__sd_buffer,(qint64)__sd_nsectors*512LL);
                sdcard.close();
            }
        }
        __sd_inserted=0;
        __sd_RCA=0;
        __sd_nsectors=0;
        if(__sd_buffer!=NULL) { free(__sd_buffer); __sd_buffer=NULL; }

        // SIMULATE AN IRQ
        __SD_irqeventinsert();


        ui->actionEject_SD_Card_Image->setEnabled(false);
        ui->actionInsert_SD_Card_Image->setEnabled(true);

}

void MainWindow::on_actionPower_ON_triggered()
{

    // STOP RPL ENGINE
    maintmr->stop();
    screentmr->stop();
    if(rpl.isRunning()) {
        __cpu_idle=0;
        __pc_terminate=1;
        __pckeymatrix^=(1ULL<<63);
        __keyb_update();
    while(rpl.isRunning());
    }

    if(__pc_terminate==2) {
        // IT WAS POWERED OFF
        __memmap_intact=2;
    }
    else __memmap_intact=1;

    // RESTART RPL ENGINE
    __pc_terminate=0;
    __pckeymatrix=0;

    rpl.start();
    maintmr->start(1);
    screentmr->start(50);

}

void MainWindow::on_actionSimulate_Alarm_triggered()
{

}

void MainWindow::on_actionTake_Screenshot_triggered()
{
    QString fname=QFileDialog::getSaveFileName(this,"Save screenshot as...",QString(),"*.png");
     if(!fname.isEmpty()) {

    QRectF r=ui->EmuScreen->sceneRect();
    QImage image(r.width()*4,r.height()*4,QImage::Format_ARGB32);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    ui->EmuScreen->scene()->render(&painter);
    image.save(fname);
     }
}


extern void Stack2Clipboard(int level,int dropit);
extern void Clipboard2Stack();
extern int SaveRPLObject(QString& filename,int level);
int LoadRPLObject(QString& filename);


void MainWindow::on_actionCopy_Level_1_triggered()
{
    if(!rpl.isRunning()) return;    // DO NOTHING

    while(!__cpu_idle)     QThread::msleep(1);  // BLOCK UNTIL RPL IS IDLE

    __cpu_idle=2;       // BLOCK REQUEST

    // NOW WORK ON THE RPL ENGINE WHILE THE THREAD IS BLOCKED
    Stack2Clipboard(1,0);

    __cpu_idle=0;   // LET GO THE SIMULATOR
}

void MainWindow::on_actionPaste_to_Level_1_triggered()
{
    if(!rpl.isRunning()) return;    // DO NOTHING

    while(!__cpu_idle)     QThread::msleep(1);  // BLOCK UNTIL RPL IS IDLE

    __cpu_idle=2;       // BLOCK REQUEST

    // NOW WORK ON THE RPL ENGINE WHILE THE THREAD IS BLOCKED
    Clipboard2Stack();

    __cpu_idle=0;   // LET GO THE SIMULATOR

}

void MainWindow::on_actionCut_Level_1_triggered()
{
    if(!rpl.isRunning()) return;    // DO NOTHING

    while(!__cpu_idle)     QThread::msleep(1);  // BLOCK UNTIL RPL IS IDLE

    __cpu_idle=2;       // BLOCK REQUEST

    // NOW WORK ON THE RPL ENGINE WHILE THE THREAD IS BLOCKED
    Stack2Clipboard(1,1);

    __cpu_idle=0;   // LET GO THE SIMULATOR
}

void MainWindow::on_actionSave_Level_1_As_triggered()
{
    QString fname=QFileDialog::getSaveFileName(this,"Select File Name",QString(),"*.nrpl");

    if(!fname.isEmpty()) {
        // GOT A NAME, APPEND EXTENSION IF NOT GIVEN

        if(!fname.endsWith(".nrpl")) fname+=".nrpl";
    }

    if(!SaveRPLObject(fname,1)) {
            QMessageBox a(QMessageBox::Warning,"Error while saving","Cannot write to file "+ fname,QMessageBox::Ok,this);
            a.exec();
            return;
    }
}

void MainWindow::on_actionOpen_file_to_Level_1_triggered()
{
    QString fname=QFileDialog::getOpenFileName(this,"Select File Name",QString(),"*.nrpl");
    if(!rpl.isRunning()) return;    // DO NOTHING

    while(!__cpu_idle)     QThread::msleep(1);  // BLOCK UNTIL RPL IS IDLE

    __cpu_idle=2;       // BLOCK REQUEST

    // NOW WORK ON THE RPL ENGINE WHILE THE THREAD IS BLOCKED
    if(!LoadRPLObject(fname)) {
            QMessageBox a(QMessageBox::Warning,"Error while opening","Cannot read file. Corrupted data?\n"+ fname,QMessageBox::Ok,this);
            a.exec();
            return;
    }

    __cpu_idle=0;   // LET GO THE SIMULATOR

}

void MainWindow::on_actionConnect_to_calc_triggered()
{
    struct hid_device_info *devs, *cur_dev;

        if (hid_init())
            return;

        devs = hid_enumerate(0x0, 0x0);
        cur_dev = devs;
        QString result;
        result.clear();
        while (cur_dev) {
            result+=QString::asprintf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
            result+=QString::asprintf("\n");
            result+=QString::asprintf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
            result+=QString::asprintf("  Product:      %ls\n", cur_dev->product_string);
            result+=QString::asprintf("  Release:      %hx\n", cur_dev->release_number);
            result+=QString::asprintf("  Interface:    %d\n",  cur_dev->interface_number);
            result+=QString::asprintf("\n");
            cur_dev = cur_dev->next;
        }
        hid_free_enumeration(devs);

        hid_exit();
}

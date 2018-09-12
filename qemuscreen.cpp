/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "qemuscreen.h"
#include <ui.h>

#include <QGraphicsPixmapItem>
#include <QBitmap>

extern int __lcd_mode;
extern unsigned int *__lcd_buffer;


#define min(a,b) (((a)>(b))? (b):(a))



QEmuScreen::QEmuScreen(QWidget *parent) :
    QGraphicsView(parent),
    annHourglass(QString(":/bitmap/bitmap/ann_busy.xbm")),
    annComms(QString(":/bitmap/bitmap/ann_io.xbm")),
    annAlpha(QString(":/bitmap/bitmap/ann_alpha.xbm")),
    annBattery(QString(":/bitmap/bitmap/ann_battery.xbm")),
    annLShift(QString(":/bitmap/bitmap/ann_left.xbm")),
    annRShift(QString(":/bitmap/bitmap/ann_right.xbm"))
{
    int i,j;


    screen_height=80;
    screen_width=131;
    BkgndColor=QColor(172,222,157);
    MainColor=QColor(0,0,0);
    BkgndPen.setColor(BkgndColor);
    BkgndPen.setStyle(Qt::NoPen);
    BkgndPen.setWidthF(0.05);

    for(i=0;i<16;++i)
    {
        Grays[i].setRed((MainColor.red()-BkgndColor.red())*((qreal)(i+1))/((qreal)16.0)+BkgndColor.red());
        Grays[i].setGreen((MainColor.green()-BkgndColor.green())*((qreal)(i+1))/((qreal)16.0)+BkgndColor.green());
        Grays[i].setBlue((MainColor.blue()-BkgndColor.blue())*((qreal)(i+1))/((qreal)16.0)+BkgndColor.blue());
        Grays[i].setAlpha(255);
        GrayBrush[i].setColor(Grays[i]);
        GrayBrush[i].setStyle(Qt::SolidPattern);
    }

    Pixels=new QGraphicsRectItem *[screen_height*screen_width];

    scr.clear();

    scr.setBackgroundBrush(QBrush(BkgndColor));
    for(j=0;j<screen_height;++j) {
    for(i=0;i<screen_width;++i)
    {
     Pixels[j*screen_width+i]=scr.addRect(i,j,1.0,1.0,BkgndPen,GrayBrush[0]);
    }
    }

    for(j=0;j<screen_height;++j) {
    for(i=0;i<screen_width;++i)
    {
     Pixels[j*screen_width+i]->setBrush(GrayBrush[i&15]);
    }
    }

    // ADD SOME ANNUNCIATORS
    annHourglass.setMask(annHourglass.createMaskFromColor(Qt::white));
    annBattery.setMask(annBattery.createMaskFromColor(Qt::white));
    annComms.setMask(annComms.createMaskFromColor(Qt::white));
    annAlpha.setMask(annAlpha.createMaskFromColor(Qt::white));
    annLShift.setMask(annLShift.createMaskFromColor(Qt::white));
    annRShift.setMask(annRShift.createMaskFromColor(Qt::white));

    /*
     * (131, 0) - Comms
     * (131, 1) - Left Shift
     * (131, 2) - Right Shift
     * (131, 3) - Alpha
     * (131, 4) - Low Battery
     * (131, 5) - Wait
    */


    Annunciators[0]=scr.addPixmap(annComms);
    Annunciators[0]->setScale(0.25);
    Annunciators[0]->setOffset(120*4-0*80,-20);
    Annunciators[0]->setOpacity(1.0);

    Annunciators[1]=scr.addPixmap(annLShift);
    Annunciators[1]->setScale(0.25);
    Annunciators[1]->setOffset(120*4-5*80,-20);
    Annunciators[1]->setOpacity(1.0);

    Annunciators[2]=scr.addPixmap(annRShift);
    Annunciators[2]->setScale(0.25);
    Annunciators[2]->setOffset(120*4-4*80,-20);
    Annunciators[2]->setOpacity(1.0);

    Annunciators[3]=scr.addPixmap(annAlpha);
    Annunciators[3]->setScale(0.25);
    Annunciators[3]->setOffset(120*4-3*80,-20);
    Annunciators[3]->setOpacity(1.0);

    Annunciators[4]=scr.addPixmap(annBattery);
    Annunciators[4]->setScale(0.25);
    Annunciators[4]->setOffset(120*4-2*80,-20);
    Annunciators[4]->setOpacity(1.0);

    Annunciators[5]=scr.addPixmap(annHourglass);
    Annunciators[5]->setScale(0.25);
    Annunciators[5]->setOffset(120*4-1*80,-20);
    Annunciators[5]->setOpacity(1.0);

    setScene(&scr);
    setSceneRect(0,-5,screen_width,screen_height+5);
    centerOn(qreal(screen_width)/2,qreal(screen_height)/2);
    m_scale=1.0;
    setScale(4.0);


    show();
}

// SET A PIXEL IN THE SPECIFIED COLOR
void QEmuScreen::setPixel(int offset,int color)
{
    Pixels[offset]->setBrush(GrayBrush[color&15]);
}

void QEmuScreen::setWord(int offset,unsigned int color)
{
    int f;
    for(f=0;f<32;++f) {
        Pixels[offset+f]->setBrush(GrayBrush[(color>>(f*4))&15]);
    }
}

void QEmuScreen::setScale(qreal _scale)
{
    scale(_scale/m_scale,_scale/m_scale);
    m_scale=_scale;

    QSize s;
    s.setWidth(0);
    s.setHeight((screen_height+5)*m_scale);
    setMinimumSize(s);
}


void QEmuScreen::update()
{
    int i,j;
    unsigned int color;

    if(__lcd_mode==0) {
        // MONOCHROME SCREEN
        unsigned int *ptr;
        int mask;
        for(i=0;i<screen_height;++i) {
            mask=1;
            ptr=__lcd_buffer+(LCD_W>>5)*i;
            for(j=0;j<screen_width;++j) {
                color=*ptr&mask;
                Pixels[i*screen_width+j]->setBrush(GrayBrush[ (color? 15:0)]);
                Pixels[i*screen_width+j]->setPen(BkgndPen);
                mask<<=1;
                if(!mask) { mask=1; ++ptr; }
            }
        }

        // UPDATE ANNUNCIATORS
        mask=1<<3;
        for(i=0;i<6;++i)
        {
            ptr=__lcd_buffer+(LCD_W>>5)*i;
            color=(*ptr&mask)>>3;
            Annunciators[i]->setOpacity(color? 1.0:0.0);
        }

    return;
    }

    if(__lcd_mode==2) {
        // 16-GRAYS SCREEN
        unsigned int *ptr;
        int mask;
        for(i=0;i<screen_height;++i) {
            mask=0xf;
            ptr=__lcd_buffer+(LCD_W>>3)*i;
            for(j=0;j<screen_width;++j) {
                color=(*ptr&mask)>>((j&7)*4);
                Pixels[i*screen_width+j]->setBrush(GrayBrush[color]);
                Pixels[i*screen_width+j]->setPen(BkgndPen);
                mask<<=4;
                if(!mask) { mask=0xf; ++ptr; }
            }
        }
        // UPDATE ANNUNCIATORS
        mask=0xf<<12;
        for(i=0;i<6;++i)
        {
            ptr=__lcd_buffer+16+(LCD_W>>3)*i;
            color=(*ptr&mask)>>12;
            Annunciators[i]->setOpacity(((qreal)color)/15.0);
        }

    return;
    }

    // ANY OTHER MODE IS UNSUPPORTED, SHOW BLANK SCREEN
    for(i=0;i<screen_height;++i) {
        for(j=0;j<screen_width;++j) {
        Pixels[i*screen_width+j]->setBrush(GrayBrush[0]);
        }
        // UPDATE ANNUNCIATORS
        for(i=0;i<6;++i)
        {
            Annunciators[i]->setOpacity(0.0);
        }

    }

}


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
#include <QTimer>

extern int __lcd_mode;
extern int __lcd_needsupdate;
extern unsigned int *__lcd_buffer;

#define min(a,b) (((a)>(b))? (b):(a))

QEmuScreen::QEmuScreen(QWidget * parent):
QGraphicsView(parent),
annHourglass(QString(":/bitmap/bitmap/ann_busy.xbm")),
annComms(QString(":/bitmap/bitmap/ann_io.xbm")),
annAlpha(QString(":/bitmap/bitmap/ann_alpha.xbm")),
annBattery(QString(":/bitmap/bitmap/ann_battery.xbm")),
annLShift(QString(":/bitmap/bitmap/ann_left.xbm")),
annRShift(QString(":/bitmap/bitmap/ann_right.xbm")),
mainPixmap(SCREEN_WIDTH,SCREEN_HEIGHT)
{
    int i;

    screentmr = nullptr;

    screen_height = SCREEN_HEIGHT;
    screen_width = SCREEN_WIDTH;
    BkgndColor = QColor(172, 222, 157);
    MainColor = QColor(0, 0, 0);
    BkgndPen.setColor(BkgndColor);
    BkgndPen.setStyle(Qt::NoPen);
    BkgndPen.setWidthF(0.05);

    for(i = 0; i < 16; ++i) {
        Grays[i].setRed((MainColor.red() - BkgndColor.red()) * ((qreal) (i +
                        1)) / ((qreal) 16.0) + BkgndColor.red());
        Grays[i].setGreen((MainColor.green() -
                    BkgndColor.green()) * ((qreal) (i + 1)) / ((qreal) 16.0) +
                BkgndColor.green());
        Grays[i].setBlue((MainColor.blue() - BkgndColor.blue()) * ((qreal) (i +
                        1)) / ((qreal) 16.0) + BkgndColor.blue());
        Grays[i].setAlpha(255);
        GrayBrush[i].setColor(Grays[i]);
        GrayBrush[i].setStyle(Qt::SolidPattern);
    }


    scr.clear();

    scr.setBackgroundBrush(/*QBrush(BkgndColor)*/ QBrush(Qt::black));


    mainPixmap.fill(Grays[8]);
    mainScreen = scr.addPixmap(mainPixmap);
    mainScreen->setOffset(0.0,0.0);

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

    Annunciators[0] = scr.addPixmap(annComms);
    Annunciators[0]->setScale(0.25);
    Annunciators[0]->setOffset(((SCREEN_WIDTH-10) - 0 * ((SCREEN_WIDTH-30)/5))*4, -20);
    Annunciators[0]->setOpacity(1.0);

    Annunciators[1] = scr.addPixmap(annLShift);
    Annunciators[1]->setScale(0.25);
    Annunciators[1]->setOffset(((SCREEN_WIDTH-10) - 5 * ((SCREEN_WIDTH-30)/5))*4, -20);
    Annunciators[1]->setOpacity(1.0);

    Annunciators[2] = scr.addPixmap(annRShift);
    Annunciators[2]->setScale(0.25);
    Annunciators[2]->setOffset(((SCREEN_WIDTH-10) - 4 * ((SCREEN_WIDTH-30)/5))*4, -20);
    Annunciators[2]->setOpacity(1.0);

    Annunciators[3] = scr.addPixmap(annAlpha);
    Annunciators[3]->setScale(0.25);
    Annunciators[3]->setOffset(((SCREEN_WIDTH-10) - 3 * ((SCREEN_WIDTH-30)/5))*4, -20);
    Annunciators[3]->setOpacity(1.0);

    Annunciators[4] = scr.addPixmap(annBattery);
    Annunciators[4]->setScale(0.25);
    Annunciators[4]->setOffset(((SCREEN_WIDTH-10) - 2 * ((SCREEN_WIDTH-30)/5))*4, -20);
    Annunciators[4]->setOpacity(1.0);

    Annunciators[5] = scr.addPixmap(annHourglass);
    Annunciators[5]->setScale(0.25);
    Annunciators[5]->setOffset(((SCREEN_WIDTH-10) - 1 * ((SCREEN_WIDTH-30)/5))*4, -20);
    Annunciators[5]->setOpacity(1.0);

    setScene(&scr);
    setSceneRect(0, -5, screen_width, screen_height + 5);
    centerOn(qreal(screen_width) / 2, qreal(screen_height) / 2);
    m_scale = 1.0;
    setScale(4.0);

    show();
}

QEmuScreen::~QEmuScreen()
{
 //   delete[] Pixels;
}

void QEmuScreen::setTimer(QTimer * tmr)
{
    screentmr = tmr;
}

// SET A PIXEL IN THE SPECIFIED COLOR
void QEmuScreen::setPixel(int offset, int color)
{
    //Pixels[offset]->setBrush(GrayBrush[color & 15]);

   QPainter pt(&mainPixmap);
   pt.setPen(Grays[color & 15]);
   pt.drawPoint(offset%SCREEN_WIDTH,offset/SCREEN_HEIGHT);

}

void QEmuScreen::setWord(int offset, unsigned int color)
{
    int f;
    QPainter pt(&mainPixmap);

    for(f = 0; f < 32; ++f) {
//        Pixels[offset + f]->setBrush(GrayBrush[(color >> (f * 4)) & 15]);
          pt.setPen(Grays[(color >> (f * 4)) & 15]);
          pt.drawPoint((offset+f)%SCREEN_WIDTH,(offset+f)/SCREEN_HEIGHT);
    }
}

void QEmuScreen::setScale(qreal _scale)
{
    scale(_scale / m_scale, _scale / m_scale);
    m_scale = _scale;

    QSize s;
    s.setWidth(0);
    s.setHeight((screen_height + 5) * m_scale);
    setMinimumSize(s);
}

void QEmuScreen::update()
{

    if(__lcd_needsupdate)
        __lcd_needsupdate = 0;
    else {
        if(screentmr) {
            screentmr->setSingleShot(true);
            screentmr->start(20);
        }
        return;
    }

    int i, j;
    unsigned int color;

    if(__lcd_mode == 0) {
        // MONOCHROME SCREEN

        QPainter pt(&mainPixmap);

        unsigned int *ptr;
        int mask;
        for(i = 0; i < screen_height; ++i) {
            mask = 1;
            ptr = __lcd_buffer + (LCD_W >> 5) * i;
            for(j = 0; j < screen_width; ++j) {
                color = *ptr & mask;
                //Pixels[i * screen_width +
                //        j]->setBrush(GrayBrush[(color ? 15 : 0)]);
                pt.setPen(Grays[(color ? 15 : 0)]);
                //Pixels[i * screen_width + j]->setPen(BkgndPen);
                pt.drawPoint(j,i);
                mask <<= 1;
                if(!mask) {
                    mask = 1;
                    ++ptr;
                }
            }
        }
        pt.end();

        // UPDATE ANNUNCIATORS
        mask = 1 << 3;
        for(i = 0; i < 6; ++i) {
            ptr = __lcd_buffer + (LCD_W >> 5) * i;
            color = (*ptr & mask) >> 3;
            Annunciators[i]->setOpacity(color ? 1.0 : 0.0);
        }


        mainScreen->setPixmap(mainPixmap);
        QGraphicsView::update();
        if(screentmr) {
            screentmr->setSingleShot(true);
            screentmr->start(20);
        }

        return;
    }

    if(__lcd_mode == 2) {
        // 16-GRAYS SCREEN
        unsigned int *ptr;
        int mask;

        QPainter pt(&mainPixmap);

        for(i = 0; i < screen_height; ++i) {
            mask = 0xf;
            ptr = __lcd_buffer + (LCD_W >> 3) * i;
            for(j = 0; j < screen_width; ++j) {
                color = (*ptr & mask) >> ((j & 7) * 4);
                //Pixels[i * screen_width + j]->setBrush(GrayBrush[color]);
                //Pixels[i * screen_width + j]->setPen(BkgndPen);
                pt.setPen(Grays[color]);
                pt.drawPoint(j,i);

                mask <<= 4;
                if(!mask) {
                    mask = 0xf;
                    ++ptr;
                }
            }
        }

        pt.end();

        // UPDATE ANNUNCIATORS
        mask = (((1<<BITSPERPIXEL)-1) << (BITSPERPIXEL*(ANN_X_COORD % (PIXELS_PER_WORD))));
        for(i = 0; i < 6; ++i) {
            ptr = __lcd_buffer + ANN_X_COORD / (PIXELS_PER_WORD);
            ptr += i * (SCREEN_W / PIXELS_PER_WORD);
            color = (*ptr & mask) >> (BITSPERPIXEL*(ANN_X_COORD % (PIXELS_PER_WORD)));
            Annunciators[i]->setOpacity(((qreal) color) / 15.0);
        }

        mainScreen->setPixmap(mainPixmap);
        QGraphicsView::update();
        if(screentmr) {
            screentmr->setSingleShot(true);
            screentmr->start(20);
        }

        return;
    }

    if(__lcd_mode == 3) {
        // RGB COLOR SCREEN (5-6-5)

        QPainter pt(&mainPixmap);

        QImage lcdimage((const unsigned char *)__lcd_buffer,screen_width,screen_height,(screen_width*4)/PIXELS_PER_WORD,QImage::Format_RGB16);

        pt.drawImage(0,0,lcdimage,0,0,screen_width,screen_height,Qt::AutoColor);

        pt.end();
        /*
        for(i = 0; i < screen_height; ++i) {
            mask = 0xf;
            ptr = __lcd_buffer + (LCD_W >> 3) * i;
            for(j = 0; j < screen_width; ++j) {
                color = (*ptr & mask) >> ((j & 7) * 4);
                //Pixels[i * screen_width + j]->setBrush(GrayBrush[color]);
                //Pixels[i * screen_width + j]->setPen(BkgndPen);
                pt.setPen(Grays[color]);
                pt.drawPoint(j,i);

                mask <<= 4;
                if(!mask) {
                    mask = 0xf;
                    ++ptr;
                }
            }
        }

        pt.end();
        */

        // RGB SCREENS DON'T HAVE SEPARATE ANNUNCIATORS TO UPDATE

        /*
        mask = (((1<<BITSPERPIXEL)-1) << (BITSPERPIXEL*(ANN_X_COORD % (PIXELS_PER_WORD))));
        for(i = 0; i < 6; ++i) {
            ptr = __lcd_buffer + ANN_X_COORD / (PIXELS_PER_WORD);
            ptr += i * (SCREEN_W / PIXELS_PER_WORD);
            color = (*ptr & mask) >> (BITSPERPIXEL*(ANN_X_COORD % (PIXELS_PER_WORD)));
            Annunciators[i]->setOpacity(((qreal) color) / 15.0);
        }
        */
        mainScreen->setPixmap(mainPixmap);
        QGraphicsView::update();
        if(screentmr) {
            screentmr->setSingleShot(true);
            screentmr->start(20);
        }

        return;
    }

    // ANY OTHER MODE IS UNSUPPORTED, SHOW BLANK SCREEN

        mainPixmap.fill(Grays[8]);


        // UPDATE ANNUNCIATORS
        for(i = 0; i < 6; ++i) {
            Annunciators[i]->setOpacity(0.0);
        }

    mainScreen->setPixmap(mainPixmap);
    QGraphicsView::update();
    if(screentmr) {
        screentmr->setSingleShot(true);
        screentmr->start(20);
    }

}

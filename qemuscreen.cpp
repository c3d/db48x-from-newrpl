/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "qemuscreen.h"

#include <QBitmap>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <ui.h>

extern int           lcd_mode;
extern int           lcd_needsupdate;
extern int           lcd_activebuffer;
extern unsigned int *lcd_buffer;

#define min(a, b) (((a) > (b)) ? (b) : (a))

QEmuScreen::QEmuScreen(QWidget *parent)
    : QGraphicsView(parent),
      annHourglass(QString(":/bitmap/bitmap/ann_busy.xbm")),
      annComms(QString(":/bitmap/bitmap/ann_io.xbm")),
      annAlpha(QString(":/bitmap/bitmap/ann_alpha.xbm")),
      annBattery(QString(":/bitmap/bitmap/ann_battery.xbm")),
      annLShift(QString(":/bitmap/bitmap/ann_left.xbm")),
      annRShift(QString(":/bitmap/bitmap/ann_right.xbm")),
      mainPixmap(LCD_W, LCD_H)
{
    int i;

    screentmr     = nullptr;

    screen_height = LCD_H;
    screen_width  = LCD_W;
#ifndef TARGET_PC_DM42
    BkgndColor    = QColor(172, 222, 157);
#else // TARGET_PC_DM42
    BkgndColor    = QColor(230, 230, 230);
#endif // TARGET_PC_DM42
    MainColor     = QColor(0, 0, 0);
    BkgndPen.setColor(BkgndColor);
    BkgndPen.setStyle(Qt::NoPen);
    BkgndPen.setWidthF(0.05);

    for (i = 0; i < 16; ++i)
    {
        Grays[i].setRed((MainColor.red() - BkgndColor.red()) *
                            ((qreal) (i + 1)) / ((qreal) 16.0) +
                        BkgndColor.red());
        Grays[i].setGreen((MainColor.green() - BkgndColor.green()) *
                              ((qreal) (i + 1)) / ((qreal) 16.0) +
                          BkgndColor.green());
        Grays[i].setBlue((MainColor.blue() - BkgndColor.blue()) *
                             ((qreal) (i + 1)) / ((qreal) 16.0) +
                         BkgndColor.blue());
        Grays[i].setAlpha(255);
        GrayBrush[i].setColor(Grays[i]);
        GrayBrush[i].setStyle(Qt::SolidPattern);
    }


    scr.clear();

    scr.setBackgroundBrush(/*QBrush(BkgndColor)*/ QBrush(Qt::black));

    mainPixmap.fill(Grays[8]);
    mainScreen = scr.addPixmap(mainPixmap);
    mainScreen->setOffset(0.0, 0.0);

    // Add some annunciators
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
    Annunciators[0]->setOffset(((LCD_W - 10) - 0 * ((LCD_W - 30) / 5)) * 4,
                               -20);
    Annunciators[0]->setOpacity(1.0);

    Annunciators[1] = scr.addPixmap(annLShift);
    Annunciators[1]->setScale(0.25);
    Annunciators[1]->setOffset(((LCD_W - 10) - 5 * ((LCD_W - 30) / 5)) * 4,
                               -20);
    Annunciators[1]->setOpacity(1.0);

    Annunciators[2] = scr.addPixmap(annRShift);
    Annunciators[2]->setScale(0.25);
    Annunciators[2]->setOffset(((LCD_W - 10) - 4 * ((LCD_W - 30) / 5)) * 4,
                               -20);
    Annunciators[2]->setOpacity(1.0);

    Annunciators[3] = scr.addPixmap(annAlpha);
    Annunciators[3]->setScale(0.25);
    Annunciators[3]->setOffset(((LCD_W - 10) - 3 * ((LCD_W - 30) / 5)) * 4,
                               -20);
    Annunciators[3]->setOpacity(1.0);

    Annunciators[4] = scr.addPixmap(annBattery);
    Annunciators[4]->setScale(0.25);
    Annunciators[4]->setOffset(((LCD_W - 10) - 2 * ((LCD_W - 30) / 5)) * 4,
                               -20);
    Annunciators[4]->setOpacity(1.0);

    Annunciators[5] = scr.addPixmap(annHourglass);
    Annunciators[5]->setScale(0.25);
    Annunciators[5]->setOffset(((LCD_W - 10) - 1 * ((LCD_W - 30) / 5)) * 4,
                               -20);
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

void QEmuScreen::setTimer(QTimer *tmr)
{
    screentmr = tmr;
}

// Set a pixel in the specified color
void QEmuScreen::setPixel(int offset, QEmuScreen::palette_index color)
{
    // Pixels[offset]->setBrush(GrayBrush[color & 15]);

    QPainter pt(&mainPixmap);
    pt.setPen(Grays[color & 15]);
    pt.drawPoint(offset % LCD_W, offset / LCD_H);
}

void QEmuScreen::setPixelWord(int offset, QEmuScreen::pixword color)
{
    int      f;
    QPainter pt(&mainPixmap);

    for (f = 0; f < 32; ++f)
    {
        //        Pixels[offset + f]->setBrush(GrayBrush[(color >> (f * 4)) &
        //        15]);
        pt.setPen(Grays[(color >> (f * 4)) & 15]);
        pt.drawPoint((offset + f) % LCD_W, (offset + f) / LCD_H);
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
    if (lcd_needsupdate)
        lcd_needsupdate = 0;
    else
    {
        if (screentmr)
        {
            screentmr->setSingleShot(true);
            screentmr->start(20);
        }
        return;
    }

    int     i, j;
    pixword color;

    if (lcd_mode == 0)
    {
        // Monochrome screen
        scr.setBackgroundBrush(QBrush(BkgndColor));
        QPainter      pt(&mainPixmap);

        unsigned int *ptr, *buffer;
        int           mask;
        buffer = lcd_buffer +
                 (lcd_activebuffer ? (LCD_W * LCD_H / PIXELS_PER_WORD) : 0);
        for (i = 0; i < screen_height; ++i)
        {
            mask = 1;
            ptr  = buffer + (LCD_SCANLINE >> 5) * i;
            for (j = 0; j < screen_width; ++j)
            {
                color = *ptr & mask;
                // Pixels[i * screen_width +
                //         j]->setBrush(GrayBrush[(color ? 15 : 0)]);
#ifndef TARGET_PC_DM42
                pt.setPen(Grays[(color ? 15 : 0)]);
#else // TARGET_PC_DM42
                pt.setPen(color ? BkgndColor : Qt::black);
#endif // TARGET_PC_DM42
                // Pixels[i * screen_width + j]->setPen(BkgndPen);
                pt.drawPoint(j, i);
                mask <<= 1;
                if (!mask)
                {
                    mask = 1;
                    ++ptr;
                }
            }
        }
        pt.end();

        // UPDATE ANNUNCIATORS
        mask = 1 << 3;
        for (i = 0; i < 6; ++i)
        {
            ptr   = buffer + (LCD_SCANLINE >> 5) * i;
            color = (*ptr & mask) >> 3;
            Annunciators[i]->setOpacity(color ? 1.0 : 0.0);
        }


        mainScreen->setPixmap(mainPixmap);
        QGraphicsView::update();
        if (screentmr)
        {
            screentmr->setSingleShot(true);
            screentmr->start(20);
        }

        return;
    }

    if (lcd_mode == 2)
    {
        // 16-GRAYS SCREEN
        unsigned int *ptr, *buffer;
        buffer = lcd_buffer +
                 (lcd_activebuffer ? (LCD_W * LCD_H / PIXELS_PER_WORD) : 0);
        int mask;
        scr.setBackgroundBrush(QBrush(BkgndColor));
        QPainter pt(&mainPixmap);

        for (i = 0; i < screen_height; ++i)
        {
            mask = 0xf;
            ptr  = buffer + (LCD_SCANLINE >> 3) * i;
            for (j = 0; j < screen_width; ++j)
            {
                color = (*ptr & mask) >> ((j & 7) * 4);
                // Pixels[i * screen_width + j]->setBrush(GrayBrush[color]);
                // Pixels[i * screen_width + j]->setPen(BkgndPen);
                pt.setPen(Grays[color]);
                pt.drawPoint(j, i);

                mask <<= 4;
                if (!mask)
                {
                    mask = 0xf;
                    ++ptr;
                }
            }
        }

        pt.end();

        // UPDATE ANNUNCIATORS
        mask = (((1 << BITSPERPIXEL) - 1)
                << (BITSPERPIXEL * (ANN_X_COORD % (PIXELS_PER_WORD))));
        for (i = 0; i < 6; ++i)
        {
            ptr = buffer + ANN_X_COORD / (PIXELS_PER_WORD);
            ptr += i * (LCD_SCANLINE / PIXELS_PER_WORD);
            color = (*ptr & mask) >>
                    (BITSPERPIXEL * (ANN_X_COORD % (PIXELS_PER_WORD)));
            Annunciators[i]->setOpacity(((qreal) color) / 15.0);
        }

        mainScreen->setPixmap(mainPixmap);
        QGraphicsView::update();
        if (screentmr)
        {
            screentmr->setSingleShot(true);
            screentmr->start(20);
        }

        return;
    }

    if (lcd_mode == 3)
    {
        // RGB COLOR SCREEN (5-6-5)

        scr.setBackgroundBrush(QBrush(Qt::black));
        QPainter      pt(&mainPixmap);
        unsigned int *buffer =
            lcd_buffer +
            (lcd_activebuffer ? (LCD_W * LCD_H / PIXELS_PER_WORD) : 0);

        QImage lcdimage((const unsigned char *) buffer,
                        screen_width,
                        screen_height,
                        (screen_width * 4) / PIXELS_PER_WORD,
                        QImage::Format_RGB16);

        pt.drawImage(0,
                     0,
                     lcdimage,
                     0,
                     0,
                     screen_width,
                     screen_height,
                     Qt::AutoColor);

        pt.end();
        /*
        for(i = 0; i < screen_height; ++i) {
            mask = 0xf;
            ptr = lcd_buffer + (LCD_SCANLINE >> 3) * i;
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
        mask = (((1<<BITSPERPIXEL)-1) << (BITSPERPIXEL*(ANN_X_COORD %
        (PIXELS_PER_WORD)))); for(i = 0; i < 6; ++i) { ptr = lcd_buffer +
        ANN_X_COORD / (PIXELS_PER_WORD); ptr += i * (LCD_SCANLINE /
        PIXELS_PER_WORD); color = (*ptr & mask) >> (BITSPERPIXEL*(ANN_X_COORD %
        (PIXELS_PER_WORD))); Annunciators[i]->setOpacity(((qreal) color)
        / 15.0);
        }
        */
        mainScreen->setPixmap(mainPixmap);
        QGraphicsView::update();
        if (screentmr)
        {
            screentmr->setSingleShot(true);
            screentmr->start(20);
        }

        return;
    }

    // ANY OTHER MODE IS UNSUPPORTED, SHOW BLANK SCREEN

    mainPixmap.fill(Grays[8]);


    // UPDATE ANNUNCIATORS
    for (i = 0; i < 6; ++i)
        Annunciators[i]->setOpacity(0.0);

    mainScreen->setPixmap(mainPixmap);
    QGraphicsView::update();
    if (screentmr)
    {
        screentmr->setSingleShot(true);
        screentmr->start(20);
    }
}

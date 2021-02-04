/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef QEMUSCREEN_H
#define QEMUSCREEN_H

#include <QGraphicsView>

class QEmuScreen:public QGraphicsView
{
  Q_OBJECT public:
    // SIZE OF THE SCREEN TO EMULATE
    int screen_width;
    int screen_height;
    qreal m_scale;
    // MAIN COLORS, ALL GRAYS WILL BE INTERPOLATED
    QColor BkgndColor;
    QColor MainColor;

    QTimer *screentmr;
    QColor Grays[16];   // ARRAY WITH ALL THE DIFFERENT GRAY LEVELS
    QBrush GrayBrush[16];
    QPen BkgndPen;
    QPixmap annHourglass, annComms, annAlpha, annBattery, annLShift, annRShift,mainPixmap;

    QGraphicsScene scr;

    QGraphicsPixmapItem *Annunciators[6], *mainScreen;

    void setTimer(QTimer * tmr);
    void setPixel(int offset, int color);
    void setWord(int offset, unsigned int color);
    void setMode(int _mode, unsigned int *_buffer);
    void setScale(qreal _scale);
    explicit QEmuScreen(QWidget * parent = 0);
    ~QEmuScreen();
    
        signals: public slots: void update();

};

#endif // QSCREEN_H

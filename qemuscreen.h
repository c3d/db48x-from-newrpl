#ifndef QEMUSCREEN_H
#define QEMUSCREEN_H

#include <QGraphicsView>

class QEmuScreen : public QGraphicsView
{
    Q_OBJECT
public:
    // SIZE OF THE SCREEN TO EMULATE
    int screen_width;
    int screen_height;
    // MAIN COLORS, ALL GRAYS WILL BE INTERPOLATED
    QColor BkgndColor;
    QColor MainColor;

    QColor Grays[16];   // ARRAY WITH ALL THE DIFFERENT GRAY LEVELS
    QBrush GrayBrush[16];
    QPixmap annHourglass,annComms,annAlpha,annBattery,annLShift,annRShift;


    QGraphicsScene scr;

    QGraphicsRectItem **Pixels;
    QGraphicsPixmapItem *Annunciators[6];

    void setPixel(int offset,int color);
    void setWord(int offset,unsigned int color);
    void setMode(int _mode,unsigned int *_buffer);
    explicit QEmuScreen(QWidget *parent = 0);

signals:

public slots:
    void update();

};

#endif // QSCREEN_H

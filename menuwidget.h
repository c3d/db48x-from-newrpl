#ifndef MENUWIDGET_H
#define MENUWIDGET_H

#include <QObject>
#include <QWidget>
#include <QQuickWidget>

class menuWidget : public QQuickWidget
{
public:
    menuWidget();
    menuWidget(QWidget *parent);
};

#endif // MENUWIDGET_H

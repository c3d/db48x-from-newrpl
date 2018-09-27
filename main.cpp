/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include "mainwindow.h"
#include <QApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QWindow>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);
/*
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    QWindow *qmlWindow = qobject_cast<QWindow*>(engine.rootObjects().at(0));
    QWidget *container = QWidget::createWindowContainer(qmlWindow);
    container->setMinimumSize (qmlWindow->size ().width (), qmlWindow->size ().height ()+30);
*/
    MainWindow w;

//    w.setMenuWidget (container);


    w.show();


//    engine.rootContext()->setContextProperty("mymainWindow", &w);

    return a.exec();
}

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
#include <recorder.h>

RECORDER(main, 16, "Main entry point of the simulator");

int main(int argc, char *argv[])
{
    const char *traces = getenv("NEWRPL_TRACES");
    if (traces)
        recorder_trace_set(traces);
    recorder_dump_on_common_signals(0, 0);
    record(main,
           "Simulator build %u invoked as %+s with %d arguments",
           NEWRPL_BUILDNUM, argv[0], argc-1);
    for (int a = 1; a < argc; a++)
    {
        record(main, "  %u: %+s", a, argv[a]);
        if (argv[a][0] == '-' && argv[a][1] == 't')
            recorder_trace_set(argv[a]+2);
    }

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

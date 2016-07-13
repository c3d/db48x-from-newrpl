/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include "rplthread.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QTimer *screentmr,*maintmr;
    QFile sdcard;
    QString currentfile;
    RPLThread rpl;

public:
    QFile *fileptr;

    static void WriteWord(unsigned int word);
    static unsigned int ReadWord();

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    virtual void keyPressEvent(QKeyEvent *ev);
    virtual void keyReleaseEvent(QKeyEvent *ev);
    void closeEvent(QCloseEvent *event);

private slots:
    void on_EmuScreen_destroyed();
    void on_actionExit_triggered();

    void on_actionSave_triggered();

    void on_actionOpen_triggered();

    void on_actionSaveAs_triggered();

    void on_actionNew_triggered();

    void on_actionInsert_SD_Card_Image_triggered();

    void on_actionEject_SD_Card_Image_triggered();

public slots:
    void domaintimer();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

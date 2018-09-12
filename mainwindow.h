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
    QString currentusb,currentusbpath;
    RPLThread rpl;

public:
    QFile *fileptr;

    static int WriteWord(unsigned int word);
    static unsigned int ReadWord();

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    virtual void keyPressEvent(QKeyEvent *ev);
    virtual void keyReleaseEvent(QKeyEvent *ev);
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *ev);
    void resizeEvent(QResizeEvent *event);

private slots:
    void on_EmuScreen_destroyed();
    void on_actionExit_triggered();

    void on_actionSave_triggered();

    void on_actionOpen_triggered();

    void on_actionSaveAs_triggered();

    void on_actionNew_triggered();

    void on_actionInsert_SD_Card_Image_triggered();

    void on_actionEject_SD_Card_Image_triggered();

    void on_actionPower_ON_triggered();

    void on_actionSimulate_Alarm_triggered();

    void on_actionTake_Screenshot_triggered();

    void on_actionCopy_Level_1_triggered();

    void on_actionPaste_to_Level_1_triggered();

    void on_actionCut_Level_1_triggered();

    void on_actionSave_Level_1_As_triggered();

    void on_actionOpen_file_to_Level_1_triggered();

    void on_actionConnect_to_calc_triggered();



    void on_usbconnectButton_clicked();


    void on_actionUSB_Remote_ARCHIVE_to_file_triggered();

    void on_actionRemote_USBRESTORE_from_file_triggered();

    void on_actionShow_LCD_grid_toggled(bool arg1);


public slots:
    void usbupdate();
    void domaintimer();
private:
    int OpenFile(QString fname);
    void SaveFile(QString fname);

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

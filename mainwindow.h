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

public slots:
    void domaintimer();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

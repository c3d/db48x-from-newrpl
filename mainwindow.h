#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    virtual void keyPressEvent(QKeyEvent *ev);
    virtual void keyReleaseEvent(QKeyEvent *ev);

private slots:
    void on_EmuScreen_destroyed();
public slots:
    void domaintimer();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

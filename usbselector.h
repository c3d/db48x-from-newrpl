#ifndef USBSELECTOR_H
#define USBSELECTOR_H

#include <QDialog>
#include <QThread>

namespace Ui {
class USBSelector;
}

class FWThread : public QThread
{
    Q_OBJECT
public:
    void run();

    FWThread(QObject *parent);
    ~FWThread();
};



class USBSelector : public QDialog
{
    Q_OBJECT
    QTimer *tmr;
    QString SelectedDevicePath;
    QString SelectedDeviceName;
    int numberoftries;
    FWThread update_thread;
    QByteArray filedata;

    virtual void closeEvent(QCloseEvent *event);

public:
    void RefreshList();
    QString& getSelectedDevicePath();
    QString& getSelectedDeviceName();

    explicit USBSelector(QWidget *parent = 0);
    ~USBSelector();

private slots:
    void on_USBtreeWidget_itemSelectionChanged();

    void on_USBSelector_accepted();

    void on_USBSelector_rejected();
    void refresh();
    void reconnect();

    void on_updateFirmware_clicked();

    void finishedupdate();
    void updateprogress();

    virtual void reject();


private:
    Ui::USBSelector *ui;
};

#endif // USBSELECTOR_H

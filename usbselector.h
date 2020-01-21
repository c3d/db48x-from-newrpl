#ifndef USBSELECTOR_H
#define USBSELECTOR_H

#include <QDialog>
#include <QThread>
#include <QList>

namespace Ui
{
    class USBSelector;
}

class FWThread:public QThread
{
  Q_OBJECT public:
    void run();

        FWThread(QObject * parent);
       ~FWThread();

        signals: void FirmwareUpdateError(QString message);

};

#define MAX_DIALOG_DEVICES 50

class USBSelector:public QDialog
{
    Q_OBJECT QString SelectedDevicePath;
    QString SelectedDeviceName;
    int numberoftries;
    bool norefresh;
    FWThread update_thread;
    QByteArray filedata;

    virtual void closeEvent(QCloseEvent * event);

  public:
    void RefreshList();
        QString & getSelectedDevicePath();
        QString & getSelectedDeviceName();

    explicit USBSelector(QWidget * parent = 0);
       ~USBSelector();

    private slots: void on_USBtreeWidget_itemSelectionChanged();

    void on_USBSelector_accepted();

    void on_USBSelector_rejected();
    void refresh();

    void on_updateFirmware_clicked();

    void finishedupdate();
    void updateprogress();

    void on_Error(QString message);

    virtual void reject();

  private:
        Ui::USBSelector * ui;

};

#endif // USBSELECTOR_H

#ifndef USBSELECTOR_H
#define USBSELECTOR_H

#include <QDialog>

namespace Ui {
class USBSelector;
}

class USBSelector : public QDialog
{
    Q_OBJECT
    QTimer *tmr;
    QString SelectedDevicePath;

public:
    void RefreshList();
    QString& getSelectedDevicePath();

    explicit USBSelector(QWidget *parent = 0);
    ~USBSelector();

private slots:
    void on_USBtreeWidget_itemSelectionChanged();

    void on_USBSelector_accepted();

    void on_USBSelector_rejected();
    void refresh();

private:
    Ui::USBSelector *ui;
};

#endif // USBSELECTOR_H

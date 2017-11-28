#ifndef USBSELECTOR_H
#define USBSELECTOR_H

#include <QDialog>

namespace Ui {
class USBSelector;
}

class USBSelector : public QDialog
{
    Q_OBJECT

public:
    explicit USBSelector(QWidget *parent = 0);
    ~USBSelector();

private slots:
    void on_USBtreeWidget_itemSelectionChanged();

private:
    Ui::USBSelector *ui;
};

#endif // USBSELECTOR_H

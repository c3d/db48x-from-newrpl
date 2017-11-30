#include <QMessageBox>
#include <QTreeWidget>
#include <QTimer>

#include "hidapi.h"
#include "usbselector.h"
#include "ui_usbselector.h"

extern "C" {
#include "newrpl.h"
#include "libraries.h"

extern hid_device *__usb_curdevice;

BINT64 rplObjChecksum(WORDPTR object);
}


USBSelector::USBSelector(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::USBSelector)
{
    ui->setupUi(this);


    SelectedDevicePath.clear();

    RefreshList();

    tmr = new QTimer(this);
    if(tmr) {
    connect(tmr, SIGNAL(timeout()), this, SLOT(refresh()));
    tmr->start(2000);
    }

}

USBSelector::~USBSelector()
{
    if(tmr) {
        tmr->stop();
        delete tmr;
        tmr=0;
    }

    delete ui;
}

void USBSelector::on_USBtreeWidget_itemSelectionChanged()
{


    QString result;

    result.clear();
    QTreeWidgetItem *newitem;

    if(ui->USBtreeWidget->selectedItems().count()>=1) newitem=ui->USBtreeWidget->selectedItems().first();
    else {
        return;
    }

    if(newitem->text(2)==QString("[Device not responding]")) {
        ui->buttonBox->setStandardButtons( QDialogButtonBox::Cancel);
        SelectedDevicePath.clear();
        ui->selectedCalc->setText(QString("No device selected."));
    }
    else {
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        SelectedDevicePath=newitem->data(0,Qt::UserRole+3).toString();
        ui->selectedCalc->setText(newitem->text(0)+QString("[build ")+newitem->text(2).right(4)+QString("]"));
    }

}

QString& USBSelector::getSelectedDevicePath()
{
    return SelectedDevicePath;
}

void USBSelector::RefreshList()
{
    struct hid_device_info *devs, *cur_dev;
    QTreeWidgetItem *newitem;


        if (hid_init())
        {
            ui->USBtreeWidget->clear();
            return;
        }

        devs = hid_enumerate(0x0, 0x0);
        if(!devs) {
            ui->USBtreeWidget->clear();
        return;
        }

        cur_dev = devs;
        QString result;
        result.clear();

        {
        // FIRST DISABLE ALL ITEMS IN THE LIST

        QTreeWidgetItemIterator it(ui->USBtreeWidget);
        while(*it) {
            (*it)->setDisabled(true);
            ++it;
        }

        }

        while (cur_dev) {

            QString manuf;

            if(cur_dev->manufacturer_string) manuf=QString::fromStdWString(cur_dev->manufacturer_string);

            if(manuf.startsWith("newRPL")) {
            QTreeWidgetItemIterator it(ui->USBtreeWidget);

            newitem=0;
            while(*it) {
                if( ((*it)->data(0,Qt::UserRole+1).toInt()==cur_dev->vendor_id)
                        && ((*it)->data(0,Qt::UserRole+2).toInt()==cur_dev->product_id)
                        && ((*it)->data(0,Qt::UserRole+3).toString()==QString(cur_dev->path))
                        )
                {
                    // FOUND THE SAME ITEM AGAIN
                    newitem=*it;
                    (*it)->setDisabled(false);
                }
                ++it;

            }

            if(!newitem) {
                newitem=new QTreeWidgetItem(ui->USBtreeWidget);

                if(!newitem) return;
            }

            QString tmp;

            if(cur_dev->product_string) tmp=QString::fromStdWString(cur_dev->product_string);
            else tmp="[Unknown]";
            if(cur_dev->serial_number) tmp+=QString("|SN=")+QString::fromStdWString(cur_dev->serial_number);

                newitem->setText(0,tmp);

                if(cur_dev->manufacturer_string) tmp=QString::fromStdWString(cur_dev->manufacturer_string);
                else tmp="[Unknown]";

                    newitem->setText(1,tmp);

                        newitem->setData(0,Qt::UserRole+1,QVariant(cur_dev->vendor_id));
                        newitem->setData(0,Qt::UserRole+2,QVariant(cur_dev->product_id));
                        newitem->setData(0,Qt::UserRole+3,QVariant(QString(cur_dev->path)));
                        newitem->setData(0,Qt::UserRole+4,QVariant(QString::fromStdWString(cur_dev->serial_number)));

                        hid_device *thisdev;

                        thisdev=hid_open_path(cur_dev->path);

                        if(thisdev)
                        {
                            // ATTEMPT TO SEND SOMETHING TO SEE IF IT'S ACTIVELY RESPONDING
                            uint32_t getversion[]={
                                0,          // 0 = DON'T USE REPORT ID'S - THIS IS REQUIRED ONLY FOR HIDAPI
                                0xab,       // BLOCK SIZE AND MARKER
                                0,         // CRC32
                                MKPROLOG(SECO,5),  // ACTUAL DATA
                                CMD_VERSION,
                                CMD_DROP,
                                CMD_USBSEND,
                                CMD_DROP,
                                CMD_QSEMI
                            };

                            getversion[1]|=(1+OBJSIZE(getversion[3]))<<10;
                            getversion[2]=usb_crc32((BYTEPTR) &(getversion[3]),(1+OBJSIZE(getversion[3]))*4);



                            int res=hid_write(thisdev,((const unsigned char *)getversion)+3,(getversion[1]>>8)+9);
                            if(res<0) {
                                hid_close(thisdev);
                                tmp="[Device not responding]";
                                newitem->setText(2,tmp);
                            }
                            else {
                                unsigned char buffer[1024];
                                res=hid_read_timeout(thisdev,buffer,1024,500);
                                hid_close(thisdev);

                                if(res<=0) {
                                    // DEVICE UNAVAILABLE
                                    tmp="[Device not responding]";
                                    newitem->setText(2,tmp);
                                }
                                else {
                                    // WE GOT A RESPONSE, THE DEVICE IS ALIVE!
                                    unsigned int strprolog;
                                    strprolog=buffer[8]+(buffer[9]<<8)+(buffer[10]<<16)+(buffer[11]<<24);

                                    tmp=QString::fromUtf8((const char *)(buffer+12),rplStrSize(&strprolog));
                                    newitem->setText(2,tmp);
                                }



                            }

                        }




            }
            cur_dev = cur_dev->next;
        }
        hid_free_enumeration(devs);

        // NOW ELIMINATE ANY ITEMS THAT ARE NOT ENABLED
        {

        QTreeWidgetItemIterator it(ui->USBtreeWidget);
        while(*it) {
            if((*it)->isDisabled()) {
                if(SelectedDevicePath==(*it)->data(0,Qt::UserRole+3).toString()) {

                    ui->buttonBox->setStandardButtons( QDialogButtonBox::Cancel);
                    SelectedDevicePath.clear();
                    ui->selectedCalc->setText(QString("No device selected."));
                    ui->USBtreeWidget->clearSelection();
                }


                delete (*it);
            }
            ++it;
        }

        }

        // DONE, THE LIST WAS REFRESHED

}

void USBSelector::on_USBSelector_accepted()
{
    if(tmr) {
        tmr->stop();
        delete tmr;
        tmr=0;
    }


}

void USBSelector::on_USBSelector_rejected()
{
    if(tmr) {
        tmr->stop();
        delete tmr;
        tmr=0;
    }
}

void USBSelector::refresh()
{
   RefreshList();
}

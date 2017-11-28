#include <QMessageBox>
#include <QTreeWidget>
#include <QTimer>

#include "hidapi.h"
#include "usbselector.h"
#include "ui_usbselector.h"

extern "C" {
#include "newrpl.h"
#include "libraries.h"

BINT64 rplObjChecksum(WORDPTR object);
}


USBSelector::USBSelector(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::USBSelector)
{
    ui->setupUi(this);

    RefreshList();

    tmr = new QTimer(this);
    if(tmr) {
    connect(tmr, SIGNAL(timeout()), this, SLOT(refresh()));
    tmr->start(1000);
    }

}

USBSelector::~USBSelector()
{
    delete ui;
}

void USBSelector::on_USBtreeWidget_itemSelectionChanged()
{

    tmr->stop();

    QString result;

    result.clear();
    QTreeWidgetItem *newitem;

    if(ui->USBtreeWidget->selectedItems().count()>=1) newitem=ui->USBtreeWidget->selectedItems().first();
    else {
        tmr->start();
        return;
    }
    QString path=newitem->data(0,3).toString(),tmp;

    hid_device *thisdev;

                    thisdev=hid_open_path(path.toUtf8());

                    if(thisdev)
                    {
                        // ATTEMPT TO SEND SOMETHING TO SEE IF IT'S ACTIVELY RESPONDING
                        uint32_t getversion[]={
                            0xab,       // BLOCK SIZE AND MARKER
                            0,         // CRC32
                            MKPROLOG(SECO,4),  // ACTUAL DATA
                            CMD_VERSION,
                            CMD_DROP,
                            CMD_USBSEND,
                            CMD_QSEMI
                        };

                        getversion[0]|=(1+OBJSIZE(getversion[2]))<<10;
                        getversion[1]=usb_crc32((BYTEPTR) &(getversion[2]),(1+OBJSIZE(getversion[2]))*4);



                        int res=hid_write(thisdev,(const unsigned char *)getversion,(getversion[0]>>8)+8);
                        if(res<0) {
                            hid_close(thisdev);
                            tmp="[Device not responding]";
                            newitem->setText(2,tmp);
                            ui->buttonBox->setStandardButtons(QDialogButtonBox::Cancel);

                        }
                        else {
                            unsigned char buffer[1024];
                            res=hid_read_timeout(thisdev,buffer,1024,1000);
                            hid_close(thisdev);

                            if(res<=0) {
                                // DEVICE UNAVAILABLE
                                tmp="[Device not responding]";
                                newitem->setText(2,tmp);
                                ui->buttonBox->setStandardButtons( QDialogButtonBox::Cancel);

                            }
                            else {
                                // WE GOT A RESPONSE, THE DEVICE IS ALIVE!
                                unsigned int strprolog;
                                strprolog=buffer[0]+(buffer[1]<<8)+(buffer[2]<<16)+(buffer[3]<<24);

                                tmp=QString::fromUtf8((const char *)(buffer+4),rplStrSize(&strprolog));
                                newitem->setText(2,tmp);

                                ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                            }



                        }

                    }


                    tmr->start();
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
                if( ((*it)->data(0,0).toInt()==cur_dev->vendor_id)
                        && ((*it)->data(0,1).toInt()==cur_dev->product_id)
                        && ((*it)->data(0,3).toString()==QString(cur_dev->path))
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

                newitem->setText(0,tmp);

                if(cur_dev->manufacturer_string) tmp=QString::fromStdWString(cur_dev->manufacturer_string);
                else tmp="[Unknown]";

                    newitem->setText(1,tmp);

                    if(cur_dev->serial_number) tmp=QString::fromStdWString(cur_dev->serial_number);
                    else tmp="[Unknown]";

                        newitem->setText(2,tmp);

                        newitem->setData(0,0,QVariant(cur_dev->vendor_id));
                        newitem->setData(0,1,QVariant(cur_dev->product_id));
                        newitem->setData(0,3,QVariant(QString(cur_dev->path)));

                        hid_device *thisdev;

                        thisdev=hid_open_path(cur_dev->path);

                        if(thisdev)
                        {
                            // ATTEMPT TO SEND SOMETHING TO SEE IF IT'S ACTIVELY RESPONDING
                            uint32_t getversion[]={
                                0xab,       // BLOCK SIZE AND MARKER
                                0,         // CRC32
                                MKPROLOG(SECO,4),  // ACTUAL DATA
                                CMD_VERSION,
                                CMD_DROP,
                                CMD_USBSEND,
                                CMD_QSEMI
                            };

                            getversion[0]|=(1+OBJSIZE(getversion[2]))<<10;
                            getversion[1]=usb_crc32((BYTEPTR) &(getversion[2]),(1+OBJSIZE(getversion[2]))*4);



                            int res=hid_write(thisdev,(const unsigned char *)getversion,(getversion[0]>>8)+8);
                            if(res<0) {
                                hid_close(thisdev);
                                tmp="[Device not responding]";
                                newitem->setText(2,tmp);
                            }
                            else {
                                unsigned char buffer[1024];
                                res=hid_read_timeout(thisdev,buffer,1024,1000);
                                hid_close(thisdev);

                                if(res<=0) {
                                    // DEVICE UNAVAILABLE
                                    tmp="[Device not responding]";
                                    newitem->setText(2,tmp);
                                }
                                else {
                                    // WE GOT A RESPONSE, THE DEVICE IS ALIVE!
                                    unsigned int strprolog;
                                    strprolog=buffer[0]+(buffer[1]<<8)+(buffer[2]<<16)+(buffer[3]<<24);

                                    tmp=QString::fromUtf8((const char *)(buffer+4),rplStrSize(&strprolog));
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
            if((*it)->isDisabled()) delete (*it);
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
    }
}

void USBSelector::on_USBSelector_rejected()
{
    if(tmr) {
        tmr->stop();
        delete tmr;
    }
}

void USBSelector::refresh()
{
   RefreshList();
}

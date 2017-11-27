#include <QMessageBox>

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

    struct hid_device_info *devs, *cur_dev;

        if (hid_init())
        {
            QMessageBox a(QMessageBox::Warning,"Error","Cannot initialize USB subsystem",QMessageBox::Ok,this);
            a.exec();
            return;
        }

        devs = hid_enumerate(0x0, 0x0);
        if(!devs) {
        QMessageBox a(QMessageBox::Information,"Connected devices","No USB devices detected",QMessageBox::Ok,this);
        a.exec();
        return;
        }

        cur_dev = devs;
        QString result;
        result.clear();
        while (cur_dev) {

            QString manuf;

            if(cur_dev->manufacturer_string) manuf=QString::fromStdWString(cur_dev->manufacturer_string);

            if(manuf.startsWith("newRPL")) {
            QTreeWidgetItem *newitem=new QTreeWidgetItem(ui->USBtreeWidget);

            if(!newitem) return;

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

                                if(res<=0) {
                                    // DEVICE UNAVAILABLE
                                    hid_close(thisdev);
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
}

USBSelector::~USBSelector()
{
    delete ui;
}

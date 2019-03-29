#include <QMessageBox>
#include <QTreeWidget>
#include <QTimer>
#include <QStandardPaths>
#include <QFileDialog>

#include "string.h"
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
    ui->updateFirmware->hide();

    RefreshList();

    tmr = new QTimer(this);
    if(tmr) {
    connect(tmr, SIGNAL(timeout()), this, SLOT(refresh()));
    tmr->start(500);
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
        SelectedDeviceName.clear();
        ui->selectedCalc->setText(QString("No device selected."));
        ui->updateFirmware->hide();

    }
    else {
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        SelectedDevicePath=newitem->data(0,Qt::UserRole+3).toString();
        SelectedDeviceName=newitem->text(0)+QString("[build ")+newitem->text(2).right(4)+QString("]");
        ui->selectedCalc->setText(SelectedDeviceName);
        ui->updateFirmware->show();

    }

}

QString& USBSelector::getSelectedDevicePath()
{
    return SelectedDevicePath;
}

QString& USBSelector::getSelectedDeviceName()
{
    return SelectedDeviceName;
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
                            uint32_t getversion[16]={
                                0,          // 0 = DON'T USE REPORT ID'S - THIS IS REQUIRED ONLY FOR HIDAPI
                                USB_BLOCKMARK_SINGLE,       // BLOCK SIZE AND MARKER
                                0,         // CRC32
                                MKPROLOG(SECO,5),  // ACTUAL DATA
                                CMD_VERSION,
                                CMD_DROP,
                                CMD_USBSEND,
                                CMD_DROP,
                                CMD_QSEMI,
                                0,0,0,0,0,0,0
                            };

                            getversion[1]|=(1+OBJSIZE(getversion[3]))<<10;
                            getversion[2]=usb_crc32((BYTEPTR) &(getversion[3]),(1+OBJSIZE(getversion[3]))*4);



                            int res=hid_write(thisdev,((const unsigned char *)getversion)+3,RAWHID_TX_SIZE+1);//(getversion[1]>>8)+9);
                            int available=0;
                            if(res>0) {
                                unsigned char buffer[1024];
                                res=hid_read_timeout(thisdev,buffer,1024,500);

                                if(res>0) {
                                    // WE GOT A RESPONSE, THE DEVICE IS ALIVE!


                                    if(buffer[0]==USB_BLOCKMARK_GETSTATUS) {
                                        // REMOTE IS ASKING IF WE ARE READY TO RECEIVE DATA
                                        memset(buffer,0,RAWHID_TX_SIZE+1);
                                        buffer[0]=0;    // REPORT ID
                                        buffer[1]=USB_BLOCKMARK_RESPONSE;   // RE ARE RESPONDING TO THE REQUEST
                                        buffer[2]=0;    // WE ARE NOT BUSY

                                        res=hid_write(thisdev,buffer,RAWHID_TX_SIZE+1);

                                        if(res>0) {
                                            res=hid_read_timeout(thisdev,buffer,1024,2000);

                                            if(res>0) {
                                                // WE GOT A RESPONSE, THE DEVICE IS ALIVE!
                                                if(buffer[0]==USB_BLOCKMARK_SINGLE) {
                                                unsigned int strprolog;
                                                strprolog=buffer[8]+(buffer[9]<<8)+(buffer[10]<<16)+(buffer[11]<<24);

                                                tmp=QString::fromUtf8((const char *)(buffer+12),rplStrSize(&strprolog));
                                                newitem->setText(2,tmp);
                                                available=1;
                                                }
                                            }

                                        }
                                    }

                                    }


                                }


                        hid_close(thisdev);

                        if(!available) {

                            tmp="[Device not responding]";
                            newitem->setText(2,tmp);
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
                    ui->updateFirmware->hide();
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

void USBSelector::reconnect()
{
    if(!__usb_curdevice) {
   RefreshList();
   if(!SelectedDevicePath.isEmpty()) {
       // CONNECT TO THE USB DEVICE
       __usb_curdevice=hid_open_path(SelectedDevicePath.toUtf8().constData());
   }
    }

    if(!__usb_curdevice) {
        ++numberoftries;
        if(numberoftries>10) {
            if(tmr) {
                tmr->stop();
                delete tmr;
                tmr=0;
            }
            // FAILED TO RECONNECT AFTER 5 SECONDS

            // TODO: SHOW SOME ERROR DIALOG
        }
        return;    // WAIT FOR ANOTHER TRY
    }

    // THE DEVICE IS BACK!

    if(tmr) {
        tmr->stop();
        delete tmr;
        tmr=0;
    }

    // TODO: ASK THE USER IF HE WANTS TO RESTORE FIRMWARE AFTER SUCCE

    return;
}

extern "C" int usbremotefwupdatestart();
extern "C" int usbsendtoremote(uint32_t *data,int nwords);
extern "C" void usbflush();


void USBSelector::on_updateFirmware_clicked()
{

        QString path;
        QByteArray filedata;

        path=QStandardPaths::locate(QStandardPaths::DocumentsLocation,"newRPL",QStandardPaths::LocateDirectory);
        if(path.isEmpty()) path=QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

        QString fname=QFileDialog::getOpenFileName(this,"Select firmware file to send to calculator",path,"firmware binary files (*.bin *.* *)");

        if(!fname.isEmpty()) {
            QFile file(fname);

            if(!file.open(QIODevice::ReadOnly)) {
                QMessageBox a(QMessageBox::Warning,"Error while opening","Cannot open file "+ fname,QMessageBox::Ok,this);
                a.exec();
                return;
            }


            // FILE IS OPEN AND READY FOR READING


            filedata=file.readAll();

            file.close();

            // THIS IS ONLY VALID FOR 50G AND COUSINS, FIX LATER

            if(strncmp(filedata.constData(),"KINPOUPDATEIMAGE",16)==0) {
                unsigned int address=0x4000;
                unsigned int nwords=filedata.size()>>2;

                filedata.replace(0,16,"Kinposhcopyright");
                filedata.insert(0,(const char *)(&nwords),4);
                filedata.insert(0,(const char *)(&address),4);
                filedata.insert(0,"FWUP",4);
            }
            else {
                QMessageBox a(QMessageBox::Warning,"Invalid firmware image","Invalid firmware image",QMessageBox::Ok,this);
                a.exec();
                return;
            }

            QMessageBox warn(QMessageBox::Warning,"Firmware update","Firmware on the remote device is about to be updated. Do NOT disconnect the device. OK to proceed?",QMessageBox::Yes | QMessageBox::No,this);

            if(warn.exec()==QMessageBox::No) return;

        } else return;






    // STOP REFRESHING THE LIST

    if(tmr) {
        tmr->stop();
        delete tmr;
        tmr=0;
    }

    // CONNECT TO THE USB DEVICE
    __usb_curdevice=hid_open_path(SelectedDevicePath.toUtf8().constData());

    if(!__usb_curdevice) {
        // TODO: ERROR PROCESS
        return;
    }

    // TODO: SHOW NICE WINDOW WITH UPDATE STEPS

    int j;
    for(j=0;j<500;++j) usbflush();

    // SEND CMD_USBFWUPDATE TO THE CALC
    if(!usbremotefwupdatestart()) {
        // TODO: SOME KIND OF ERROR
        return;
    }


    for(j=0;j<1000;++j) usbflush();

    int nwords=filedata.size()+3,result;
    nwords/=sizeof(WORD);

    result=usbsendtoremote((unsigned int *)filedata.constData(),nwords);


    for(j=0;j<500;++j) usbflush();

    // AT THIS POINT, THE CALC MUST'VE RESET TO LOAD THE NEW FIRMWARE
    hid_close(__usb_curdevice);

    __usb_curdevice=0;

    numberoftries=0;

    tmr = new QTimer(this);
    if(tmr) {
    connect(tmr, SIGNAL(timeout()), this, SLOT(reconnect()));
    tmr->start(500);
    }



    if(result!=1) {
        QMessageBox a(QMessageBox::Warning,"Communication error while sending firmware","USB communication error",QMessageBox::Ok,this);
        a.exec();
    }



    // AND JUST HOPE IT WILL RECONENCT SOME TIME
    return;

}

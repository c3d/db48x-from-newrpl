#include <QMessageBox>
#include <QTreeWidget>
#include <QTimer>
#include <QStandardPaths>
#include <QFileDialog>
#include <QCloseEvent>

#include "string.h"
#include "hidapi.h"
#include "usbselector.h"
#include "ui_usbselector.h"

extern "C" {
#include "newrpl.h"
#include "libraries.h"

extern hid_device *__usb_curdevice;
extern volatile int __usb_paused;
int __fwupdate_progress;
int __fwupdate_address;
int __fwupdate_nwords;
BYTEPTR __fwupdate_buffer;


BINT64 rplObjChecksum(WORDPTR object);
}





USBSelector::USBSelector(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::USBSelector),
    update_thread(this)
{
    ui->setupUi(this);


    SelectedDevicePath.clear();
    ui->updateFirmware->hide();
    ui->updateProgress->hide();

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


void USBSelector::closeEvent(QCloseEvent *event)
{
    if(!update_thread.isRunning()) event->accept();
    else event->ignore();
}

void USBSelector::reject()
{
    if(!update_thread.isRunning()) QDialog::reject();
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

                        // STOP THE DRIVER AND REINITIALIZE COMPLETELY
                        __usb_paused=1;
                        while(__usb_paused>=0);


                        thisdev=hid_open_path(cur_dev->path);

                        if(thisdev)
                        {
                            unsigned char buffer[1024];
                            int res;
                            int available=0;

                            // SET THE DRIVER TO USE THIS DEVICE AND START THE DRIVER
                            __usb_curdevice=thisdev;
                            __usb_timeout=200;      // SET TIMEOUT TO 200 ms FOR QUICK DETECTION

                            usb_init(1);        // FORCE REINITIALIZATION, CLOSE ANY PREVIOUS HANDLES IF THEY EXIST


                            __usb_paused=0;

                            do {


                             usb_sendcontrolpacket(P_TYPE_GETSTATUS);

                             tmr_t start,end;
                             // WAIT FOR THE CONTROL PACKET TO BE SENT
                             start=tmr_ticks();
                             res=0;
                             while(__usb_drvstatus&USB_STATUS_TXCTL) {

                                 if((__usb_drvstatus&(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED))!=(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED)) break;

                                 cpu_waitforinterrupt();
                                 end=tmr_ticks();
                                 if(tmr_ticks2ms(start,end)>__usb_timeout) {
                                 res=-1;
                                 break;
                                 }
                            }

                             if(res<0) break;

                             if(!usb_waitforreport()) { res=-1; break; }
                             // WAIT FOR A RESPONSE
                             USB_PACKET *pkt=usb_getreport();

                             if(P_FILEID(pkt)!=0) {
                                 usb_sendcontrolpacket(P_TYPE_ABORT);

                                 tmr_t start,end;


                                 // WAIT FOR THE CONTROL PACKET TO BE SENT
                                 start=tmr_ticks();
                                 res=0;
                                 while(__usb_drvstatus&USB_STATUS_TXCTL) {

                                     if((__usb_drvstatus&(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED))!=(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED)) break;

                                     cpu_waitforinterrupt();
                                     end=tmr_ticks();
                                     if(tmr_ticks2ms(start,end)>__usb_timeout) {
                                     res=-1;
                                     break;
                                     }
                                }
                                 continue;
                             }

                             usb_releasereport();

                             if(res<0) break;
                             // GOT AN ANSWER, MAKE SURE REMOTE IS READY TO RECEIVE
                             if(__usb_drvstatus&(USB_STATUS_HALT|USB_STATUS_ERROR)) { res=-1; break; }

                            // ATTEMPT TO SEND SOMETHING TO SEE IF IT'S ACTIVELY RESPONDING
                            uint32_t getversion[6]={
                                MKPROLOG(SECO,5),  // ACTUAL DATA
                                CMD_VERSION,
                                CMD_DROP,
                                CMD_USBSEND,
                                CMD_DROP,
                                CMD_QSEMI
                            };

                            int fileid;
                            res=fileid=usb_txfileopen('O');
                            if(!res) break;

                            res=usb_filewrite(fileid,(BYTEPTR)getversion,6*sizeof(uint32_t));

                            if(!res) break;

                            res=usb_txfileclose(fileid);

                            if(res<0) break;


                             // WAIT FOR THE FILE TO ARRIVE
                             start=tmr_ticks();
                             res=0;
                             while(!usb_hasdata()) {

                                 if((__usb_drvstatus&(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED))!=(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED)) break;

                                 cpu_waitforinterrupt();
                                 end=tmr_ticks();
                                 if(tmr_ticks2ms(start,end)>__usb_timeout) {
                                 res=-1;
                                 break;
                                 }
                            }
                            if(res<0) break;

                            res=fileid=usb_rxfileopen();

                            if(res<0) break;

                            res=usb_fileread(fileid,buffer,1024);

                            if(res<=0) break;

                            usb_rxfileclose(fileid);

                            {
                            unsigned int strprolog;
                            strprolog=buffer[0]+(buffer[1]<<8)+(buffer[2]<<16)+(buffer[3]<<24);

                            tmp=QString::fromUtf8((const char *)(buffer+4),rplStrSize(&strprolog));
                            newitem->setText(2,tmp);
                            available=1;
                            }


                            } while(!available);


                            __usb_paused=1;
                            while(__usb_paused>=0);
                            usb_shutdown();

                            __usb_timeout=5000;      // SET TIMEOUT TO THE DEFAULT 5000ms

                        //hid_close(thisdev);   // ALREADY CLOSED BY usb_shutdown()

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


    // STOP REFRESHING THE LIST

    if(tmr) {
        tmr->stop();
        delete tmr;
        tmr=0;
    }



        QString path;
        // THIS IS ONLY FOR 50g/40g/39g HARDWARE
        // TODO: IMPROVE ON THIS FOR OTHER HARDWARE PLATFORMS
        unsigned int address;
        unsigned int nwords;

        path=QStandardPaths::locate(QStandardPaths::DocumentsLocation,"newRPL",QStandardPaths::LocateDirectory);
        if(path.isEmpty()) path=QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

        QString fname=QFileDialog::getOpenFileName(this,"Select firmware file to send to calculator",path,"firmware binary files (*.bin *.* *)");

        if(!fname.isEmpty()) {
            QFile file(fname);

            if(!file.open(QIODevice::ReadOnly)) {
                QMessageBox a(QMessageBox::Warning,"Error while opening","Cannot open file "+ fname,QMessageBox::Ok,this);
                a.exec();


                // START REFRESHING THE LIST AGAIN
                tmr = new QTimer(this);
                if(tmr) {
                connect(tmr, SIGNAL(timeout()), this, SLOT(reconnect()));
                tmr->start(500);
                }


                return;
            }


            // FILE IS OPEN AND READY FOR READING


            filedata=file.readAll();

            file.close();

            // THIS IS ONLY VALID FOR 50G AND COUSINS, FIX LATER

            if((strncmp(filedata.constData(),"KINPOHP39G+IMAGE",16)==0)||
               (strncmp(filedata.constData(),"KINPOHP40G+IMAGE",16)==0)||
               (strncmp(filedata.constData(),"KINPOUPDATEIMAGE",16)==0)) {
                address=0x4000;
                nwords=filedata.size()>>2;

                filedata.replace(0,16,"Kinposhcopyright");
            }
            else {
                QMessageBox a(QMessageBox::Warning,"Invalid firmware image","Invalid firmware image",QMessageBox::Ok,this);
                a.exec();
                // START REFRESHING THE LIST AGAIN
                tmr = new QTimer(this);
                if(tmr) {
                connect(tmr, SIGNAL(timeout()), this, SLOT(reconnect()));
                tmr->start(500);
                }

                return;
            }

            QMessageBox warn(QMessageBox::Warning,"Firmware update","Firmware on the remote device is about to be updated. Do NOT disconnect the device. OK to proceed?",QMessageBox::Yes | QMessageBox::No,this);

            if(warn.exec()==QMessageBox::No) {
                // START REFRESHING THE LIST AGAIN
                tmr = new QTimer(this);
                if(tmr) {
                connect(tmr, SIGNAL(timeout()), this, SLOT(reconnect()));
                tmr->start(500);
                }

                return;
            }

        } else {

            // START REFRESHING THE LIST AGAIN
            tmr = new QTimer(this);
            if(tmr) {
            connect(tmr, SIGNAL(timeout()), this, SLOT(reconnect()));
            tmr->start(500);
            }

            return;
        }




        ui->USBtreeWidget->setEnabled(false);

        ui->updateFirmware->setEnabled(false);
        ui->updateProgress->setRange(0,nwords);
        ui->updateProgress->show();
        ui->updateProgress->setValue(0);
        ui->buttonBox->setEnabled(false);


        // CONNECT TO THE USB DEVICE
        __usb_curdevice=hid_open_path(SelectedDevicePath.toUtf8().constData());

        if(!__usb_curdevice) {
            // TODO: ERROR PROCESS
            // START REFRESHING THE LIST AGAIN
            tmr = new QTimer(this);
            if(tmr) {
            connect(tmr, SIGNAL(timeout()), this, SLOT(reconnect()));
            tmr->start(500);
            }

            return;
        }

   __fwupdate_progress=0;
   __fwupdate_address=address;
   __fwupdate_nwords=nwords;
   __fwupdate_buffer=(BYTEPTR)filedata.constData();

    connect(&update_thread,SIGNAL(finished()),this,SLOT(finishedupdate()));

    update_thread.start();

    while(!update_thread.isRunning());  // WAIT FOR THE THREAD TO START

    // START REPORTING PROGRESS
    QTimer::singleShot(0, this, SLOT(updateprogress()));


}


void USBSelector::finishedupdate()
{
    // PUT THE USB DRIVER TO REST
    __usb_paused=1;
    while(__usb_paused>=0) ;

    // AT THIS POINT, THE CALC MUST'VE RESET TO LOAD THE NEW FIRMWARE
    hid_close(__usb_curdevice);
    __usb_curdevice=0;


   int result=__fwupdate_address;

    if(!result) {
        QMessageBox a(QMessageBox::Warning,"Communication error while sending firmware","USB communication error",QMessageBox::Ok,this);
        a.exec();
    }


    ui->USBtreeWidget->clear();
    ui->USBtreeWidget->setEnabled(true);

    ui->updateFirmware->setEnabled(true);
    ui->updateProgress->hide();
    ui->updateProgress->setValue(0);
    ui->buttonBox->setEnabled(true);
    SelectedDevicePath.clear();


    numberoftries=0;

    // START REFRESHING THE LIST AGAIN
    tmr = new QTimer(this);
    if(tmr) {
    connect(tmr, SIGNAL(timeout()), this, SLOT(reconnect()));
    tmr->start(500);
    }


    // AND JUST HOPE IT WILL RECONENCT SOME TIME

}

void USBSelector::updateprogress()
{
    if(!update_thread.isRunning()) return;

    ui->updateProgress->setValue(__fwupdate_progress);

    QTimer::singleShot(0, this, SLOT(updateprogress()));
}



// ****************************************** USB DRIVER ON A SEPARATE THREAD

FWThread::FWThread(QObject *parent)
    : QThread(parent)
{
}

FWThread::~FWThread()
{
}


void FWThread::run()
{

    int nwords=__fwupdate_nwords;
    __fwupdate_progress=0;



    // START USB DRIVER
    __usb_paused=0;

    {
    // WAIT TWO FULL SECONDS BEFORE STARTING ANOTHER CONVERSATION WITH THE DEVICE
    tmr_t start,end;
    start=tmr_ticks();
    do end=tmr_ticks(); while(tmr_ticks2ms(start,end)<200);
    }

    // SEND CMD_USBFWUPDATE TO THE CALC
    if(!usbremotefwupdatestart()) {
        __fwupdate_address=0;
        return;
    }

    {
    // WAIT 500ms BEFORE STARTING ANOTHER CONVERSATION WITH THE DEVICE
    tmr_t start,end;
    start=tmr_ticks();
    do end=tmr_ticks(); while(tmr_ticks2ms(start,end)<500);
    }


    WORD header[3];
    int result=1,offset=0;
    int fileid;


    while(result && (nwords>1024)) {

    if(result) result=fileid=usb_txfileopen('W');
    if(!result) {
        // TODO: SOME KIND OF ERROR
        break;
    }

    // SEND FIRMWARE BLOCK MARKER

    header[0]=TEXT2WORD('F','W','U','P');
    header[1]=__fwupdate_address+(offset<<2);
    header[2]=1024;

    if(result && (!usb_filewrite(fileid,(BYTEPTR)header,3*sizeof(WORD)))) {
        // TODO: SOME KIND OF ERROR
        result=0;
        break;
    }

    if(result) {
    BYTEPTR buffer=__fwupdate_buffer+offset*sizeof(WORD);
    if(!usb_filewrite(fileid,buffer,1024*sizeof(WORD))) {
        result=0;
        break;
    }
    offset+=1024;
    }



    if(result && (!usb_txfileclose(fileid))) {
        // TODO: SOME KIND OF ERROR
       result=0;
       break;
    }

    nwords-=1024;

    __fwupdate_progress=offset;

    }

    if(result && nwords) {
        result=fileid=usb_txfileopen('W');

        // SEND FIRMWARE BLOCK MARKER

        header[0]=TEXT2WORD('F','W','U','P');
        header[1]=__fwupdate_address+(offset<<2);
        header[2]=nwords;

        if(result && (!usb_filewrite(fileid,(BYTEPTR)header,3*sizeof(WORD)))) {
            // TODO: SOME KIND OF ERROR
            result=0;
        }

        if(result) {
        BYTEPTR buffer=__fwupdate_buffer+offset*sizeof(WORD);
        if(!usb_filewrite(fileid,buffer,nwords*sizeof(WORD))) {
            result=0;
        }
        offset+=nwords;
        }

        if(result && (!usb_txfileclose(fileid))) {
            // TODO: SOME KIND OF ERROR
           result=0;
        }
        __fwupdate_progress=offset;

        }

    // DONE SENDING THE LAST BLOCK
    {
    // WAIT TWO FULL SECONDS BEFORE STARTING ANOTHER CONVERSATION WITH THE DEVICE
    tmr_t start,end;
    start=tmr_ticks();
    do end=tmr_ticks(); while(tmr_ticks2ms(start,end)<2000);
    }


    // NOW FINISH THE TEST BY RESETTING
    result=fileid=usb_txfileopen('W');

    header[0]=TEXT2WORD('F','W','U','P');
    header[1]=0xffffffff;
    header[2]=0;

    if(result && (!usb_filewrite(fileid,(BYTEPTR)header,3*sizeof(WORD)))) {
        // TODO: SOME KIND OF ERROR
       result=0;
    }

    if(result && (!usb_txfileclose(fileid))) {
        // TODO: SOME KIND OF ERROR
       result=0;
    }

    __fwupdate_address=result;

}




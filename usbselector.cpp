#include <QMessageBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QTimer>
#include <QStandardPaths>
#include <QFileDialog>
#include <QCloseEvent>

//#include "string.h"
#include "hidapi.h"
#include "usbselector.h"
#include "ui_usbselector.h"

// ONLY REQUIRED UNDER MINGW
#ifdef DrawText
#undef DrawText
#endif
#define WORD _WORD

extern "C"
{
#include "newrpl.h"
#include "libraries.h"

    extern hid_device *__usb_curdevice;
    extern char __usb_devicepath[8192];

    extern volatile int __usb_paused;
    int __fwupdate_progress;
    int __fwupdate_address;
    int __fwupdate_nwords;
    BYTEPTR __fwupdate_buffer;

    BINT64 rplObjChecksum(WORDPTR object);
}

USBSelector::USBSelector(QWidget * parent):
QDialog(parent), update_thread(this), ui(new Ui::USBSelector)
{
    ui->setupUi(this);

    SelectedDevicePath.clear();
    SelectedDeviceName.clear();
    ui->updateFirmware->hide();
    ui->updateProgress->hide();
    ui->USBtreeWidget->clear();
    ui->USBtreeWidget->hideColumn(1);
    ui->USBtreeWidget->hideColumn(3);
    ui->USBtreeWidget->hideColumn(4);

    norefresh = false;

    connect(&update_thread, SIGNAL(FirmwareUpdateError(QString)), this,
            SLOT(on_Error(QString)));

    QTimer::singleShot(200, this, SLOT(refresh()));

}

void USBSelector::on_Error(QString message)
{
    QString info =
            "\nIf the device doesn't react anymore, please use the standard HP firmware update procedure to flash the newRPL firmware again.";
    QMessageBox warn(QMessageBox::Warning,
            "Communication error while sending firmware", message + info,
            QMessageBox::Ok, this);
    warn.exec();
}

USBSelector::~USBSelector()
{

    delete ui;
}

void USBSelector::closeEvent(QCloseEvent * event)
{
    if(!update_thread.isRunning())
        event->accept();
    else
        event->ignore();
}

void USBSelector::reject()
{
    if(!update_thread.isRunning())
        QDialog::reject();
}

void USBSelector::on_USBtreeWidget_itemSelectionChanged()
{

    QString result;

    result.clear();
    QTreeWidgetItem *newitem;

    if(ui->USBtreeWidget->selectedItems().count() >= 1)
        newitem = ui->USBtreeWidget->selectedItems().first();
    else {
        return;
    }

    if(newitem->text(2) == QString("[Device not responding]")) {
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
        SelectedDevicePath.clear();
        SelectedDeviceName.clear();
        ui->selectedCalc->setText(QString("No device selected."));
        ui->updateFirmware->hide();

    }
    else {
        ui->buttonBox->
                setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::
                Cancel);
        SelectedDevicePath = newitem->text(3);
        SelectedDeviceName =
                newitem->text(0) + QString("[build ") +
                newitem->text(2).right(4) + QString("]");
        ui->selectedCalc->setText(SelectedDeviceName);
        ui->updateFirmware->show();

    }

}

QString & USBSelector::getSelectedDevicePath()
{
    return SelectedDevicePath;
}

QString & USBSelector::getSelectedDeviceName()
{
    return SelectedDeviceName;
}

void USBSelector::RefreshList()
{
    struct hid_device_info *devs, *cur_dev;
    QTreeWidgetItem *newitem;

    // MAKE SURE WE CLOSE EVERYTHING AND RESET THE ENTIRE LIBRARY BEFORE ENUMERATION
    usb_shutdown();

    if(hid_init()) {
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

    QString manuf;
    QString tmp;
    QString pid;
    QString newpath;

    while(cur_dev) {

        if(cur_dev->manufacturer_string)
            manuf = QString::fromStdWString(cur_dev->manufacturer_string);
        else
            manuf.clear();

        manuf.detach();

        if(manuf.startsWith("newRPL")) {

            pid = QString::number(cur_dev->vendor_id,
                    16) + ":" + QString::number(cur_dev->product_id, 16);
            newpath = QString(cur_dev->path);
            newpath.detach();

            int nitems = ui->USBtreeWidget->topLevelItemCount();
            int k;
            QTreeWidgetItem *item;

            newitem = 0;
            for(k = 0; k < nitems; ++k) {
                item = ui->USBtreeWidget->topLevelItem(k);
                if(item) {
                    if((item->text(4) == pid) && (item->text(3) == newpath)) {
                        // FOUND THE SAME ITEM AGAIN
                        newitem = item;
                        item->setDisabled(false);
                        break;
                    }
                }
            }

            if(!newitem) {
                newitem = new QTreeWidgetItem();
                if(!newitem)
                    return;

                newitem->setText(0, "Empty");
                newitem->setText(1, "Empty");
                newitem->setText(2, "Empty");
                newitem->setText(3, "Empty");
                newitem->setText(4, "Empty");

                ui->USBtreeWidget->addTopLevelItem(newitem);

            }

            if(cur_dev->product_string)
                tmp = QString::fromStdWString(cur_dev->product_string);
            else
                tmp = "[Unknown]";
            if(cur_dev->serial_number)
                tmp += QString("|SN=") +
                        QString::fromStdWString(cur_dev->serial_number);

            tmp.detach();
            newitem->setText(0, tmp);

            if(cur_dev->manufacturer_string)
                tmp = QString::fromStdWString(cur_dev->manufacturer_string);
            else
                tmp = "[Unknown]";

            tmp.detach();
            newitem->setText(1, tmp);

            tmp = QString::number(cur_dev->vendor_id,
                    16) + ":" + QString::number(cur_dev->product_id, 16);

            tmp.detach();
            newitem->setText(4, tmp);

            newitem->setText(3, newpath);
        }

        cur_dev = cur_dev->next;
    }

    hid_free_enumeration(devs);

    // NOW ELIMINATE ANY ITEMS THAT ARE NOT ENABLED
    {
        int restart = 1;

        while(restart) {
            int nitems = ui->USBtreeWidget->topLevelItemCount();
            if(nitems == 0)
                break;

            int k;
            QTreeWidgetItem *item;
            for(k = 0; k < nitems; ++k) {
                item = ui->USBtreeWidget->topLevelItem(k);

                if(item) {
                    if(item->isDisabled()) {
                        if(SelectedDevicePath == item->text(3)) {

                            ui->buttonBox->
                                    setStandardButtons(QDialogButtonBox::
                                    Cancel);
                            SelectedDevicePath.clear();
                            ui->selectedCalc->
                                    setText(QString("No device selected."));
                            ui->USBtreeWidget->clearSelection();
                            ui->updateFirmware->hide();
                        }
                        QTreeWidgetItem *pparent = item->parent();
                        if(pparent)
                            pparent->removeChild(item);
                        delete item;
                        restart = 1;
                        break;
                    }

                    // THE DEVICE IS ACTIVE

                    tmp = item->text(3);
                    tmp.detach();

                    // STOP THE DRIVER AND REINITIALIZE COMPLETELY
                    __usb_paused = 1;
                    while(__usb_paused >= 0);

                    usb_shutdown();
                    // SET THE DRIVER TO USE THIS DEVICE AND START THE DRIVER
                    if(safe_stringcpy(__usb_devicepath, 8192,
                                tmp.toUtf8().constData()))
                        __usb_devicepath[0] = 0;
                    __usb_timeout = 200;        // SET TIMEOUT TO 200 ms FOR QUICK DETECTION
                    usb_init(0);        // FORCE REINITIALIZATION, CLOSE ANY PREVIOUS HANDLES IF THEY EXIST

                    if(usb_isconnected()) {
                        unsigned char buffer[1024];
                        int res;
                        int available = 0;

                        __usb_paused = 0;

                        do {

                            usb_sendcontrolpacket(P_TYPE_GETSTATUS);

                            tmr_t start, end;
                            // WAIT FOR THE CONTROL PACKET TO BE SENT
                            start = tmr_ticks();
                            res = 1;
                            while(__usb_drvstatus & USB_STATUS_TXCTL) {

                                if((__usb_drvstatus & (USB_STATUS_CONFIGURED |
                                                USB_STATUS_INIT |
                                                USB_STATUS_CONNECTED)) !=
                                        (USB_STATUS_CONFIGURED | USB_STATUS_INIT
                                            | USB_STATUS_CONNECTED))
                                    break;

                                QThread::yieldCurrentThread();

                                end = tmr_ticks();
                                if(tmr_ticks2ms(start, end) > __usb_timeout) {
                                    res = 0;
                                    break;
                                }
                            }

                            if(!res)
                                break;

                            if(!usb_waitforreport()) {
                                res = 0;
                                break;
                            }
                            // WAIT FOR A RESPONSE
                            USB_PACKET *pkt = usb_getreport();

                            if(P_FILEID(pkt) != 0) {

                                // REQUEST UNCONDITIONAL ABORT
                                __usb_fileid = 0xffff;
                                usb_sendcontrolpacket(P_TYPE_ABORT);
                                __usb_fileid = 0;

                                tmr_t start, end;

                                // WAIT FOR THE CONTROL PACKET TO BE SENT
                                start = tmr_ticks();

                                while(__usb_drvstatus & USB_STATUS_TXCTL) {

                                    if((__usb_drvstatus & (USB_STATUS_CONFIGURED
                                                    | USB_STATUS_INIT |
                                                    USB_STATUS_CONNECTED)) !=
                                            (USB_STATUS_CONFIGURED |
                                                USB_STATUS_INIT |
                                                USB_STATUS_CONNECTED))
                                        break;

                                    QThread::yieldCurrentThread();
                                    end = tmr_ticks();
                                    if(tmr_ticks2ms(start, end) > __usb_timeout) {
                                        res = 0;
                                        break;
                                    }
                                }
                                if(!res) break;
                                continue;
                            }

                            usb_releasereport();

                            if(!res)
                                break;
                            // GOT AN ANSWER, MAKE SURE REMOTE IS READY TO RECEIVE
                            if(__usb_drvstatus & (USB_STATUS_HALT |
                                        USB_STATUS_ERROR)) {
                                res = 0;
                                break;
                            }

                            // ATTEMPT TO SEND SOMETHING TO SEE IF IT'S ACTIVELY RESPONDING
                            uint32_t getversion[6] = {
                                MKPROLOG(SECO, 5),      // ACTUAL DATA
                                CMD_VERSION,
                                CMD_DROP,
                                CMD_USBSEND,
                                CMD_DROP,
                                CMD_QSEMI
                            };

                            int fileid;
                            res = fileid = usb_txfileopen('O');
                            if(!res)
                                break;

                            res = usb_filewrite(fileid, (BYTEPTR) getversion,
                                    6 * sizeof(uint32_t));

                            if(!res)
                                break;

                            res = usb_txfileclose(fileid);

                            if(!res)
                                break;

                            // WAIT FOR THE FILE TO ARRIVE
                            start = tmr_ticks();
                            res = 1;
                            while(!usb_hasdata()) {

                                if((__usb_drvstatus & (USB_STATUS_CONFIGURED |
                                                USB_STATUS_INIT |
                                                USB_STATUS_CONNECTED)) !=
                                        (USB_STATUS_CONFIGURED | USB_STATUS_INIT
                                            | USB_STATUS_CONNECTED))
                                    break;

                                QThread::yieldCurrentThread();
                                end = tmr_ticks();
                                if(tmr_ticks2ms(start, end) > __usb_timeout) {
                                    res = 0;
                                    break;
                                }
                            }
                            if(!res)
                                break;

                            res = fileid = usb_rxfileopen();

                            if(!res)
                                break;

                            res = usb_fileread(fileid, buffer, 1024);

                            if(!res)
                                break;

                            usb_rxfileclose(fileid);

                            {
                                unsigned int strprolog;
                                strprolog =
                                        buffer[0] + (buffer[1] << 8) +
                                        (buffer[2] << 16) + (buffer[3] << 24);
                                int length = rplStrSize(&strprolog);
                                tmp = QString::fromUtf8((char *)(buffer + 4),
                                        length);
                                tmp.detach();
                                available = 1;
                            }

                        }
                        while(!available);

                        __usb_paused = 1;
                        while(__usb_paused >= 0);
                        usb_shutdown();
                        __usb_curdevice = 0;
                        __usb_timeout = 5000;   // SET TIMEOUT TO THE DEFAULT 5000ms

                        if(!available) {
                            tmp = "[Device not responding]";
                        }
                        item->setText(2, tmp);

                    }

                    restart = 0;

                }

            }
        }

        __usb_timeout = 5000;   // MAKE SURE WE LEAVE THE TIMEOUT TO THE DEFAULT VALUE

    }

    ui->USBtreeWidget->resizeColumnToContents(0);
    // DONE, THE LIST WAS REFRESHED

}

void USBSelector::on_USBSelector_accepted()
{

}

void USBSelector::on_USBSelector_rejected()
{

}

void USBSelector::refresh()
{
    if(norefresh)
        return;
    RefreshList();
    QTimer::singleShot(500, this, SLOT(refresh()));

}

extern "C" int usbremotefwupdatestart();
extern "C" int usbsendtoremote(uint32_t * data, int nwords);

void USBSelector::on_updateFirmware_clicked()
{

    // STOP REFRESHING THE LIST
    norefresh = true;

    QString path;
    // THIS IS ONLY FOR 50g/40g/39g HARDWARE
    // TODO: IMPROVE ON THIS FOR OTHER HARDWARE PLATFORMS
    unsigned int address;
    unsigned int nwords;

    path = QStandardPaths::locate(QStandardPaths::DocumentsLocation, "newRPL",
            QStandardPaths::LocateDirectory);
    if(path.isEmpty())
        path = QStandardPaths::writableLocation(QStandardPaths::
                DocumentsLocation);

    QString fname =
            QFileDialog::getOpenFileName(this,
            "Select firmware file to send to calculator", path,
            "firmware binary files (*.bin *.* *)");

    if(!fname.isEmpty()) {
        QFile file(fname);

        if(!file.open(QIODevice::ReadOnly)) {
            QMessageBox a(QMessageBox::Warning, "Error while opening",
                    "Cannot open file " + fname, QMessageBox::Ok, this);
            a.exec();

            norefresh = false;
            QTimer::singleShot(500, this, SLOT(refresh()));

            return;
        }

        // FILE IS OPEN AND READY FOR READING

        filedata = file.readAll();

        file.close();

        // THIS IS ONLY VALID FOR 50G AND COUSINS, FIX LATER

        if((strncmp(filedata.constData(), "KINPOHP39G+IMAGE", 16) == 0) ||
                (strncmp(filedata.constData(), "KINPOHP40G+IMAGE", 16) == 0) ||
                (strncmp(filedata.constData(), "KINPOUPDATEIMAGE", 16) == 0)) {
            address = 0x4000;
            nwords = filedata.size() >> 2;

            filedata.replace(0, 16, "Kinposhcopyright");
        }
        else {
            QMessageBox a(QMessageBox::Warning, "Invalid firmware image",
                    "Invalid firmware image", QMessageBox::Ok, this);
            a.exec();
            // START REFRESHING THE LIST AGAIN
            norefresh = false;
            QTimer::singleShot(500, this, SLOT(refresh()));
            return;
        }

        QMessageBox warn(QMessageBox::Warning, "Firmware update",
                "Firmware on the remote device is about to be updated. Do NOT disconnect the device. OK to proceed?",
                QMessageBox::Yes | QMessageBox::No, this);

        if(warn.exec() == QMessageBox::No) {
            // START REFRESHING THE LIST AGAIN
            norefresh = false;
            QTimer::singleShot(500, this, SLOT(refresh()));
            return;
        }

    }
    else {

        // START REFRESHING THE LIST AGAIN
        norefresh = false;
        QTimer::singleShot(500, this, SLOT(refresh()));

        return;
    }

    ui->USBtreeWidget->setEnabled(false);

    ui->updateFirmware->setEnabled(false);
    ui->updateProgress->setRange(0, nwords);
    ui->updateProgress->show();
    ui->updateProgress->setValue(0);
    ui->buttonBox->setEnabled(false);

    // CONNECT TO THE USB DEVICE
    __usb_paused = 1;
    while(__usb_paused >= 0);
    usb_shutdown();
    if(safe_stringcpy(__usb_devicepath, 8192,
                SelectedDevicePath.toUtf8().constData()))
        __usb_devicepath[0] = 0;
    usb_init(0);

    __fwupdate_progress = 0;

    if(!usb_isconnected()) {
        // TODO: ERROR PROCESS
        // START REFRESHING THE LIST AGAIN
        finishedupdate();

        return;
    }

    __fwupdate_progress = 0;
    __fwupdate_address = address;
    __fwupdate_nwords = nwords;
    __fwupdate_buffer = (BYTEPTR) filedata.constData();

    connect(&update_thread, SIGNAL(finished()), this, SLOT(finishedupdate()));

    update_thread.start();

    while(!update_thread.isRunning());  // WAIT FOR THE THREAD TO START

    // START REPORTING PROGRESS
    QTimer::singleShot(0, this, SLOT(updateprogress()));

}

void USBSelector::finishedupdate()
{

    // PUT THE USB DRIVER TO REST
    __usb_paused = 1;
    while(__usb_paused >= 0);

    usb_shutdown();

    //ui->USBtreeWidget->clear();
    ui->USBtreeWidget->setEnabled(true);

    ui->updateFirmware->setEnabled(true);
    ui->updateProgress->hide();
    ui->updateProgress->setValue(0);
    ui->buttonBox->setEnabled(true);

    numberoftries = 0;

    norefresh = false;

    // AND JUST HOPE IT WILL RECONENCT SOME TIME
    // START REFRESHING THE LIST AGAIN
    QTimer::singleShot(0, this, SLOT(refresh()));

}

void USBSelector::updateprogress()
{
    if(!update_thread.isRunning())
        return;

    ui->updateProgress->setValue(__fwupdate_progress);

    QTimer::singleShot(0, this, SLOT(updateprogress()));
}

// ****************************************** USB DRIVER ON A SEPARATE THREAD

FWThread::FWThread(QObject * parent)
:      QThread(parent)
{
}

FWThread::~FWThread()
{
}

static void busywait(int ms)
{
    tmr_t start, end;
    start = tmr_ticks();
    do {
        end = tmr_ticks();
    }
    while(tmr_ticks2ms(start, end) < ms);
}

static QString send_package(int nwords, int offset)
{
    WORD header[3];
    int fileid;

    fileid=usb_txfileopen('W');
    if(!fileid) {
        return "Could not initiate package transmission";
    }

    // SEND FIRMWARE BLOCK MARKER

    header[0]=TEXT2WORD('F','W','U','P');
    header[1]=__fwupdate_address+(offset<<2);
    header[2]=nwords;

    if(!usb_filewrite(fileid,(BYTEPTR)header,3*sizeof(WORD))) {
        return "Could not send data to usb device";
    }


    BYTEPTR buffer=__fwupdate_buffer+offset*sizeof(WORD);
    if(!usb_filewrite(fileid,buffer,nwords*sizeof(WORD))) {
        return "Could not send data to usb device";
    }

    if(!usb_txfileclose(fileid)) {
       return "Could not finalize package transmission";
    }
    return NULL;
}

static QString send_reset()
{
    WORD header[3];
    int fileid;

    fileid=usb_txfileopen('W');
    if(!fileid) {
        return "Could not initiate reset";
    }

    header[0]=TEXT2WORD('F','W','U','P');
    header[1]=0xffffffff;
    header[2]=0;

    if(!usb_filewrite(fileid,(BYTEPTR)header,3*sizeof(WORD))) {
        return "Could not initiate reset";
    }

    if(!usb_txfileclose(fileid)) {
        return "Could not initiate reset";
    }

    return NULL;
}

void FWThread::run()
{
    int nwords = __fwupdate_nwords;
    __fwupdate_progress = 0;

    // START USB DRIVER
    __usb_paused = 0;

    // WAIT 200ms BEFORE STARTING ANOTHER CONVERSATION WITH THE DEVICE
    busywait(200);
    // SEND CMD_USBFWUPDATE TO THE CALC
    if(!usbremotefwupdatestart()) {
        emit FirmwareUpdateError("Could not send firmware update command");
        return;
    }

    // WAIT 500ms BEFORE STARTING ANOTHER CONVERSATION WITH THE DEVICE
    busywait(500);

    while(nwords > 0) {
        int size = std::min(1024, nwords);
        QString error = send_package(size, __fwupdate_progress);
        if (error != NULL) {
            emit FirmwareUpdateError(error);
            // still going to reset the device...
            break;
        }
        nwords-=size;
        __fwupdate_progress+=size;
    }

    // WAIT TWO FULL SECONDS BEFORE STARTING ANOTHER CONVERSATION WITH THE DEVICE
    busywait(2000);

    QString error = send_reset();
    if (error != NULL) {
        emit FirmwareUpdateError(error);
    }
}

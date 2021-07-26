#include "maindialog.h"
#include "ui_maindialog.h"
#include <QStandardPaths>
#include <QFileDialog>
#include <QFileInfo>
#include <QByteArrayList>
#include <QByteArrayListIterator>

#include "sddriver.h"
extern "C" {
#include "fsystem/fsystem.h"
#include "thirdparty/md5.h"
}

extern volatile int __sd_inserted;
extern volatile int __sd_nsectors;     // TOTAL SIZE OF SD CARD IN 512-BYTE SECTORS
extern volatile int __sd_RCA;
extern volatile unsigned char *__sd_buffer;    // BUFFER WITH THE ENTIRE CONTENTS OF THE SD CARD
unsigned char *__prime_app_buffer;

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainDialog)
{
    ui->setupUi(this);
    ui->firmwareFolder->setText(getPrimeG1FolderLocation());
    ui->newrplFile->setText(getDocumentsLocation()+"/newrplfwG1.bin");
}

MainDialog::~MainDialog()
{
    delete ui;
}

QString MainDialog::getDocumentsLocation()
{
    QString path;

             path = QStandardPaths::writableLocation(QStandardPaths::
                    DocumentsLocation);
    return path;
}

QString MainDialog::getPrimeG1FolderLocation()
{
    QString path;

        path = QStandardPaths::locate(QStandardPaths::DocumentsLocation,
                "HP Connectivity Kit", QStandardPaths::LocateDirectory);
        if(path.isEmpty())
            path = QStandardPaths::writableLocation(QStandardPaths::
                    DocumentsLocation);
        else path += "/Firmware/PrimeG1";
    return path;
}

void MainDialog::on_folderBrowseButton_clicked()
{
    QString path = ui->firmwareFolder->text();

    QFileInfo f(path);

    if(!f.exists()) path=getDocumentsLocation();

    QString fname = QFileDialog::getExistingDirectory(this, "Select Folder with Firmware", path,
            QFileDialog::ShowDirsOnly);

    ui->firmwareFolder->setText(fname);
}



void MainDialog::on_firmwareBrowseButton_clicked()
{
    QString path = ui->newrplFile->text();

    QFileInfo f(path);

    if(!f.exists()) path=getDocumentsLocation();
    else path=f.path();

    QString fname = QFileDialog::getOpenFileName(this, "Select file with newRPL latest Firmware", path,"Firmware images (*.bin *.* *)");

    ui->newrplFile->setText(fname);
}


void MainDialog::printLine(const QString line)
{
ui->logEdit->append(line);
}




void MainDialog::on_proceedButton_clicked()
{
    bool error=false;

printLine("Starting installation...");

// Verify given folder exists
if(!error) {
QFileInfo f(ui->firmwareFolder->text());
if(f.exists()) printLine("Firmware folder exists");
else { printLine("ERROR: Firmware folder does not exist"); error=true; }
}

// Verify given folder contains PRIME_OS.ROM
if(!error) {
QFileInfo f(ui->firmwareFolder->text()+"/PRIME_OS.ROM");
if(f.exists()) printLine("PRIME_OS.ROM exists");
else { printLine("ERROR: PRIME_OS.ROM does not exist"); error=true; }
}

// Verify given folder contains PRIME_APP.DAT
if(!error) {
QFileInfo f(ui->firmwareFolder->text()+"/PRIME_APP.DAT");
if(f.exists()) printLine("PRIME_APP.DAT exists");
else { printLine("ERROR: PRIME_APP.DAT does not exist"); error=true; }
}

// Verify given folder contains Prime_FW.md5
if(!error) {
QFileInfo f(ui->firmwareFolder->text()+"/Prime_FW.md5");
if(f.exists()) printLine("Prime_FW.md5 exists");
else { printLine("ERROR: Prime_FW.md5 does not exist"); error=true; }
}

QByteArray newrplFW;


// Verify given newRPL firmware file exists and is valid
if(!error) {
    QFile file(ui->newrplFile->text());

    if(!file.open(QIODevice::ReadOnly)) {
        printLine("ERROR: Cannot open file " + ui->newrplFile->text());
        error=true;
    }
    else {
     // FILE IS OPEN AND READY FOR READING

    newrplFW = file.readAll();

    file.close();

    if(newrplFW.mid(24,4).compare("2416")!=0) {
        printLine("ERROR: Not a valid newRPL firmware image");
        error=true;
    }
    }
}

if(!error) {
// Read FAT partition to RAM
QFile sdcard(ui->firmwareFolder->text()+"/PRIME_APP.DAT");

    if(!sdcard.open(QIODevice::ReadOnly)) {
        printLine( "ERROR: Cannot open file " + sdcard.fileName());
        error=true;
    }

    __sd_inserted = 0;
    __sd_RCA = 0;
    __sd_nsectors = 0;

    // FILE IS OPEN AND READY FOR READING
    __prime_app_buffer = (unsigned char *)malloc(sdcard.size());
    if(__prime_app_buffer == NULL) {
        printLine( "ERROR: Cannot allocate enough memory");
        error=true;
    }

    if(sdcard.read((char *)__prime_app_buffer, sdcard.size()) != sdcard.size()) {
       printLine("ERROR: Can't read FAT partition image");
       free(__prime_app_buffer);
       error=true;
    }

    __sd_buffer = __prime_app_buffer + 8192;  // FAT partition starts at an offset within the file
    __sd_nsectors = (sdcard.size()-8192) / 512;
    __sd_inserted = 1;
    sdcard.close();

    printLine("FAT Partition read OK");

}

// Mount FAT partition

if(!error) {
int freespace = FSGetVolumeFree(0);

if(!FSVolumeMounted(0)) {
    printLine("ERROR: Failed to mount FAT partition");
    error=true;
}
else {
    printLine(QString("FAT Partition mounted OK - Free space = ") + QString::number(freespace*512/1024) + " kbytes.");
}

}

// Copy PRIME_OS.ROM into FAT partition
if(!error) {
 QFile f(ui->firmwareFolder->text()+"/PRIME_OS.ROM");
 QByteArray primeos;

 if(!f.open(QIODevice::ReadOnly)) {
     printLine("ERROR: Unable to open PRIME_OS.ROM");
     error=true;
 }
 else {
     primeos = f.readAll();
     f.close();

     FS_FILE *fatfile;
     if(FSOpen("PRIME_OS.ROM",FSMODE_WRITE,&fatfile)!=FS_OK) {
         printLine("Cannot write inside FAT partition");
         error=true;
     }
     else {
         if(FSWrite((unsigned char *)primeos.constData(),primeos.length(),fatfile)!=primeos.length()) {
             FSClose(fatfile);
             printLine("Cannot write inside FAT partition");
             error=true;
         }
         else {
             printLine("PRIME_OS.ROM moved into FAT partition OK");
         }
     }
 }


}
// Add newRPL firmware into FAT partition
if(!error) {

     FS_FILE *fatfile;
     if(FSOpen("NEWRPL.ROM",FSMODE_WRITE,&fatfile)!=FS_OK) {
         printLine("Cannot write inside FAT partition");
         error=true;
     }
     else {
         if(FSWrite((unsigned char *)newrplFW.constData(),newrplFW.length(),fatfile)!=newrplFW.length()) {
             FSClose(fatfile);
             printLine("Cannot write inside FAT partition");
             error=true;
         }
         else {
             printLine("NEWRPL.ROM created inside FAT partition OK");
         }
     }
}

// Close FAT partition and save
FSShutdown();

// Save PRIME_APP.DAT back to disk
if(!error) {
 QFile f(ui->firmwareFolder->text()+"/PRIME_APP.DAT");

 if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
     printLine("ERROR: Unable to open PRIME_APP.DAT for writing");
     error=true;
 }
 else {
     if(f.write((const char *)__prime_app_buffer,__sd_nsectors*512+8192)!=__sd_nsectors*512+8192) {
         printLine("ERROR: Unable to save FAT partition");
         f.close();
         error=true;
     }
     else {
         f.close();
         printLine("FAT partition saved correctly");
     }
}

}



// Setup Multiboot
if(!error) {
 QFile f(ui->firmwareFolder->text()+"/PRIME_OS.ROM");
 QFile mbfile(":/bin/binaries/MultiBoot.bin");
 QByteArray multiboot;

 if(!mbfile.open(QIODevice::ReadOnly)) {
     printLine("ERROR: Unable to open resource file for MultiBoot");
     error=true;
 }
 else {
     multiboot = mbfile.readAll();
     mbfile.close();
 }
if(!error) {
 if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
     printLine("ERROR: Unable to open PRIME_OS.ROM for writing");
     error=true;
 }
 else {
     if(f.write(multiboot)!=multiboot.length()) {
         printLine("ERROR: Unable to setup Multiboot");
         f.close();
         error=true;
     }
     else {
         f.close();
         printLine("MultiBoot installed correctly");
     }
}

}
}

// Recompute all MD5 values and update the list file
if(!error) {
    QFile file(ui->firmwareFolder->text()+"/Prime_FW.md5");
    QByteArrayList md5Data;

    if(!file.open(QIODevice::ReadOnly)) {
        printLine("ERROR: Cannot open file " + file.fileName());
        error=true;
    }
    else {
     // FILE IS OPEN AND READY FOR READING

    QByteArray tmp;
    do {
        tmp=file.readLine();
        if(tmp.length()>0) md5Data.append(tmp);
    } while(tmp.length()>0);
    file.close();

    // We have all MD5 values in a list
    // We need to replace the recomputed values for PRIME_OS.ROM and PRIME_APP.DAT

    // Recompute both MD5 values
    MD5_CTX context;
    unsigned char digest_primeapp[16],digest_primeos[16];
    MD5Init(&context);

    MD5Update(&context,__prime_app_buffer,__sd_nsectors*512+8192);

    MD5Final(digest_primeapp,&context);

    MD5Init(&context);
    MD5Update(&context,(const unsigned char *)newrplFW.constData(),newrplFW.length());
    MD5Final(digest_primeos,&context);

    if(!file.open(QIODevice::WriteOnly)) {
        printLine("ERROR: Cannot open file for writing " + file.fileName());
        error=true;
    }
    else {
     // FILE IS OPEN AND READY FOR WRITING

     QByteArrayListIterator it(md5Data);

    while(it.hasNext()) {
        QString line(it.next());

        if(line.contains("PRIME_OS.ROM")) {
            // Replace the MD5 sum
            int k;
            for(k=0;k<16;++k) {
            QString bytehex;
            bytehex=QString::asprintf("%02x",digest_primeos[k]);
            line.replace(k*2,2,bytehex);
            }

        }
        if(line.contains("PRIME_APP.DAT")) {
            // Replace the MD5 sum
            int k;
            for(k=0;k<16;++k) {
            QString bytehex;
            bytehex=QString::asprintf("%02x",digest_primeapp[k]);
            line.replace(k*2,2,bytehex);
            }

        }
        file.write(line.toUtf8());
    }

    file.close();

    printLine("MD5 sums updated successfully");

    }



}

}

if(!error) {
printLine("Installation **SUCCESSFUL**");
printLine("Please update calculator firmware using the connectivity kit");
printLine("");
printLine("All tasks completed.");
return;
}
printLine("Installation **FAILED**, see errors above");
}

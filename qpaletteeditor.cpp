#include "mainwindow.h"
#include "qpaletteeditor.h"
#include "ui_qpaletteeditor.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>


extern "C" {
#include "xgl.h"
}

extern "C" int lcd_needsupdate;
extern void FullScreenUpdate();
extern int SaveRPLObject(QString & filename, int level);
extern int LoadRPLObject(QString & filename);
extern int SaveColorTheme(QString & filename);
extern int LoadColorTheme(QString & filename);
extern volatile int cpu_idle;


const char *pal_descriptions[]={
    "Gray Conversion - 0",
    "Gray Conversion - 1",
    "Gray Conversion - 2",
    "Gray Conversion - 3",
    "Gray Conversion - 4",
    "Gray Conversion - 5",
    "Gray Conversion - 6",
    "Gray Conversion - 7",
    "Gray Conversion - 8",
    "Gray Conversion - 9",
    "Gray Conversion - 10",
    "Gray Conversion - 11",
    "Gray Conversion - 12",
    "Gray Conversion - 13",
    "Gray Conversion - 14",
    "Gray Conversion - 15",

    "Stack: Background",
    "Stack: Index numbers",
    "Stack: Vertical dividing line",
    "Stack: Index numbers background",
    "Stack: Items text",
    "Stack: Selected item background",
    "Stack: Selected items text",
    "Stack: Interactive stack cursor",

    "Command Line: Background",
    "Command Line: Text",
    "Command Line: Selection background",
    "Command Line: Selection text",
    "Command Line: Cursor background",
    "Command Line: Cursor text",
    "Command Line: Horizontal dividing line",

    "Menu: Background",
    "Menu: Inverted menu background",
    "Menu: Text",
    "Menu: Inverted menu text",
    "Menu: Directory mark",
    "Menu: Directory mark for inverted menu",
    "Menu: Horizontal dividing line",
    "Menu: Focused horizontal dividing line",
    "Menu: Pressed item background",
    "Menu: Pressed item background for inverted menu",

    "Status Area: Background",
    "Status Area: Text",
    "Status Area: Annunciators - press+hold",
    "Status Area: Annunciators",
    "Status Area: Battery level",
    "Status Area: User flags - cleared flags",
    "Status Area: User flags - set flags",

    "Help Area: Background",
    "Help Area: Text",
    "Help Area: Decoration lines",

    "Forms: Background",
    "Forms: Text",
    "Forms: Selection text",
    "Forms: Selection background",
    "Forms: Current item cursor",

    "Reserved: Future use",
    "Reserved: Future use",
    "Reserved: Future use",
    "Reserved: Future use",
    "Reserved: Future use",
    "Reserved: Future use",
    "Reserved: Future use",
    "Reserved: Future use"
};

QPaletteEditor::QPaletteEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QPaletteEditor)
{
    ui->setupUi(this);
    ui->pTable->horizontalHeader()->resizeSection(0,40);
    ui->pTable->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);

    int k;
    for(k=0;k<64;++k)
    {
        ui->pTable->insertRow(k);
        QTableWidgetItem *item=new QTableWidgetItem("     ");
        QBrush tempbkgnd(QColor::fromRgb(RGBRED(cgl_palette[k]),RGBGREEN(cgl_palette[k]),RGBBLUE(cgl_palette[k])));
        item->setBackground(tempbkgnd);
        ui->pTable->setItem(k,0,item);
        ui->pTable->setItem(k,1,new QTableWidgetItem(QString(pal_descriptions[k])));

    }


}

QPaletteEditor::~QPaletteEditor()
{
    delete ui;
}

void QPaletteEditor::on_QPaletteEditor_accepted()
{

}

void QPaletteEditor::on_pTable_cellDoubleClicked(int row, int column)
{
QColor newcolor = QColorDialog::getColor(ui->pTable->item(row,0)->background().color());
if(newcolor.isValid()) {
    QTableWidgetItem *item=ui->pTable->takeItem(row,0);
    QBrush tempbkgnd(newcolor);
    item->setBackground(tempbkgnd);
    ui->pTable->setItem(row,0,item);
}

}

void QPaletteEditor::on_buttonBox_clicked(QAbstractButton *button)
{
    if(button->text()=="Apply") {
        int k;
        for(k=0;k<64;++k)
        {
            QColor color=ui->pTable->item(k,0)->background().color();
            cgl_palette[k]=RGB_TO_RGB16(color.red(),color.green(),color.blue());
            FullScreenUpdate();
        }
        return;
    }

    if(button->text()=="Save")
        {
            MainWindow *mw=(MainWindow *)this->parent();
            QString path = mw->getDocumentsLocation();

            QString fname = QFileDialog::getSaveFileName(this,
                    "Select file name to store object", path,
                    "newRPL objects (*.nrpl *.* *)");

            if(!fname.isEmpty()) {
                // GOT A NAME, APPEND EXTENSION IF NOT GIVEN
                if(!mw->rpl.isRunning())
                    return;     // DO NOTHING

                //if(!fname.endsWith(".nrpl")) fname+=".nrpl";

                while(!cpu_idle)
                    QThread::msleep(1); // BLOCK UNTIL RPL IS IDLE

                cpu_idle = 2; // BLOCK REQUEST

                if(!SaveColorTheme(fname)) {
                    QMessageBox a(QMessageBox::Warning, "Error while saving",
                            "Cannot write to file " + fname, QMessageBox::Ok, this);
                    a.exec();
                    return;
                }
                cpu_idle = 0;     // LET GO THE SIMULATOR
            }
       return;
    }
    if(button->text()=="Open")
    {
        MainWindow *mw=(MainWindow *)this->parent();
        QString path = mw->getDocumentsLocation();

        QString fname = QFileDialog::getOpenFileName(this, "Select File Name", path,
                "newRPL objects (*.nrpl *.* *)",nullptr,QFileDialog::DontUseNativeDialog);

        if(!fname.isEmpty()) {
            if(!mw->rpl.isRunning())
                return;     // DO NOTHING

            while(!cpu_idle)
                QThread::msleep(1); // BLOCK UNTIL RPL IS IDLE

            cpu_idle = 2; // BLOCK REQUEST

            // NOW WORK ON THE RPL ENGINE WHILE THE THREAD IS BLOCKED
            if(!LoadColorTheme(fname)) {
                QMessageBox a(QMessageBox::Warning, "Error while opening",
                        "Cannot read file. Corrupted data?\n" + fname,
                        QMessageBox::Ok, this);
                a.exec();
                return;
            }

        }

        cpu_idle = 0;     // LET GO THE SIMULATOR
        return;
    }
}

void QPaletteEditor::ReadPalette()
{
    int k;
    for(k=0;k<64;++k)
    {
        QTableWidgetItem *item=ui->pTable->takeItem(k,0);
        QBrush tempbkgnd(QColor::fromRgb(RGBRED(cgl_palette[k]),RGBGREEN(cgl_palette[k]),RGBBLUE(cgl_palette[k])));
        item->setBackground(tempbkgnd);
        ui->pTable->setItem(k,0,item);
    }
}

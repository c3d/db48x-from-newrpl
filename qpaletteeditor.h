#ifndef QPALETTEEDITOR_H
#define QPALETTEEDITOR_H

#include <QDialog>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QAbstractButton>

namespace Ui {
class QPaletteEditor;
}

class QPaletteEditor : public QDialog
{
    Q_OBJECT

public:
    void ReadPalette();

    explicit QPaletteEditor(QWidget *parent = nullptr);
    ~QPaletteEditor();

private slots:
    void on_QPaletteEditor_accepted();



    void on_pTable_cellDoubleClicked(int row, int column);

    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::QPaletteEditor *ui;
};

#endif // QPALETTEEDITOR_H

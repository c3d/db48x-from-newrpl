#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class MainDialog; }
QT_END_NAMESPACE

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    QString getDocumentsLocation();
    QString getPrimeG1FolderLocation();
    void printLine(const QString line);

    MainDialog(QWidget *parent = nullptr);
    ~MainDialog();

private slots:
    void on_folderBrowseButton_clicked();

    void on_proceedButton_clicked();

    void on_firmwareBrowseButton_clicked();

private:
    Ui::MainDialog *ui;
};
#endif // MAINDIALOG_H

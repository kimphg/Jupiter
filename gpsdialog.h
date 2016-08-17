#ifndef GPSDIALOG_H
#define GPSDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include "Config.h"
#include <QUdpSocket>
namespace Ui {
class GPSDialog;
}

class GPSDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GPSDialog(QWidget *initparent = 0);
    //{parent=(*MainWindow)(initparent);}
    ~GPSDialog();
    void loadConfig(CConfig* loadconfig);
private slots:
    void on_buttonBox_accepted();

    void on_pushButton_clicked();


    //void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::GPSDialog *ui;
    CConfig* config;
};

#endif // GPSDIALOG_H

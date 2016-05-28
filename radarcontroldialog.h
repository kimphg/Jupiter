#ifndef RADARCONTROLDIALOG_H
#define RADARCONTROLDIALOG_H

#include <QDialog>
#include <QtNetwork>
#include <QHostAddress>
namespace Ui {
class RadarControlDialog;
}

class RadarControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RadarControlDialog(QWidget *parent = 0);
    ~RadarControlDialog();
    void sendFrame(const char* hexdata,QHostAddress host,int port );
private slots:
    void on_pushButton_clicked();

    void on_pushButton_TX_On_clicked();

    void on_SendCommandBtn_clicked();

    void on_commandlineEdit_editingFinished();

private:
    Ui::RadarControlDialog *ui;
    QUdpSocket      *udpSendSocket;
};

#endif // RADARCONTROLDIALOG_H

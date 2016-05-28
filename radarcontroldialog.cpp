#include "radarcontroldialog.h"
#include "ui_radarcontroldialog.h"
static QString command;
RadarControlDialog::RadarControlDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RadarControlDialog)
{
    ui->setupUi(this);
    udpSendSocket = new QUdpSocket(this);
    udpSendSocket->bind(5800);
    udpSendSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 10);

}
int char2int(char input)
{
  if(input >= '0' && input <= '9')
    return input - '0';
  if(input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if(input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  return 0;
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
void hex2bin(const char* src, char* target)
{
  while(*src && src[1])
  {
    *(target++) = char2int(*src)*16 + char2int(src[1]);
    src += 2;
  }
  *(target++)=0;
}//
void RadarControlDialog::sendFrame(const char* hexdata,QHostAddress host,int port )
{
    short len = strlen(hexdata)/2+1;
    char* sendBuff = new char[len];
    hex2bin(hexdata,sendBuff);
    udpSendSocket->writeDatagram(sendBuff,len-1,host,port);
    delete[] sendBuff;
}
RadarControlDialog::~RadarControlDialog()
{
    delete ui;
}

void RadarControlDialog::on_pushButton_clicked()
{
    sendFrame("138501001800000001000000", QHostAddress(ui->radarIPlabel->text()),2573);
    sendFrame("0180010001000000", QHostAddress(ui->radarIPlabel->text()),2573);
}


void RadarControlDialog::on_pushButton_TX_On_clicked()
{
    //4142434445464748494a4b4c4d4e4f50
    for(short i=0;i<10;i++)
    sendFrame("4142434445464748494a4b4c4d4e4f50", QHostAddress(ui->radarIPlabel->text()),5800);
}

void RadarControlDialog::on_SendCommandBtn_clicked()
{

    QByteArray ba = command.toLatin1();
    const char *c_str2 = ba.data();
    sendFrame(c_str2,QHostAddress("192.168.0.44"),2573);
}

void RadarControlDialog::on_commandlineEdit_editingFinished()
{
    command = ui->commandlineEdit->text();
}

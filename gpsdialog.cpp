#include "gpsdialog.h"
#include "ui_gpsdialog.h"

GPSDialog::GPSDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GPSDialog)
{
    ui->setupUi(this);

}
void GPSDialog::loadConfig(CConfig* loadconfig)
{

    this->config=loadconfig;
    ui->latInput->setText(QString::number(config->m_config.m_lat));
    ui->longInput->setText(QString::number(config->m_config.m_long));
    ui->NorRadarInput_4->setText(QString::number(config->m_config.trueN));
    ui->checkBox->setChecked(config->m_config.mapEnabled);
    ui->comboBox_code_type->setCurrentIndex(config->m_config.codeType);
}
GPSDialog::~GPSDialog()
{
    delete ui;
}

void GPSDialog::on_buttonBox_accepted()
{
    config->m_config.m_lat  = ui->latInput->text().toDouble();
    config->m_config.m_long = ui->longInput->text().toDouble();
    config->m_config.trueN  = ui->NorRadarInput_4->text().toDouble();
    config->m_config.mapEnabled = ui->checkBox->isChecked();
    config->m_config.codeType = ui->comboBox_code_type->currentIndex();
    config->SaveToFile();
    this->accept();
}

void GPSDialog::on_pushButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,    QString::fromUtf8("M? file b?n d?"), NULL, tr("ISM file (*.ism)"));
    if(!filename.size())return;
    config->m_config.mapFilename =  filename.toStdString();
}

//void GPSDialog::on_pushButton_2_clicked()
//{
//    unsigned char        bytes[8];
//    QUdpSocket      *udpSendSocket;//radar control
//    udpSendSocket = new QUdpSocket(this);
//    udpSendSocket->bind(5800);
//    udpSendSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 10);
//    hex2bin(ui->lineEdit_byte_1->text().toStdString().data(),&bytes[0]);
//    hex2bin(ui->lineEdit_byte_2->text().toStdString().data(),&bytes[1]);
//    hex2bin(ui->lineEdit_byte_3->text().toStdString().data(),&bytes[2]);
//    hex2bin(ui->lineEdit_byte_4->text().toStdString().data(),&bytes[3]);
//    hex2bin(ui->lineEdit_byte_5->text().toStdString().data(),&bytes[4]);
//    hex2bin(ui->lineEdit_byte_6->text().toStdString().data(),&bytes[5]);
//    hex2bin(ui->lineEdit_byte_7->text().toStdString().data(),&bytes[6]);
//    hex2bin(ui->lineEdit_byte_8->text().toStdString().data(),&bytes[7]);
//    udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);

//}

void GPSDialog::on_pushButton_3_clicked()
{

    unsigned char        bytes[8];
    QUdpSocket      *udpSendSocket;//radar control
    udpSendSocket = new QUdpSocket(this);
    udpSendSocket->bind(5800);
    udpSendSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 10);
    bytes[0]=0x0e;
    bytes[1]=0xab;
    bytes[2]=0x01;
    bytes[3]=0x00;
    bytes[4]=0x00;
    bytes[5]=0x00;
    bytes[6]=0x00;
    bytes[7]=0x00;
    udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
}

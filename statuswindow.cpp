#include "statuswindow.h"
#include "ui_statuswindow.h"
double lookupTable5V[256];
StatusWindow::StatusWindow(dataProcessingThread *radar,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StatusWindow)
{
    waitTime = 0;
    ansTrue = false;
    ui->setupUi(this);
    mRadar = radar;
    timerId = startTimer(500);
    moduleId = 0;
    paramId = 0xaa;
    command[0]=0xaa;
    command[1]=0xab;
    command[2]=moduleId;
    command[3]=0xaa;
    command[4]=0x00;
    command[5]=0x00;
    command[6]=0x00;
    this->setWindowFlags(Qt::WindowStaysOnTopHint);

}
void StatusWindow::closeEvent(QCloseEvent *event)
{
    killTimer(timerId);
}
void StatusWindow::sendReq()
{
    if(ansTrue)// change to next parameter if answer is correct
    {
        paramId++;
        ansTrue = false;
        waitTime = 0;
    }
    else
    {
        waitTime++;
        if(waitTime>5)// change to next parameter if too many failures
        {
            paramId++;
            ansTrue = false;
            waitTime = 0;
        }
    }
    if(paramId>0xac)
    {
        paramId = 0xaa;
        moduleId++;
        if(moduleId>3)moduleId = 0;
    }
    command[2]=moduleId;
    command[3]=paramId;
    mRadar->sendCommand(&command[0],7);
    mRadar->sendCommand(&command[0],7);
    mRadar->sendCommand(&command[0],7);
}
bool StatusWindow::receiveRes()
{
    QString resVal;
    double hsTap = mRadar->mRadarData->getHsTapAverage();
    hsTap = 20*log10(hsTap/165.0)+77.0;
    ui->label_res_receiver->setText(QString::number(hsTap,'f',1));
    int resModId = mRadar->mRadarData->tempType;
    unsigned char * pFeedBack = mRadar->mRadarData->getFeedback();
    if(   (pFeedBack[0]==command[0])
          &&(pFeedBack[1]==command[1])
          &&(pFeedBack[2]==command[2])
          &&(pFeedBack[3]==command[3])
          &&(pFeedBack[4]==command[4])
          &&(pFeedBack[5]==command[5])
          &&(pFeedBack[6]==command[6])
          &&(resModId==moduleId)
       )
    {
        double x =mRadar->mRadarData->moduleVal;
        switch (resModId) {
        case 0:
            if(paramId==0xaa)
            {
                resVal = QString::number(x/4.0,'f',1);
                ui->label_res_main_temp->setText(resVal);
            }
            else if(paramId==0xab)
            {
                resVal = QString::number(x-72.0,'f',1);
                if(((x-72.0)>10||(x-72.0)<4))ui->label_res_dds_out->setStyleSheet("border: 3px solid red;");
                else ui->label_res_dds_out->setStyleSheet("");
                ui->label_res_dds_out->setText(resVal);
            }
            else if(paramId==0xac)
            {
                resVal = QString::number(x-78.0,'f',1);
                if(((x-78.0)>-5||(x-78.0)<-15))ui->label_trans_input->setStyleSheet("border: 3px solid red;");
                else ui->label_trans_input->setStyleSheet("");
                ui->label_trans_input->setText(resVal);
            }
            break;
        case 2:
            if(paramId==0xaa)
            {
                resVal = QString::number(x,'f',1);
                ui->label_res_trans_temp->setText(resVal);
            }
            else if(paramId==0xab)
            {
                double val = (x/60.0)*(x/60.0)*100.0;
                resVal = QString::number(val,'f',1);
                if((val<80||val>100))ui->label_trans_output->setStyleSheet("border: 3px solid red;");
                else ui->label_trans_output->setStyleSheet("");
                ui->label_trans_output->setText(resVal);
            }
            break;

        case 3:
            if(paramId==0xaa)
            {

            }
            else if(paramId==0xab)
            {
                resVal = QString::number((x-126.0)/1.515,'f',1);
                ui->label_vco2_output->setText(resVal);
            }
            break;
        default:
            break;
        }
        return true;
    }
    printf("wrong respond:%d;%d\n",paramId,moduleId);
    return false;
}

void StatusWindow::timerEvent(QTimerEvent *event)
{
    if(!mRadar->isConnected())
    {
        return;
    }
    warningBlink=!warningBlink;
    ansTrue = receiveRes();
    sendReq();

}
StatusWindow::~StatusWindow()
{
    delete ui;
    killTimer(timerId);
}

#include "dialogaisinfo.h"
#include "ui_dialogaisinfo.h"

DialogAisInfo::DialogAisInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAisInfo)
{
    ui->setupUi(this);
    connect(this,SIGNAL(closed()), this, SLOT(destroy()));
    timerId = this->startTimer(500);

}

DialogAisInfo::~DialogAisInfo()
{

    delete ui;
    this->killTimer(timerId);
}

void DialogAisInfo::setAisData(QList<AIS_object_t> *data, int id)
{
    aisData = data;
    aisMmsi = id;
    UpdateData();
}

void DialogAisInfo::timerEvent(QTimerEvent *event)
{
    UpdateData();
}
void DialogAisInfo::UpdateData()
{
    QList<AIS_object_t>::iterator iter = aisData->begin();
    while(iter!=aisData->end())
    {

        if((*iter).mMMSI==aisMmsi)
        {
            ui->textBrowser->setText((*iter).printData());break;
        }
        iter++;
    }
}

#include "dialogaisinfo.h"
#include "ui_dialogaisinfo.h"

DialogAisInfo::DialogAisInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAisInfo)
{
    ui->setupUi(this);
    timerId = this->startTimer(1000);
}

DialogAisInfo::~DialogAisInfo()
{
    QMutableListIterator<AIS_object_t> i(*aisData);
    while (i.hasNext())
    {
        AIS_object_t obj = i.next();
        if(obj.mMMSI == aisMmsi)
        {
            ui->textBrowser->setText(obj.printData());
            obj.isSelected = false;
            i.setValue(obj);
            break;
        }

    }
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
    QMutableListIterator<AIS_object_t> i(*aisData);
    while (i.hasNext())
    {
        AIS_object_t obj = i.next();
        if(obj.mMMSI == aisMmsi)
        {
            ui->textBrowser->setText(obj.printData());
            obj.isSelected = true;
            i.setValue(obj);
            break;
        }

    }
}

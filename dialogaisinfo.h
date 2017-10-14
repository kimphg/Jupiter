#ifndef DIALOGAISINFO_H
#define DIALOGAISINFO_H

#include <QDialog>
#include "c_arpa_data.h"
namespace Ui {
class DialogAisInfo;
}

class DialogAisInfo : public QDialog
{
    Q_OBJECT

public:

    explicit DialogAisInfo(QWidget *parent = 0);
    ~DialogAisInfo();
    int timerId;
    void setAisData(QList<AIS_object_t> * data,int id);
    QList<AIS_object_t> * aisData;
    int aisMmsi;

private:
    Ui::DialogAisInfo *ui;
    void UpdateData();
private slots:
    void timerEvent(QTimerEvent *event);


};

#endif // DIALOGAISINFO_H

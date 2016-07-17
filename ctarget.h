#ifndef CTARGET_H
#define CTARGET_H
#include <QWidget>
#include <QFrame>
#include <qevent.h>
#include <QPainter>
class CTarget : public QFrame
{
    Q_OBJECT
public:
    explicit CTarget(QWidget *parent = 0);
    float m_lat,m_lon,speed;
    float x,y;
    float azi,range;
    short trackId;
    bool isUsed,isManual,isLost;
    bool selected, clicked,doubleClicked;
    void setCoordinates(float lat, float lon, float rg, float az);
    void setPosistion(float x, float y);
    void setScrPos(short x, short y);
    void setSelected(bool selected);
protected:
    void highLight();
    void resetView();
    void hoverEnter(QHoverEvent *event);
    void hoverLeave(QHoverEvent *event);
    void hoverMove(QHoverEvent *event);
    void OnClick();
    bool event(QEvent *event);
    void paintEvent(QPaintEvent *event);

signals:

public slots:


};

#endif // CTARGET_H

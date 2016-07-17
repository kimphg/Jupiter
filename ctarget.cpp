#include "ctarget.h"
#define SIZE 22
#define SIZE_HALF 11

CTarget::CTarget(QWidget *parent) : QFrame(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
    this->setCursor(Qt::PointingHandCursor);
    setGeometry(0,0,SIZE,SIZE);
    selected = false;
    clicked = false;
    doubleClicked = false;
//    isLost = false;
    //resetView();
}
void CTarget::setCoordinates(float lat,float lon,float rg,float az)
{
    azi = az;
    range = rg;
    m_lat = lat;
    m_lon = lon;
}
void CTarget::hoverEnter(QHoverEvent *)
{
    //highLight();
}
void CTarget::setPosistion(float x, float y)
{
    this->x=x;
    this->y=y;
    //setGeometry(x-SIZE_HALF,y-SIZE_HALF,SIZE,SIZE);
}
void CTarget::setScrPos(short x, short y)
{
    setGeometry(x-SIZE_HALF,y-SIZE_HALF,SIZE,SIZE);
}
void CTarget::hoverLeave(QHoverEvent *)
{
    //resetView();
}

void CTarget::hoverMove(QHoverEvent *)
{
    //highLight();

}
void CTarget::paintEvent(QPaintEvent *event)
{
//    if(isLost)
//    {
//        this->setStyleSheet("background-color: rgba(0,0,0,0);border : 2px dashed  cyan;");
//    }
//    else
    if(isManual)this->setStyleSheet("border: 2px dashed magenta;");
    else this->setStyleSheet("background-color: rgba(0,0,0,0);border : 2px dashed  cyan;");
    //QFrame::paintEvent(event);
//    QPainter p(this);
//    p.setRenderHint(QPainter::Antialiasing, true);
//    if(selected)p.setPen(QPen(Qt::magenta));
//        else
//        p.setPen(QPen(Qt::cyan));
//    QFont font;
//    font.setPointSize(12);
//    p.setFont(font);
    //p.drawText(4,0,20,20,0,QString::number(this->id.toInt()));
}
void CTarget::highLight()
{
    if(isManual)this->setStyleSheet("border: 2px dashed magenta;");
    else this->setStyleSheet("background-color: rgba(0,0,0,0);border : 2px dashed   cyan;");
    //repaint();
}
void CTarget::resetView()
{
    //this->setStyleSheet("background-color: rgb(16, 32, 64);color:rgb(255, 255, 255);");
    //this->setStyleSheet("background-color: rgba(0,0,0,0);border : 2px solid red;");

    //this->setStyleSheet("background-color: rgba(0,0,0,0);border : 2px dashed  cyan;");
    if(isManual)this->setStyleSheet("border: 2px dashed magenta;");
    else this->setStyleSheet("background-color: rgba(0,0,0,0);border : 2px dashed   cyan;");
    //setGeometry(400,400,22,22);
    //repaint();

}
void CTarget::OnClick()
{
    //setSelected(true);

    //resetView();
    //this->setStyleSheet("background-color: rgba(0,0,0,0);border : 2px dashed  blue;");
    //this->
}
void CTarget::setSelected(bool selected)
{
    this->selected = selected;

    update();
    //this->setStyleSheet("background-color: rgba(0,0,0,0);border : 2px dashed  blue;");
    //this->
}
bool CTarget::event(QEvent *event)
{
    switch(event->type())
    {

    case QEvent::MouseButtonPress:
        clicked = true;
        return true;
        break;
    case QEvent::MouseButtonDblClick:
        doubleClicked = true;
        return true;
        break;

    default:
        break;
    }
    return QWidget::event(event);
}

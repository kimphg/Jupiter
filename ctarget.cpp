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
    resetView();
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
    highLight();
}
void CTarget::setPosistion(short x, short y)
{
    setGeometry(x-SIZE_HALF,y-SIZE_HALF,SIZE,SIZE);
}
void CTarget::hoverLeave(QHoverEvent *)
{
    resetView();
}

void CTarget::hoverMove(QHoverEvent *)
{
    highLight();

}
void CTarget::highLight()
{


    this->setStyleSheet("border: 2px dashed magenta;");

    repaint();
}
void CTarget::resetView()
{
    //this->setStyleSheet("background-color: rgb(16, 32, 64);color:rgb(255, 255, 255);");
    //this->setStyleSheet("background-color: rgba(0,0,0,0);border : 2px solid red;");

    this->setStyleSheet("background-color: rgba(0,0,0,0);border : 2px dashed  cyan;");

    //setGeometry(400,400,22,22);
    repaint();

}
void CTarget::OnClick()
{
    selected = !selected;
    highLight();
    //this->setStyleSheet("background-color: rgba(0,0,0,0);border : 2px dashed  blue;");
    //this->
}
bool CTarget::event(QEvent *event)
{
    switch(event->type())
    {
    case QEvent::HoverEnter:
        hoverEnter(static_cast<QHoverEvent*>(event));
        return true;
        break;
    case QEvent::HoverLeave:
        hoverLeave(static_cast<QHoverEvent*>(event));
        return true;
        break;
    case QEvent::HoverMove:
        hoverMove(static_cast<QHoverEvent*>(event));
        return true;
        break;
    case QEvent::MouseButtonPress:
        OnClick();
        return true;
        break;
    case QEvent::MouseButtonDblClick:
        //OnClick();
        return true;
        break;
    default:
        break;
    }
    return QWidget::event(event);
}

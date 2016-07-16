#include "qcustomgroupbox.h"

QCustomGroupBox::QCustomGroupBox(QWidget *parent) : QGroupBox(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
    this->setCursor(Qt::ArrowCursor);
    resetView();
}

void QCustomGroupBox::hoverEnter(QHoverEvent *)
{

    highLight();
}

void QCustomGroupBox::hoverLeave(QHoverEvent *)
{
    resetView();
}

void QCustomGroupBox::hoverMove(QHoverEvent *)
{
    highLight();

}
void QCustomGroupBox::highLight()
{
    this->setStyleSheet("background-color: rgb(30, 50, 70);color:rgb(255, 255, 255);font: 12pt \"MS Shell Dlg 2\";");
    repaint();
}
void QCustomGroupBox::resetView()
{
    //this->setStyleSheet("border: none;background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #a6a6a6, stop: 0.08 #7f7f7f,stop: 0.39999 #717171, stop: 0.4 #626262,stop: 0.9 #4c4c4c, stop: 1 #333333);");
    this->setStyleSheet("background-color: rgb(30, 50, 70);color:rgb(255, 255, 255);font: 12pt \"MS Shell Dlg 2\";");
    repaint();

}
bool QCustomGroupBox::event(QEvent *event)
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
    default:
        break;
    }
    return QWidget::event(event);
}

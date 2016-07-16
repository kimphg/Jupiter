#include "qcustomframe.h"

QCustomFrame::QCustomFrame(QWidget *parent) : QFrame(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
    this->setCursor(Qt::ArrowCursor);
    resetView();
}

void QCustomFrame::hoverEnter(QHoverEvent *)
{

    highLight();
}

void QCustomFrame::hoverLeave(QHoverEvent *)
{
    resetView();
}

void QCustomFrame::hoverMove(QHoverEvent *)
{
    highLight();

}
void QCustomFrame::highLight()
{
    this->setStyleSheet("background-color: rgb(16, 32, 64);color:rgb(255, 255, 255);font: bold 12pt \"MS Shell Dlg 2\";border : 3px solid gray;");
    repaint();
}
void QCustomFrame::resetView()
{
    //this->setStyleSheet("background-color: rgb(16, 32, 64);color:rgb(255, 255, 255);");
    this->setStyleSheet("background-color: rgb(16, 32, 64);color:rgb(255, 255, 255);font: 12pt \"MS Shell Dlg 2\";");
    repaint();

}
bool QCustomFrame::event(QEvent *event)
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

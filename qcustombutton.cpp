#include "qcustombutton.h"

QCustomButton::QCustomButton(QWidget *parent) : QToolButton(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover);
    this->setCursor(Qt::ArrowCursor);
    resetView();
}

void QCustomButton::hoverEnter(QHoverEvent *)
{

    highLight();
}

void QCustomButton::hoverLeave(QHoverEvent *)
{
    resetView();
}

void QCustomButton::hoverMove(QHoverEvent *)
{
    highLight();

}
void QCustomButton::highLight()
{
    this->setStyleSheet("background-color: rgb(16, 32, 64);color:rgb(255, 255, 255);font: bold 12pt \"MS Shell Dlg 2\";");
    repaint();
}
void QCustomButton::resetView()
{
    //this->setStyleSheet("background-color: rgb(16, 32, 64);color:rgb(255, 255, 255);");
    this->setStyleSheet("background-color: rgb(16, 32, 64);color:rgb(255, 255, 255);font: 12pt \"MS Shell Dlg 2\";");
    repaint();

}
bool QCustomButton::event(QEvent *event)
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

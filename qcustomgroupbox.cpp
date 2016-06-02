#include "QCustomGroupBox.h"

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
    //this->setStyleSheet("background-color: rgb(16, 32, 64);color:rgb(255, 255, 255);");
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

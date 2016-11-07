#include "qcustomtabwidget.h"

QCustomTabWidget::QCustomTabWidget(QWidget *parent) : QTabWidget(parent)
{
    setMouseTracking(true);
    //this->setStyleSheet("background-color: rgb(30, 50, 70);color:rgb(255, 255, 255);font: 12pt \"MS Shell Dlg 2\";");
    setAttribute(Qt::WA_Hover);
    this->setCursor(Qt::ArrowCursor);
    resetView();
}

void QCustomTabWidget::hoverEnter(QHoverEvent *)
{

   // highLight();
}

void QCustomTabWidget::hoverLeave(QHoverEvent *)
{
    //resetView();
}

void QCustomTabWidget::hoverMove(QHoverEvent *)
{
    //highLight();

}
void QCustomTabWidget::highLight()
{

    if(this->currentIndex()>1)
    {
        this->setStyleSheet("background-color: rgb(0, 0, 0,0);color:rgb(255, 255, 255);font: 12pt \"MS Shell Dlg 2\";");
    }
    else
    {
        this->setStyleSheet("background-color: rgb(30, 50, 70,255);color:rgb(255, 255, 255);font: 12pt \"MS Shell Dlg 2\";");
    }
    //repaint();
}
void QCustomTabWidget::paintEvent(QPaintEvent *)
{
    //resetView();
}

void QCustomTabWidget::resetView()
{
    this->setStyleSheet("background-color: rgb(0, 0, 0,0);"
                        "color:rgb(30, 50, 70,255);font: 12pt \"MS Shell Dlg 2\"; "
                        "border-style: groove; border-width: 1px;border-color:white;");

}
bool QCustomTabWidget::event(QEvent *event)
{
//    switch(event->type())
//    {
//    case QEvent::HoverEnter:
//        hoverEnter(static_cast<QHoverEvent*>(event));
//        return true;
//        break;
//    case QEvent::HoverLeave:
//        hoverLeave(static_cast<QHoverEvent*>(event));
//        return true;
//        break;
//    case QEvent::HoverMove:
//        hoverMove(static_cast<QHoverEvent*>(event));
//        return true;
//        break;
//    default:
//        break;
//    }
    return QWidget::event(event);
}

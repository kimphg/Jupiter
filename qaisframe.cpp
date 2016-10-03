#include "qaisframe.h"

qAISFrame::qAISFrame(QWidget *parent): QFrame(parent)
{
    setMouseTracking(true);
    setAtttribute(Qt::WA_Hover);
    this->setCursor(Qt::ArrowCursor);
    resetView();
}

    void qAISFrame::hoverEnter(QHoverEvent *)
    {

        highLight();
    }

    void qAISFrame::hoverLeave(QHoverEvent *)
    {
        resetView();
    }

    void qAISFrame::hoverMove(QHoverEvent *)
    {
        highLight();

    }
    void qAISFrame::highLight()
    {
        this->setStyleSheet("background-color: rgb(16, 32, 64);color:rgb(255, 255, 255);font: bold 12pt \"MS Shell Dlg 2\";border : 3px solid gray;");
        repaint();
    }
    void qAISFrame::resetView()
    {
        //this->setStyleSheet("background-color: rgb(16, 32, 64);color:rgb(255, 255, 255);");
        this->setStyleSheet("background-color: rgb(16, 32, 64);color:rgb(255, 255, 255);font: 12pt \"MS Shell Dlg 2\";");
        repaint();

    }
    bool qAISFrame::event(QEvent *event)
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



#ifndef QCUSTOM_FRAME_H
#define QCUSTOM_FRAME_H

#include <QWidget>
#include <QFrame>
#include <qevent.h>
class QCustomFrame: public QFrame
{
    Q_OBJECT
public:
    explicit QCustomFrame(QWidget *parent = 0);

protected:
    void highLight();
    void resetView();
    void hoverEnter(QHoverEvent *event);
    void hoverLeave(QHoverEvent *event);
    void hoverMove(QHoverEvent *event);
    bool event(QEvent *event);

signals:

public slots:
};

#endif // QCUSTOM_FRAME_H

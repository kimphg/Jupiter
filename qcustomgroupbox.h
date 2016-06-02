#ifndef QCUSTOM_GROUP_BOX_H
#define QCUSTOM_GROUP_BOX_H

#include <QWidget>
#include <QGroupBox>
#include <qevent.h>
class QCustomGroupBox: public QGroupBox
{
    Q_OBJECT
public:
    explicit QCustomGroupBox(QWidget *parent = 0);

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

#endif // QCUSTOM_GROUP_BOX_H

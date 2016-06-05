#ifndef QCUSTOM_TAB_WIDGET_H
#define QCUSTOM_TAB_WIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <qevent.h>
#include <dataprocessingthread.h>
#include <QPainter>
class QCustomTabWidget: public QTabWidget
{
    Q_OBJECT
public:
    explicit QCustomTabWidget(QWidget *parent = 0);
    dataProcessingThread *processing;
protected:
    void highLight();
    void resetView();
    void hoverEnter(QHoverEvent *event);
    void hoverLeave(QHoverEvent *event);
    void hoverMove(QHoverEvent *event);
    bool event(QEvent *event);
    void paintEvent(QPaintEvent *);
signals:

public slots:
};

#endif // QCUSTOM_TAB_WIDGET_H

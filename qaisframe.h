#ifndef QAISFRAME_H
#define QAISFRAME_H


class qAISFrame: public QFrame
{
public:
    string: aisTargetName;
    qAISFrame();

protected:
    void highLight();
    void resetView();
    void hoverEnter(QHoverEvent *event);
    void hoverLeave(QHoverEvent *event);
    void hoverMove(QHoverEvent *event);
    bool event(QEvent *event);
};

#endif // QAISFRAME_H

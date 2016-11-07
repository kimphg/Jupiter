#include "jviewport.h"




jViewPort::jViewPort()
{
    setStyleSheet("background: transparent");

}

jViewPort::jViewPort(QGraphicsScene *scene, QWidget *parent):QGraphicsView(scene,parent )
{
    setStyleSheet("background: transparent; border: noborder");
    pMap = NULL;
    dxMap = 0;
    dyMap = 0;
}

void jViewPort::hoverEnter(QHoverEvent *event)
{
    Q_UNUSED(event);
    printf("aa");
}

void jViewPort::hoverLeave(QHoverEvent *event)
{

}

void jViewPort::hoverMove(QHoverEvent *event)
{

}




void jViewPort::mousePressEvent(QMouseEvent *event)
{
    //if(event->x()>scrCtX+scrCtY)return;
    mouseX = (event->x());
    mouseY = (event->y());
    if(event->buttons() & Qt::LeftButton) {
//        if(ui->toolButton_manual_track->isChecked())
//        {
//            float xRadar = (mouseX - scrCtX+dx)/processing->radarData->scale_ppi ;//coordinates in  radar xy system
//            float yRadar = -(mouseY - scrCtY+dy)/processing->radarData->scale_ppi;
//            processing->radarData->addTrackManual(xRadar,yRadar);
//            ui->toolButton_manual_track->setChecked(false);
//        }



//        else
        {
            isDraging = true;
        }
    }
}
void jViewPort::mouseMoveEvent(QMouseEvent *event) {
    short dxMax = height()/4-10;
    if(isDraging&&(event->buttons() & Qt::LeftButton)) {

        short olddx = dx;
        short olddy = dy;
        dx+= mouseX - event->x();
        dy+= mouseY - event->y();

        dxMap += mouseX - event->x();
        dyMap += mouseY - event->y();
        while(dx*dx+dy*dy>dxMax*dxMax)
        {
            if(abs(dx)>abs(dy))
            {
                if(dx>0){dx--;dxMap--;}else {dx++;dxMap++;}}
            else
            {
                if(dy>0){dy--;dyMap--;}else {dy++;dyMap++;}
            }
        }
        mousePointerX += olddx - dx;
        mousePointerY += olddy - dy;
        mouseX=event->x();
        mouseY=event->y();
    }
}
void jViewPort::mouseDoubleClickEvent( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton )
    {
        mousePointerX = (e->x());
        mousePointerY = (e->y());
        //processing->radarData->updateZoomRect(mousePointerX - scrCtX+dx,mousePointerY - scrCtY+dy);
    }
    //Test doc AIS

}

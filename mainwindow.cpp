#include "mainwindow.h"
#include "ui_mainwindow.h"

//#include "gdal/ogr/ogrsf_frmts/ogrsf_frmts.h"
//#include "gdal/gcore/gdal.h"
//#define mapWidth 2000
//#define mapWidth mapWidth
//#define mapHeight mapWidth
#define CONST_NM 1.852f// he so chuyen doi tu km sang hai ly
#define MAX_VIEW_RANGE_KM 50
//#include <queue>

QPixmap                     *pMap=NULL;// painter cho ban do
CMap *osmap ;
dataProcessingThread        *processing;// thread xu ly du lieu radar
QThread                     *t2,*t1;
//Q_vnmap                     vnmap;
QTimer                      scrUpdateTimer,readBuffTimer ;
QTimer                      syncTimer1s,syncTimer5p ;
QTimer                      dataPlaybackTimer ;
bool                        displayAlpha = false;
//QList<CTarget*>             targetDisplayList;
short                       dxMax,dyMax;
C_ARPA_data                 arpa_data;
short                       scrCtX, scrCtY, dx =0,dy=0,dxMap=0,dyMap=0;
short                       mousePointerX,mousePointerY,mouseX,mouseY;
//bool                        isDraging = false;
bool                        isScaleChanged =true;
double                       mScale;
//QGraphicsScene* scene;
//jViewPort* view;
CConfig         config;
QStringList     warningList;
short selectedTargetIndex;
mouseMode mouse_mode = MouseNormal;
enum drawModes{
    SGN_DIRECT_DRAW,SGN_IMG_DRAW,NOTERR_DRAW
}drawMode = SGN_IMG_DRAW;
enum TargetType{
    RADAR,AIS,NOTARGET
}selectedTargetType  = NOTARGET;
int targetID = -1;
//short config.getRangeView() = 1;
float rangeStep = 1;
//typedef struct {
//    unsigned char        bytes[8];
//}
//Command_Control;
//typedef std::queue<Command_Control> CommandList;
//static CommandList command_queue;
bool isDrawSubTg = true;

class guard_zone_t
{
public:
    guard_zone_t(){}
    ~guard_zone_t(){}
    short x1,y1,x2,y2;
    float maxAzi,minAzi;
    float maxR,minR;
    char  isActive;
    void update()
    {
        //float azi,rg;
//        C_radar_data::kmxyToPolar((x1 - scrCtX+dx)/mScale,-(y1 - scrCtY+dy)/mScale,&minAzi,&minR);
//        C_radar_data::kmxyToPolar((x2 - scrCtX+dx)/mScale,-(y2 - scrCtY+dy)/mScale,&maxAzi,&maxR);
//        if(minAzi<0)minAzi += PI_NHAN2;
//        minAzi = minAzi*DEG_RAD;
//        if(maxAzi<0)maxAzi += PI_NHAN2;
//        maxAzi = maxAzi*DEG_RAD;
    }
};
guard_zone_t gz1,gz2,gz3;
//static unsigned short cur_object_index = 0;
short lon2x(float lon)
{
   float refLat = (config.getLat() )*0.00872664625997f;
   return  (- dx + scrCtX + ((lon - config.getLon()) * 111.31949079327357f*cosf(refLat))*mScale);
}
short lat2y(float lat)
{

   return (- dy + scrCtY - ((lat - config.getLat()) * 111.31949079327357f)*mScale);
}
double y2lat(short y)
{
   return (y  )/mScale/111.31949079327357f + config.getLat();
}
double x2lon(short x)
{
    float refLat = (config.getLat() )*0.00872664625997;
   return (x  )/mScale/111.31949079327357f/cosf(refLat) + config.getLon();
}
void Mainwindow::mouseDoubleClickEvent( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton )
    {
        mousePointerX = (e->x());
        mousePointerY = (e->y());
        processing->radarData->updateZoomRect(mousePointerX - scrCtX+dx,mousePointerY - scrCtY+dy);
    }
    //Test doc AIS

}
void Mainwindow::sendToRadarHS(const char* hexdata)
{
    short len = strlen(hexdata)/2+1;
    unsigned char* sendBuff = new unsigned char[len];
    hex2bin(hexdata,sendBuff);
    processing->sendCommand(sendBuff,len);
    delete[] sendBuff;
}
void Mainwindow::sendToRadar(unsigned char* hexdata)
{

    m_udpSocket->writeDatagram((char*)hexdata,8,QHostAddress("192.168.0.44"),2572);
    //printf("\a");

}
//ham test ve tu AIS
void Mainwindow::drawAisTarget2(QPainter *p, short xAIS, short yAIS)
{
    //draw radar
    QPen penTarget(QColor(255,50,150));
    penTarget.setWidth(0);

    //hightlight target
    QPen penSelectTarger (QColor(0,166,173));
    penSelectTarger.setWidth(0);


    for(uint i=0; i<m_AISList.size(); i++)
    {
        double fx,fy;

        osmap->ConvWGSToKm(&fx,&fy,m_AISList.at(i).getLon(),m_AISList.at(i).getLat());

        short x = (fx*mScale)+scrCtX-dx;
        short y = (fy*mScale)+scrCtY-dy;

        if( qAbs(xAIS-x) <5 && qAbs(yAIS-y)<5)
        {
            p->setPen((penSelectTarger));
        }
        else p->setPen((penTarget));

        //draw ais mark
        QPolygon poly;
        QPoint point;
        float head = m_AISList.at(i).m_Head*PI_NHAN2/(1<<16);
        point.setX(x+8*sinf(head));
        point.setY(y-8*cosf(head));
        poly<<point;
        point.setX(x+8*sinf(head+2.3562f));
        point.setY(y-8*cosf(head+2.3562f));
        poly<<point;
        point.setX(x);
        point.setY(y);
        poly<<point;
        point.setX(x+8*sinf(head-2.3562f));
        point.setY(y-8*cosf(head-2.3562f));
        poly<<point;
        p->drawPolygon(poly);
        //draw ais name
        if(ui->toolButton_ais_name->isChecked())
        {
            QFont font = p->font() ;
            font.setPointSize(6);
            p->setFont(font);
            p->drawText(x+5,y+10,(m_AISList.at(i).m_szName));
        }
//        QPushButton *m_button;
//        m_button = new QPushButton("My Button", this);
//            // set size and location of the button
//        m_button->setGeometry(QRect(QPoint(x, y),
//        QSize(16, 16)));

        //p->drawText(x+5,y+5,QString::fromAscii((char*)&m_trackList.at(i).m_MMSI[0],9));
        //printf("\nj:%d,%d,%d,%f,%f",j,x,y,arpa_data.ais_track_list[i].object_list[j].mlong,arpa_data.ais_track_list[i].object_list[j].mlat);
    }
}
void Mainwindow::mouseReleaseEvent(QMouseEvent *event)
{
    event = event;
    setMouseMode(MouseDrag,false);
//    if(isAddingTarget)
//    {
//        float xRadar = (mouseX - scrCtX+dx)/signsize ;//coordinates in  radar xy system
//        float yRadar = -(mouseY - scrCtY+dy)/signsize;
//        processing->radarData->addTrack(xRadar,yRadar);
//        ui->actionAddTarget->toggle();
//        isScreenUp2Date = false;
//        return;
//    }

    DrawMap();
//    isScreenUp2Date = false;
    //isDraging = false;
    /*currMaxRange = (sqrtf(dx*dx+dy*dy)+scrCtY)/signsize;
    if(currMaxRange>RADAR_MAX_RESOLUTION)currMaxRange = RADAR_MAX_RESOLUTION;
    if((dx*dx+dy*dy)*3>scrCtX*scrCtX)
    {
        if(dx<0)
        {
            currMaxAzi = (unsigned short)((atanf((float)dy/(float)dx)-processing->radarData->trueN)/PI_NHAN2*4096.0f);
            if(currMaxAzi<0)currMaxAzi+=MAX_AZIR;
            if(currMaxAzi>MAX_AZIR)currMaxAzi-=MAX_AZIR;
        }
        if(dx>0)
        {
            currMaxAzi = (unsigned short)(((atanf((float)dy/(float)dx)+PI-processing->radarData->trueN))/PI_NHAN2*4096.0f);
            if(currMaxAzi>MAX_AZIR)currMaxAzi-=MAX_AZIR;
            if(currMaxAzi<0)currMaxAzi+=MAX_AZIR;
        }
        currMinAzi = currMaxAzi - MAX_AZIR/2;
        if(currMinAzi<0)currMinAzi+=MAX_AZIR;
        //printf("\n currMinAzi:%d currMaxAzi:%d ",currMinAzi,currMaxAzi);
    }else
    {
        currMaxAzi = MAX_AZIR;
        currMinAzi = 0;
    }*/
}
void Mainwindow::wheelEvent(QWheelEvent *event)
{
    event = event;
    //if(event->delta()>0)ui->horizontalSlider->setValue(ui->horizontalSlider->value()+1);
    //if(event->delta()<0)ui->horizontalSlider->setValue(ui->horizontalSlider->value()-1);
}
void Mainwindow::mouseMoveEvent(QMouseEvent *event) {
    if((mouse_mode&MouseDrag)&&(event->buttons() & Qt::LeftButton)) {
        short olddx = dx;
        short olddy = dy;
        dx+= mouseX-event->x();
        dy+= mouseY-event->y();

        dxMap += mouseX-event->x();
        dyMap += mouseY-event->y();
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
        mousePointerX+= olddx - dx;
        mousePointerY+= olddy - dy;
        mouseX=event->x();
        mouseY=event->y();
    }
}
void Mainwindow::keyPressEvent(QKeyEvent *event)
{
    this->setFocus();
    if(event->key() == Qt::Key_Space)
    {
        short   x=this->mapFromGlobal(QCursor::pos()).x();
        short   y=this->mapFromGlobal(QCursor::pos()).y();
        float xRadar = (x - scrCtX+dx) ;//coordinates in  radar xy system
        float yRadar = -(y - scrCtY+dy);
        processing->radarData->addTrackManual(xRadar,yRadar);
        ui->toolButton_manual_track->setChecked(false);
//        isScreenUp2Date = false;
    }
}
short selZone_x1, selZone_x2, selZone_y1, selZone_y2;
bool isSelectingTarget = false;
void Mainwindow::detectZone()
{
    short sx,sy;
    float scale_ppi = processing->radarData->scale_ppi;
    if(selZone_x1>selZone_x2)
    {
        short tmp = selZone_x1;
        selZone_x1 = selZone_x2;
        selZone_x2 = tmp;
    }
    if(selZone_y1>selZone_y2)
    {
        short tmp = selZone_y1;
        selZone_y1 = selZone_y2;
        selZone_y2 = tmp;
    }
    trackList* trackListPt = &processing->radarData->mTrackList;
    if(ui->toolButton_blue_tracks->isChecked())
    {
        for(uint trackId=0;trackId<trackListPt->size();trackId++)
        {
            if(!trackListPt->at(trackId).isConfirmed)continue;
            if(trackListPt->at(trackId).isManual)continue;
            if(!trackListPt->at(trackId).state)continue;
            sx = trackListPt->at(trackId).estX*scale_ppi + scrCtX - dx;
            sy = -trackListPt->at(trackId).estY*scale_ppi + scrCtY - dy;
            if((sx>=selZone_x1)&&(sx<=selZone_x2)&&(sy>selZone_y1)&&(sy<selZone_y2))
            {
                trackListPt->at(trackId).setManual(true);
            }
        }

    }
}
bool Mainwindow::isInsideViewZone(short x, short y)
{
    short dx = x-scrCtX;
    short dy = y-scrCtY;
    if((dx*dx+dy*dy)>(scrCtY*scrCtY))return false;
    else return true;
}
void Mainwindow::mousePressEvent(QMouseEvent *event)
{
    mouseX = (event->x());
    mouseY = (event->y());
    if(!isInsideViewZone(mouseX,mouseY))return;
    if(event->buttons() & Qt::LeftButton) {
        if(mouse_mode&MouseAddingTrack)//(ui->toolButton_manual_track->isChecked())
        {
            float xRadar = (mouseX - scrCtX+dx)/processing->radarData->scale_ppi ;//coordinates in  radar xy system
            float yRadar = -(mouseY - scrCtY+dy)/processing->radarData->scale_ppi;
            processing->radarData->addTrackManual(xRadar,yRadar);
            ui->toolButton_manual_track->setChecked(false);
        }
        else if(mouse_mode&MouseAutoSelect)//(ui->toolButton_auto_select->isChecked())
        {
            if(isSelectingTarget)
            {
                selZone_x2 = mouseX;
                selZone_y2 = mouseY;
                detectZone();
                isSelectingTarget = false;
            }else
            {
                selZone_x1 = mouseX;
                selZone_y1 = mouseY;
                isSelectingTarget = true;
            }
        }
        else if(mouse_mode&MouseScope)
        {
            double azid,rg;
            C_radar_data::kmxyToPolarDeg((mouseX - scrCtX+dx)/mScale,-(mouseY - scrCtY+dy)/mScale,&azid,&rg);
            processing->radarData->drawRamp(azid);

        }
        else if(ui->toolButton_create_zone->isChecked())
        {
            gz1.isActive = 1;

            gz1.x1 = event->x();
            gz1.y1 = event->y();
        }
        else if(ui->toolButton_create_zone_2->isChecked())
        {
            gz2.isActive = 1;
            gz2.x1 = event->x();
            gz2.y1 = event->y();
        }
        else if(ui->toolButton_create_zone_3->isChecked())
        {
            gz3.isActive = 1;
            gz3.x1 = event->x();
            gz3.y1 = event->y();
        }
        else
        {
            setMouseMode(MouseDrag,true);
            //mouse_mode=MouseDrag;//isDraging = true;
        }
    }
    if(event->buttons() & Qt::RightButton)
    {
        //select radar target
        trackList* trackListPt = &processing->radarData->mTrackList;
        for(uint trackId=0;trackId<trackListPt->size();trackId++)
        {
            if(!trackListPt->at(trackId).isConfirmed)continue;
            if(!trackListPt->at(trackId).isManual)continue;
            //if(trackListPt->at(trackId).state<5)continue;
            short sx = trackListPt->at(trackId).estX*processing->radarData->scale_ppi + scrCtX - dx;
            short sy = -trackListPt->at(trackId).estY*processing->radarData->scale_ppi + scrCtY - dy;
            if( qAbs(sx-event->x()) <5 && qAbs(sy-event->y())<5)
            {
                selectedTargetType = RADAR;
                selectedTargetIndex = trackId;
            }
        }


        //select ais target
        if(ui->toolButton_ais_show->isChecked())
        {
            //lay vi tri con tro chuot
            float xAIS = event->x();//(e->x() - scrCtX+dx)/mScale ;//coordinates in  radar xy system
            float yAIS = event->y();//-(e->y() - scrCtY+dy)/mScale;

            for(ushort i=0; i<m_AISList.size(); i++)
            {
                //p->setPen((penTarget));
//                float mlat, mlong; //kinh do
//                mlat = m_trackList.at(i).m_Lat;
//                mlat = mlat/bit23*180.0f;
//                mlong = m_trackList.at(i).m_Long;
//                mlong = mlong/bit23*180.0f;
                double fx,fy;
                ConvWGSToKm(&fx,&fy,m_AISList.at(i).getLon(),m_AISList.at(i).getLat());
                short x = (fx*mScale)+scrCtX-dx;
                short y = (fy*mScale)+scrCtY-dy;

                //float head = m_trackList.at(i).m_Head*PI_NHAN2/(1<<16);
                //double x1 = x+8*sinf(head);
                //double y1 = y-8*cosf(head);

                if( qAbs(xAIS-x) <5 && qAbs(yAIS-y)<5)
                {
                    //printf("ais select");
                    selectedTargetType = AIS;
                    selectedTargetIndex = i;
                    break;

                }

            }
        }
    }

}
/*void MainWindow::wheelEvent(QWheelEvent *event)
{
//    if(event->delta()>0)ui->horizontalSlider->raise();
//    if(event->delta()<0)ui->horizontalSlider->setValue(3);
//    if(scale>SCALE_MAX)scale=SCALE_MAX;
//    if(scale<SCALE_MIN)scale=SCALE_MIN;
//    //signsize = SIGNAL_SCALE/scale;
//    DrawMap();
//    update();
}*/
Mainwindow::Mainwindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //ui->frame_RadarViewOptions->hide();
    QFont font;
    font.setPointSize(12);
    //ui->listTargetWidget->setFont(font);
    //ui->frame_2->setStyleSheet("#frame_2 { border: 2px solid darkgreen; }");
    //ui->frame_3->setStyleSheet("#frame_3 { border: 2px solid darkgreen; }");
    //if(!this->isFullScreen())this->showFullScreen();
    InitNetwork();
    InitTimer();
    setFocusPolicy(Qt::StrongFocus);
    InitSetting();
    setRadarState(DISCONNECTED);
//    GDALAllRegister();
//    GDALDataset       *poDS;

    //init drawing context

    //this->setFixedSize(900 + ui->toolBar_Main->width()*3,850);
    //scale = SCALE_MIN;



    //isSettingUp2Date = false;
    //UpdateSetting();

}

void Mainwindow::DrawSignal(QPainter *p)
{
    QRectF signRect(DISPLAY_RES-(scrCtX-dx),DISPLAY_RES-(scrCtY-dy),width(),height());
    QRectF screen(0,0,width(),height());
    p->drawImage(screen,*processing->radarData->img_ppi,signRect,Qt::AutoColor);
}

//void MainWindow::createMenus()
//{
//    m_fileMenu = menuBar()->addMenu(tr("&File"));
//    m_fileMenu->addAction(a_openShp);
//    m_fileMenu->addAction(a_openPlace);
//    m_fileMenu->addAction(a_openSignal);

//    //
//    m_connectionMenu = menuBar()->addMenu(tr("&Connect"));
//    m_connectionMenu->addAction(a_gpsOption);
//}
void Mainwindow::gpsOption()
{
    //GPSDialog *dlg = new GPSDialog;
    //dlg->show();
}

void Mainwindow::PlaybackRecFile()//
{


}
//void MainWindow::createActions()
//{
//    a_openShp = new QAction(tr("&Open Shp"), this);
//    a_openShp->setShortcuts(QKeySequence::Open);
//    a_openShp->setStatusTip(tr("Open shp file"));
//    connect(a_openShp, SIGNAL(triggered()), this, SLOT(openShpFile()));
//    //______________________________________//
//    a_openPlace = new QAction(tr("&Open place file"), this);
//    a_openPlace->setStatusTip(tr("Open place file"));
//    connect(a_openPlace, SIGNAL(triggered()), this, SLOT(openPlaceFile()));
//    //______________________________________//
//    a_gpsOption = new QAction(tr("&GPS option"), this);
//    a_gpsOption->setStatusTip(tr("GPS option"));
//    connect(a_gpsOption, SIGNAL(triggered()), this, SLOT(gpsOption()));
//    //______________________________________//
//    a_openSignal = new QAction(tr("&Open signal file"), this);
//    a_openSignal->setStatusTip(tr("Mở file tín hiệu đã lưu."));
//    connect(a_openSignal, SIGNAL(triggered()), this, SLOT(openSignalFile()));

//}
//void MainWindow::openSignalFile()
//{
//    //printf("shp file max ");
//    QString fileName = QFileDialog::getOpenFileName(this,
//        tr("Open signal file"), NULL, tr("Signal data Files (*.dat)"));
//    rawData.OpenFile(fileName.toStdString().c_str());

//    //SHPHandle hSHP = SHPOpen(fileName.toStdString().c_str(), "rb" );
//    //if(hSHP == NULL) return;
//}
/*
static short curMapLayer=0;

void MainWindow::openShpFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open SHP file"), NULL, tr("Shp Files (*.shp)"));
    if(!fileName.size())return;
    vnmap.OpenShpFile(fileName.toStdString().c_str(), curMapLayer );
    vnmap.LoadPlaces(fileName.toStdString().c_str());
    curMapLayer++;
    DrawMap();
    //DrawToPixmap(pPixmap);
    update();

}*/

Mainwindow::~Mainwindow()
{
    delete ui;

    if(pMap)delete pMap;
}

void Mainwindow::DrawMap()
{


    if(!pMap) return;
    pMap->fill(Qt::transparent);

    dxMap = 0;
    dyMap = 0;
    //
    QPainter p(pMap);

    if(1)
    {

        double dLat, dLong;
        ConvKmToWGS((double(dx))/mScale,(double(-dy))/mScale,&dLong,&dLat);
        osmap->setCenterPos(dLat,dLong);
        QPixmap pix = osmap->getImage(mScale);
        p.setOpacity(config.getMapOpacity());
        p.drawPixmap((-pix.width()/2+pMap->width()/2),
                     (-pix.height()/2+pMap->height()/2),pix.width(),pix.height(),pix
                     );
    }
    else
    {
        pMap->fill(QColor(10,20,30,255));
    }

    //DrawGrid(&p,centerX,centerY);

}
void Mainwindow::DrawGrid(QPainter* p,short centerX,short centerY)
{
    //return;
    QPen pen(QColor(0x8f,0x8f,0x8f,0xff));
    pen.setStyle(Qt::DashLine);
    p->setBrush(QBrush(Qt::NoBrush));
    p->setPen(pen);
    p->drawLine(centerX-5,centerY,centerX+5,centerY);
    p->drawLine(centerX,centerY-5,centerX,centerY+5);
    //pen.setColor(QColor(30,90,150,120));
    pen.setWidth(1);
    p->setPen(pen);
    for(short i = 1;i<8;i++)
    {
    p->drawEllipse(QPoint(centerX,centerY),
                  (short)(i*rangeStep*CONST_NM*mScale),
                  (short)(i*rangeStep*CONST_NM*mScale));
    }



        //p.drawEllipse(QPoint(centerX,centerY),(int)(20*CONST_NM*scale),(int)(20*CONST_NM*scale));
        //p.drawEllipse(QPoint(centerX,centerY),(int)(5*CONST_NM*scale),(short)(5*CONST_NM*scale));
        //pen.setWidth(1);
        //p->setPen(pen);
        short theta;
        short gridR = rangeStep*1.852f*mScale*7;
        for(theta=0;theta<360;theta+=90){
            QPoint point1,point2;
                short dx = gridR*cosf(theta/DEG_RAD);
                short dy = gridR*sinf(theta/DEG_RAD);
                point1.setX(centerX);
                point1.setY(centerY);
                point2.setX(centerX+dx);
                point2.setY(centerY+dy);
                p->drawLine(point1,point2);

        }
        for(theta=0;theta<360;theta+=30){
            QPoint point1,point2;
                short dx = gridR*cosf(theta/DEG_RAD);
                short dy = gridR*sinf(theta/DEG_RAD);
                point1.setX(centerX);
                point1.setY(centerY);
                point2.setX( centerX+dx);
                point2.setY(centerY+dy);
                p->drawLine(point1,point2);
                point2.setX(centerX+dx*1.02-9);
                point2.setY(centerY+dy*1.02+5);
                if(theta<270)p->drawText(point2,QString::number(theta+90));
                else p->drawText(point2,QString::number(theta-270));

        }

        //end grid



}

void Mainwindow::initGraphicView()
{
    //scene = new QGraphicsScene(-200, -200, 400, 400);
    //view = new jViewPort(scene,this);
    //view->setGeometry(SCR_LEFT_MARGIN,0,SCR_H,SCR_H);
    //view->lower();
    //view->setRenderHint(QPainter::Antialiasing);
    //view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    //view->setBackgroundBrush(Qt::transparent);

}

void Mainwindow::DrawRadarTargetByPainter(QPainter* p)//draw radar target from processing->radarData->mTrackList
{

    QPen penTarget(Qt::magenta);
    penTarget.setWidth(2);

    QPen penSelTarget(Qt::magenta);
    penSelTarget.setWidth(2);
    penSelTarget.setStyle(Qt::DashLine);
    QPen penTargetBlue(Qt::cyan);
    penTargetBlue.setWidth(2);
    //penTargetBlue.setStyle(Qt::DashLine);
    //QPen penARPATrack(Qt::darkYellow);
    //draw radar targets
    //float x,y;
    short sx,sy;
    float scale_ppi = processing->radarData->scale_ppi;
    //short targetId = 0;
    trackList* trackListPt = &processing->radarData->mTrackList;
    if(ui->toolButton_blue_tracks->isChecked())
    {
        for(uint trackId=0;trackId<trackListPt->size();trackId++)
        {
            if(!trackListPt->at(trackId).isConfirmed)continue;
            if(trackListPt->at(trackId).isManual)continue;
            //if(trackListPt->at(trackId).state<5)continue;
            sx = trackListPt->at(trackId).estX*scale_ppi + scrCtX - dx;
            sy = -trackListPt->at(trackId).estY*scale_ppi + scrCtY - dy;
            p->setPen(penTargetBlue);
            p->drawRect(sx-5,sy-5,10,10);
        }

    }

    //draw red targets
        for(uint trackId=0;trackId<trackListPt->size();trackId++)
        {
            if(!trackListPt->at(trackId).isManual)continue;
            if(!trackListPt->at(trackId).isLost)
            {
                //x= trackListPt->at(trackId).estX*scale_ppi/mScale;
                //y= trackListPt->at(trackId).estY*scale_ppi/mScale;
                sx = trackListPt->at(trackId).estX*scale_ppi + scrCtX - dx;
                sy = -trackListPt->at(trackId).estY*scale_ppi + scrCtY - dy;
                if(trackListPt->at(trackId).dopler==17)//diem dau dat bang tay
                {
                    p->setPen(penTargetBlue);
                    p->drawEllipse(sx-6,sy-6,12,12);
                    continue;
                }
                else if(trackListPt->at(trackId).isManual)
                {
                    p->setPen(penTarget);
                    //ve huong di chuyen
                    if(trackListPt->at(trackId).object_list.size()>12)
                    {
                        sx = trackListPt->at(trackId).estX*scale_ppi + scrCtX - dx;
                        sy =-trackListPt->at(trackId).estY*scale_ppi + scrCtY - dy;
                        p->drawLine(sx,sy,sx+15*sin(trackListPt->at(trackId).heading),sy-15*cos(trackListPt->at(trackId).heading));
                    }
                    //ve so hieu MT
                    p->drawText(sx+7,sy+7,300,40,0,QString::number(trackListPt->at(trackId).idCount));
                    //ve lich su qui dao
                    if(selectedTargetIndex==trackId)
                    {
                        for(uint j=0 ;j<trackListPt->at(trackId).object_list.size();j+=3)
                        {
                            sx = trackListPt->at(trackId).object_list.at(j).x*scale_ppi + scrCtX - dx;
                            sy = -trackListPt->at(trackId).object_list.at(j).y*scale_ppi + scrCtY - dy;
                            p->drawPoint(sx,sy);
                        }
                        p->drawRect(sx-6,sy-6,12,12);
                        p->setPen(penSelTarget);
                        p->drawRect(sx-9,sy-9,18,18);
                    }
                    else
                    {
                        p->drawRect(sx-6,sy-6,12,12);

                    }
                    continue;
                }
            }
//                draw track:

//                j--;
//                if(j<0)continue;
                //printf("red");
                /*if(trackListPt->at(trackId).confirmed)
                {
                    p->setPen(penTargetRed);

                }
                else
                {
                    p->setPen(penTargetBlue);
                }
                p->drawRect(x-6,y-6,12,12);
                p->drawText(x+5,y+5,300,40,0,QString::number(trackListPt->at(trackId).state)
                            + "-" + QString::number(trackListPt->at(trackId).terrain)
                            + "-" + QString::number(trackListPt->at(trackId).dopler),0);*/
                /*if(false)//trackListPt->at(i).isMoving) // moving obj
                {

                    QPolygon poly;
                    QPoint   point,point2;
                    point2.setX(x+trackListPt->at(i).velocity*500*sinf(trackListPt->at(i).course));
                    point2.setY(y-trackListPt->at(i).velocity*500*cosf(trackListPt->at(i).course));

                    point.setX(x+10*sinf(trackListPt->at(i).course));
                    point.setY(y-10*cosf(trackListPt->at(i).course));
                    p->setPen(penTargetBlue);
                    p->drawLine(point,point2);
                    poly<<point;
                    point.setX(x+10*sinf(trackListPt->at(i).course+2.3562));
                    point.setY(y-10*cosf(trackListPt->at(i).course+2.3562));
                    poly<<point;
                    point.setX(x);
                    point.setY(y);
                    poly<<point;
                    point.setX(x+10*sinf(trackListPt->at(i).course-2.3562));
                    point.setY(y-10*cosf(trackListPt->at(i).course-2.3562));
                    poly<<point;
                    point.setX(x+10*sinf(trackListPt->at(i).course));
                    point.setY(y-10*cosf(trackListPt->at(i).course));
                    poly<<point;
                    p->setPen(penTargetRed);
                    p->drawPolygon(poly);
                    p->drawText(x-30,y-20,100,40,0,QString::number(i+1),0);
                }else*/

            /*}
            else if(trackListPt->at(i).tclass==BLUE_OBJ)
            {
                p->setPen(penTargetBlue);
                //printf("b");
                p->drawRect(x-6,y-6,12,12);
                p->drawText(x-30,y-20,100,40,0,QString::number(i+1),0);
                p->drawLine(x,
                            y,
                            x+trackListPt->at(i).velocity*500*sinf(trackListPt->at(i).course),
                            y-trackListPt->at(i).velocity*500*cosf(trackListPt->at(i).course));

            }*/

    }
    /*else for(uint i=0;i<trackListPt->size();i++)
    {
        if(!trackListPt->at(i).state)continue;
        short x,y;
        p->setPen(penTrack);
        short j;
        //draw track:
        for(j=0;j<((short)trackListPt->at(i).object_list.size());j++)
        {
            x = (trackListPt->at(i).object_list[j].x + RADAR_MAX_RESOLUTION)*signsize - (RADAR_MAX_RESOLUTION*signsize-scrCtX)-dx;
            y = (RADAR_MAX_RESOLUTION - trackListPt->at(i).object_list[j].y)*signsize - (RADAR_MAX_RESOLUTION*signsize-scrCtY)-dy;
            p->drawPoint(x,y);
        }
        j--;
        if(j<0)continue;


            if(trackListPt->at(i).isMoving) // moving obj
            {

                QPolygon poly;
                QPoint   point,point2;
                point2.setX(x+trackListPt->at(i).velocity*500*sinf(trackListPt->at(i).course));
                point2.setY(y-trackListPt->at(i).velocity*500*cosf(trackListPt->at(i).course));

                point.setX(x+10*sinf(trackListPt->at(i).course));
                point.setY(y-10*cosf(trackListPt->at(i).course));
                p->setPen(penTargetSub);
                p->drawLine(point,point2);
                poly<<point;
                point.setX(x+10*sinf(trackListPt->at(i).course+2.3562));
                point.setY(y-10*cosf(trackListPt->at(i).course+2.3562));
                poly<<point;
                point.setX(x);
                point.setY(y);
                poly<<point;
                point.setX(x+10*sinf(trackListPt->at(i).course-2.3562));
                point.setY(y-10*cosf(trackListPt->at(i).course-2.3562));
                poly<<point;
                point.setX(x+10*sinf(trackListPt->at(i).course));
                point.setY(y-10*cosf(trackListPt->at(i).course));
                poly<<point;
                p->setPen(penTarget);
                p->drawPolygon(poly);
                p->drawText(x-30,y-20,100,40,0,QString::number(i+1),0);
            }else
            {
                p->setPen(penTarget);
                p->drawRect(x-6,y-6,12,12);
                p->drawText(x-30,y-20,100,40,0,QString::number(i+1),0);
            }


    }*/
    //draw arpa targets
    /*
    for(uint i=0;i<processing->arpaData->track_list.size();i++)
    {
        short x,y;
        if(!processing->arpaData->track_list[i].lives)continue;
        for(uint j=0;j<(processing->arpaData->track_list[i].object_list.size());j++)
        {
            x = processing->arpaData->track_list[i].object_list[j].centerX*scale+(scrCtX-dx);
            y = processing->arpaData->track_list[i].object_list[j].centerY*scale+(scrCtY-dy);
            //printf("\n x:%d y:%d",x,y);
            p->setPen(penARPATrack);
            p->drawPoint(x,y);
        }
        QPolygon poly;
        QPoint point;

        point.setX(x+10*sinf(processing->arpaData->track_list[i].course));
        point.setY(y-10*cosf(processing->arpaData->track_list[i].course));
        poly<<point;
        point.setX(x+10*sinf(processing->arpaData->track_list[i].course+2.3562f));
        point.setY(y-10*cosf(processing->arpaData->track_list[i].course+2.3562f));
        poly<<point;
        point.setX(x);
        point.setY(y);
        poly<<point;
        point.setX(x+10*sinf(processing->arpaData->track_list[i].course-2.3562f));
        point.setY(y-10*cosf(processing->arpaData->track_list[i].course-2.3562f));
        poly<<point;
        /*if(processing->arpaData->track_list[i].selected)
        {
            char buf[50];
            p.setPen(penyellow);
            sprintf(buf, "%3d:%3.3fNM:%3.3f\260",processing->arpaData->track_list[i].id,processing->arpaData->track_list[i].centerR/DEFAULT_NM, processing->arpaData->track_list[i].centerA*57.2957795);
            QString info = QString::fromAscii(buf);
            p.drawText(10,infoPosy,150,20,0,info);
            infoPosy+=20;
            if(processing->arpaData->track_list[i].id==curTargetId)
            {
                p.setPen(penyellow);
                p.setBrush(Qt::red);
            }
                else
            {
                p.setPen(penTarget);
                p.setBrush(Qt::red);
            }

        }else
        {

            p.setPen(pensubtaget);
            p.setBrush(QColor(100,100,50,100));
        }
        p->setPen(penARPATarget);
        //p->setBrush(Qt::red);
        p->drawPolygon(poly);
        p->drawText(x-20,y-20,20,40,0,QString::number(processing->arpaData->track_list[i].id),0);

    }*/

}
void Mainwindow::ConvWGSToKm(double* x, double *y, double m_Long,double m_Lat)
{
    double mCenterLat = config.getLat();
    double mCenterLon = config.getLon();
    double refLat = (mCenterLat + (m_Lat))*0.00872664625997;//pi/360
    *x	= (((m_Long) - mCenterLon) * 111.31949079327357)*cos(refLat);// 3.14159265358979324/180.0*6378.137);//deg*pi/180*rEarth
    *y	= ((mCenterLat - (m_Lat)) * 111.132954);
    //tinh toa do xy KM so voi diem center khi biet lat-lon
}
void Mainwindow::ConvKmToWGS(double x, double y, double *m_Long, double *m_Lat)
{
    double mCenterLat = config.getLat();
    double mCenterLon = config.getLon();
    *m_Lat  = mCenterLat +  (y)/(111.132954);
    double refLat = (mCenterLat +(*m_Lat))*0.00872664625997;//3.14159265358979324/180.0/2;
    *m_Long = (x)/(111.31949079327357*cos(refLat))+ mCenterLon;
    //tinh toa do lat-lon khi biet xy km (truong hop coi trai dat hinh cau)
}
void Mainwindow::drawAisTarget(QPainter *p)
{
    //draw radar  target:
    QPen penTargetRed(QColor(255,50,150));
    penTargetRed.setWidth(0);
    for(uint i=0;i<m_AISList.size();i++)
    {
            p->setPen(penTargetRed);
//            short j;
            //draw track:
            double fx,fy;
//            float mlat = m_trackList.at(i).getLat();
//            mlat =  mlat/bit23* 180.0f ;
//            float mlon = m_trackList.at(i).mLong_double;
//            mlon = mlon/bit23* 180.0f ;
                ConvWGSToKm(&fx,&fy,m_AISList.at(i).getLon(),m_AISList.at(i).getLat());

                short x = (fx*mScale)+scrCtX-dx;
                short y = (fy*mScale)+scrCtY-dy;
                //draw ais mark
                QPolygon poly;
                QPoint point;
                float head = m_AISList.at(i).m_Head*PI_NHAN2/(1<<16);
                point.setX(x+8*sinf(head));
                point.setY(y-8*cosf(head));
                poly<<point;
                point.setX(x+8*sinf(head+2.3562f));
                point.setY(y-8*cosf(head+2.3562f));
                poly<<point;
                point.setX(x);
                point.setY(y);
                poly<<point;
                point.setX(x+8*sinf(head-2.3562f));
                point.setY(y-8*cosf(head-2.3562f));
                poly<<point;
                p->drawPolygon(poly);
                //draw ais name
                if(ui->toolButton_ais_name->isChecked())
                {
                    QFont font = p->font() ;
                    font.setPointSize(6);
                    p->setFont(font);
                    p->drawText(x+5,y+10,(m_AISList.at(i).m_szName));
                }
//                p->drawText(x+5,y+5,QString::fromAscii((char*)&m_trackList.at(i).m_MMSI[0],9));
                //printf("\nj:%d,%d,%d,%f,%f",j,x,y,arpa_data.ais_track_list[i].object_list[j].mlong,arpa_data.ais_track_list[i].object_list[j].mlat);

    }
}
void Mainwindow::UpdateMouseStat(QPainter *p)
{
    short   mx=this->mapFromGlobal(QCursor::pos()).x();
    short   my=this->mapFromGlobal(QCursor::pos()).y();
    if(!isInsideViewZone(mx,my))return;
    QPen penmousePointer(QColor(0x50ffffff));
    penmousePointer.setWidth(2);
    double azi,rg;
    short r = sqrt((mx - scrCtX+dx)*(mx - scrCtX+dx)+(my - scrCtY+dy)*(my - scrCtY+dy));
    p->setPen(penmousePointer);
    if(mouse_mode&MouseVRM)p->drawEllipse(QPoint(scrCtX-dx,scrCtY-dy),r,r);
    if(mouse_mode&MouseELB)p->drawLine(QPoint(scrCtX-dx,scrCtY-dy),QPoint(scrCtX-dx-(scrCtX-dx-mx)*100,scrCtY-dy-(scrCtY-dy-my)*100));
    if(ui->toolButton_measuring->isChecked())
    {
        C_radar_data::kmxyToPolarDeg((mx - mouseX)/mScale,-(my - mouseY)/mScale,&azi,&rg);

    }
    else
    {
        C_radar_data::kmxyToPolarDeg((mx - scrCtX+dx)/mScale,-(my - scrCtY+dy)/mScale,&azi,&rg);
    }

    ui->label_cursor_range->setText(QString::number(rg,'f',2)+"Nm");
    ui->label_cursor_azi->setText(QString::number((short)azi)+QString::fromLocal8Bit("\260")+QString::number((azi - (short)azi)*60,'f',2)+"'");
    ui->label_cursor_lat->setText(QString::number( (short)y2lat(-(my - scrCtY+dy)))+QString::fromLocal8Bit("\260")+
                                  QString::number(((float)y2lat(-(my - scrCtY+dy))-(short)(y2lat(-(my - scrCtY+dy))))*60,'f',2)+"'N");
    ui->label_cursor_long->setText(QString::number( (short)x2lon(mx - scrCtX+dx))+QString::fromLocal8Bit("\260")+
                                       QString::number(((float)x2lon(mx - scrCtX+dx)-(short)(x2lon(mx - scrCtX+dx)))*60,'f',2)+"'E");

    if(isSelectingTarget)
    {
        QPen penmousePointer(QColor(0x50ffffff));
        penmousePointer.setWidth(2);
        penmousePointer.setStyle(Qt::DashDotLine);
        p->setPen(penmousePointer);
        p->drawLine(selZone_x1,selZone_y1,mx,selZone_y1);
        p->drawLine(selZone_x1,selZone_y1,selZone_x1,my);
        p->drawLine(selZone_x1,my,mx,my);
        p->drawLine(mx,selZone_y1,mx,my);
    }
}
void Mainwindow::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter p(this);

    p.setRenderHint(QPainter::Antialiasing, true);

    if(pMap)
    {
        p.drawPixmap(scrCtX-scrCtY,0,height(),height(),
                         *pMap,
                         dxMap,dyMap,height(),height());
    }
    //draw signal
    DrawSignal(&p);

    DrawRadarTargetByPainter(&p);

    //if(ui->toolButton_ais_show->isChecked())drawAisTarget(&p);
    //draw cursor
//    QPen penmousePointer(QColor(0x50ffffff));

//    penmousePointer.setWidth(2);
//    p.setPen(penmousePointer);
//    p.drawLine(mousePointerX-15,mousePointerY,mousePointerX-10,mousePointerY);
//    p.drawLine(mousePointerX+15,mousePointerY,mousePointerX+10,mousePointerY);
//    p.drawLine(mousePointerX,mousePointerY-10,mousePointerX,mousePointerY-15);
//    p.drawLine(mousePointerX,mousePointerY+10,mousePointerX,mousePointerY+15);
    //draw mouse coordinates
    UpdateMouseStat(&p);
    if(ui->toolButton_ais_show->isChecked())drawAisTarget(&p);
    DrawViewFrame(&p);
    DrawZoomArea(&p);
//    updateTargets();
}
void Mainwindow::DrawZoomArea(QPainter* p)
{
    QRect rect = ui->tabWidget_2->geometry();
    rect.adjust(4,30,-5,-5);
    if(ui->tabWidget_2->currentIndex()==2)
    {

        if(config.getRangeView()>2)
        {
            short zoom_size = ui->tabWidget_2->width()/processing->radarData->scale_zoom_ppi*processing->radarData->scale_ppi;
            p->setPen(QPen(QColor(255,255,255,200),0,Qt::DashLine));
            p->drawRect(mousePointerX-zoom_size/2.0,mousePointerY-zoom_size/2.0,zoom_size,zoom_size);
        }

        p->setPen(QPen(Qt::black));
        p->setBrush(QBrush(Qt::black));
        p->drawRect(rect);
        p->drawImage(rect,*processing->radarData->img_zoom_ppi,processing->radarData->img_zoom_ppi->rect());

    }
    else if(ui->tabWidget_2->currentIndex()==3)
    {
        QRect rect = ui->tabWidget_2->geometry();

        p->setPen(QPen(Qt::black));
        p->setBrush(QBrush(Qt::black));
        p->drawRect(rect);
        p->drawImage(rect,*processing->radarData->img_histogram,
                    processing->radarData->img_histogram->rect());

    }
    else if(ui->tabWidget_2->currentIndex()==4)
    {


        p->setPen(QPen(Qt::black));
        p->setBrush(QBrush(Qt::black));
        p->drawRect(rect);
        p->drawImage(rect,*processing->radarData->img_spectre,
                    processing->radarData->img_spectre->rect());
    }
    else if(ui->tabWidget_2->currentIndex()==5)
    {
        if(ui->toolButton_scope->isChecked()==false)processing->radarData->drawRamp();
        QRect rect1 = rect;
        rect1.adjust(0,0,0,-rect.height()/2);
//        pengrid.setWidth(10);
//        p->setPen(pengrid);
         p->drawImage(rect1,*processing->radarData->img_RAmp);
         double rampos = ui->horizontalSlider_ramp_pos->value()/(double(ui->horizontalSlider_ramp_pos->maximum()));
         QRect rect2 = rect;
         rect2.adjust(0,rect.height()/2,0,0);
         int zoomw = rect2.width()/2;
         int ramposInt = (processing->radarData->img_RAmp->width()-zoomw)*rampos;
         QRect srect(ramposInt,0,zoomw,processing->radarData->img_RAmp->height());
         p->drawImage(rect2,*processing->radarData->img_RAmp,srect);
        //p->drawRect(rect1,processing->radarData->img_RAmp->width()+5,processing->radarData->img_RAmp->height()+5);
//        pengrid.setWidth(2);
//        pengrid.setColor(QColor(128,128,0,120));
//        p->setPen(pengrid);
//        for(short i=60;i<processing->radarData->img_RAmp->height();i+=50)
//        {
//            p->drawLine(0,height()-i,processing->radarData->img_RAmp->width()+5,height()-i);
//        }
//        for(short i=110;i<processing->radarData->img_RAmp->width();i+=100)
//        {
//            p->drawLine(i,height()-266,i,height());
//        }
    }
}
//void MainWindow::keyPressEvent(QKeyEvent *event)
//{
//    if(event->key() == Qt::Key_F1)
//    {
//    selectobject = true;
//    }
//    switch(event->key())
//    {
//    case Qt::Key_Alt:
//        if(ui->menuBar->isVisible())
//            ui->menuBar->hide();
//        else
//            ui->menuBar->show();
//        break;
//    default:
//        break;
//    }

//}


void Mainwindow::SaveBinFile()
{
    //vnmap.SaveBinFile();

}

void Mainwindow::InitSetting()
{
    //load openstreetmap
    osmap = new CMap();
    osmap->setCenterPos(config.getLat(),config.getLon());
    osmap->setImgSize(height(),height());
    osmap->SetType(0);
    //
    setMouseTracking(true);
    //initGraphicView();21.433170, 106.624043
    //init the guard zone
    gz1.isActive = 0;
    gz2.isActive = 0;
    gz3.isActive = 0;
    ui->groupBox_3->setCurrentIndex(0);
    ui->tabWidget_2->setCurrentIndex(2);
    QRect rec = QApplication::desktop()->screenGeometry(0);
    setFixedSize(SCR_W,SCR_H);
    if((rec.height()==SCR_H)&&(rec.width()==SCR_W))
    {
        this->showFullScreen();
        this->setGeometry(QApplication::desktop()->screenGeometry(0));//show on first screen
    }
    else
    {

        rec = QApplication::desktop()->screenGeometry(1);
        if((rec.height()==SCR_H)&&(rec.width()==SCR_W))
        {
            this->showFullScreen();
            //printf("error");
            this->setGeometry(QApplication::desktop()->screenGeometry(1));//show on second screen
            //setFixedSize(QApplication::desktop()->screenGeometry(1));
        }

    }

    dxMax = SCR_H/4-10;
    dyMax = SCR_H/4-10;
    mousePointerX = scrCtX = SCR_H/2 + SCR_LEFT_MARGIN;//+ ui->toolBar_Main->width()+20;//ENVDEP
    mousePointerY = scrCtY = SCR_H/2;
    UpdateScale();
    ui->textEdit_heading->setText(QString::number(config.getTrueN()));
    processing->radarData->setTrueN(config.getTrueN());
    //ui->horizontalSlider_2->setValue(config.m_config.cfarThresh);

    ui->horizontalSlider_brightness->setValue(ui->horizontalSlider_brightness->maximum()/4);
    ui->horizontalSlider_gain->setValue(ui->horizontalSlider_gain->maximum());
    ui->horizontalSlider_rain->setValue(ui->horizontalSlider_rain->minimum());
    ui->horizontalSlider_sea->setValue(ui->horizontalSlider_sea->minimum());
    //ui->comboBox_radar_resolution->setCurrentIndex(0);
    connect(ui->textEdit_heading, SIGNAL(returnPressed()),ui->toolButton_set_heading,SIGNAL(clicked()));
    connect(ui->lineEdit_byte_1, SIGNAL(returnPressed()),ui->toolButton_send_command,SIGNAL(clicked()));
    connect(ui->lineEdit_byte_2, SIGNAL(returnPressed()),ui->toolButton_send_command,SIGNAL(clicked()));
    connect(ui->lineEdit_byte_3, SIGNAL(returnPressed()),ui->toolButton_send_command,SIGNAL(clicked()));
    connect(ui->lineEdit_byte_4, SIGNAL(returnPressed()),ui->toolButton_send_command,SIGNAL(clicked()));
    connect(ui->lineEdit_byte_5, SIGNAL(returnPressed()),ui->toolButton_send_command,SIGNAL(clicked()));
    connect(ui->lineEdit_byte_6, SIGNAL(returnPressed()),ui->toolButton_send_command,SIGNAL(clicked()));
    connect(ui->lineEdit_byte_7, SIGNAL(returnPressed()),ui->toolButton_send_command,SIGNAL(clicked()));
    setCursor(QCursor(Qt::ArrowCursor));
    UpdateScale();

    SetGPS(config.getLat(), config.getLon());
    //vnmap.setUp(config.m_config.lat(), config.m_config.lon(), 200,config.m_config.mapFilename.data());
    if(pMap)delete pMap;
    pMap = new QPixmap(height(),height());
    DrawMap();


    update();
}
void Mainwindow::ReloadSetting()
{



}


void Mainwindow::DrawViewFrame(QPainter* p)
{
    //draw grid

    if(ui->toolButton_grid->isChecked())
    {

        if(ui->toolButton_measuring->isChecked())
        {
            DrawGrid(p,mouseX,mouseY);
        }
        else
        {
            DrawGrid(p,scrCtX-dx,scrCtY-dy);
        }
    }
    short d = height()-50;
    QPen penBackground(QColor(40,60,100,255));
    short linewidth = 0.6*height();
    penBackground.setWidth(linewidth/10);
    p->setPen(penBackground);
    for (short i=linewidth/12;i<linewidth;i+=linewidth/6)
    {
        p->drawEllipse(-i/2+(scrCtX-scrCtY)+25,-i/2+25,d+i,d+i);
    }
    penBackground.setWidth(0);
    p->setPen(penBackground);
    p->setBrush(QColor(40,60,100,255));
    p->drawRect(scrCtX+scrCtY,0,width()-scrCtX-scrCtY,height());
    p->drawRect(0,0,scrCtX-scrCtY,height());
    p->setBrush(Qt::NoBrush);

    QPen pengrid(QColor(255,255,50,255));
    pengrid.setWidth(4);
    p->setPen(pengrid);
    p->drawEllipse(scrCtX-scrCtY+25,25,d,d);
    pengrid.setWidth(2);
    p->setPen(pengrid);
    QFont font = p->font() ;
    font.setPointSize(8);
    p->setFont(font);
    //short theta;
    for(short theta=0;theta<360;theta+=10){
        QPoint point0,point1,point2;
        float tanA = tanf(theta/57.295779513f);
        float sinA = sinf(theta/57.295779513f);
        float cosA = cosf(theta/57.295779513f);
        float a = (1+1.0f/tanA/tanA);//4*(dy/tanA-dx)*(dy/tanA-dx) -4*(1+1/tanA)*(dx*dx+dy*dy-width()*width()/4);
        float b= 2.0f*(dy/tanA - dx);
        float c= dx*dx+dy*dy-d*d/4.0f;
        float delta = b*b-4.0f*a*c;
        if(delta<30.0f)continue;
        delta = sqrtf(delta);

        if(theta==0)
                {
                    point2.setX(scrCtX  - dx);
                    point2.setY(scrCtY - sqrt((d*d/4.0f- dx*dx)));
                    point1.setX(point2.x());
                    point1.setY(point2.y()-5.0);
                    point0.setX(point2.x());
                    point0.setY(point2.y()-18);
                }
        else if (theta<180)
        {
            short rx = (-b + delta)/2.0f/a;
            short ry = -rx/tanA;
            if(abs(rx)<100&&abs(ry)<100)continue;
            point2.setX(scrCtX + rx -dx);
            point2.setY(scrCtY + ry-dy);
            point1.setX(point2.x()+5.0*sinA);
            point1.setY(point2.y()-5.0*cosA);
            point0.setX(point2.x()+18.0*sinA);
            point0.setY(point2.y()-18.0*cosA);
        }
        else if(theta==180)
                {

                    point2.setX(scrCtX  - dx);
                    point2.setY(scrCtY + sqrt((d*d/4.0- dx*dx)));
                    point1.setX(point2.x());
                    point1.setY(point2.y()+5.0);
                    point0.setX(point2.x());
                    point0.setY(point2.y()+18.0);
                }
        else
        {
            short rx;
            short ry;
            rx =  (-b - delta)/2.0f/a;
            ry = -rx/tanA;
            if(abs(rx)<100&&abs(ry)<100)continue;
            point2.setX(scrCtX + rx - dx);
            point2.setY(scrCtY + ry - dy);
            point1.setX(point2.x()+5.0*sinA);
            point1.setY(point2.y()-5.0*cosA);
            point0.setX(point2.x()+18.0*sinA);
            point0.setY(point2.y()-18.0*cosA);
        }

        p->drawLine(point1,point2);
        /*if(theta%10==0)*/p->drawText(point0.x()-25,point0.y()-10,50,20,
                   Qt::AlignHCenter|Qt::AlignVCenter,
                   QString::number(theta));

    }


    //HDC dc = ui->tabWidget->getDC();
}
//void Mainwindow::setScaleNM(unsigned short rangeNM)
//{
//    float oldScale = mScale;
//    mScale = (float)height()/((float)rangeNM*CONST_NM)*0.7f;
//    //printf("scale:%f- %d",scale,rangeNM);
//    isScaleChanged = true;// scale*SIGNAL_RANGE_KM/2048.0f;

//    dyMax = MAX_VIEW_RANGE_KM*mScale;
//    dxMax = dyMax;
//    dx =short(mScale/oldScale*dx);
//    dy =short(mScale/oldScale*dy);
//    DrawMap();
//    /*currMaxRange = (sqrtf(dx*dx+dy*dy)+scrCtY)/signsize;
//    if(currMaxRange>RADAR_MAX_RESOLUTION)currMaxRange = RADAR_MAX_RESOLUTION;*/
////    isScreenUp2Date = false;
//}
short waittimer =0;
void Mainwindow::UpdateRadarData()
{
    if(!processing->getIsDrawn())
    {
        if(processing->radarData->isClkAdcChanged)
        {
            //ui->comboBox_radar_resolution->setCurrentIndex(processing->radarData->clk_adc);
            switch(processing->radarData->clk_adc)
            {
            case 0:
                ui->label_range_resolution->setText("15m");
                break;
            case 1:
                ui->label_range_resolution->setText("30m");
                break;
            case 2:
                ui->label_range_resolution->setText("60m");
                break;
            case 3:
                ui->label_range_resolution->setText("90m");
                break;
            case 4:
                ui->label_range_resolution->setText("120m");
                break;
            case 5:
                ui->label_range_resolution->setText("150m");
                break;
            case 6:
                ui->label_range_resolution->setText("180m");
                break;
            default:
                ui->label_range_resolution->setText("NA");
                break;

            }
            processing->radarData->setScalePPI(mScale);
            this->UpdateScale();
//            printf("\nsetScale:%d",processing->radarData->clk_adc);
            processing->radarData->isClkAdcChanged = false;
        }
        processing->radarData->redrawImg();

    }
    update();
    if(processing->isConnected())
        setRadarState(CONNECTED);
    else
        setRadarState(DISCONNECTED);




    /*QStandardItemModel* model = new QStandardItemModel(trackListPt->size(), 5);
    for (int row = 0; row < trackListPt->size(); ++row)
    {
       for (int column = 0; column < 5; ++column)
       {
           QString text = QString('A' + row) + QString::number(column + 1);
           QStandardItem* item = new QStandardItem(text);
           model->setItem(row, column, item);
       }
    }
    ui->tableTargetList->setModel(model);*/
}
void Mainwindow::readBuffer()
{

}
void Mainwindow::InitTimer()
{


    t2 = new QThread();

    processing = new dataProcessingThread();

    connect(&syncTimer1s, SIGNAL(timeout()), this, SLOT(sync1S()));
    syncTimer1s.start(1000);
    connect(&syncTimer5p, SIGNAL(timeout()), this, SLOT(sync5p()));
    syncTimer5p.start(300000);
    //syncTimer1s.moveToThread(t);



    connect(&scrUpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateRadarData()));
    scrUpdateTimer.start(40);//ENVDEP
    scrUpdateTimer.moveToThread(t2);

    connect(this,SIGNAL(destroyed()),processing,SLOT(deleteLater()));
    connect(&dataPlaybackTimer,SIGNAL(timeout()),processing,SLOT(playbackRadarData()));

    connect(t2,SIGNAL(finished()),t2,SLOT(deleteLater()));
    processing->start(QThread::TimeCriticalPriority);
    t2->start(QThread::IdlePriority);
}
void Mainwindow::InitNetwork()
{
        m_udpSocket = new QUdpSocket(this);
        if(!m_udpSocket->bind(8900))
        {
            if(!m_udpSocket->bind(8901))
            {
                m_udpSocket->bind(8902);
            }
        }
        m_udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 10);

    connect(m_udpSocket, SIGNAL(readyRead()),
            this, SLOT(processARPA()));

}
void Mainwindow::processARPA()
{

    while (m_udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(datagram.data(), datagram.size());
        //printf(datagram.data());
		QString str(datagram.data());
        QStringList list = str.split(",");
        short dataStart = 0;
        for(short i=0;i<list.size()-5;i++)
        {

            if(list.at(i).contains("RATTM"))
            {
    //            short tNum = (*(list.begin()+1)).toInt();
    //            float tDistance = (*(list.begin()+2)).toFloat();
    //            float tRange = (*(list.begin()+3)).toFloat();
    //            arpa_data.adde(tNum,tDistance,tRange);
            }
            else if(list.at(i).contains("AI"))
            {
                ProcDataAIS((BYTE*)(datagram.data()+ dataStart), datagram.size() - dataStart);
            }
            dataStart+= list.at(i).size();
        }

    }

}
void Mainwindow::processFrame()
{
//    while (udpSocket->hasPendingDatagrams()) {
//        unsigned short len = udpSocket->pendingDatagramSize();
//        QByteArray buff;
//        buff.resize(len);
//        udpSocket->readDatagram(buff.data(), len);
//        if((len==1422))//hr2d
//        {
//            ProcHR(&buff);
//        }
//    }
}




//void MainWindow::on_pauseButton_clicked()
//{
//    if(playbackTimer.isActive()){
//        playbackTimer.stop();
//        //ui->pauseButton->setText("Start");
//    }else
//    {
//        playbackTimer.start(10);
//        //ui->pauseButton->setText("Stop");
//    }
//}



//void MainWindow::on_comboBoxViewMode_currentIndexChanged(int index)
//{
//    viewMode=index;
//}
/*
void MainWindow::CameraControl(int x,int y, int zoom)
{
    char* sendBuff = new char[25];
    sprintf(sendBuff,"PTZSET %05d %05d %05d", x, y, zoom);
    udpSocket->writeDatagram(sendBuff,24,QHostAddress("127.0.0.1"),1989);
    delete[] sendBuff;
}
void MainWindow::CameraControl(int direction)
{
    char* sendBuff = new char[12];
    switch(direction)
    {
    case 1:

        sprintf(sendBuff,"PTZMOV IN  ");
        udpSocket->writeDatagram(sendBuff,11,QHostAddress("127.0.0.1"),1989);

        break;
    case 2:
        sprintf(sendBuff,"PTZMOV OUT ");
        udpSocket->writeDatagram(sendBuff,11,QHostAddress("127.0.0.1"),1989);
        break;
    case 3:
        sprintf(sendBuff,"PTZMOV LEFT");
        udpSocket->writeDatagram(sendBuff,11,QHostAddress("127.0.0.1"),1989);
        break;
    case 4:
        sprintf(sendBuff,"PTZMOV RGHT");
        udpSocket->writeDatagram(sendBuff,11,QHostAddress("127.0.0.1"),1989);
        break;
    default:
        break;
    }
    delete[] sendBuff;
}
*/


/*
void MainWindow::sendFrame(const char* hexdata,QHostAddress host,int port )
{
    short len = strlen(hexdata)/2+1;
    unsigned char* sendBuff = new unsigned char[len];
    hex2bin(hexdata,sendBuff);
    udpSendSocket->writeDatagram((char*)sendBuff,len-1,host,port);
    delete[] sendBuff;
}
*/
void Mainwindow::on_actionExit_triggered()
{
//    OnExitDialog *dlg = new OnExitDialog(this);
//    dlg->setModal(true);
//    dlg->setAttribute(Qt::WA_DeleteOnClose);
//    //dlg->setWindowFlags(Qt::WindowMinMaxButtonsHint);
//    dlg->show();

//    connect(dlg, SIGNAL(accepted()),this, SLOT(ExitProgram()));
//    //
    processing->stopThread();
    processing->wait();
    ExitProgram();
}
void Mainwindow::ExitProgram()
{
    config.SaveToFile();
    QApplication::quit();
#ifdef _WIN32
    QProcess::startDetached("shutdown -s -f -t 0");
#else
    //system("/sbin/halt -p");
#endif
}

void Mainwindow::on_actionConnect_triggered()
{

}
void Mainwindow::sync5p()//period 10 second
{

    if(radar_state!=DISCONNECTED)
    {
        QFile logFile;
        QDateTime now = QDateTime::currentDateTime();
        if(!QDir("C:\\logs\\"+now.toString("\\dd.MM\\")).exists())
        {
            QDir().mkdir("C:\\logs\\"+now.toString("\\dd.MM\\"));
        }
        logFile.setFileName("C:\\logs\\"+now.toString("\\dd.MM\\")+now.toString("dd.MM-hh.mm.ss")+"_radar_online.log");
        logFile.open(QIODevice::WriteOnly);

        logFile.close();

    }

}
void Mainwindow::updateTargetInfo()
{
    if(selectedTargetType==RADAR)
    {
        trackList* trackListPt = &processing->radarData->mTrackList;
        for(uint trackId=0;trackId<trackListPt->size();trackId++)
        {

            if(!trackListPt->at(trackId).isConfirmed)continue;
            if(selectedTargetIndex == trackId)
            {
                //printf("\ntrackId:%d",trackId);
                double mLat,mLon;
                this->ConvKmToWGS(trackListPt->at(trackId).estX*processing->radarData->scale_ppi/mScale,trackListPt->at(trackId).estY*processing->radarData->scale_ppi/mScale,&mLon,&mLat);
                ui->label_data_id->setText(QString::number(trackListPt->at(trackId).idCount));
                float tmpazi = trackListPt->at(trackId).estA*DEG_RAD;
                if(tmpazi<0)tmpazi+=360;
                ui->label_data_type->setText("Radar");
                ui->label_data_range->setText(QString::number(trackListPt->at(trackId).estR*processing->radarData->scale_ppi/mScale/1.852f,'f',2)+"Nm");
                ui->label_data_azi->setText( QString::number(tmpazi,'f',2)+QString::fromLocal8Bit("\260"));
                ui->label_data_lat->setText( QString::number((short)mLat)+QString::fromLocal8Bit("\260")+QString::number((mLat-(short)mLat)*60,'f',2)+"'N");
                ui->label_data_long->setText(QString::number((short)mLon)+QString::fromLocal8Bit("\260")+QString::number((mLon-(short)mLon)*60,'f',2)+"'E");
                ui->label_data_speed->setText(QString::number(trackListPt->at(trackId).speed,'f',2)+"Kn");
                ui->label_data_heading->setText(QString::number(trackListPt->at(trackId).heading*DEG_RAD)+QString::fromLocal8Bit("\260"));
                ui->label_data_dopler->setText(QString::number(trackListPt->at(trackId).dopler));
            }
        }

    }
    else if(selectedTargetType == AIS){

    C2_Track *selectedTrack = &m_AISList.at(selectedTargetIndex);
    double azi,rg;
    double fx,fy;
    ConvWGSToKm(&fx,&fy,selectedTrack->getLon(),selectedTrack->getLat());
    C_radar_data::kmxyToPolarDeg(fx,fy,&azi,&rg);
    ui->label_data_id->setText(QString::fromUtf8((char*)(&selectedTrack->m_MMSI),9));
    ui->label_data_range->setText(QString::number(rg,'f',2));
    ui->label_data_azi->setText(QString::number(azi,'f',2));
    ui->label_data_type->setText("AIS");
    ui->label_data_lat->setText( QString::number((short)selectedTrack->getLat())+QString::fromLocal8Bit("\260")+QString::number((selectedTrack->getLat()-(short)selectedTrack->getLat())*60,'f',2)+"N");
    ui->label_data_long->setText(QString::number((short)selectedTrack->getLon())+QString::fromLocal8Bit("\260")+QString::number((selectedTrack->getLon()-(short)selectedTrack->getLon())*60,'f',2)+"E");
    ui->label_data_speed->setText(QString::number(selectedTrack->m_Speed,'f',2)+"Kn");
    ui->label_data_heading->setText(QString::number(selectedTrack->getHead()*DEG_RAD)+QString::fromLocal8Bit("\260"));
    }
    else if(selectedTargetType==NOTARGET)
    {
        ui->label_data_id->setText("--");
        ui->label_data_type->setText("--");
        ui->label_data_range->setText("--");
        ui->label_data_azi->setText( "--");
        ui->label_data_lat->setText( "--");
        ui->label_data_long->setText("--");
        ui->label_data_speed->setText("--");
        ui->label_data_heading->setText("--");
        ui->label_data_dopler->setText("--");
    }
}
void Mainwindow::sync1S()//period 1 second
{
    // display radar temperature:
    ui->label_temp->setText(QString::number(processing->radarData->tempType)+"|"+QString::number(processing->radarData->temp,'f',0)+ QString::fromLocal8Bit("\260 C"));
    this->updateTargetInfo();

//    int n = 32*256.0f/((processing->radarData->noise_level[0]*256 + processing->radarData->noise_level[1]));
//    int m = 256.0f*((processing->radarData->noise_level[2]*256 + processing->radarData->noise_level[3]))
//            /((processing->radarData->noise_level[4]*256 + processing->radarData->noise_level[5]));


    //display target list:
    /*for(uint i=0;i<trackListPt->size();i++)
    {
        if(trackListPt->at(i).state==0)
        {
            QList<QListWidgetItem *> items = (ui->listTargetWidget->findItems(QString::number(i+1),Qt::MatchStartsWith));
            if(items.size())delete items[0];
            continue;
        }
        QList<QListWidgetItem *> items = (ui->listTargetWidget->findItems(QString::number(i+1),Qt::MatchStartsWith));
        QString str;
        float targetSpeed = trackListPt->at(i).velocity*3600*signsize/scale/CONST_NM;//mile per hours
        // check track parameters

        if(targetSpeed>TARGET_MAX_SPEED)
        {
            //processing->radarData->deleteTrack(i);
            continue;
        }//
        if(trackListPt->at(i).tclass==RED_OBJ)
        {
            str.append(QString::number(i+1)+":");
            str.append(QString::number(trackListPt->at(i).estR*signsize/scale/CONST_NM,'f',2,3)+" | ");
            str.append(QString::number((short)(trackListPt->at(i).estA*57.2957795f),'f',2,3)+" | ");
            str.append(QString::number((short)(targetSpeed),'f',2,4)+" | ");
            str.append(QString::number((short)(trackListPt->at(i).course*57.2957795f),'f',2,3)+" | ");
            if(items.size())
            {
                (items[0])->setText(str);
            }else
            {
                ui->listTargetWidget->addItem(str);
            }
        }

    }*/

    if(isScaleChanged ) {

        processing->radarData->setScalePPI(mScale);
        isScaleChanged = false;
    }
    //update signal code:

    //display time
    showTime();
    if(radar_state!=DISCONNECTED)
    {
        processing->radRequestTemp(ui->comboBox_temp_type->currentIndex());
        //udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
    }
    QByteArray array(processing->radarData->getFeedback(), 8);
    switch(radar_state)
    {
    case DISCONNECTED:
        ui->label_status->setText(QString::fromUtf8("Chưa k. nối"));
        //ui->toolButton_tx->setEnabled(false);
//        ui->toolButton_scan->setEnabled(false);
        if(ui->label_command->isHidden())
        {
            ui->label_command->setText(QString::fromUtf8("Chưa kết nối radar"));
            ui->label_command->setHidden(false);
        }
        else
        {
            ui->label_command->setText(QString::fromUtf8("Chưa kết nối radar"));
            ui->label_command->setHidden(true);
        }
        m_udpSocket->writeDatagram("d",1,QHostAddress("127.0.0.1"),8001);
        break;
    case CONNECTED:
        //printf("\ns_tx");
        ui->label_status->setText(QString::number(processing->radarData->overload));
        ui->label_command->setHidden(false);

        ui->label_command->setText(QString(array.toHex()));
        m_udpSocket->writeDatagram("c",1,QHostAddress("127.0.0.1"),8001);
        break;
    default:
        break;
    }

    switch((processing->radarData->sn_stat>>8)&0x07)
    {
    case 4:
        ui->label_sn_type->setText("Ma DTTT");
        //ui->label_sn_param->setText(QString::number(32<<());
        if(((processing->radarData->sn_stat)&0x07)==0)
        {
            ui->label_sn_param->setText("32");
        }
        else if(((processing->radarData->sn_stat)&0x07)==1)
        {
            ui->label_sn_param->setText("48");
        }
        else if(((processing->radarData->sn_stat)&0x07)==2)
        {
            ui->label_sn_param->setText("64");
        }
        else if(((processing->radarData->sn_stat)&0x07)==3)
        {
            ui->label_sn_param->setText("96");
        }
        else if(((processing->radarData->sn_stat)&0x07)==4)
        {
            ui->label_sn_param->setText("128");
        }
        else if(((processing->radarData->sn_stat)&0x07)==5)
        {
            ui->label_sn_param->setText("192");

        }
        else
        {
            ui->label_sn_param->setText("256");
        }
        break;
    case 0:
        ui->label_sn_type->setText("Xung don");
        ui->label_sn_param->setText(QString::number((((processing->radarData->sn_stat)&0x07))));
        break;
    case 2:
        ui->label_sn_type->setText("Ma M");
        ui->label_sn_param->setText(QString::number((((processing->radarData->sn_stat)&0x07))));
        break;
    case 3:
        ui->label_sn_type->setText("Ma ngau nhien");
        ui->label_sn_param->setText(QString::number((((processing->radarData->sn_stat)&0x07))));
        break;
    case 1:
        ui->label_sn_type->setText("Ma baker");
        ui->label_sn_param->setText(QString::number((((processing->radarData->sn_stat)&0x07))));
        break;
    default:
        ui->label_sn_param->setText(QString::number(((processing->radarData->sn_stat)&0x07)));
        break;
    }
    switch((processing->radarData->rotation_speed)&0x07)
    {
    case 0:
        ui->label_speed->setText(QString::fromUtf8("Dừng quay"));break;
    case 1:
        ui->label_speed->setText("5 v/p");break;
    case 2:
        ui->label_speed->setText("8 v/p");break;
    case 3:
        ui->label_speed->setText("12 v/p");break;
    case 4:
        ui->label_speed->setText("15 v/p");break;
    case 5:
        ui->label_speed->setText("18 v/p");break;
    default:

        break;
    }
    ui->label_speed_2->setText(QString::number(processing->radarData->rotation_per_min)+"v/p");



}
void Mainwindow::setRadarState(radarSate radarState)
{

        radar_state = radarState;
        //display radar state

}

void Mainwindow::on_actionTx_On_triggered()
{
    //sendFrame("aaab030200000000", QHostAddress("192.168.0.44"),2573);
    //on_actionRotateStart_toggled(true);
//    Command_Control new_com;
//    new_com.bytes[0] = 0xaa;
//    new_com.bytes[1] = 0xab;
//    new_com.bytes[2] = 0x02;
//    new_com.bytes[3] = 0x01;
//    new_com.bytes[4] = 0x00;
//    new_com.bytes[5] = 0x00;
//    new_com.bytes[6] = 0x00;
//    new_com.bytes[7] = 0;//new_com.bytes[0]+new_com.bytes[1]+new_com.bytes[2]+new_com.bytes[3]+new_com.bytes[4]+new_com.bytes[5]+new_com.bytes[6];
//    command_queue.push(new_com);
//    new_com.bytes[0] = 0xaa;
//    new_com.bytes[1] = 0xab;
//    new_com.bytes[2] = 0x00;
//    new_com.bytes[3] = 0x01;
//    new_com.bytes[4] = 0x00;
//    new_com.bytes[5] = 0x00;
//    new_com.bytes[6] = 0x00;
//    new_com.bytes[7] = 0;//new_com.bytes[0]+new_com.bytes[1]+new_com.bytes[2]+new_com.bytes[3]+new_com.bytes[4]+new_com.bytes[5]+new_com.bytes[6];
//    command_queue.push(new_com);

}

void Mainwindow::on_actionTx_Off_triggered()
{
//    //on_actionRotateStart_toggled(false);
//    Command_Control new_com;
//    new_com.bytes[0] = 0xaa;
//    new_com.bytes[1] = 0xab;
//    new_com.bytes[2] = 0x00;
//    new_com.bytes[3] = 0x00;
//    new_com.bytes[4] = 0x00;
//    new_com.bytes[5] = 0x00;
//    new_com.bytes[6] = 0x00;
//    new_com.bytes[7] = 0;//new_com.bytes[0]+new_com.bytes[1]+new_com.bytes[2]+new_com.bytes[3]+new_com.bytes[4]+new_com.bytes[5]+new_com.bytes[6];
//    command_queue.push(new_com);
//    new_com.bytes[0] = 0xaa;
//    new_com.bytes[1] = 0xab;
//    new_com.bytes[2] = 0x02;
//    new_com.bytes[3] = 0x00;
//    new_com.bytes[4] = 0x00;
//    new_com.bytes[5] = 0x00;
//    new_com.bytes[6] = 0x00;
//    new_com.bytes[7] = 0;//new_com.bytes[0]+new_com.bytes[1]+new_com.bytes[2]+new_com.bytes[3]+new_com.bytes[4]+new_com.bytes[5]+new_com.bytes[6];
//    command_queue.push(new_com);
}

void Mainwindow::on_actionRecording_toggled(bool arg1)
{
    if(arg1)
    {
        QDateTime now = QDateTime::currentDateTime();
        processing->startRecord(now.toString("dd.MM-hh.mm.ss")+HR_FILE_EXTENSION);
    }
    else
    {        
        processing->stopRecord();
    }
}

void Mainwindow::on_actionOpen_rec_file_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,    tr("Open signal file"), NULL, tr("HR signal record files (*.r2d)"));
    if(!filename.size())return;
    processing->loadRecordDataFile(filename);
}



void Mainwindow::on_actionOpen_map_triggered()
{
    //openShpFile();
}
void Mainwindow::showTime()
{
    QDateTime time = QDateTime::currentDateTime();
    QString text = time.toString("hh:mm:ss");
    ui->label_date->setText(text);
    text = time.toString("dd/MM/yy");
    ui->label_time->setText(text);
}

void Mainwindow::on_actionSaveMap_triggered()
{
    //vnmap.SaveBinFile();
}

void Mainwindow::on_actionSetting_triggered()
{
//    GPSDialog *dlg = new GPSDialog(this);
//    dlg->setModal(false);
//    dlg->loadConfig(&config);
//    dlg->show();
//    dlg->setAttribute(Qt::WA_DeleteOnClose);
//    connect(dlg, SIGNAL(destroyed(QObject*)), SLOT(UpdateSetting()));
//    connect(dlg, SIGNAL(destroyed(QObject*)), SLOT(setCodeType()));
}
void Mainwindow::on_actionAddTarget_toggled(bool arg1)
{
    //isAddingTarget=arg1;

}




void Mainwindow::on_actionClear_data_triggered()
{
    processing->radarData->resetData();
//    isScreenUp2Date = false;
}

//void Mainwindow::on_actionView_grid_triggered(bool checked)
//{
//    gridOff = checked;
//    dx=0;dy=0;
//    DrawMap();
//    //UpdateSetting();
//}


void Mainwindow::on_actionPlayPause_toggled(bool arg1)
{
    processing->togglePlayPause(arg1);
    if(arg1)dataPlaybackTimer.start(25);else dataPlaybackTimer.stop();

}


/*
void MainWindow::on_pushButton_clicked()
{

    Command_Control new_com;
    hex2bin(ui->lineEdit_byte_1->text().toStdString().data(),&new_com.bytes[0]);
    hex2bin(ui->lineEdit_byte_2->text().toStdString().data(),&new_com.bytes[1]);
    hex2bin(ui->lineEdit_byte_3->text().toStdString().data(),&new_com.bytes[2]);
    hex2bin(ui->lineEdit_byte_4->text().toStdString().data(),&new_com.bytes[3]);
    hex2bin(ui->lineEdit_byte_5->text().toStdString().data(),&new_com.bytes[4]);
    hex2bin(ui->lineEdit_byte_6->text().toStdString().data(),&new_com.bytes[5]);
    hex2bin(ui->lineEdit_byte_7->text().toStdString().data(),&new_com.bytes[6]);
    hex2bin(ui->lineEdit_byte_8->text().toStdString().data(),&new_com.bytes[7]);
    command_queue.push(new_com);
}
*/

void Mainwindow::SendCommandControl()
{/*
      if(command_queue.size())
      {

          if(processing->radarData->checkFeedback(&command_queue.front().bytes[0]))// check if the radar has already recieved the command
          {


              command_queue.pop();
              udpFailure = 0;

          }
          else
          {
            if(udpFailure<20)//ENVDEP 20*50ms = 1s
            {udpFailure++;}
            else{
                setRadarState( DISCONNECTED);
                udpFailure = 0;
            }
            udpSendSocket->writeDatagram((char*)&command_queue.front().bytes[0],8,QHostAddress("192.168.0.44"),2572);
            //
            char xx[3];
            xx[2]=0;
            QString str;
            for(short i =0;i<8;i++)
            {
                bin2hex(command_queue.front().bytes[i],&xx[0]);
                str.append(xx);
                str.append('-');
            }

            ui->label_command->setText(str);
            //printf((const char*)str.data());
            //

          }

      }*/

}

void Mainwindow::on_actionRecording_triggered()
{

}


void Mainwindow::on_comboBox_temp_type_currentIndexChanged(int index)
{

 //!!!
}

//void RadarGui::on_horizontalSlider_brightness_actionTriggered(int action)
//{

//}

void Mainwindow::on_horizontalSlider_brightness_valueChanged(int value)
{
    processing->radarData->brightness = 0.5f+(float)value/ ui->horizontalSlider_brightness->maximum()*4.0f;
}

/*void MainWindow::on_horizontalSlider_3_valueChanged(int value)
{
    switch (value) {
    case 1:
        Command_Control new_com;
        new_com.bytes[0] = 4;
        new_com.bytes[1] = 0xab;
        new_com.bytes[2] = 0;
        new_com.bytes[3] = 0;
        new_com.bytes[4] = 1;
        new_com.bytes[5] = 0;
        new_com.bytes[6] = 0;
        new_com.bytes[7] = 0;//new_com.bytes[0]+new_com.bytes[1]+new_com.bytes[2]+new_com.bytes[3]+new_com.bytes[4]+new_com.bytes[5]+new_com.bytes[6];
        command_queue.push(new_com);
        break;
    case 2:
        printf("2");
        break;
    case 3:
        printf("3");
        break;
    default:
        break;
    }
}*/



//void MainWindow::on_toolButton_toggled(bool checked)
//{
//    //if(checked)ui->toolBar_Main->show();
//    //else ui->toolBar_Main->hide();
//}

void Mainwindow::on_actionSector_Select_triggered()
{

}


//void MainWindow::on_toolButton_10_clicked()
//{
//    //if(ui->frame_RadarViewOptions->isHidden())ui->frame_RadarViewOptions->show();
//    //else ui->frame_RadarViewOptions->hide();
//}




/*
void MainWindow::on_toolButton_14_clicked()
{
    //if(event->delta()>0)ui->horizontalSlider->setValue(ui->horizontalSlider->value()+1);
}

void MainWindow::on_toolButton_13_clicked()
{
    //if(event->delta()<0)ui->horizontalSlider->setValue(ui->horizontalSlider->value()-1);
}
*/
void Mainwindow::setScaleRange(double srange)
{
    mScale = (height()/2-5)/(CONST_NM*srange );
    rangeStep = srange/6.0f;
    ui->label_range->setText(QString::number(srange)+" NM");
    ui->toolButton_grid->setText(QString::fromUtf8("Lưới tọa độ(")+QString::number(rangeStep)+"NM)");
}
void Mainwindow::UpdateScale()
{
    float oldScale = mScale;
    //char byte2;
    switch(config.getRangeView())
    {
    case 0:
        setScaleRange(1.5);
        break;
    case 1:
        setScaleRange(3);
        break;
    case 2:
        setScaleRange(6);
        break;
    case 3:
        setScaleRange(12);
        break;
    case 4:
        setScaleRange(24);
        break;
    case 5:
        setScaleRange(36);
        break;
    case 6:
        setScaleRange(48);
        break;
    case 7:
        setScaleRange(72);
        break;
    case 8:
        setScaleRange(96);
        break;
    case 9:
        setScaleRange(120);
        break;
    default:
        setScaleRange(150);
        break;
    }


    isScaleChanged = true;
    short sdx = mousePointerX - scrCtX + dx;
    short sdy = mousePointerY - scrCtY + dy;
    sdx =(sdx*mScale/oldScale);
    sdy =(sdy*mScale/oldScale);
    mousePointerX = scrCtX+sdx-dx;
    mousePointerY = scrCtY+sdy-dy;
}




//void MainWindow::on_toolButton_10_toggled(bool checked)
//{

//}

//void MainWindow::on_actionRotateStart_toggled(bool arg1)
//{
//    if(arg1)
//    {
//        Command_Control new_com;
//        new_com.bytes[0] = 0xaa;
//        new_com.bytes[1] = 0xab;
//        new_com.bytes[2] = 0x03;
//        new_com.bytes[3] = 0x02;
//        new_com.bytes[4] = 0x00;
//        new_com.bytes[5] = 0x00;
//        new_com.bytes[6] = 0x00;
//        new_com.bytes[7] = 0;//new_com.bytes[0]+new_com.bytes[1]+new_com.bytes[2]+new_com.bytes[3]+new_com.bytes[4]+new_com.bytes[5]+new_com.bytes[6];
//        command_queue.push(new_com);
//    }
//    else
//    {

//        Command_Control new_com;
//        new_com.bytes[0] = 0xaa;
//        new_com.bytes[1] = 0xab;
//        new_com.bytes[2] = 0x03;
//        new_com.bytes[3] = 0x00;
//        new_com.bytes[4] = 0x00;
//        new_com.bytes[5] = 0x00;
//        new_com.bytes[6] = 0x00;
//        new_com.bytes[7] = 0;//new_com.bytes[0]+new_com.bytes[1]+new_com.bytes[2]+new_com.bytes[3]+new_com.bytes[4]+new_com.bytes[5]+new_com.bytes[6];
//        command_queue.push(new_com);
//    }
//}


//void MainWindow::on_comboBox_temp_type_2_currentIndexChanged(int index)
//{



//}

//void MainWindow::on_toolButton_11_toggled(bool checked)
//{


//}

//void MainWindow::on_pushButton_removeTarget_2_clicked()
//{

//}

//void MainWindow::on_pushButton_removeTarget_2_released()
//{
//    processing->radarData->resetTrack();
//}

//void MainWindow::on_pushButton_avtodetect_toggled(bool checked)
//{
//    isDrawSubTg = !checked;
//    processing->radarData->avtodetect = checked;
//    processing->radarData->terrain_init_time = 3;
//}


void Mainwindow::setCodeType(short index)// chuyen ma
{
    unsigned char bytes[8];
    bytes[0] = 1;
    bytes[1] = 0xab;

    //printf("\n code:%d",index);
    switch (index)
    {
    case 0://M32
        bytes[2] = 2;
        bytes[3] = 0;
        break;
    case 1://M64
        bytes[2] = 2;
        bytes[3] = 1;
        break;
    case 2://M128
        bytes[2] = 2;
        bytes[3] = 2;
        break;
    case 3://M255
        bytes[2] = 2;
        bytes[3] = 3;
        break;
    case 4://M32x2
        bytes[2] = 2;
        bytes[3] = 4;
        break;
    case 5://M64x2
        bytes[2] = 2;
        bytes[3] = 5;
        break;
    case 6://M128x2
        bytes[2] = 2;
        bytes[3] = 6;
        break;
    case 7://baker
        bytes[2] = 1;
        bytes[3] = 1;
        break;
    case 8://single pulse
        bytes[2] = 0;
        bytes[3] = 1;

        break;
    default:
        bytes[2] = 0;
        bytes[3] = 0;
        break;
    }
    bytes[4] = 0;
    bytes[5] = 0;
    bytes[6] = 0;
    bytes[7] = 0;//new_com.bytes[0]+new_com.bytes[1]+new_com.bytes[2]+new_com.bytes[3]+new_com.bytes[4]+new_com.bytes[5]+new_com.bytes[6];
    sendToRadar(&bytes[0]);

}
//void MainWindow::on_toolButton_4_toggled(bool checked)
//{
//    if(checked)
//    {
//        this->on_actionTx_On_triggered();
//    }
//    else
//    {
//        this->on_actionTx_Off_triggered();
//    }

//}



void Mainwindow::on_horizontalSlider_gain_valueChanged(int value)
{
    processing->radarData->kgain = 7-(float)value/(ui->horizontalSlider_gain->maximum())*10;
    ui->label_gain->setText("Gain:"+QString::number(-processing->radarData->kgain));
    //printf("processing->radarData->kgain %f \n",processing->radarData->kgain);
}

void Mainwindow::on_horizontalSlider_rain_valueChanged(int value)
{
    processing->radarData->krain = (float)value/(ui->horizontalSlider_rain->maximum()+ui->horizontalSlider_rain->maximum()/3);
    ui->label_rain->setText("Rain:" + QString::number(processing->radarData->krain,'f',2));
}

void Mainwindow::on_horizontalSlider_sea_valueChanged(int value)
{
    processing->radarData->ksea = (float)value/(ui->horizontalSlider_sea->maximum());
    //ui->label_rain->setText("Rain:" + QString::number(-processing->radarData->krain));
}


/*
void MainWindow::on_pushButton_loadAis_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,    QString::fromUtf8("M? file "), NULL, tr("ISM file (*.txt)"));
    if(!filename.size())return;
    QFile gpsfile( filename);
    if (!gpsfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return ;
    }
    QTextStream in(&gpsfile);
    QString line ;int k=0;
    line = in.readLine();

    while(!in.atEnd()) {
        //printf((char*)line.data());
        QStringList  list = line.split(",");

        if (list[0] == "$GPRMC")
        {

            float mlat = (*(list.begin()+3)).toFloat()/100.0f +0.0097;
            float mlong = (*(list.begin()+5)).toFloat()/100.0f + 0.355;
            arpa_data.addAIS(list[0].toStdString(),mlat,mlong,0,0);

        }line = in.readLine();
        k=list.size();
        //printf("size:%d",arpa_data.ais_track_list[0].id.data());
    }

}

*/


void Mainwindow::on_toolButton_exit_clicked()
{
    on_actionExit_triggered();
}

//void Mainwindow::on_toolButton_setting_clicked()
//{
//    this->on_actionSetting_triggered();
//}


void Mainwindow::on_toolButton_tx_toggled(bool checked)
{

//    if(checked)

//    {   //0xaa,0xab,0x00,0x01,0x00,0x00,0x00
//        unsigned char        bytes[8] = {0xaa,0xab,0x02,0x01,0x00,0x00,0x00};
//        udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
//        bytes[2] = 0x00;//{0xaa,0xab,0x00,0x01,0x00,0x00,0x00};
//        Sleep(100);
//        udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
//        //ui->toolButton_tx->setChecked(false);
//    }
//    else
//    {

//        unsigned char        bytes[8] = {0xaa,0xab,0x02,0x00,0x00,0x00,0x00};
//        udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
//        bytes[2] = 0x00;// = {0xaa,0xab,0x00,0x01,0x00,0x00,0x00};
//        Sleep(100);
//        udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
//        //ui->toolButton_tx->setChecked(true);
//    }

}



void Mainwindow::on_toolButton_xl_nguong_toggled(bool checked)
{
    processing->radarData->setAutorgs(checked);
    if(checked)
    {
        ui->horizontalSlider_gain->setVisible(false);
        ui->horizontalSlider_rain->setVisible(false);
        ui->horizontalSlider_sea->setVisible(false);
    }
    else
    {
        ui->horizontalSlider_gain->setVisible(true);
        ui->horizontalSlider_rain->setVisible(true);
        ui->horizontalSlider_sea->setVisible(true);
    }
}

void Mainwindow::on_toolButton_replay_toggled(bool checked)
{
    this->on_actionPlayPause_toggled(checked);
}


void Mainwindow::on_toolButton_replay_fast_toggled(bool checked)
{
    if(checked)
    {
        processing->playRate = 300;
    }else
    {
        processing->playRate = 80;
    }
}

void Mainwindow::on_toolButton_record_toggled(bool checked)
{
    this->on_actionRecording_toggled(checked);
}

void Mainwindow::on_toolButton_open_record_clicked()
{
    this->on_actionOpen_rec_file_triggered();
}



//void Mainwindow::on_toolButton_alphaView_toggled(bool checked)
//{
//    displayAlpha = checked;
//    processing->radarData->isDisplayAlpha = checked;
//}

/*
void Mainwindow::updateTargets()
{
    trackList* trackListPt = &processing->radarData->mTrackList;

    for(short i = 0;i<targetDisplayList.size();i++)
    {
        if(!targetDisplayList.at(i)->isUsed)
        {
            continue;

            targetDisplayList.at(i)->hide();

        }
        if(trackListPt->at(targetDisplayList.at(i)->trackId).isManual == 0)
        {
            targetDisplayList.at(i)->isUsed = false;
            ui->label_status_warning->setText(QString::fromUtf8("Mất MT số:")+QString::number(i+1));
            warningList.append(QString::fromUtf8("Mất MT số:")+QString::number(i+1));
            ui->label_status_warning->setStyleSheet("background-color: rgb(255, 150, 50,255);");
            targetDisplayList.at(i)->hide();
            //targetList.at(i)->isLost=true;
            continue;
        }
        float x	= targetDisplayList.at(i)->x*mScale + scrCtX-dx ;
        float y	= -targetDisplayList.at(i)->y*mScale + scrCtY-dy ;
        float w = scrCtY-30;
        float dx = x-scrCtX;
        float dy = y-scrCtY;
        if(dx*dx+dy*dy>(w*w))
        {
            targetDisplayList.at(i)->hide();
        }
        else
        {
            targetDisplayList.at(i)->show();
            targetDisplayList.at(i)->setScrPos(x,y);
        }

        if(targetDisplayList.at(i)->clicked)
        {

            selected_target_index = i;
            targetDisplayList.at(i)->setSelected(true);
            targetDisplayList.at(i)->clicked = false;
        }
        if(targetDisplayList.at(i)->doubleClicked)
        {

            selected_target_index = i;
            trackListPt->at((targetDisplayList.at(i)->trackId)).isManual = true;
            targetDisplayList.at(i)->isManual = true;
            targetDisplayList.at(i)->doubleClicked = false;
        }
        if(selected_target_index == i)
        {
            float tmpazi = trackListPt->at(targetDisplayList.at(i)->trackId).estA*DEG_RAD;
            if(tmpazi<0)tmpazi+=360;
            ui->label_data_id->setText(QString::number(i+1));
            ui->label_data_type->setText("Radar");
            ui->label_data_range->setText(QString::number(trackListPt->at(targetDisplayList.at(i)->trackId).estR*processing->radarData->scale_ppi/mScale/1.852f,'f',2)+"Nm");
            ui->label_data_azi->setText( QString::number(tmpazi,'f',2)+QString::fromLocal8Bit("\260"));
            ui->label_data_lat->setText( QString::number((short)targetDisplayList.at(i)->m_lat)+QString::fromLocal8Bit("\260")+QString::number((targetDisplayList.at(i)->m_lat-(short)targetDisplayList.at(i)->m_lat)*60,'f',2)+"'N");
            ui->label_data_long->setText(QString::number((short)targetDisplayList.at(i)->m_lon)+QString::fromLocal8Bit("\260")+QString::number((targetDisplayList.at(i)->m_lon-(short)targetDisplayList.at(i)->m_lon)*60,'f',2)+"'E");
            ui->label_data_speed->setText(QString::number(trackListPt->at(targetDisplayList.at(i)->trackId).speed,'f',2)+"Kn");
            ui->label_data_heading->setText(QString::number(trackListPt->at(targetDisplayList.at(i)->trackId).head_r*DEG_RAD)+QString::fromLocal8Bit("\260"));
            ui->label_data_dopler->setText(QString::number(trackListPt->at(targetDisplayList.at(i)->trackId).dopler));
        }
        else
        {
            targetDisplayList.at(i)->setSelected(false);// = false;
        }
        //printf("\nx:%f y:%f",x,y);
    }
    //ui->
    //t1.setGeometry(400,400,20,20);
    //targetList.append(t1);
}
*/
void Mainwindow::on_toolButton_centerView_clicked()
{
    dx = 0;
    dy = 0;
    DrawMap();
//    isScreenUp2Date = false;
}

void Mainwindow::on_comboBox_currentIndexChanged(int index)
{
    switch (index)
    {
    case 0:
        processing->radarData->dataOver = m_only;
        break;
    case 1:
        processing->radarData->dataOver = s_m_150;
        break;
    case 2:
        processing->radarData->dataOver = s_m_200;
        break;
    case 3:
        processing->radarData->dataOver = max_s_m_150;
        break;
    case 4:
        processing->radarData->dataOver = max_s_m_200;
        break;
    default:
        break;
    }

}

void Mainwindow::on_comboBox_img_mode_currentIndexChanged(int index)
{
    processing->radarData->imgMode = imgDrawMode(index) ;
}


void Mainwindow::on_toolButton_send_command_clicked()
{
    if(ui->lineEdit_password->text()=="ccndt3108")
    {
        unsigned char        bytes[8];
        hex2bin(ui->lineEdit_byte_1->text().toStdString().data(),&bytes[0]);
        hex2bin(ui->lineEdit_byte_2->text().toStdString().data(),&bytes[1]);
        hex2bin(ui->lineEdit_byte_3->text().toStdString().data(),&bytes[2]);
        hex2bin(ui->lineEdit_byte_4->text().toStdString().data(),&bytes[3]);
        hex2bin(ui->lineEdit_byte_5->text().toStdString().data(),&bytes[4]);
        hex2bin(ui->lineEdit_byte_6->text().toStdString().data(),&bytes[5]);
        hex2bin(ui->lineEdit_byte_7->text().toStdString().data(),&bytes[6]);
        bytes[7] = bytes[0]+bytes[1]+bytes[2]+bytes[3]+bytes[4]+bytes[5]+bytes[6];
        ui->lineEdit_byte_8->setText(QString::number(bytes[7]));
        //hex2bin(ui->lineEdit_byte_8->text().toStdString().data(),&bytes[7]);
        sendToRadar((unsigned char*)&bytes[0]);
        //udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
    }
}


void Mainwindow::on_toolButton_zoom_in_clicked()
{
    if(config.getRangeView()>0)config.setRangeView( config.getRangeView()-1);
    UpdateScale();
    DrawMap();
}

void Mainwindow::on_toolButton_zoom_out_clicked()
{
    if(config.getRangeView()<9)config.setRangeView( config.getRangeView()+1);
    UpdateScale();
    DrawMap();
}

//void Mainwindow::on_toolButton_reset_clicked()
//{
//    processing->radarData->resetSled();
//}

//void Mainwindow::on_toolButton_send_command_2_clicked()
//{
//    unsigned char        bytes[8] = {0xaa,0xab,0x02,0x02,0x0a,0,0,0};
//    udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
////    bytes[0] = 0xaa;
////    bytes[1] = 0xab;
////    bytes[2] = 0x02;
////    bytes[3] = 0x02;
////    bytes[4] = 0x0a;
////    bytes[5] = 0x00;
////    bytes[6] = 0x00;
////    bytes[7] = 0x00;

//}

void Mainwindow::SetGPS(double mlat,double mlong)
{
    config.setLat(mlat);
    config.setLon(mlong);
    ui->text_latInput_2->setText(QString::number(mlat,'g',10));
    ui->text_longInput_2->setText(QString::number(mlong,'g',10));
    osmap->setCenterPos(config.getLat(), config.getLon());
    DrawMap();
    update();
}

//void Mainwindow::on_dial_valueChanged(int value)
//{
//    float heading = value/100.0f;
//    ui->textEdit_heading->setText(QString::number(heading));

//}

void Mainwindow::on_toolButton_set_heading_clicked()
{

    float heading = ui->textEdit_heading->text().toFloat();
    config.setTrueN(heading);
    processing->radarData->setTrueN(config.getTrueN());

}

void Mainwindow::on_toolButton_gps_update_clicked()
{

    SetGPS(ui->text_latInput_2->text().toFloat(),ui->text_longInput_2->text().toFloat());

}




//void Mainwindow::on_toolButton_centerZoom_clicked()
//{
//    processing->radarData->updateZoomRect(mousePointerX - scrCtX+dx,mousePointerY - scrCtY+dy);
//}

void Mainwindow::on_toolButton_xl_dopler_clicked()
{

}

void Mainwindow::on_toolButton_xl_dopler_toggled(bool checked)
{
    processing->radarData->xl_dopler = checked;
}


void Mainwindow::on_toolButton_xl_nguong_3_toggled(bool checked)
{
    processing->radarData->cut_thresh = checked;
}

void Mainwindow::on_groupBox_3_currentChanged(int index)
{
    if(index==1)
    {
        processing->radarData->isManualTune = true;
    }
    else
    {
        processing->radarData->isManualTune = false;
    }
}

void Mainwindow::on_toolButton_xl_dopler_2_toggled(bool checked)
{
    processing->radarData->bo_bang_0 = checked;
}



void Mainwindow::on_toolButton_reset_3_clicked()
{
    processing->radarData->resetTrack();
//    for(short i = 0;i<targetDisplayList.size();i++)
//    {
//        targetDisplayList.at(i)->deleteLater();
//    }
//    targetDisplayList.clear();
}

void Mainwindow::on_toolButton_reset_2_clicked()
{
    processing->radarData->resetSled();
}




void Mainwindow::on_toolButton_vet_clicked(bool checked)
{
    processing->radarData->isSled = checked;
}

void Mainwindow::on_label_status_warning_clicked()
{
    if(warningList.size())warningList.removeAt(warningList.size()-1);
    if(warningList.size())
    {
        ui->label_status_warning->setText(warningList.at(warningList.size()-1));
    }
    else
    {
        ui->label_status_warning->setText(QString::fromUtf8("Không cảnh báo"));
        ui->label_status_warning->setStyleSheet("background-color: rgb(20, 40, 60,255);");
    }
}

void Mainwindow::on_toolButton_delete_target_clicked()
{
    /*if(targetList.at(selected_target_index)->isLost)
    {
        targetList.at(selected_target_index)->hide();
    }

    else*/
//    processing->radarData->mTrackList.at(targetDisplayList.at(selected_target_index)->trackId).isManual = false;
}

void Mainwindow::on_toolButton_tx_clicked()
{
    processing->radTxOn();
}


void Mainwindow::on_toolButton_tx_off_clicked()
{
    processing->radTxOff();
}

void Mainwindow::on_toolButton_filter2of3_clicked(bool checked)
{
    processing->radarData->filter2of3 = checked;
}




//void Mainwindow::on_comboBox_radar_resolution_currentIndexChanged(int index)
//{

//}

void Mainwindow::on_toolButton_create_zone_2_clicked(bool checked)
{
    if(checked)
        gz2.isActive = false;
}

void Mainwindow::on_toolButton_measuring_clicked()
{
    mouseX = scrCtX-dx;
    mouseY = scrCtY-dy;
}


void Mainwindow::on_comboBox_2_currentIndexChanged(int index)
{
    return;

}

void Mainwindow::on_toolButton_measuring_clicked(bool checked)
{
    ui->toolButton_grid->setChecked(true);
}

void Mainwindow::on_toolButton_export_data_clicked(bool checked)
{
    processing->radarData->data_export = checked;
}
bool Mainwindow::ProcDataAIS(BYTE *szBuff, int nLeng )
 {
     C2_Track       nTkNew;                              // New receive Track
     short nIndex = -1;
    int nRec;
     // Connect 2 buffer is fragment
     if(!m_CLocal.OnLinkBuff(szBuff, nLeng,nRec))
         return 0;

     if(!m_CLocal.GetTrackAIS(m_CLocal.m_Buff, m_CLocal.m_Leng, &nTkNew,nRec))
         return 0;
     for(short i = 0;i<m_AISList.size();i++)
     {
         if(m_AISList.at(i).CheckMMSI(nTkNew.m_MMSI))
         {
             m_AISList.at(i).Update(&nTkNew);
             nIndex = i;
             return true;
         }
     }
     if(nIndex<0)
     {
        m_AISList.push_back(nTkNew);
     }
     return true;
}




void Mainwindow::on_toolButton_auto_select_toggled(bool checked)
{
    setMouseMode(MouseAutoSelect,checked);
//    if(!checked)
//    {
//        this->setCursor(Qt::ArrowCursor);
//    }
//    else
//    {
//        this->setCursor(Qt::CrossCursor);
//    }
}

void Mainwindow::on_toolButton_ais_reset_clicked()
{
    m_AISList.clear();
}



//void Mainwindow::on_toolButton_2x_zoom_clicked(bool checked)
//{
//    if(checked)
//    {
//        processing->radarData->setScaleZoom(8);
//    }
//    else
//    {
//        processing->radarData->setScaleZoom(4);
//    }
//}

void Mainwindow::on_toolButton_auto_adapt_clicked()
{
    if(config.getRangeView()<=2)
    {
        sendToRadarHS("1aab200100000000");// bat thich nghi
        sendToRadarHS("14abff1100000000");// do trong
        sendToRadarHS("08ab000000000000");//do phan giai
        sendToRadarHS("01ab040000000000");//tin hieu dttt32
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("1aab200000000000");//tat thich nghi
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab030500000000");//toc do quay
        sendToRadarHS("aaab030500000000");//toc do quay
        sendToRadarHS("aaab030500000000");//toc do quay

    }
    else if(config.getRangeView() ==3)
    {
        sendToRadarHS("1aab200100000000");// bat thich nghi
        sendToRadarHS("08ab010000000000");//do phan giai
        sendToRadarHS("14abff0100000000");// do trong
        sendToRadarHS("01ab040100000000");//tin hieu dttt64
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("1aab200000000000");//tat thich nghi
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab030400000000");//toc do quay
        sendToRadarHS("aaab030400000000");//toc do quay
        sendToRadarHS("aaab030400000000");//toc do quay
    }
    else if(config.getRangeView()==4)
    {
        sendToRadarHS("1aab200100000000");// bat thich nghi
        sendToRadarHS("08ab020000000000");//do phan giai 30
        sendToRadarHS("14abff0100000000");// do trong
        sendToRadarHS("01ab040200000000");//tin hieu dttt128
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("1aab200000000000");//tat thich nghi
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab030400000000");//toc do quay
        sendToRadarHS("aaab030400000000");//toc do quay
        sendToRadarHS("aaab030400000000");//toc do quay
    }
    else if(config.getRangeView()==5)
    {
        sendToRadarHS("1aab200100000000");// bat thich nghi
        sendToRadarHS("08ab020000000000");//do phan giai 60
        sendToRadarHS("14abff0100000000");// do trong
        sendToRadarHS("01ab040300000000");//tin hieu dttt256
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("1aab200000000000");//tat thich nghi
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab030300000000");//toc do quay
        sendToRadarHS("aaab030300000000");//toc do quay
        sendToRadarHS("aaab030300000000");//toc do quay
    }
    else if(config.getRangeView()==6)
    {
        sendToRadarHS("1aab200100000000");// bat thich nghi
        sendToRadarHS("08ab030000000000");//do phan giai 90
        sendToRadarHS("14abff0100000000");// do trong
        sendToRadarHS("01ab040300000000");//tin hieu dttt256
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("1aab200000000000");//tat thich nghi
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab030200000000");//toc do quay
        sendToRadarHS("aaab030200000000");//toc do quay
        sendToRadarHS("aaab030200000000");//toc do quay
    }
    else if(config.getRangeView()==7)
    {
        sendToRadarHS("1aab200100000000");// bat thich nghi
        sendToRadarHS("08ab040000000000");//do phan giai 120
        sendToRadarHS("14abff0100000000");// do trong
        sendToRadarHS("01ab040300000000");//tin hieu dttt256
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("1aab200000000000");//tat thich nghi
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab030100000000");//toc do quay
        sendToRadarHS("aaab030100000000");//toc do quay
        sendToRadarHS("aaab030100000000");//toc do quay
    }
    else if(config.getRangeView() ==8)
    {
        sendToRadarHS("1aab200100000000");// bat thich nghi
        sendToRadarHS("08ab050000000000");//do phan giai 150
        sendToRadarHS("14abff0100000000");// do trong
        sendToRadarHS("01ab040300000000");//tin hieu dttt256
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("1aab200000000000");//tat thich nghi
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab030100000000");//toc do quay
        sendToRadarHS("aaab030100000000");//toc do quay
        sendToRadarHS("aaab030100000000");//toc do quay
    }
    else if(config.getRangeView()==9)
    {
        sendToRadarHS("1aab200100000000");// bat thich nghi
        sendToRadarHS("08ab060000000000");//do phan giai 180
        sendToRadarHS("14abff0100000000");// do trong
        sendToRadarHS("01ab040300000000");//tin hieu dttt256
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("aaab020000000000");//tat phat
        sendToRadarHS("1aab200000000000");//tat thich nghi
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab020100000000");//bat phat
        sendToRadarHS("aaab030100000000");//toc do quay
        sendToRadarHS("aaab030100000000");//toc do quay
        sendToRadarHS("aaab030100000000");//toc do quay
    }
    processing->radarData->resetTrack();
//    for(short i = 0;i<targetDisplayList.size();i++)
//    {
//        targetDisplayList.at(i)->deleteLater();
//    }
//    targetDisplayList.clear();
}

void Mainwindow::on_toolButton_set_header_size_clicked()
{
    processing->radarData->SetHeaderLen(ui->textEdit_header_len->text().toInt());
}

void Mainwindow::on_toolButton_xl_nguong_clicked()
{

}

void Mainwindow::on_toolButton_xl_nguong_clicked(bool checked)
{

}

void Mainwindow::on_toolButton_filter2of3_2_clicked(bool checked)
{
    processing->radarData->setDoubleFilter(checked);
}

void Mainwindow::on_toolButton_command_dopler_clicked()
{
    ui->lineEdit_byte_1->setText("05");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("02");
    ui->lineEdit_byte_4->setText("00");
    ui->lineEdit_byte_5->setText("00");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_toolButton_command_period_clicked()
{
    ui->lineEdit_byte_1->setText("14");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("ff");
    ui->lineEdit_byte_4->setText("00");
    ui->lineEdit_byte_5->setText("00");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_toolButton_noise_balance_clicked()
{
    ui->lineEdit_byte_1->setText("1a");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("20");
    ui->lineEdit_byte_4->setText("01");
    ui->lineEdit_byte_5->setText("00");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_toolButton_limit_signal_clicked()
{
    ui->lineEdit_byte_1->setText("17");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("64");
    ui->lineEdit_byte_4->setText("00");
    ui->lineEdit_byte_5->setText("00");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_toolButton_command_amplifier_clicked()
{
    ui->lineEdit_byte_1->setText("aa");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("01");
    ui->lineEdit_byte_4->setText("01");
    ui->lineEdit_byte_5->setText("1f");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_toolButton_command_dttt_clicked()
{
    ui->lineEdit_byte_1->setText("01");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("04");
    ui->lineEdit_byte_4->setText("00");
    ui->lineEdit_byte_5->setText("00");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_toolButton_command_res_clicked()
{
    ui->lineEdit_byte_1->setText("08");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("00");
    ui->lineEdit_byte_4->setText("00");
    ui->lineEdit_byte_5->setText("00");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_toolButton_command_antenna_rot_clicked()
{
    ui->lineEdit_byte_1->setText("aa");
    ui->lineEdit_byte_2->setText("ab");
    ui->lineEdit_byte_3->setText("03");
    ui->lineEdit_byte_4->setText("00");
    ui->lineEdit_byte_5->setText("00");
    ui->lineEdit_byte_6->setText("00");
}

void Mainwindow::on_comboBox_3_currentIndexChanged(int index)
{
    osmap->SetType(index);
    DrawMap();
    update();
}

void Mainwindow::on_horizontalSlider_map_brightness_valueChanged(int value)
{
    config.setMapOpacity(value/50.0);
    DrawMap();
}



void Mainwindow::on_toolButton_selfRotation_toggled(bool checked)
{
    if(checked)
    {
        double rate = ui->lineEdit_selfRotationRate->text().toDouble();
        rate = rate/MAX_AZIR;
        processing->radarData->SelfRotationOn(rate);
    }
    else
        processing->radarData->SelfRotationOff();
}

void Mainwindow::on_toolButton_scope_toggled(bool checked)
{
    setMouseMode(MouseScope,checked);
}

void Mainwindow::on_toolButton_manual_track_toggled(bool checked)
{
    setMouseMode(MouseAddingTrack,checked);
}
void Mainwindow::setMouseMode(mouseMode mode,bool isOn)
{
    if(isOn)
    {
        mouse_mode = static_cast<mouseMode>(mouse_mode|mode) ;
        //printf("\ntrue:%d",mouse_mode|mode);
    }
    else
    {
        mouse_mode = static_cast<mouseMode>(mouse_mode-(mode&mouse_mode));
        //printf("\nfalse:%d",mouse_mode-(mode&mouse_mode));
    }

}
void Mainwindow::on_toolButton_measuring_toggled(bool checked)
{
    setMouseMode(MouseMeasuring,checked);
}

void Mainwindow::on_toolButton_VRM_toggled(bool checked)
{
    setMouseMode(MouseVRM,checked);
}

void Mainwindow::on_toolButton_ELB_toggled(bool checked)
{
    setMouseMode(MouseELB,checked);
}

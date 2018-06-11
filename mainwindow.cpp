#include "mainwindow.h"
#include "statuswindow.h"
#include "ui_mainwindow.h"

//#include "gdal/ogr/ogrsf_frmts/ogrsf_frmts.h"
//#include "gdal/gcore/gdal.h"
//#define mapWidth 2000
//#define mapWidth mapWidth
//#define mapHeight mapWidth
#define CONST_NM 1.852f// he so chuyen doi tu km sang hai ly
#define MAX_VIEW_RANGE_KM 50
QStringList                 commandLogList;
QPixmap                     *pMap=NULL;// painter cho ban do
CMap *osmap ;
StatusWindow                *mstatWin;
double                      mTrueN2,mTrueN,mHeadingBase;
int                         mRangeLevel = 0;
int                         mDistanceUnit=0;//0:NM;1:KM
double                      mZoomSizeRg = 2;
double                      mZoomSizeAz = 10;
double                      mLat=DEFAULT_LAT,mLon = DEFAULT_LONG;
dataProcessingThread        *processing;// thread xu ly du lieu radar
C_radar_data                *pRadar;
QThread                     *t2,*t1;
QPen penBackground(QBrush(QColor(40 ,60 ,100,255)),60);
QPen penOuterGrid4(QBrush(QColor(255,255,50 ,255)),4);
QPen penOuterGrid2(QBrush(QColor(255,255,50 ,255)),2);
QPen mGridViewPen1(QBrush(QColor(150,150,150,255)),1);
double                      mMapOpacity;
int                         mMaxTapMayThu=18;
//Q_vnmap                     vnmap;
QTimer                      scrUpdateTimer ;
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
double                      mScale;

double                      temperature[255];
int         curTempIndex = 0;
double      rangeRatio = 1;
extern CConfig         mGlobbalConfig;
QStringList     warningList;
QString         strDistanceUnit;
short selectedTargetIndex;
mouseMode mouse_mode = MouseNormal;
DialogCommandLog *cmLog;
enum drawModes{
    SGN_DIRECT_DRAW,SGN_IMG_DRAW,NOTERR_DRAW
}drawMode = SGN_IMG_DRAW;
enum TargetType{
    RADAR,AIS,NOTARGET
}selectedTargetType  = NOTARGET;
int targetID = -1;
//short config.getRangeView() = 1;
float ringStep = 1;
double curAziRad = 3;
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
    float refLat = mLat*0.00872664625997f;
    return  (- dx + scrCtX + ((lon - mLon) * 111.31949079327357f*cosf(refLat))*mScale);
}
short lat2y(float lat)
{

    return (- dy + scrCtY - ((lat - mLat) * 111.31949079327357f)*mScale);
}
double y2lat(short y)
{
    return (y  )/mScale/111.31949079327357f + mLat;
}
double x2lon(short x)
{
    float refLat = mLat*0.00872664625997;
    return (x  )/mScale/111.31949079327357f/cosf(refLat) + mLon;
}
void Mainwindow::mouseDoubleClickEvent( QMouseEvent * e )
{
    if ( e->button() == Qt::RightButton )
    {

    }
    else
    {

        float xRadar = (mouseX - scrCtX+dx)/pRadar->scale_ppi ;//coordinates in  radar xy system
        float yRadar = -(mouseY - scrCtY+dy)/pRadar->scale_ppi;
        pRadar->addTrackManual(xRadar,yRadar);
        //ui->toolButton_manual_track->setChecked(false);

    }
    //Test doc AIS

}
void Mainwindow::sendToRadarString(QString command)
{
    QStringList list = command.split(';');
    for(int i=0;i<list.size();i++)
    {
        QByteArray ba=list.at(i).toLatin1();
        sendToRadarHS(ba.data());
    }

}
void Mainwindow::sendToRadarHS(const char* hexdata)//todo:move to radar class
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
}
//ham test ve tu AIS
void Mainwindow::drawAisTarget(QPainter *p)
{
    //draw targets
    QPen penTarget(QColor(250,100,250));
    penTarget.setWidth(1);
    QPen penSelectedtarget = penTarget;
    penSelectedtarget.setWidth(2);
    p->setFont(QFont("Times", 6));
    QList<AIS_object_t>::iterator iter = processing->m_aisList.begin();
    while(iter!=processing->m_aisList.end())
    {
        AIS_object_t aisObj = *iter;
        iter++;

        double fx,fy;
        ConvWGSToKm(&fx,&fy,aisObj.mLong,aisObj.mLat);
        short x = (fx*mScale)+scrCtX-dx;
        short y = (fy*mScale)+scrCtY-dy;
        if((aisObj.mType/10)==3)continue;
        if(aisObj.isNewest)
        {
            //printf("ais draw\n");
            //draw ais mark
            QPolygon poly;
            QPoint point;
            float head = aisObj.mCog*PI_NHAN2/360.0;
            point.setX(x+8*sinf(head));
            point.setY(y-8*cosf(head));
            poly<<point;
            point.setX(x+8*sinf(head+2.3562f));
            point.setY(y-8*cosf(head+2.3562f));
            poly<<point;
            point.setX(x+8*sinf(head-2.3562f));
            point.setY(y-8*cosf(head-2.3562f));
            poly<<point;
            point.setX(x+8*sinf(head));
            point.setY(y-8*cosf(head));
            poly<<point;
            point.setX(x+16*sinf(head));
            point.setY(y-16*cosf(head));
            poly<<point;
            if(aisObj.isSelected)
            {
                p->setPen(penSelectedtarget);
                p->drawPolygon(poly);
            }
            else
            {
                p->setPen(penTarget);
                p->drawPolygon(poly);
                if(ui->toolButton_ais_name->isChecked())
                    p->drawText(x,y,100,20,0,aisObj.mName);
            }


        }
        else
        {
            p->drawPoint(x,y);
        }
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
    //        pRadar->addTrack(xRadar,yRadar);
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
            currMaxAzi = (unsigned short)((atanf((float)dy/(float)dx)-pRadar->trueN)/PI_NHAN2*4096.0f);
            if(currMaxAzi<0)currMaxAzi+=MAX_AZIR;
            if(currMaxAzi>MAX_AZIR)currMaxAzi-=MAX_AZIR;
        }
        if(dx>0)
        {
            currMaxAzi = (unsigned short)(((atanf((float)dy/(float)dx)+PI-pRadar->trueN))/PI_NHAN2*4096.0f);
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
        update();
    }
}
bool controlPressed = false;
void Mainwindow::keyPressEvent(QKeyEvent *event)
{
    this->setFocus();
    int key = event->key();
    if(key == Qt::Key_Control)
    {
        controlPressed  = true;
    }
    else if(controlPressed)
    {
        if(key==Qt::Key_1)
        {
            short   mx=this->mapFromGlobal(QCursor::pos()).x();
            short   my=this->mapFromGlobal(QCursor::pos()).y();
            double mlat = y2lat(-(my - scrCtY+dy));
            double mlon = x2lon(mx - scrCtX+dx);
            SetGPS(mlat,mlon);
            DrawMap();
            this->repaint();

        }
        else if(key==Qt::Key_2)
        {
            pRadar->clearPPI();
        }

    }
    else if(key == Qt::Key_Space)
    {
        short   mx=this->mapFromGlobal(QCursor::pos()).x();
        short   my=this->mapFromGlobal(QCursor::pos()).y();
        mousePointerX = mx;
        mousePointerY = my;
        if(!isInsideViewZone(mx,my))return;
        pRadar->setZoomRectAR((mx - scrCtX+dx)/mScale,
                              -(my - scrCtY+dy)/mScale,
                              mZoomSizeRg,mZoomSizeAz);
        pRadar->setZoomRectXY(mx - scrCtX+dx,my - scrCtY+dy);
    }

}
void Mainwindow::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control)
    {
        controlPressed  = false;
    }
}


short selZone_x1, selZone_x2, selZone_y1, selZone_y2;
bool isSelectingTarget = false;
void Mainwindow::detectZone()
{
    short sx,sy;
    float scale_ppi = pRadar->scale_ppi;
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
    trackList* trackListPt = &pRadar->mTrackList;
    if(true)
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
        if(mouse_mode&MouseAutoSelect)//(ui->toolButton_auto_select->isChecked())
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
            pRadar->drawRamp(azid);
        }
        /*else if(ui->toolButton_create_zone->isChecked())
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
        }*/
        else
        {
            setMouseMode(MouseDrag,true);
            //mouse_mode=MouseDrag;//isDraging = true;
        }
    }
    if(event->buttons() & Qt::RightButton)
    {
        //select radar target
        trackList* trackListPt = &pRadar->mTrackList;
        for(uint trackId=0;trackId<trackListPt->size();trackId++)
        {
            if(!trackListPt->at(trackId).isConfirmed)continue;
            if(!trackListPt->at(trackId).isManual)continue;
            //if(trackListPt->at(trackId).state<5)continue;
            short sx = trackListPt->at(trackId).estX*pRadar->scale_ppi + scrCtX - dx;
            short sy = -trackListPt->at(trackId).estY*pRadar->scale_ppi + scrCtY - dy;
            if( qAbs(sx-event->x()) <5 && qAbs(sy-event->y())<5)
            {
                selectedTargetType = RADAR;
                selectedTargetIndex = trackId;
            }
        }


        //select ais target
        if(ui->toolButton_ais_show->isChecked())
        {
            checkClickAIS(event->x(),event->y());
        }
    }

}
void Mainwindow::checkClickAIS(int xclick, int yclick)
{
    QList<AIS_object_t>::iterator iter = processing->m_aisList.begin();
    while(iter!=processing->m_aisList.end())
    {
        AIS_object_t aisObj = *iter;
        iter++;
        if(aisObj.isSelected)continue;
        if(!aisObj.isNewest)continue;
        double fx,fy;
        ConvWGSToKm(&fx,&fy,aisObj.mLong,aisObj.mLat);
        int x = (fx*mScale)+scrCtX-dx;
        int y = (fy*mScale)+scrCtY-dy;
        if(abs(x-xclick)<5&&abs(y-yclick)<5)
        {
            DialogAisInfo *dialog = new DialogAisInfo(this);
            dialog->setAttribute( Qt::WA_DeleteOnClose, true );
            dialog->setWindowFlags(dialog->windowFlags()&(~Qt::WindowContextHelpButtonHint));
            dialog->setFixedSize(dialog->width(),dialog->height());
            dialog->setAisData(&processing->m_aisList,aisObj.mMMSI);
            dialog->show();
            break;
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
    cmLog = new DialogCommandLog();

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
    p->drawImage(screen,*pRadar->img_ppi,signRect,Qt::AutoColor);

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
    pMap->fill(QColor(5,10,15,255));

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

        p.setOpacity(mMapOpacity);
        p.drawPixmap((-pix.width()/2+pMap->width()/2),
                     (-pix.height()/2+pMap->height()/2),pix.width(),pix.height(),pix
                     );
    }
    else
    {
        pMap->fill(QColor(10,20,30,255));
    }

    //DrawGrid(&p,centerX,centerY);
    update();

}
void Mainwindow::DrawGrid(QPainter* p,short centerX,short centerY)
{
    //return;

    p->setCompositionMode(QPainter::CompositionMode_Plus);
    mGridViewPen1.setStyle(Qt::DashLine);
    mGridViewPen1.setWidth(1);
    QFont font;
    font.setPointSize(10);
    p->setFont(font);
    p->setBrush(QBrush(Qt::NoBrush));
    p->setPen(mGridViewPen1);
    p->drawLine(centerX-5,centerY,centerX+5,centerY);
    p->drawLine(centerX,centerY-5,centerX,centerY+5);
    //pen.setColor(QColor(30,90,150,120));

    p->setPen(mGridViewPen1);
    for(short i = 1;i<8;i++)
    {
        int rad = i*ringStep*rangeRatio*mScale;
        p->drawEllipse(QPoint(centerX,centerY),
                       (short)(rad),
                       (short)(rad));
        p->drawText(centerX+2,centerY-rad+2,100,20,0,QString::number(i*ringStep)+strDistanceUnit);
        p->drawText(centerX+2,centerY+rad+2,100,20,0,QString::number(i*ringStep)+strDistanceUnit);
        p->drawText(centerX+rad+2,centerY+2,100,20,0,QString::number(i*ringStep)+strDistanceUnit);
        p->drawText(centerX-rad+2,centerY+2,100,20,0,QString::number(i*ringStep)+strDistanceUnit);
    }

    short theta;
    short gridR = ringStep*1.852f*mScale*7;
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
        //                if(theta<270)p->drawText(point2,QString::number(theta+90));
        //                else p->drawText(point2,QString::number(theta-270));

    }

    //end grid
    p->setCompositionMode(QPainter::CompositionMode_SourceOver);



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

void Mainwindow::DrawRadarTargetByPainter(QPainter* p)//draw radar target from pRadar->mTrackList
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
    float scale_ppi = pRadar->scale_ppi;
    //short targetId = 0;
    trackList* trackListPt = &pRadar->mTrackList;
    if(true)//ui->toolButton_blue_tracks->isChecked())
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

    double refLat = (mLat + (m_Lat))*0.00872664625997;//pi/360
    *x	= (((m_Long) - mLon) * 111.31949079327357)*cos(refLat);// 3.14159265358979324/180.0*6378.137);//deg*pi/180*rEarth
    *y	= ((mLat - (m_Lat)) * 111.132954);
    //tinh toa do xy KM so voi diem center khi biet lat-lon
}
void Mainwindow::ConvKmToWGS(double x, double y, double *m_Long, double *m_Lat)
{

    *m_Lat  = mLat +  (y)/(111.132954);
    double refLat = (mLat +(*m_Lat))*0.00872664625997;//3.14159265358979324/180.0/2;
    *m_Long = (x)/(111.31949079327357*cos(refLat))+ mLon;
    //tinh toa do lat-lon khi biet xy km (truong hop coi trai dat hinh cau)
}
void Mainwindow::drawAisTarget2(QPainter *p)
{
    /*
    //draw radar  target:
    QPen penTargetRed(QColor(255,50,150));
    penTargetRed.setWidth(0);
    for(uint i=0;i<processing->m_AISList.size();i++)
    {
            p->setPen(penTargetRed);
//            short j;
            //draw track:
            double fx,fy;
//            float mlat = m_trackList.at(i).getLat();
//            mlat =  mlat/bit23* 180.0f ;
//            float mlon = m_trackList.at(i).mLong_double;
//            mlon = mlon/bit23* 180.0f ;
                ConvWGSToKm(&fx,&fy,processing->m_AISList.at(i).getLon(),processing->m_AISList.at(i).getLat());

                short x = (fx*mScale)+scrCtX-dx;
                short y = (fy*mScale)+scrCtY-dy;
                //draw ais mark
                QPolygon poly;
                QPoint point;
                float head = processing->m_AISList.at(i).m_Head*PI_NHAN2/(1<<16);
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
                    p->drawText(x+5,y+10,(processing->m_AISList.at(i).m_szName));
                }
//                p->drawText(x+5,y+5,QString::fromAscii((char*)&m_trackList.at(i).m_MMSI[0],9));
                //printf("\nj:%d,%d,%d,%f,%f",j,x,y,arpa_data.ais_track_list[i].object_list[j].mlong,arpa_data.ais_track_list[i].object_list[j].mlat);

    }*/
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
    rg/=rangeRatio;
    ui->label_cursor_range->setText(QString::number(rg,'f',2)+strDistanceUnit);
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
    //ve luoi cu ly phuong vi

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
        p->setPen(QPen(Qt::black));
        p->setBrush(QBrush(Qt::black));
        p->drawRect(rect);
        p->setPen(QPen(Qt::white,2));
        QPoint p1(rect.x(),rect.y());
        //QPoint p2(rect.x(),rect.y());
        QPoint p11(rect.x()+rect.width(),rect.y());
        QPoint p22(rect.x(),rect.y()+rect.height());
        p->drawLine(p1,p11);
        p->drawLine(p1,p22);
        int step = rect.width()/5;
        for(int i = 0;i<5;i++)
        {
            p->drawLine(rect.x()+step*i,rect.y(),rect.x()+step*i,rect.y()+5);
            p->drawLine(rect.x(),rect.y()+step*i,rect.x()+5,rect.y()+step*i);
        }
        p->setFont(QFont("Times",10));
        p->drawText(rect.x()+rect.width()-50,rect.y()+15,QString::number(mZoomSizeRg/0.1852,'f',1)+QString::fromUtf8(" Liên"));
        p->drawText(rect.x()+5,rect.y()+rect.height()-5,QString::number(mZoomSizeAz,'f',1)+QString::fromUtf8(" Độ"));
        if(pRadar->img_zoom_ar)
        {
            QImage img = pRadar->img_zoom_ar->scaled(rect.width(),rect.height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
            p->drawImage(rect,img);//todo:resize

        }

    }
    else if(ui->tabWidget_2->currentIndex()==6)
    {
        //C_radar_data *prad = pRadar;
        if(mRangeLevel>2)
        {
            short zoom_size = ui->tabWidget_2->width()/pRadar->scale_zoom_ppi*pRadar->scale_ppi;
            p->setPen(QPen(QColor(255,255,255,200),0,Qt::DashLine));
            p->drawRect(mousePointerX-zoom_size/2.0,mousePointerY-zoom_size/2.0,zoom_size,zoom_size);
        }
        p->drawImage(rect,*pRadar->img_zoom_ppi,pRadar->img_zoom_ppi->rect());

    }
    else if(ui->tabWidget_2->currentIndex()==3)
    {
        QRect rect = ui->tabWidget_2->geometry();

        p->setPen(QPen(Qt::black));
        p->setBrush(QBrush(Qt::black));
        p->drawRect(rect);
        p->drawImage(rect,*pRadar->img_histogram,
                     pRadar->img_histogram->rect());

    }
    else if(ui->tabWidget_2->currentIndex()==4)
    {


        p->setPen(QPen(Qt::black));
        p->setBrush(QBrush(Qt::black));
        p->drawRect(rect);
        p->drawImage(rect,*pRadar->img_spectre,
                     pRadar->img_spectre->rect());
    }
    else if(ui->tabWidget_2->currentIndex()==5)
    {
        if(ui->toolButton_scope->isChecked()==false)pRadar->drawRamp();
        QRect rect1 = rect;
        rect1.adjust(0,0,0,-rect.height()/2);
        //        pengrid.setWidth(10);
        //        p->setPen(pengrid);
        p->drawImage(rect1,*pRadar->img_RAmp);
        double rampos = ui->horizontalSlider_ramp_pos->value()/(double(ui->horizontalSlider_ramp_pos->maximum()));
        QRect rect2 = rect;
        rect2.adjust(0,rect.height()/2,0,0);
        int zoomw = rect2.width()/2;
        int ramposInt = (pRadar->img_RAmp->width()-zoomw)*rampos;
        QRect srect(ramposInt,0,zoomw,pRadar->img_RAmp->height());
        p->drawImage(rect2,*pRadar->img_RAmp,srect);
        //p->drawRect(rect1,pRadar->img_RAmp->width()+5,pRadar->img_RAmp->height()+5);
        //        pengrid.setWidth(2);
        //        pengrid.setColor(QColor(128,128,0,120));
        //        p->setPen(pengrid);
        //        for(short i=60;i<pRadar->img_RAmp->height();i+=50)
        //        {
        //            p->drawLine(0,height()-i,pRadar->img_RAmp->width()+5,height()-i);
        //        }
        //        for(short i=110;i<pRadar->img_RAmp->width();i+=100)
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
void Mainwindow::setDistanceUnit(int unit)//0:NM, 1:KM
{
    mDistanceUnit = unit;
    mGlobbalConfig.setValue("mDistanceUnit",mDistanceUnit);
    if(mDistanceUnit==0)
    {
        rangeRatio = 1.852;
        strDistanceUnit = "NM";
        ui->toolButton_setRangeUnit->setText(QString::fromUtf8("Đơn vị đo:NM"));
        UpdateScale();
    }
    else if(mDistanceUnit==1)
    {
        rangeRatio = 1.0;
        strDistanceUnit = "KM";
        ui->toolButton_setRangeUnit->setText(QString::fromUtf8("Đơn vị đo:KM"));
        UpdateScale();
    }
    DrawMap();
}
void Mainwindow::InitSetting()
{
    on_comboBox_currentIndexChanged(0);
    processing->mSerialBaud = mGlobbalConfig.getInt("mSerialBaud");
    mMaxTapMayThu = mGlobbalConfig.getInt("mMaxTapMayThu");
    mRangeLevel = mGlobbalConfig.getInt("mRangeLevel");
    assert(mRangeLevel>=0&&mRangeLevel<8);
    setDistanceUnit(mGlobbalConfig.getInt("mDistanceUnit"));
    assert(mDistanceUnit>=0&&mDistanceUnit<2);
    mR0Command = mGlobbalConfig.getString("mR0Command");
    mR1Command = mGlobbalConfig.getString("mR1Command");
    mR2Command = mGlobbalConfig.getString("mR2Command");
    mR3Command = mGlobbalConfig.getString("mR3Command");
    mR4Command = mGlobbalConfig.getString("mR4Command");
    mR5Command = mGlobbalConfig.getString("mR5Command");
    mR6Command = mGlobbalConfig.getString("mR6Command");
    mR7Command = mGlobbalConfig.getString("mR7Command");
    mRxCommand = mGlobbalConfig.getString("mRxCommand");
    mTxCommand = mGlobbalConfig.getString("mTxCommand");
    mFreq1Command = mGlobbalConfig.getString("mFreq1Command");
    mFreq2Command = mGlobbalConfig.getString("mFreq2Command");
    mFreq3Command = mGlobbalConfig.getString("mFreq3Command");
    mFreq4Command = mGlobbalConfig.getString("mFreq4Command");
    mFreq5Command = mGlobbalConfig.getString("mFreq5Command");
    mFreq6Command = mGlobbalConfig.getString("mFreq6Command");
    mTrueN2 = mGlobbalConfig.getDouble("mTrueN2");
    mTrueN = mGlobbalConfig.getDouble("mTrueN");
    mHeadingBase = mGlobbalConfig.getDouble("mHeadingBase");

    pRadar->setTrueN(mTrueN);
    ui->textEdit_heading->setText(mGlobbalConfig.getString("mHeadingAntenna"));
    ui->textEdit_heading_2->setText(mGlobbalConfig.getString("mHeadingSelsyn"));
    ui->textEdit_heading_base->setText(mGlobbalConfig.getString("mHeadingBase"));
    mZoomSizeAz = mGlobbalConfig.getDouble("mZoomSizeAz");
    ui->textEdit_size_ar_a->setText(QString::number(mZoomSizeAz));
    mZoomSizeRg = mGlobbalConfig.getDouble("mZoomSizeRg");
    ui->textEdit_size_ar_r->setText(QString::number(mZoomSizeRg));
    //load map
    osmap = new CMap();
    SetGPS(mGlobbalConfig.getDouble("mLat"), mGlobbalConfig.getDouble("mLon"));
    osmap->setCenterPos(mLat,mLon);
    osmap->setImgSize(height(),height());
    osmap->SetType(0);
    mMapOpacity = mGlobbalConfig.getDouble("mMapOpacity");
    //config.setMapOpacity(value/50.0);
    ui->horizontalSlider_map_brightness->setValue(mMapOpacity*50);
    ui->toolButton_xl_nguong_3->setChecked(mGlobbalConfig.getInt("is_normalize_signal"));
    ui->groupBox_control->setHidden(true);
    ui->groupBox_control_setting->setHidden(true);
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
    mousePointerX = scrCtX = SCR_H/2 + SCR_LEFT_MARGIN;//ENVDEP
    mousePointerY = scrCtY = SCR_H/2;
    UpdateScale();

    //ui->horizontalSlider_2->setValue(config.m_config.cfarThresh);

    ui->horizontalSlider_brightness->setValue(ui->horizontalSlider_brightness->maximum()/3.5);
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


    //init view settings and variables
    if(pMap)delete pMap;
    pMap = new QPixmap(height(),height());
    mDViewFrame = height()-50;
    setMouseMode(MouseDrag,true);
    DrawMap();
    update();
}
void Mainwindow::ReloadSetting()
{



}
bool Mainwindow::CalcAziContour(double theta, QPoint *point0,QPoint *point1,QPoint *point2,int d)
{
    if(theta>=360)theta-=360.0;
    if(theta<0)theta+=360.0;
    double tanA = tan(theta/57.295779513);
    double sinA = sin(theta/57.295779513);
    double cosA = cos(theta/57.295779513);

    if(theta==0)
    {
        point2->setX(scrCtX  - dx);
        point2->setY(scrCtY - sqrt((d*d/4.0- dx*dx)));
        point1->setX(point2->x());
        point1->setY(point2->y()-5.0);
        point0->setX(point2->x());
        point0->setY(point2->y()-18.0);
    }
    else if(theta==180)
    {

        point2->setX(scrCtX  - dx);
        point2->setY(scrCtY + sqrt((d*d/4.0- dx*dx)));
        point1->setX(point2->x());
        point1->setY(point2->y()+5.0);
        point0->setX(point2->x());
        point0->setY(point2->y()+18.0);
    }
    else if (theta<180)
    {
        double a = (1.0+1.0/tanA/tanA);//4*(dy/tanA-dx)*(dy/tanA-dx) -4*(1+1/tanA)*(dx*dx+dy*dy-width()*width()/4);
        double b= 2.0*(dy/tanA - dx);
        double c= dx*dx+dy*dy-d*d/4.0;
        double delta = b*b-4.0*a*c;
        if(delta<30.0)return false;
        delta = sqrt(delta);
        double rx = (-b + delta)/2.0/a;
        double ry = -rx/tanA;
        if(abs(rx)<100&&abs(ry)<100)return false;
        point2->setX(scrCtX + rx -dx);
        point2->setY(scrCtY + ry-dy);
        point1->setX(point2->x()+5.0*sinA);
        point1->setY(point2->y()-5.0*cosA);
        point0->setX(point2->x()+18.0*sinA);
        point0->setY(point2->y()-18.0*cosA);
    }
    else
    {
        double a = (1+1.0/tanA/tanA);//4*(dy/tanA-dx)*(dy/tanA-dx) -4*(1+1/tanA)*(dx*dx+dy*dy-width()*width()/4);
        double b= 2.0*(dy/tanA - dx);
        double c= dx*dx+dy*dy-d*d/4.0;
        double delta = b*b-4.0*a*c;
        if(delta<30.0)return false;
        delta = sqrt(delta);
        double rx;
        double ry;
        rx =  (-b - delta)/2.0/a;
        ry = -rx/tanA;
        if(abs(rx)<100&&abs(ry)<100)return false;
        point2->setX(scrCtX + rx - dx);
        point2->setY(scrCtY + ry - dy);
        point1->setX(point2->x()+5.0*sinA);
        point1->setY(point2->y()-5.0*cosA);
        point0->setX(point2->x()+18.0*sinA);
        point0->setY(point2->y()-18.0*cosA);
    }
    return true;

}
QPoint points[6];
void Mainwindow::DrawViewFrame(QPainter* p)
{

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
    //ve phuong vi ang ten
    //,point1,point2,pointA,pointB;


    //double minazi = rad2deg(pRadar->getArcMinAziRad());
    //double maxazi = rad2deg(pRadar->getArcMaxAziRad());
    //if(maxazi<minazi)minazi-=360.0;
    //double dazi = maxazi-minazi;
    //drwa arc
    /*QRect rect(scrCtX-dx-50,scrCtY-dy-50,100,100);
    p->setPen(QPen(Qt::white,2,Qt::DashLine));
    p->drawArc(rect,16*((-maxazi+90)),dazi*16);
    */


    //draw view frame


    //fill back ground
    p->setBrush(QColor(40,60,100,255));
    p->drawRect(scrCtX+scrCtY,0,width()-scrCtX-scrCtY,height());
    p->drawRect(0,0,scrCtX-scrCtY,height());
    p->setBrush(Qt::NoBrush);
    p->setPen(penOuterGrid4);
    p->drawEllipse(scrCtX-scrCtY+25,25,mDViewFrame,mDViewFrame);
    p->setPen(penBackground);
    for (short i=60;i<650;i+=110)
    {
        p->drawEllipse(-i/2+(scrCtX-scrCtY)+25,-i/2+25,mDViewFrame+i,mDViewFrame+i);
    }


    p->setPen(penOuterGrid2);
    QFont font10 = p->font() ;
    font10.setPointSize(10);
    p->setFont(font10);
    for(short theta=0;theta<360;theta+=10)
    {
        if(CalcAziContour(theta,&points[0],&points[1],&points[2],mDViewFrame))
        {
            p->drawLine(points[1],points[2]);
            p->drawText(points[0].x()-25,points[0].y()-10,50,20,
                    Qt::AlignHCenter|Qt::AlignVCenter,
                    QString::number(theta));
        }
    }
    double aziDeg = rad2deg(pRadar->getCurAziRad());
    //plot center azi
    double centerAzi = processing->getSelsynAzi()+mTrueN2 ;
    if(centerAzi>360)centerAzi-=360;
    if(CalcAziContour(centerAzi,&points[0],&points[2],&points[1],height()-70))
    {
        p->setPen(QPen(Qt::yellow,8,Qt::SolidLine,Qt::RoundCap,Qt::MiterJoin));
        p->drawLine(points[2],points[1]);
        p->drawText(720,40,200,20,0,"Sector:  "+QString::number(centerAzi,'f',1));
    }
    //plot heading base azi
    if(CalcAziContour(mHeadingBase,&points[0],&points[1],&points[2],height()-70))
    {
        p->setPen(QPen(Qt::cyan,6,Qt::SolidLine,Qt::RoundCap));
        p->drawLine(points[2],points[1]);
        p->drawText(720,60,200,20,0,"Heading: "+QString::number(mHeadingBase,'f',1));

    }
    //plot cur azi
    if(CalcAziContour(aziDeg,&points[0],&points[1],&points[2],height()-70))
    {
        p->setPen(QPen(Qt::red,4,Qt::SolidLine,Qt::RoundCap));
        p->drawLine(points[2],points[1]);
        //draw text
        p->drawText(720,20,200,20,0,"Antenna: "+QString::number(aziDeg,'f',1));

    }




}

short waittimer =0;
void Mainwindow::DisplayClkAdc(int clk)
{
    switch(clk)
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
        ui->label_range_resolution->setText("120m");
        break;
    case 4:
        ui->label_range_resolution->setText("240m");
        break;
    case 5:
        ui->label_range_resolution->setText("0x05");
        break;
    case 6:
        ui->label_range_resolution->setText("0x06");
        break;
    default:
        ui->label_range_resolution->setText("N/A");
        break;

    }
}
void Mainwindow::UpdateRadarData()
{
    if(!processing->getIsDrawn())
    {
        if(pRadar->isClkAdcChanged)
        {
            //ui->comboBox_radar_resolution->setCurrentIndex(pRadar->clk_adc);
            DisplayClkAdc(pRadar->clk_adc);
            pRadar->setScalePPI(mScale);
            this->UpdateScale();
            //            printf("\nsetScale:%d",pRadar->clk_adc);
            pRadar->isClkAdcChanged = false;
        }
        pRadar->UpdateData();
    }
    update();
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

    processing = new dataProcessingThread(mGlobbalConfig.getInt("mSerialBaud"));
    pRadar = processing->mRadarData;
    connect(&syncTimer1s, SIGNAL(timeout()), this, SLOT(sync1S()));
    syncTimer1s.start(1000);
    connect(&syncTimer5p, SIGNAL(timeout()), this, SLOT(sync5p()));
    syncTimer5p.start(300000);
    //syncTimer1s.moveToThread(t);

    connect(&scrUpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateRadarData()));
    scrUpdateTimer.start(30);//ENVDEP
    scrUpdateTimer.moveToThread(t2);
    //connect(t2,SIGNAL(finished()),t2,SLOT(deleteLater()));

    connect(this,SIGNAL(destroyed()),processing,SLOT(deleteLater()));
    connect(&dataPlaybackTimer,SIGNAL(timeout()),processing,SLOT(playbackRadarData()));
    processing->start(QThread::TimeCriticalPriority);
    t2->start(QThread::HighPriority);


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
        if(datagram.size()==5)
        {
            if(*((unsigned char*)datagram.data())==0xff)
            {
                curAziRad = (*((unsigned char*)datagram.data()+1)*256
                             + (*((unsigned char*)datagram.data()+2)))/2500.0*3.14159265;
            }
        }
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
//void Mainwindow::on_actionExit_triggered()
//{


//    ExitProgram();
//}
void Mainwindow::ExitProgram()
{
        processing->stopThread();
        processing->wait();
    QApplication::quit();
#ifdef _WIN32
    QProcess::startDetached("shutdown -s -f -t 0");
#else
    //system("/sbin/halt -p");
#endif
}


void Mainwindow::sync5p()//period 10 second
{

    if(radar_state!=DISCONNECTED)
    {
        QFile logFile;
        QDateTime now = QDateTime::currentDateTime();
        if(!QDir("D:\\logs\\"+now.toString("\\dd.MM\\")).exists())
        {
            QDir().mkdir("D:\\logs\\"+now.toString("\\dd.MM\\"));
        }
        logFile.setFileName("D:\\logs\\"+now.toString("\\dd.MM\\")+now.toString("dd.MM-hh.mm.ss")+"_radar_online.log");
        logFile.open(QIODevice::WriteOnly);

        logFile.close();

    }

}
void Mainwindow::updateTargetInfo()
{
    if(selectedTargetType==RADAR)
    {
        trackList* trackListPt = &pRadar->mTrackList;
        for(uint trackId=0;trackId<trackListPt->size();trackId++)
        {

            if(!trackListPt->at(trackId).isConfirmed)continue;
            if(selectedTargetIndex == trackId)
            {
                //printf("\ntrackId:%d",trackId);
                double mLat,mLon;
                this->ConvKmToWGS(trackListPt->at(trackId).estX*pRadar->scale_ppi/mScale,trackListPt->at(trackId).estY*pRadar->scale_ppi/mScale,&mLon,&mLat);
                ui->label_data_id->setText(QString::number(trackListPt->at(trackId).idCount));
                float tmpazi = trackListPt->at(trackId).estA*DEG_RAD;
                if(tmpazi<0)tmpazi+=360;
                ui->label_data_type->setText("Radar");
                ui->label_data_range->setText(QString::number(trackListPt->at(trackId).estR*pRadar->scale_ppi/mScale/1.852f,'f',2)+"Nm");
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
        /*
    C2_Track *selectedTrack = &processing->m_AISList.at(selectedTargetIndex);
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
    */}
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
void Mainwindow::setFreq(int newfreq)
{
    switch(newfreq)
    {
    case 0:
        ui->toolButton_tx_2->setChecked(true);
        sendToRadarString(mFreq1Command);break;
    case 1:
        ui->toolButton_tx_3->setChecked(true);
        sendToRadarString(mFreq2Command);break;
    case 2:
        ui->toolButton_tx_4->setChecked(true);
        sendToRadarString(mFreq3Command);break;
    case 3:
        ui->toolButton_tx_5->setChecked(true);
        sendToRadarString(mFreq4Command);break;
    case 4:
        ui->toolButton_tx_6->setChecked(true);
        sendToRadarString(mFreq5Command);break;
    case 5:
        ui->toolButton_tx_7->setChecked(true);
        sendToRadarString(mFreq6Command);break;
    default:break;
    }

}
void Mainwindow::autoSwitchFreq()
{
    int newFreq = rand()%6;
    setFreq(newFreq);

}
void Mainwindow::sync1S()//period 1 second
{
    //processing->SerialEncoderRead();

    this->updateTargetInfo();
    if(processing->isConnected())
        setRadarState(CONNECTED);
    else
        setRadarState(DISCONNECTED);
    if(warningList.size())
    {
        ui->label_status_warning->setText(warningList.at(warningList.size()-1));
        ui->label_status_warning->setStyleSheet("background-color: rgb(230, 160, 10);");
        if(ui->label_status_warning->isHidden())ui->label_status_warning->setHidden(false);
        else ui->label_status_warning->setHidden(true);
    }


    if(isScaleChanged ) {

        pRadar->setScalePPI(mScale);
        isScaleChanged = false;
    }
    showTime();

    // display radar temperature:
    temperature[pRadar->tempType] = pRadar->moduleVal;

    ui->label_noiseAverrage->setText(QString::number(pRadar->getNoiseAverage(),'f',1));
    ui->label_temp->setText(QString::number(pRadar->tempType)
                            +"|"+QString::number(pRadar->moduleVal,'f',0)
                            + QString::fromLocal8Bit("\260 C"));
    // request radar temperature:
    if(radar_state!=DISCONNECTED)
    {
        if(ui->toolButton_temp_update->isChecked())
        {
            processing->radRequestTemp(curTempIndex);
            curTempIndex++;
            if(curTempIndex>4)curTempIndex=0;
        }
        QByteArray array((char*)pRadar->getFeedback(), 8);
        QString commandLog = QString(array.toHex());
        ui->label_command->setText(commandLog);
        cmLog->AddString(commandLog);
    }


    switch(radar_state)
    {
    case DISCONNECTED:
        ui->label_status->setText(QString::fromUtf8("Chưa k. nối"));
        //ui->toolButton_tx->setEnabled(false);
        //        ui->toolButton_scan->setEnabled(false);
        if(!warningList.size())warningList.push_back(QString::fromUtf8("Chưa kết nối radar"));
        m_udpSocket->writeDatagram("d",1,QHostAddress("127.0.0.1"),8001);
        break;
    case CONNECTED:
        //printf("\ns_tx");
        //ui->label_status_warning->setText(QString::fromUtf8("Đã kết nối radar"));
        ui->label_status->setText(QString::fromUtf8("sẵn sàng"));
        //ui->label_status_warning->setHidden(false);

        m_udpSocket->writeDatagram("c",1,QHostAddress("127.0.0.1"),8001);
        break;
    default:
        break;
    }
    //return;
    //ui->label_debug_data->setText("Chu ky: "+QString::number(pRadar->chu_ky));
    /*unsigned int chuKy = 1000000/(pRadar->chu_ky*(pow(2,pRadar->clk_adc))/10.0);

    ui->label_sn_freq->setText(QString::number(chuKy));
    */
    ui->label_debug->setText(QString::number(pRadar->getNoiseAverage()));
    ui->label_he_so_tap->setText(QString::fromUtf8("Hệ số tạp: ")+QString::number(pRadar->get_tb_tap()));
    if(ui->toolButton_auto_freq->isChecked())
    {
        if(pRadar->mJammingDetected)
        {
            this->autoSwitchFreq();
            pRadar->mJammingDetected = false;
        }
    }

    int value;
    switch((pRadar->sn_stat>>8)&0x07)
    {
    case 4:
        ui->label_sn_type->setText("DTTT");
        //ui->label_sn_param->setText(QString::number(32<<());
        if(((pRadar->sn_stat)&0xff)==0)
        {
            ui->label_sn_param->setText("32");
        }
        else if(((pRadar->sn_stat)&0xff)==1)
        {
            ui->label_sn_param->setText("48");
        }
        else if(((pRadar->sn_stat)&0xff)==2)
        {
            ui->label_sn_param->setText("64");
        }
        else if(((pRadar->sn_stat)&0xff)==3)
        {
            ui->label_sn_param->setText("96");
        }
        else if(((pRadar->sn_stat)&0xff)==4)
        {
            ui->label_sn_param->setText("128");
        }
        else if(((pRadar->sn_stat)&0xff)==5)
        {
            ui->label_sn_param->setText("192");

        }
        else
        {
            ui->label_sn_param->setText("256");
        }
        break;
    case 0:
        ui->label_sn_type->setText(QString::fromUtf8("Xung đơn"));
        ui->label_sn_param->setText(QString::number((((pRadar->sn_stat)&0xff))));
        break;
    case 2:
        ui->label_sn_type->setText(QString::fromUtf8("Mã M"));
        value = ((pRadar->sn_stat)&0xff);
        if(value<5)
        {
            ui->label_sn_param->setText(QString::number(16*pow(2,value)));

            ui->label_sn_pulsew->setText(QString::number(((16*pow(2,value))*(pow(2,pRadar->clk_adc))/10.0),'f',1));
        }
        else
        {
            ui->label_sn_param->setText(QString::number(16*pow(2,value-5))+"x2");
            ui->label_sn_pulsew->setText(QString::number(((32*(value-5))*(pow(2,pRadar->clk_adc))/10.0),'f',1));
        }

        break;
    case 3:
        ui->label_sn_type->setText(QString::fromUtf8("Mã 2 pha"));//todo:change back to "ma ngau nhien" later
        value = ((pRadar->sn_stat)&0xff);
        ui->label_sn_param->setText(QString::number(value-15.0));
        ui->label_sn_pulsew->setText(QString::number(((value-15.0)*(pow(2,pRadar->clk_adc))/10.0),'f',1));
        break;
    case 1:
        ui->label_sn_type->setText(QString::fromUtf8("Mã baker"));
        ui->label_sn_param->setText(QString::number((((pRadar->sn_stat)&0xff))));
        break;
    default:
        ui->label_sn_param->setText(QString::number(((pRadar->sn_stat)&0xff)));
        break;
    }
    /*switch((pRadar->rotation_speed)&0x07)
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
            }*/
    ui->label_speed_2->setText(QString::number(pRadar->rotation_per_min,'f',1)+"v/p");



}
void Mainwindow::setRadarState(radarSate radarState)
{
    if(radar_state!=radarState)
    {
        radar_state = radarState;
        on_label_status_warning_clicked();
    }
}




void Mainwindow::on_actionRecording_toggled(bool arg1)
{
    if(arg1)
    {
        QDateTime now = QDateTime::currentDateTime();
        QString filename = now.toString("dd.MM_hh.mm.ss")+
                "_"+ui->label_range_resolution->text()+
                "_"+ui->label_sn_type->text()+
                "_"+ui->label_sn_param->text();
        ui->label_record_file_name->setText(filename);
        processing->startRecord("D:/HR2D/rec_"+filename+HR_FILE_EXTENSION);
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
    ui->label_record_file_name->setText(filename);
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
    pRadar->resetData();
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

                  if(pRadar->checkFeedback(&command_queue.front().bytes[0]))// check if the radar has already recieved the command
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

}

//void RadarGui::on_horizontalSlider_brightness_actionTriggered(int action)
//{

//}

void Mainwindow::on_horizontalSlider_brightness_valueChanged(int value)
{
    pRadar->brightness = 0.5f+(float)value/ ui->horizontalSlider_brightness->maximum()*4.0f;
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
    if(mDistanceUnit==0)
    {
        mScale = (height()/2.0-5.0)/(rangeRatio*srange );
        ringStep = srange/6.0f;
        ui->label_range->setText(QString::number(srange)+strDistanceUnit);
    }
    else if(mDistanceUnit==1)
    {
        mScale = (height()/2.0-5.0)/(rangeRatio*srange );
        ringStep = srange/5;
        ui->label_range->setText(QString::number(srange)+strDistanceUnit);
    }
}
void Mainwindow::UpdateScale()
{
    float oldScale = mScale;
    //char byte2;
    bool isAdaptSn = ui->toolButton_auto_adapt->isChecked();
    if(mDistanceUnit==0)//NM
    {
        switch(mRangeLevel)
        {
        case 0:
            setScaleRange(1.5);
            if(isAdaptSn) sendToRadarString(mR0Command);
            break;
        case 1:
            setScaleRange(3);
            if(isAdaptSn) sendToRadarString(mR1Command);
            break;
        case 2:
            setScaleRange(6);
            if(isAdaptSn) sendToRadarString(mR2Command);
            break;
        case 3:
            setScaleRange(12);
            if(isAdaptSn) sendToRadarString(mR3Command);
            break;
        case 4:
            setScaleRange(24);
            if(isAdaptSn) sendToRadarString(mR4Command);
            break;
        case 5:
            setScaleRange(48);
            if(isAdaptSn) sendToRadarString(mR5Command);
            break;
        case 6:
            setScaleRange(96);
            if(isAdaptSn) sendToRadarString(mR6Command);
            break;
        case 7:
            setScaleRange(192);
            if(isAdaptSn) sendToRadarString(mR7Command);
            break;
        default:
            setScaleRange(48);
            break;
        }
    }
    else if(mDistanceUnit==1)
    {
        switch(mRangeLevel)
        {
        case 0:
            setScaleRange(2.5);
            if(isAdaptSn) sendToRadarString(mR0Command);
            break;
        case 1:
            setScaleRange(5);
            if(isAdaptSn) sendToRadarString(mR1Command);
            break;
        case 2:
            setScaleRange(10);
            if(isAdaptSn) sendToRadarString(mR2Command);
            break;
        case 3:
            setScaleRange(20);
            if(isAdaptSn) sendToRadarString(mR3Command);
            break;
        case 4:
            setScaleRange(50);
            if(isAdaptSn) sendToRadarString(mR4Command);
            break;
        case 5:
            setScaleRange(100);
            if(isAdaptSn) sendToRadarString(mR5Command);
            break;
        case 6:
            setScaleRange(200);
            if(isAdaptSn) sendToRadarString(mR6Command);
            break;
        case 7:
            setScaleRange(400);
            if(isAdaptSn) sendToRadarString(mR7Command);
            break;
        default:
            break;
        }
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
//    pRadar->resetTrack();
//}

//void MainWindow::on_pushButton_avtodetect_toggled(bool checked)
//{
//    isDrawSubTg = !checked;
//    pRadar->avtodetect = checked;
//    pRadar->terrain_init_time = 3;
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
    pRadar->kgain = 7-(float)value/(ui->horizontalSlider_gain->maximum())*10;
    ui->label_gain->setText("Gain:"+QString::number(-pRadar->kgain));
    //printf("pRadar->kgain %f \n",pRadar->kgain);
}

void Mainwindow::on_horizontalSlider_rain_valueChanged(int value)
{
    pRadar->krain = (float)value/(ui->horizontalSlider_rain->maximum()+ui->horizontalSlider_rain->maximum()/3);
    ui->label_rain->setText("Rain:" + QString::number(pRadar->krain,'f',2));
}

void Mainwindow::on_horizontalSlider_sea_valueChanged(int value)
{
    pRadar->ksea = (float)value/(ui->horizontalSlider_sea->maximum());
    //ui->label_rain->setText("Rain:" + QString::number(-pRadar->krain));
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
    ExitProgram();
}

//void Mainwindow::on_toolButton_setting_clicked()
//{
//    this->on_actionSetting_triggered();
//}

/*
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
        */


void Mainwindow::on_toolButton_xl_nguong_toggled(bool checked)
{
    pRadar->setAutorgs(checked);
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
        processing->playRate = 200;
    }else
    {
        processing->playRate = 50;
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
//    pRadar->isDisplayAlpha = checked;
//}

/*
        void Mainwindow::updateTargets()
        {
            trackList* trackListPt = &pRadar->mTrackList;

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
                    ui->label_data_range->setText(QString::number(trackListPt->at(targetDisplayList.at(i)->trackId).estR*pRadar->scale_ppi/mScale/1.852f,'f',2)+"Nm");
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
        pRadar->dataOver = m_only;
        break;
    case 1:
        pRadar->dataOver = s_m_150;
        break;
    case 2:
        pRadar->dataOver = s_m_200;
        break;
    case 3:
        pRadar->dataOver = max_s_m_150;
        break;
    case 4:
        pRadar->dataOver = max_s_m_200;
        break;
    default:
        break;
    }

}

void Mainwindow::on_comboBox_img_mode_currentIndexChanged(int index)
{
    pRadar->imgMode = imgDrawMode(index) ;
}


void Mainwindow::on_toolButton_send_command_clicked()
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


void Mainwindow::on_toolButton_zoom_in_clicked()
{
    if(mRangeLevel >0) mRangeLevel-=1;
    mGlobbalConfig.setValue("mRangeLevel",mRangeLevel);
    UpdateScale();
    DrawMap();
}

void Mainwindow::on_toolButton_zoom_out_clicked()
{
    if(mRangeLevel <7) mRangeLevel+=1;
    mGlobbalConfig.setValue("mRangeLevel",mRangeLevel);
    UpdateScale();
    DrawMap();
}

//void Mainwindow::on_toolButton_reset_clicked()
//{
//    pRadar->resetSled();
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

void Mainwindow::SetGPS(double lat,double lon)
{
    mLat = lat;
    mLon = lon;
    mGlobbalConfig.setValue("mLat",mLat);
    mGlobbalConfig.setValue("mLon",mLon);
    ui->text_latInput_2->setText(QString::number(mLat,'f',5));
    ui->text_longInput_2->setText(QString::number(mLon,'f',5));
    osmap->setCenterPos(mLat, mLon);
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
    mHeadingAntenna = ui->textEdit_heading->text().toFloat();
    mHeadingSelsyn = ui->textEdit_heading_2->text().toFloat();
    mHeadingBase = ui->textEdit_heading_base->text().toFloat();
    mTrueN = mHeadingBase+mHeadingAntenna;
    mTrueN2 = mHeadingBase+ mHeadingSelsyn;
    mGlobbalConfig.setValue("mHeadingAntenna",mHeadingAntenna);
    mGlobbalConfig.setValue("mHeadingSelsyn",mHeadingSelsyn);
    mGlobbalConfig.setValue("mHeadingBase",mHeadingBase);
    mGlobbalConfig.setValue("mTrueN",mTrueN);
    mGlobbalConfig.setValue("mTrueN2",mTrueN2);
    pRadar->setTrueN(mTrueN);

}

void Mainwindow::on_toolButton_gps_update_clicked()
{

    SetGPS(ui->text_latInput_2->text().toFloat(),ui->text_longInput_2->text().toFloat());

}




//void Mainwindow::on_toolButton_centerZoom_clicked()
//{
//    pRadar->updateZoomRect(mousePointerX - scrCtX+dx,mousePointerY - scrCtY+dy);
//}

void Mainwindow::on_toolButton_xl_dopler_clicked()
{

}

void Mainwindow::on_toolButton_xl_dopler_toggled(bool checked)
{
    pRadar->xl_dopler = checked;
}


void Mainwindow::on_toolButton_xl_nguong_3_toggled(bool checked)
{
    pRadar->is_normalize_signal = checked;
    mGlobbalConfig.setValue("is_normalize_signal",checked);
}

void Mainwindow::on_groupBox_3_currentChanged(int index)
{
    if(index==1)
    {
        pRadar->isManualTune = true;
    }
    else
    {
        pRadar->isManualTune = false;
    }
}

void Mainwindow::on_toolButton_xl_dopler_2_toggled(bool checked)
{
    pRadar->bo_bang_0 = checked;
}



void Mainwindow::on_toolButton_reset_3_clicked()
{
    pRadar->resetTrack();
    //    for(short i = 0;i<targetDisplayList.size();i++)
    //    {
    //        targetDisplayList.at(i)->deleteLater();
    //    }
    //    targetDisplayList.clear();
}

void Mainwindow::on_toolButton_reset_2_clicked()
{
    pRadar->resetSled();
}




void Mainwindow::on_toolButton_vet_clicked(bool checked)
{
    pRadar->isSled = checked;
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
    //    pRadar->mTrackList.at(targetDisplayList.at(selected_target_index)->trackId).isManual = false;
}

void Mainwindow::on_toolButton_tx_clicked()
{
    //processing->radTxOn();
    sendToRadarString(mTxCommand);
}


void Mainwindow::on_toolButton_tx_off_clicked()
{
    //processing->radTxOff();
    sendToRadarString(mRxCommand);
}

void Mainwindow::on_toolButton_filter2of3_clicked(bool checked)
{
    pRadar->filter2of3 = checked;
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
    pRadar->data_export = checked;
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
    processing->m_aisList.clear();
}



//void Mainwindow::on_toolButton_2x_zoom_clicked(bool checked)
//{
//    if(checked)
//    {
//        pRadar->setScaleZoom(8);
//    }
//    else
//    {
//        pRadar->setScaleZoom(4);
//    }
//}

void Mainwindow::on_toolButton_auto_adapt_clicked()
{/*
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
            pRadar->resetTrack();*/
    //    for(short i = 0;i<targetDisplayList.size();i++)
    //    {
    //        targetDisplayList.at(i)->deleteLater();
    //    }
    //    targetDisplayList.clear();
}

void Mainwindow::on_toolButton_set_header_size_clicked()
{
    pRadar->SetHeaderLen(ui->textEdit_header_len->text().toInt());
}

//void Mainwindow::on_toolButton_xl_nguong_clicked()
//{

//}

//void Mainwindow::on_toolButton_xl_nguong_clicked(bool checked)
//{

//}

void Mainwindow::on_toolButton_filter2of3_2_clicked(bool checked)
{
    pRadar->setDoubleFilter(checked);
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
    mMapOpacity = value/50.0;
    mGlobbalConfig.setValue("mMapOpacity",mMapOpacity);
    DrawMap();
}



void Mainwindow::on_toolButton_selfRotation_toggled(bool checked)
{
    if(checked)
    {
        double rate = ui->lineEdit_selfRotationRate->text().toDouble();
        rate = rate/MAX_AZIR;
        pRadar->SelfRotationOn(rate);
    }
    else
        pRadar->SelfRotationOff();
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

void Mainwindow::on_toolButton_record_clicked()
{

}

void Mainwindow::on_toolButton_sharp_eye_toggled(bool checked)
{
    pRadar->setIsSharpEye(checked);
}

void Mainwindow::on_toolButton_help_clicked()
{

    DialogDocumentation *dlg=new DialogDocumentation();
    dlg->setModal(false);
    dlg->showNormal();
    printf("\nNew windows");

}

void Mainwindow::on_toolButton_setRangeUnit_clicked()
{
    switch(mDistanceUnit)
    {
    case 0:
        this->setDistanceUnit(1);
        break;
    case 1:
        this->setDistanceUnit(0);
        break;
    default:
        break;
    }


}



void Mainwindow::on_toolButton_tx_2_clicked()
{
    sendToRadarString(mFreq1Command);

}

void Mainwindow::on_toolButton_tx_3_clicked()
{
    sendToRadarString(mFreq2Command);
}

void Mainwindow::on_toolButton_tx_4_clicked()
{
    sendToRadarString(mFreq3Command);
}

void Mainwindow::on_toolButton_tx_5_clicked()
{
    sendToRadarString(mFreq4Command);
}

void Mainwindow::on_toolButton_tx_6_clicked()
{
    sendToRadarString(mFreq5Command);
}

void Mainwindow::on_toolButton_tx_7_clicked()
{
    sendToRadarString(mFreq6Command);
}

void Mainwindow::on_toolButton_gps_update_auto_clicked()
{
    double lat,lon;
    if(processing->getPosition(&lat,&lon))
    {
        mGlobbalConfig.setValue("mLat",lat);
        mGlobbalConfig.setValue("mLon",lon);
        SetGPS(lat, lon);
    }
    else
    {
        warningList.push_back("Chưa đủ tín hiệu vệ tinh");
    }

    DrawMap();
}


void Mainwindow::on_toolButton_set_zoom_ar_size_clicked()
{
    mZoomSizeRg = ui->textEdit_size_ar_r->text().toDouble();
    mZoomSizeAz = ui->textEdit_size_ar_a->text().toDouble();
    mGlobbalConfig.setValue("mZoomSizeRg",mZoomSizeRg);
    mGlobbalConfig.setValue("mZoomSizeAz",mZoomSizeAz);
}

void Mainwindow::on_toolButton_advanced_control_clicked()
{
    if(ui->lineEdit_password->text()=="ccndt3108")
    {
        ui->groupBox_control->setHidden(false);
    }
}



void Mainwindow::on_toolButton_set_command_clicked()
{
    mR0Command = ui->plainTextEdit_range_0->toPlainText();
    mR1Command = ui->plainTextEdit_range_1->toPlainText();
    mR2Command = ui->plainTextEdit_range_2->toPlainText();
    mR3Command = ui->plainTextEdit_range_3->toPlainText();
    mR4Command = ui->plainTextEdit_range_4->toPlainText();
    mR5Command = ui->plainTextEdit_range_5->toPlainText();
    mR6Command = ui->plainTextEdit_range_6->toPlainText();
    mR7Command = ui->plainTextEdit_range_7->toPlainText();
    mRxCommand = ui->plainTextEdit_command_rx->toPlainText();
    mTxCommand = ui->plainTextEdit_command_tx->toPlainText();
    mGlobbalConfig.setValue("mR0Command",mR0Command);
    mGlobbalConfig.setValue("mR1Command",mR1Command);
    mGlobbalConfig.setValue("mR2Command",mR2Command);
    mGlobbalConfig.setValue("mR3Command",mR3Command);
    mGlobbalConfig.setValue("mR4Command",mR4Command);
    mGlobbalConfig.setValue("mR5Command",mR5Command);
    mGlobbalConfig.setValue("mR6Command",mR6Command);
    mGlobbalConfig.setValue("mR7Command",mR7Command);
    mGlobbalConfig.setValue("mRxCommand",mRxCommand);
    mGlobbalConfig.setValue("mTxCommand",mTxCommand);
    //
    mFreq1Command =  ui->plainTextEdit_freq_1->toPlainText();
    mFreq2Command =  ui->plainTextEdit_freq_2->toPlainText();
    mFreq3Command =  ui->plainTextEdit_freq_3->toPlainText();
    mFreq4Command =  ui->plainTextEdit_freq_4->toPlainText();
    mFreq5Command =  ui->plainTextEdit_freq_5->toPlainText();
    mFreq6Command =  ui->plainTextEdit_freq_6->toPlainText();
    //
    mGlobbalConfig.setValue("mFreq1Command",mFreq1Command);
    mGlobbalConfig.setValue("mFreq2Command",mFreq2Command);
    mGlobbalConfig.setValue("mFreq3Command",mFreq3Command);
    mGlobbalConfig.setValue("mFreq4Command",mFreq4Command);
    mGlobbalConfig.setValue("mFreq5Command",mFreq5Command);
    mGlobbalConfig.setValue("mFreq6Command",mFreq6Command);
    ui->groupBox_control_setting->setHidden(true);
    ui->toolButton_set_commands->setChecked(false);

}

void Mainwindow::on_toolButton_grid_clicked(bool checked)
{
    update();
}

void Mainwindow::on_toolButton_auto_freq_toggled(bool checked)
{
    if(checked) autoSwitchFreq();
}

void Mainwindow::on_toolButton_set_default_clicked()
{
    mGlobbalConfig.setDefault();
}



void Mainwindow::on_toolButton_heading_update_clicked()
{
    if(processing->isHeadingAvaible)
    {
        mTrueN = processing->getHeading()+mGlobbalConfig.getDouble("mTrueN3");
        if(mTrueN>=360)mTrueN-=360;
        //ui->label_compass_value->setText(QString::number(processing->getHeading(),'f',1));
        ui->textEdit_heading->setText(QString::number(mTrueN));
    }
    else
    {
        warningList.push_back(QString::fromUtf8("Chưa kết nối la bàn"));
    }
}

void Mainwindow::on_toolButton_sled_clicked()
{

}

void Mainwindow::on_toolButton_sled_toggled(bool checked)
{
    pRadar->isSled=checked;
}

void Mainwindow::on_toolButton_sled_reset_clicked()
{
    pRadar->resetSled();
}

void Mainwindow::on_toolButton_ais_name_toggled(bool checked)
{

}

void Mainwindow::on_toolButton_filter2of3_toggled(bool checked)
{

}

void Mainwindow::on_toolButton_dobupsong_clicked()
{

}

void Mainwindow::on_toolButton_dobupsong_toggled(bool checked)
{
    pRadar->is_do_bup_song = checked;
    if(checked)
    {
        pRadar->setTb_tap_k(ui->textEdit_dobupsongk->text().toDouble());

    }
}

void Mainwindow::on_toolButton_set_commands_toggled(bool checked)
{

}

void Mainwindow::on_toolButton_set_commands_clicked()
{
    ui->groupBox_control_setting->setHidden(false);
    ui->plainTextEdit_range_0->setPlainText(mR0Command);
    ui->plainTextEdit_range_1->setPlainText(mR1Command);
    ui->plainTextEdit_range_2->setPlainText(mR2Command);
    ui->plainTextEdit_range_3->setPlainText(mR3Command);
    ui->plainTextEdit_range_4->setPlainText(mR4Command);
    ui->plainTextEdit_range_5->setPlainText(mR5Command);
    ui->plainTextEdit_range_6->setPlainText(mR6Command);
    ui->plainTextEdit_range_7->setPlainText(mR7Command);
    ui->plainTextEdit_command_rx->setPlainText(mRxCommand);
    ui->plainTextEdit_command_tx->setPlainText(mTxCommand);
    ui->plainTextEdit_freq_1->setPlainText(mFreq1Command);
    ui->plainTextEdit_freq_2->setPlainText(mFreq2Command);
    ui->plainTextEdit_freq_3->setPlainText(mFreq3Command);
    ui->plainTextEdit_freq_4->setPlainText(mFreq4Command);
    ui->plainTextEdit_freq_5->setPlainText(mFreq5Command);
    ui->plainTextEdit_freq_6->setPlainText(mFreq6Command);
}

void Mainwindow::on_toolButton_command_log_toggled(bool checked)
{
    if(checked)
    {
        cmLog->show();
    }
    else
    {
        cmLog->hide();
    }
}

void Mainwindow::on_toolButton_exit_2_clicked()
{
    mstatWin = new StatusWindow(processing);

    mstatWin->show();
}

void Mainwindow::on_toolButton_selfRotation_2_toggled(bool checked)
{
    pRadar->isEncoderAzi  = checked;
    if(checked)
    {
        double rate = ui->lineEdit_selfRotationRate->text().toDouble();
        rate = rate/MAX_AZIR;
        pRadar->SelfRotationOn(rate);
    }
    else
        pRadar->SelfRotationOff();
}

void Mainwindow::on_toolButton_selfRotation_clicked()
{

}

void Mainwindow::on_toolButton_tx_2_toggled(bool checked)
{

}

void Mainwindow::on_toolButton_tx_2_clicked(bool checked)
{

}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pkp.h"
//#define mapWidth 2000
//#define mapWidth mapWidth
//#define mapHeight mapWidth
#define CONST_NM 1.845f
#define MAX_VIEW_RANGE_KM 50

#include <queue>

//static bool                 isAddingTarget=false;
static QPixmap              *pMap=NULL;
//static QImage               *sgn_img = new QImage(RADAR_MAX_RESOLUTION*2,RADAR_MAX_RESOLUTION*2,QImage::Format_RGB32);
dataProcessingThread *processing;
static Q_vnmap              vnmap;
QTimer*                     scrUpdateTimer,*readBuffTimer ;
QTimer*                     syncTimer1s,*syncTimer10s ;
QTimer*                     dataPlaybackTimer ;
QThread*                    t,*t1 ;
bool displayAlpha = false;
QList<CTarget*> targetList;
//static short                currMaxRange = RADAR_MAX_RESOLUTION;
static short                currMaxAzi = MAX_AZIR,currMinAzi = 0;
static short                dxMax,dyMax;
static C_ARPA_data          arpa_data;
//static short                curAzir=-1;//,drawAzir=0;
static float                signsize,sn_scale;
static short                scrCtX, scrCtY, dx =0,dy=0,dxMap=0,dyMap=0;//mouseX,mouseY;
static short                mousePointerX,mousePointerY,mouseX,mouseY;
static char                 gridOff = false;
static char                 udpFailure = 0;//config file !!!
static bool                 isDraging = false;
static bool                 isScaleChanged =true;
float                   scale;

static QStandardItemModel   modelTargetList;
struct
CConfig         config;
QStringList warningList;
short selected_target_index;
enum drawModes{
    SGN_DIRECT_DRAW,SGN_IMG_DRAW,NOTERR_DRAW
}drawMode = SGN_IMG_DRAW;
short range = 1;
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
        float azi,rg;
        processing->radarData->getPolar((x1 - scrCtX+dx)/scale,-(y1 - scrCtY+dy)/scale,&minAzi,&minR);
        processing->radarData->getPolar((x2 - scrCtX+dx)/scale,-(y2 - scrCtY+dy)/scale,&maxAzi,&maxR);
        if(minAzi<0)minAzi += PI_NHAN2;
        minAzi = minAzi/PI*180.0;
        if(maxAzi<0)maxAzi += PI_NHAN2;
        maxAzi = maxAzi/PI*180.0;
    }
};
guard_zone_t gz1,gz2,gz3;
//static unsigned short cur_object_index = 0;
short lon2x(float lon)
{

   float refLat = (config.m_config.m_lat )*0.00872664625997;
   return  (- dx + scrCtX + ((lon - config.m_config.m_long) * 111.31949079327357f*cosf(refLat))*scale);
}
short lat2y(float lat)
{

   return (- dy + scrCtY - ((lat - config.m_config.m_lat) * 111.31949079327357f)*scale);
}
double y2lat(short y)
{
   return (y  )/scale/111.31949079327357f + config.m_config.m_lat;
}
double x2lon(short x)
{
    float refLat = (config.m_config.m_lat )*0.00872664625997;
   return (x  )/scale/111.31949079327357f/cosf(refLat) + config.m_config.m_long;
}
void Mainwindow::mouseDoubleClickEvent( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton )
    {
        mousePointerX = (e->x());
        mousePointerY = (e->y());
        processing->radarData->updateZoomRect(mousePointerX - scrCtX+dx,mousePointerY - scrCtY+dy);
    }
}
void Mainwindow::sendToRadarHS(const char* hexdata)
{
    short len = strlen(hexdata)/2+1;
    unsigned char* sendBuff = new unsigned char[len];
    hex2bin(hexdata,sendBuff);
    m_udpSocket->writeDatagram((char*)sendBuff,8,QHostAddress("192.168.0.44"),2572);
    delete[] sendBuff;

}
void Mainwindow::sendToRadar(unsigned char* hexdata)
{

    m_udpSocket->writeDatagram((char*)hexdata,8,QHostAddress("192.168.0.44"),2572);
    //printf("\a");

}

void Mainwindow::mouseReleaseEvent(QMouseEvent *event)
{

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
    isDraging = false;
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
    //if(event->delta()>0)ui->horizontalSlider->setValue(ui->horizontalSlider->value()+1);
    //if(event->delta()<0)ui->horizontalSlider->setValue(ui->horizontalSlider->value()-1);
}
void Mainwindow::mouseMoveEvent(QMouseEvent *event) {
    //draw mouse coordinates
//    float azi,range;
//    processing->radarData->getPolar((mousePointerX - scrCtX+dx)/scale,-(mousePointerY - scrCtY+dy)/scale,&azi,&range);
//    if(azi<0)azi+=PI_NHAN2;
//    azi = azi/PI*180.0;
//    range = range/CONST_NM;
//    p.drawText(mousePointerX+5,mousePointerY+5,100,20,0,QString::number(range,'g',4)+"|"+QString::number(azi,'g',4),0);
//    ui->label_cursor_range->setText(QString::number(range,'g',6));
//    ui->label_cursor_azi->setText(QString::number(azi,'g',6));
//    //

    if(isDraging&&(event->buttons() & Qt::LeftButton)) {

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
//        isScreenUp2Date = false;
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
void Mainwindow::mousePressEvent(QMouseEvent *event)
{
    if(event->x()>scrCtX+scrCtY)return;
    mouseX = (event->x());
    mouseY = (event->y());
    if(event->buttons() & Qt::LeftButton) {
        if(ui->toolButton_manual_track->isChecked())
        {
            float xRadar = (mouseX - scrCtX+dx) ;//coordinates in  radar xy system
            float yRadar = -(mouseY - scrCtY+dy);
            processing->radarData->addTrackManual(xRadar,yRadar);
            ui->toolButton_manual_track->setChecked(false);
//            isScreenUp2Date = false;
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
            isDraging = true;
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

    dxMap = 0;
    dyMap = 0;
    QPainter p(pMap);
    pMap->fill(QColor(10,18,25,255));
    //pMap->fill(Qt::transparent);
    if(ui->toolButton_map->isChecked())
    {
        QPen pen(QColor(255,255,255,180));
        QColor color[5];
        color[0].setRgb(143,137,87,255);//land
        color[1].setRgb( 34,52,60,255);//lake
        color[2].setRgb(60,50,10,255);//building
        color[3].setRgb( 34,52,60,255);//river
        color[4].setRgb(70,70,70,150);//road
        //color[0].setRgb(120,120,120,150);//land
        //color[1].setRgb( 120,120,120,150);//lake
        //color[2].setRgb(120,120,120,150);//building
        //color[3].setRgb( 120,120,120,150);//river
        //color[4].setRgb(0,100,120,150);//road

        short centerX = pMap->width()/2-dx;
        short centerY = pMap->height()/2-dy;
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(pen);
        //-----draw provinces in polygons


        for(uint i = 0; i < N_LAYER; i++) {
            //printf("vnmap.layers[%d].size()%d\n",i,vnmap.layers[i].size());
            if(i<3)
            {
                for(uint j = 0; j < vnmap.layers[i].size(); j++) {
                    QPolygon poly;
                    for(uint k = 0; k < vnmap.layers[i][j].size(); k++) { // Polygon
                        QPoint int_point;
                        float x,y;
                        vnmap.ConvDegToScr(&x,&y,&vnmap.layers[i][j][k].m_Long,&vnmap.layers[i][j][k].m_Lat);
                        int_point.setX((int)(x*scale)+centerX);
                        int_point.setY((int)(y*scale)+centerY);
                        poly<<int_point;
                    }
                    p.setBrush(color[i]);
                    pen.setColor(color[i]);
                    p.setPen(pen);
                    p.drawPolygon(poly);
                }
            }else
            {
                //pen.setColor(color[i]);
                if(i==3)pen.setWidth(2);else pen.setWidth(1);
                p.setPen(pen);
                for(uint j = 0; j < vnmap.layers[i].size(); j++) {

                    QPoint old_point;

                    for(uint k = 0; k < vnmap.layers[i][j].size(); k++) { // Polygon
                        QPoint int_point;
                        float x,y;
                        vnmap.ConvDegToScr(&x,&y,&vnmap.layers[i][j][k].m_Long,&vnmap.layers[i][j][k].m_Lat);
                        int_point.setX((int)(x*scale)+centerX);
                        int_point.setY((int)(y*scale)+centerY);
                        if(k)p.drawLine(old_point,int_point);
                        old_point=int_point;
                    }
                    //p.setBrush(color[i]);


                }
            }

        }
    }
    //DrawGrid(&p,centerX,centerY);
    //draw text
    if(ui->toolButton_map_2->isChecked())
    {
        QPen pen;
        pen.setColor(QColor(255,255,255));
        pen.setWidth(2);
        pen.setStyle(Qt::SolidLine);
        short centerX = pMap->width()/2-dx;
        short centerY = pMap->height()/2-dy;
        p.setPen(pen);
        QFont font ;
        font.setPointSize(12);
        p.setFont(font);
        for(uint i = 0; i < vnmap.placeList.size(); i++) {
                QPoint int_point;
                float x,y;
                vnmap.ConvDegToScr(&x,&y,&vnmap.placeList[i].m_Long,&vnmap.placeList[i].m_Lat);
                int_point.setX((int)(x*scale)+centerX);
                int_point.setY((int)(y*scale)+centerY);
                p.drawEllipse(int_point,2,2);
                QString str = QString::fromStdWString(vnmap.placeList[i].text);
                str.chop(2);
                p.drawText(int_point.x()+5,int_point.y(),str);
                //printf("toa do hien tai lat %f long %f\n",m_textList[i].m_Lat,m_textList[i].m_Long);
        }
    }

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
                  (short)(i*rangeStep*CONST_NM*scale),
                  (short)(i*rangeStep*CONST_NM*scale));
    }



        //p.drawEllipse(QPoint(centerX,centerY),(int)(20*CONST_NM*scale),(int)(20*CONST_NM*scale));
        //p.drawEllipse(QPoint(centerX,centerY),(int)(5*CONST_NM*scale),(short)(5*CONST_NM*scale));
        //pen.setWidth(1);
        //p->setPen(pen);
        short theta;
        short gridR = rangeStep*1.852f*scale*7;
        for(theta=0;theta<360;theta+=90){
            QPoint point1,point2;
                short dx = gridR*cosf(PI*theta/180);
                short dy = gridR*sinf(PI*theta/180);
                point1.setX(centerX);
                point1.setY(centerY);
                point2.setX(centerX+dx);
                point2.setY(centerY+dy);
                p->drawLine(point1,point2);

        }
        for(theta=0;theta<360;theta+=30){
            QPoint point1,point2;
                short dx = gridR*cosf(PI*theta/180);
                short dy = gridR*sinf(PI*theta/180);
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

void Mainwindow::DrawTarget(QPainter* p)
{
    //draw radar  target:
    QPen penTargetRed(Qt::magenta);
    penTargetRed.setWidth(2);
    //QPen penARPATarget(Qt::yellow);
    //penARPATarget.setWidth(3);
    QPen penTargetBlue(Qt::cyan);
    penTargetBlue.setWidth(2);
    //penTargetBlue.setStyle(Qt::DashLine);
    QPen penTrack;
    penTrack.setWidth(2);
    penTrack.setColor(QColor(250,50,250,150));
    //QPen penARPATrack(Qt::darkYellow);
    //draw radar targets
    float x,y;
    short sx,sy;
    //short targetId = 0;
    if(true)
    {
        trackList* trackListPt = &processing->radarData->mTrackList;
        for(uint trackId=0;trackId<trackListPt->size();trackId++)
        {
            if(!trackListPt->at(trackId).isConfirmed)continue;
            if(!trackListPt->at(trackId).state)continue;

                x= trackListPt->at(trackId).estX*processing->radarData->scale_ppi/scale;
                y= trackListPt->at(trackId).estY*processing->radarData->scale_ppi/scale;
                if(trackListPt->at(trackId).dopler==17)
                {
                    sx = trackListPt->at(trackId).estX*processing->radarData->scale_ppi + scrCtX - dx;
                    sy = -trackListPt->at(trackId).estY*processing->radarData->scale_ppi + scrCtY - dy;
                    p->setPen(penTargetBlue);

                    p->drawEllipse(sx-6,sy-6,12,12);

                    continue;
                }

                short tid =0;
                for(;tid<targetList.size();tid++)
                {
                    if(targetList.at(tid)->isUsed)
                    if(targetList.at(tid)->trackId==trackId)
                    {
                        targetList.at(tid)->setPosistion(x,y);
                        targetList.at(tid)->m_lat = y2lat(trackListPt->at(trackId).estY*processing->radarData->scale_ppi);
                        targetList.at(tid)->m_lon = x2lon(trackListPt->at(trackId).estX*processing->radarData->scale_ppi);
                        targetList.at(tid)->show();
                        tid--;
                        break;
                    }
                }
                if(tid>=targetList.size())// add new target
                {
//                    tid =0;
//                    for(;tid<targetList.size();tid++)
//                    {
//                        //if(trackListPt->at(targetList.at(tid)->trackId).state == 0)
//                        if(!targetList.at(tid)->isUsed)
//                        {
//                            targetList.at(tid)->trackId = trackId;
//                            targetList.at(tid)->isUsed = true;
//                            targetList.at(tid)->show();
//                            targetList.at(tid)->setPosistion(x,y);
//                        }
//                    }
//                    if(tid==targetList.size())
//                    {
                        CTarget*  tg1 = new CTarget(this);
                        tg1->isManual = trackListPt->at(trackId).isManual;
                        tg1->trackId = trackId;
                        tg1->isUsed = true;
                        tg1->show();
                        tg1->m_lat = y2lat(trackListPt->at(trackId).estY*processing->radarData->scale_ppi);
                        tg1->m_lon = x2lon(trackListPt->at(trackId).estX*processing->radarData->scale_ppi);
                        tg1->setPosistion(x,y);
                        targetList.append(tg1);
//                    }
                }

//                draw track:
                if(trackListPt->at(trackId).isManual)p->setPen(penTargetRed);
                else p->setPen(penTargetBlue);
                short k=0;

                p->drawText(sx+10,sy+10,300,40,0,QString::number(tid+1));
                for(short j=(trackListPt->at(trackId).object_list.size()-1);j>0;j-=1)
                {
                    k++;
                    if(k>40)break;

                    sx = trackListPt->at(trackId).object_list.at(j).x*processing->radarData->scale_ppi + scrCtX - dx;
                    sy = -trackListPt->at(trackId).object_list.at(j).y*processing->radarData->scale_ppi + scrCtY - dy;
                    p->drawPoint(sx,sy);
                }

                if(trackListPt->at(trackId).object_list.size()>12)
                {
                    sx = trackListPt->at(trackId).estX*processing->radarData->scale_ppi + scrCtX - dx;
                    sy =-trackListPt->at(trackId).estY*processing->radarData->scale_ppi + scrCtY - dy;
                    p->drawLine(sx,sy,sx+15*sinf(trackListPt->at(trackId).head_r),sy-15*cosf(trackListPt->at(trackId).head_r));
                }
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
            sprintf(buf, "%3d:%3.3fNM:%3.3f\xB0",processing->arpaData->track_list[i].id,processing->arpaData->track_list[i].centerR/DEFAULT_NM, processing->arpaData->track_list[i].centerA*57.2957795);
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
void Mainwindow::drawAisTarget(QPainter *p)
{

    //draw radar  target:
    QPen penTargetRed(Qt::cyan);
    penTargetRed.setWidth(0);

    for(uint i=0;i<m_trackList.size();i++)
    {
            p->setPen(penTargetRed);
            short j;
            //draw track:
            float fx,fy;
            float mlat = m_trackList.at(i).m_Lat ;
            mlat =  mlat/bit23* 180.0f ;
            float mlon = m_trackList.at(i).m_Long;
            mlon = mlon/bit23* 180.0f ;
                vnmap.ConvDegToScr(&fx,&fy,&mlon,&mlat);

                short x = (fx*scale)+scrCtX-dx;
                short y = (fy*scale)+scrCtY-dy;
                p->drawEllipse(x-5,y-5,10,10);
                p->drawText(x+5,y+5,QString::fromAscii((char*)&m_trackList.at(i).m_MMSI[0],9));
                //printf("\nj:%d,%d,%d,%f,%f",j,x,y,arpa_data.ais_track_list[i].object_list[j].mlong,arpa_data.ais_track_list[i].object_list[j].mlat);

    }
}
void Mainwindow::paintEvent(QPaintEvent *event)
{
    event = event;
//    isScreenUp2Date = true;
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

    DrawTarget(&p);
    drawAisTarget(&p);
    //draw cursor
    QPen penmousePointer(QColor(0x50ffffff));

    penmousePointer.setWidth(2);
    p.setPen(penmousePointer);
    p.drawLine(mousePointerX-15,mousePointerY,mousePointerX-10,mousePointerY);
    p.drawLine(mousePointerX+15,mousePointerY,mousePointerX+10,mousePointerY);
    p.drawLine(mousePointerX,mousePointerY-10,mousePointerX,mousePointerY-15);
    p.drawLine(mousePointerX,mousePointerY+10,mousePointerX,mousePointerY+15);
    //draw mouse coordinates
    float azi,rg;
    short   x=this->mapFromGlobal(QCursor::pos()).x();
    short   y=this->mapFromGlobal(QCursor::pos()).y();

    if(ui->toolButton_measuring->isChecked())
    {
        processing->radarData->getPolar((x - mouseX)/scale,-(y - mouseY)/scale,&azi,&rg);
    }
    else
    {
        processing->radarData->getPolar((x - scrCtX+dx)/scale,-(y - scrCtY+dy)/scale,&azi,&rg);
    }
    if(azi<0)azi+=PI_NHAN2;
    azi = azi/PI*180.0;
    rg = rg/CONST_NM;
//    p.drawText(mousePointerX+5,mousePointerY+5,100,20,0,QString::number(range,'g',4)+"|"+QString::number(azi,'g',4),0);

    ui->label_cursor_range->setText(QString::number(rg,'g',6));
    ui->label_cursor_azi->setText(QString::number(azi,'g',6));
    ui->label_cursor_lat->setText(QString::number( (short)y2lat(-(y - scrCtY+dy)))+"\xB0"+
                                  QString::number(((float)y2lat(-(y - scrCtY+dy))-(short)(y2lat(-(y - scrCtY+dy))))*60,'g',4)+"N");
    ui->label_cursor_long->setText(QString::number( (short)x2lon(x - scrCtX+dx))+"\xB0"+
                                       QString::number(((float)x2lon(x - scrCtX+dx)-(short)(x2lon(x - scrCtX+dx)))*60,'g',4)+"E");

    //draw zooom

    //draw frame

    DrawViewFrame(&p);
    if(ui->tabWidget_2->currentIndex()==2)
    {
        QRect rect = ui->tabWidget_2->geometry();
        if(range>2)
        {

            short zoom_size = ui->tabWidget_2->width()/processing->radarData->scale_zoom*processing->radarData->scale_ppi;
            p.setPen(QPen(QColor(255,255,255,200),0,Qt::DashLine));
            p.drawRect(mousePointerX-zoom_size/2.0,mousePointerY-zoom_size/2.0,zoom_size,zoom_size);
        }

        //QRect zoomRect(DISPLAY_RES-100,DISPLAY_RES-100,200,200);
        rect.adjust(4,30,-5,-5);
        p.setPen(QPen(Qt::black));
        p.setBrush(QBrush(Qt::black));
        p.drawRect(rect);
        p.drawImage(rect,*processing->radarData->img_zoom_ppi,processing->radarData->img_zoom_ppi->rect());

    }
    updateTargets();
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


bool Mainwindow::LoadISMapFile()
{
    if(config.m_config.mapFilename.size())
    {
        vnmap.LoadBinFile((config.m_config.mapFilename).data());
    }else return false;
    return true;
}
void Mainwindow::SaveBinFile()
{
    //vnmap.SaveBinFile();

}
void Mainwindow::InitSetting()
{
    setMouseTracking(true);
    //init the guard zone
    gz1.isActive = 0;
    gz2.isActive = 0;
    gz3.isActive = 0;
    QRect rec = QApplication::desktop()->screenGeometry(0);
    setFixedSize(1920,1080);
    if((rec.height()==1080)&&(rec.width()==1920))
    {
        this->showFullScreen();
        this->setGeometry(QApplication::desktop()->screenGeometry(0));//show on first screen
    }
    else
    {

        rec = QApplication::desktop()->screenGeometry(1);
        if((rec.height()==1080)&&(rec.width()==1920))
        {
            this->showFullScreen();
            //printf("error");
            this->setGeometry(QApplication::desktop()->screenGeometry(1));//show on second screen
            //setFixedSize(QApplication::desktop()->screenGeometry(1));
        }

    }

    dxMax = height()/4-10;
    dyMax = height()/4-10;
    mousePointerX = scrCtX = height()/2+50;//+ ui->toolBar_Main->width()+20;//ENVDEP
    mousePointerY = scrCtY = height()/2;
    UpdateScale();
    ui->textEdit_heading->setText(QString::number(config.m_config.trueN));
    processing->radarData->setTrueN(config.m_config.trueN);
    //ui->horizontalSlider_2->setValue(config.m_config.cfarThresh);

    ui->horizontalSlider_brightness->setValue(ui->horizontalSlider_brightness->maximum()/4);
    ui->horizontalSlider_gain->setValue(ui->horizontalSlider_gain->maximum());
    ui->horizontalSlider_rain->setValue(ui->horizontalSlider_rain->minimum());
    ui->horizontalSlider_sea->setValue(ui->horizontalSlider_sea->minimum());
    //ui->tabWidget_2->processing = processing;
    //ui->horizontalSlider_signal_scale->setValue(ui->horizontalSlider_sea->minimum());
    ui->comboBox_radar_resolution->setCurrentIndex(0);
    setCursor(QCursor(Qt::CrossCursor));
    range = 5; UpdateScale();
    if(true)
    {
        SetGPS(config.m_config.m_lat, config.m_config.m_long);
        //vnmap.setUp(config.m_config.m_lat, config.m_config.m_long, 200,config.m_config.mapFilename.data());
        if(pMap)delete pMap;
        if(gridOff==false)pMap = new QPixmap(height(),height());
        else pMap = new QPixmap(width(),height());
        DrawMap();


    }else
    {
        vnmap.ClearData();
        if(pMap)delete  pMap;
        pMap = NULL;
    }

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
    if(displayAlpha){

        pengrid.setWidth(10);
        p->setPen(pengrid);
         p->drawImage(10,height()-266,*processing->radarData->img_alpha);
        p->drawRect(5,height()-266,processing->radarData->img_alpha->width()+5,processing->radarData->img_alpha->height()+5);
        pengrid.setWidth(2);
        pengrid.setColor(QColor(128,128,0,120));
        p->setPen(pengrid);
        for(short i=60;i<processing->radarData->img_alpha->height();i+=50)
        {
            p->drawLine(0,height()-i,processing->radarData->img_alpha->width()+5,height()-i);
        }
        for(short i=110;i<processing->radarData->img_alpha->width();i+=100)
        {
            p->drawLine(i,height()-266,i,height());
        }
    }

    //HDC dc = ui->tabWidget->getDC();
}
void Mainwindow::setScaleNM(unsigned short rangeNM)
{
    float oldScale = scale;
    scale = (float)height()/((float)rangeNM*CONST_NM)*0.7f;
    //printf("scale:%f- %d",scale,rangeNM);
    isScaleChanged = true;// scale*SIGNAL_RANGE_KM/2048.0f;

    dyMax = MAX_VIEW_RANGE_KM*scale;
    dxMax = dyMax;
    dx =short(scale/oldScale*dx);
    dy =short(scale/oldScale*dy);
    DrawMap();
    /*currMaxRange = (sqrtf(dx*dx+dy*dy)+scrCtY)/signsize;
    if(currMaxRange>RADAR_MAX_RESOLUTION)currMaxRange = RADAR_MAX_RESOLUTION;*/
//    isScreenUp2Date = false;
}
short waittimer =0;
void Mainwindow::UpdateRadarData()
{
    if(!processing->getIsDrawn())
    {
        if(processing->radarData->isClkAdcChanged)
        {
            ui->comboBox_radar_resolution->setCurrentIndex(processing->radarData->clk_adc);
            processing->radarData->setScalePPI(scale);
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
    processing->ReadDataBuffer();
}
void Mainwindow::InitTimer()
{

    scrUpdateTimer = new QTimer();
    syncTimer1s = new QTimer();
    syncTimer10s = new QTimer();
    readBuffTimer = new QTimer();
    dataPlaybackTimer = new QTimer();
    t = new QThread();
    t1 = new QThread();
    processing = new dataProcessingThread();

//    connect(fullScreenTimer, SIGNAL(timeout()), this, SLOT(UpdateSetting()));
//    fullScreenTimer->setSingleShot(true);
//    fullScreenTimer->start(1000);
    connect(syncTimer1s, SIGNAL(timeout()), this, SLOT(sync1()));
    syncTimer1s->start(1000);
    connect(syncTimer10s, SIGNAL(timeout()), this, SLOT(sync10()));
    syncTimer10s->start(300000);
    //syncTimer1s->moveToThread(t);
    //
    connect(readBuffTimer,SIGNAL(timeout()),this,SLOT(readBuffer()));
    readBuffTimer->start(50);
    readBuffTimer->moveToThread(t1);
    //
    connect(scrUpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateRadarData()));

    scrUpdateTimer->start(30);//ENVDEP
    scrUpdateTimer->moveToThread(t);


    processing->start();
    connect(this,SIGNAL(destroyed()),processing,SLOT(deleteLater()));
    connect(dataPlaybackTimer,SIGNAL(timeout()),processing,SLOT(playbackRadarData()));
    //dataPlaybackTimer->moveToThread(t);
    connect(t,SIGNAL(finished()),t,SLOT(deleteLater()));
    t->start(QThread::TimeCriticalPriority);
    t1->start(QThread::TimeCriticalPriority);
}
void Mainwindow::InitNetwork()
{
    //connect(&playbackTimer, SIGNAL(timeout()), this, SLOT(drawSign()));

    //countdown = new CountdownReceiverDlg();

    //QHostAddress mHost = QHostAddress("224.110.210.226");
//    udpSocket = new QUdpSocket(this);
//    udpSocket->bind(5000, QUdpSocket::ShareAddress);
//    //udpSocket->joinMulticastGroup(mHost);
//    connect(udpSocket, SIGNAL(readyRead()),
//            this, SLOT(processFrame()));

        m_udpSocket = new QUdpSocket(this);
        if(!m_udpSocket->bind(8900))
        {
            if(!m_udpSocket->bind(8901))
            {
                m_udpSocket->bind(8902);
            }
        }
        m_udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 10);

//    udpARPA = new QUdpSocket(this);
//    udpARPA->bind(1990,QUdpSocket::ShareAddress);
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
        if(list.first()=="$RATTM")
        {
//            short tNum = (*(list.begin()+1)).toInt();
//            float tDistance = (*(list.begin()+2)).toFloat();
//            float tRange = (*(list.begin()+3)).toFloat();
//            arpa_data.addARPA(tNum,tDistance,tRange);
        }
        else if(list.first().contains("AI"))
        {
            ProcDataAIS((BYTE*)datagram.data(), datagram.size());
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
void Mainwindow::sync10()//period 10 second
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

void Mainwindow::sync1()//period 1 second
{
    // display radar temperature:
    ui->label_temp->setText(QString::number(processing->radarData->tempType)+"|"+QString::number(processing->radarData->temp)+"\260C");


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
            str.append(QString::number(trackListPt->at(i).estR*signsize/scale/CONST_NM,'g',3)+" | ");
            str.append(QString::number((short)(trackListPt->at(i).estA*57.2957795f),'g',3)+" | ");
            str.append(QString::number((short)(targetSpeed),'g',4)+" | ");
            str.append(QString::number((short)(trackListPt->at(i).course*57.2957795f),'g',3)+" | ");
            if(items.size())
            {
                (items[0])->setText(str);
            }else
            {
                ui->listTargetWidget->addItem(str);
            }
        }

    }*/
    //check if data too large
    /*if(processing->radarData->getDataOverload())
    {
        if(config.m_config.cfarThresh<30)config.m_config.cfarThresh++;
        ui->horizontalSlider_2->setValue(config.m_config.cfarThresh);
        isCfarThreshChanged = true;
    }*/
    //update cfar thresh value:

    //update signsize
    if(isScaleChanged ) {

        processing->radarData->setScalePPI(scale);
        isScaleChanged = false;
    }
    //update signal code:

    //display time
    showTime();
    // require temperature
    if(radar_state!=DISCONNECTED)
    {
        unsigned char bytes[8];
        bytes[0] = 0xaa;
        bytes[1] = 0xab;
        bytes[2] = ui->comboBox_temp_type->currentIndex();
        bytes[3] = 0xaa;
        bytes[4] = 0x00;
        bytes[5] = 0x00;
        bytes[6] = 0x00;
        bytes[7] = 0;
        sendToRadar(&bytes[0]);
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
        m_udpSocket->writeDatagram("d",1,QHostAddress("127.0.0.1"),8089);
        break;
    case CONNECTED:
        //printf("\ns_tx");
        ui->label_status->setText(QString::number(processing->radarData->overload));
        ui->label_command->setHidden(false);

        ui->label_command->setText(QString(array.toHex()));
        m_udpSocket->writeDatagram("c",1,QHostAddress("127.0.0.1"),8089);
        break;
    case CONNECTED_ROTATE12_TXON:
        ui->label_status->setText(QString::fromUtf8("Phát 12v/p"));
        break;
    case CONNECTED_ROTATE12_TXOFF:
        ui->label_status->setText(QString::fromUtf8("Quay 12v/p"));
        break;
    }

    switch((processing->radarData->sn_stat>>8)&0x07)
    {
    case 4:
        ui->label_sn_type->setText("Ma DTTT");
        ui->label_sn_param->setText(QString::number((((processing->radarData->sn_stat)&0x07)+1)*128));
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
        ui->label_speed->setText("9 v/p");break;
    case 2:
        ui->label_speed->setText("12 v/p");break;
    case 3:
        ui->label_speed->setText("0 v/p");break;

    default:

        break;
    }




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
    processing->startReplay(filename);
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
    GPSDialog *dlg = new GPSDialog(this);
    dlg->setModal(false);
    dlg->loadConfig(&config);
    dlg->show();
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(dlg, SIGNAL(destroyed(QObject*)), SLOT(UpdateSetting()));
    connect(dlg, SIGNAL(destroyed(QObject*)), SLOT(setCodeType()));
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

void Mainwindow::on_actionView_grid_triggered(bool checked)
{
    gridOff = checked;
    dx=0;dy=0;
    DrawMap();
    //UpdateSetting();
}


void Mainwindow::on_actionPlayPause_toggled(bool arg1)
{
    processing->togglePlayPause(arg1);
    if(arg1)dataPlaybackTimer->start(25);else dataPlaybackTimer->stop();

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
void Mainwindow::UpdateScale()
{
    float oldScale = scale;
    char byte2;
    switch(range)
    {
    case 0:
        scale = (height()/2-5)/(CONST_NM*1.5f );
        rangeStep = 1.5f/6.0f;
        byte2 = 0x00;
        ui->label_range->setText("1.5 NM");

        break;
    case 1:
        scale = (height()/2-5)/(CONST_NM*3 );
        rangeStep = 3/6.0f;
        byte2 = 0x00;
        ui->label_range->setText("3 NM");
        break;
    case 2:
        scale = (height()/2-5)/(CONST_NM*6 );
        rangeStep = 6/6.0f;
        byte2 = 0x00;
        ui->label_range->setText("6 NM");
        break;
    case 3:
        scale = (height()/2-5)/(CONST_NM*12 );
        rangeStep = 12/6.0f;
        byte2 = 0x00;
        ui->label_range->setText("12 NM");
        break;
    case 4:
        scale = (height()/2-5)/(CONST_NM*24 );
        rangeStep = 24/6.0f;
        byte2 = 0x01;
        ui->label_range->setText("24 NM");
        break;
    case 5:
        scale = (height()/2-5)/(CONST_NM*36 );
        rangeStep = 36/6.0f;
        byte2 = 0x02;
        ui->label_range->setText("36 NM");
        break;
    case 6:
        scale = (height()/2-5)/(CONST_NM*48 );
        rangeStep = 48/6.0f;
        byte2 = 0x03;
        ui->label_range->setText("48 NM");
        break;
    case 7:
        scale = (height()/2-5)/(CONST_NM*72 );
        rangeStep = 72/6.0f;
        byte2 = 0x04;
        ui->label_range->setText("72 NM");
        break;
    case 8:
        scale = (height()/2-5)/(CONST_NM*96 );
        rangeStep = 96/6.0f;
        byte2 = 0x05;
        ui->label_range->setText("96 NM");
        break;
    case 9:
        scale = (height()/2-5)/(CONST_NM*120 );
        rangeStep = 120/6.0f;
        byte2 = 0x06;
        ui->label_range->setText("120 NM");
        break;
    default:
        scale = (height()/2-5)/(CONST_NM*48  );
        ui->label_range->setText("48 NM");
        break;
    }
    if(ui->toolButton_adaptiv_scale->isChecked())

    {
        unsigned char bytes[8] = {0x08,0xab,0x00,0x00,0x00,0x00,0x00,0x00};

        bytes[2]=byte2;
        sendToRadar(bytes);
        processing->radarData->resetTrack();
        for(short i = 0;i<targetList.size();i++)
        {
            targetList.at(i)->deleteLater();
        }
        targetList.clear();
    }
    ui->toolButton_grid->setText(QString::fromUtf8("Vòng cự ly(")+QString::number(rangeStep)+"NM)");
    isScaleChanged = true;
//    isScreenUp2Date = false;

    short sdx = mousePointerX - scrCtX + dx;
    short sdy = mousePointerY - scrCtY + dy;
    sdx =(sdx*scale/oldScale);
    sdy =(sdy*scale/oldScale);
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
    //printf("processing->radarData->kgain %f \n",processing->radarData->kgain);
}

void Mainwindow::on_horizontalSlider_rain_valueChanged(int value)
{
    processing->radarData->krain = (float)value/(ui->horizontalSlider_rain->maximum()+ui->horizontalSlider_rain->maximum()/3);
}

void Mainwindow::on_horizontalSlider_sea_valueChanged(int value)
{
    processing->radarData->ksea = (float)value/(ui->horizontalSlider_sea->maximum());
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


void Mainwindow::updateTargets()
{
    trackList* trackListPt = &processing->radarData->mTrackList;

    for(short i = 0;i<targetList.size();i++)
    {
        if(!targetList.at(i)->isUsed)
        {
            continue;

            targetList.at(i)->hide();

        }
        if(trackListPt->at(targetList.at(i)->trackId).state == 0)
        {
            targetList.at(i)->isUsed = false;
            ui->label_status_warning->setText(QString::fromUtf8("Mất MT số:")+QString::number(i+1));
            warningList.append(QString::fromUtf8("Mất MT số:")+QString::number(i+1));
            ui->label_status_warning->setStyleSheet("background-color: rgb(255, 150, 50,255);");
            targetList.at(i)->hide();
            //targetList.at(i)->isLost=true;
            continue;
        }
        float x	= targetList.at(i)->x*scale + scrCtX-dx ;
        float y	= -targetList.at(i)->y*scale + scrCtY-dy ;
        float w = scrCtY-30;
        float dx = x-scrCtX;
        float dy = y-scrCtY;
        if(dx*dx+dy*dy>(w*w))
        {
            targetList.at(i)->hide();
        }
        else
        {
            targetList.at(i)->show();
            targetList.at(i)->setScrPos(x,y);
        }

        if(targetList.at(i)->clicked)
        {

            selected_target_index = i;
            targetList.at(i)->setSelected(true);
            targetList.at(i)->clicked = false;
        }
        if(targetList.at(i)->doubleClicked)
        {

            selected_target_index = i;
            trackListPt->at((targetList.at(i)->trackId)).isManual = true;
            targetList.at(i)->isManual = true;
            targetList.at(i)->doubleClicked = false;
        }
        if(selected_target_index == i)
        {
            float tmpazi = trackListPt->at(targetList.at(i)->trackId).estA/PI*180;
            if(tmpazi<0)tmpazi+=360;
            ui->label_radar_id->setText(QString::number(i+1));
            ui->label_radar_range->setText(QString::number(trackListPt->at(targetList.at(i)->trackId).estR*processing->radarData->scale_ppi/scale/1.852f)+"Nm");
            ui->label_radar_azi->setText( QString::number(tmpazi)+"\xB0");
            ui->label_radar_lat->setText( QString::number((short)targetList.at(i)->m_lat)+"\xB0"+QString::number((targetList.at(i)->m_lat-(short)targetList.at(i)->m_lat)*60,'g',4)+"N");
            ui->label_radar_long->setText(QString::number((short)targetList.at(i)->m_lon)+"\xB0"+QString::number((targetList.at(i)->m_lon-(short)targetList.at(i)->m_lon)*60,'g',4)+"E");
            ui->label_radar_speed->setText(QString::number(trackListPt->at(targetList.at(i)->trackId).speed,'g',3)+"Kn");
            ui->label_radar_heading->setText(QString::number(trackListPt->at(targetList.at(i)->trackId).head_r*180/PI)+"\xB0");
        }
        else
        {
            targetList.at(i)->setSelected(false);// = false;
        }
        //printf("\nx:%f y:%f",x,y);
    }
    //ui->
    //t1.setGeometry(400,400,20,20);
    //targetList.append(t1);
}

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
        hex2bin(ui->lineEdit_byte_8->text().toStdString().data(),&bytes[7]);
        sendToRadar((unsigned char*)&bytes[0]);
        //udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
    }
}

void Mainwindow::on_toolButton_map_toggled(bool checked)
{
    DrawMap();
}

void Mainwindow::on_toolButton_zoom_in_clicked()
{
    if(range>0)range--;
    UpdateScale();
    DrawMap();
}

void Mainwindow::on_toolButton_zoom_out_clicked()
{
    if(range<9)range++;
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

void Mainwindow::SetGPS(float mlat,float mlong)
{
    config.m_config.m_lat = mlat;
    config.m_config.m_long = mlong;
    ui->text_latInput_2->setText(QString::number(mlat));
    ui->text_longInput_2->setText(QString::number(mlong));
    vnmap.setUp(config.m_config.m_lat, config.m_config.m_long, 300,config.m_config.mapFilename.data());
    DrawMap();
    update();
}
void Mainwindow::on_toolButton_map_select_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,    QString::fromUtf8("Open Map"), NULL, tr("ISM file (*.ism)"));
    if(!filename.size())return;
    config.m_config.mapFilename =  filename.toStdString();
    vnmap.ClearData();
    if(pMap)delete pMap;
    pMap = new QPixmap(height(),height());
    vnmap.setUp(config.m_config.m_lat, config.m_config.m_long, 300,config.m_config.mapFilename.data());//100km  max range
    DrawMap();
    repaint();
}

//void Mainwindow::on_dial_valueChanged(int value)
//{
//    float heading = value/100.0f;
//    ui->textEdit_heading->setText(QString::number(heading));

//}

void Mainwindow::on_toolButton_set_heading_clicked()
{

    float heading = ui->textEdit_heading->text().toFloat();
    config.m_config.trueN = heading;
    processing->radarData->setTrueN(config.m_config.trueN);

}

void Mainwindow::on_toolButton_gps_update_clicked()
{

    SetGPS(ui->text_latInput_2->text().toFloat(),ui->text_longInput_2->text().toFloat());

}

void Mainwindow::on_comboBox_code_type_currentIndexChanged(const QString &arg1)
{

}

void Mainwindow::on_comboBox_code_type_currentIndexChanged(int index)
{
    config.m_config.codeType = index;
    setCodeType(config.m_config.codeType);
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
    for(short i = 0;i<targetList.size();i++)
    {
        targetList.at(i)->deleteLater();
    }
    targetList.clear();
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
    processing->radarData->mTrackList.at(targetList.at(selected_target_index)->trackId).state = 0;
}

void Mainwindow::on_toolButton_tx_clicked()
{

    unsigned char        bytes[8] = {0xaa,0xab,0x03,0x02,0x00,0x00,0x00};//rotation on
    sendToRadar(bytes); Sleep(100);
    sendToRadar(bytes); Sleep(100);
    sendToRadar(bytes); Sleep(100);

    bytes[0] = 0x04;//set 1536
    bytes[2] = 0x00;
    bytes[3] = 0x06;
    sendToRadar(bytes);
    Sleep(100);
    bytes[0] = 0x08;//set resolution 60m
    bytes[2] = 0x02;
    bytes[3] = 0x00;
    sendToRadar(bytes);
    Sleep(100);

    bytes[0] = 0x01;
    bytes[2] = 0x04;//set up auto noise level
    bytes[3] = 0x01;
    sendToRadar(bytes);
    Sleep(100);
    // do trong
    bytes[0] = 0x14;
    bytes[2] = 0xff;
    bytes[3] = 0x01;
    sendToRadar(bytes);
    Sleep(100);
    //tx on
    bytes[0] = 0xaa;
    bytes[2] = 0x02;
    bytes[3] = 0x01;//tx on 1
    sendToRadar(bytes);Sleep(100);
    sendToRadar(bytes);Sleep(100);
    sendToRadar(bytes);Sleep(100);
    bytes[2] = 0x00;//tx on 2
    sendToRadar(bytes);    Sleep(100);
    sendToRadar(bytes);    Sleep(100);
    sendToRadar(bytes);    Sleep(100);
    //ui->toolButton_tx->setChecked(false);
    bytes[0] = 0x1a;
    bytes[2] = 0x20;//set up auto noise level
    bytes[3] = 0x01;
    sendToRadar(bytes);    Sleep(100);
    if(radar_state!=DISCONNECTED)
    {
        QFile logFile;
        QDateTime now = QDateTime::currentDateTime();
        if(!QDir("C:\\logs\\"+now.toString("\\dd.MM\\")).exists())
        {
            QDir().mkdir("C:\\logs\\"+now.toString("\\dd.MM\\"));
        }
        logFile.setFileName("C:\\logs\\"+now.toString("\\dd.MM\\")+now.toString("dd.MM-hh.mm.ss")+"_tx_on.log");

        logFile.open(QIODevice::WriteOnly);
        //logFile.p
        logFile.close();

    }

}


void Mainwindow::on_toolButton_tx_off_clicked()
{
    unsigned char        bytes[8] = {0xaa,0xab,0x00,0x00,0x00,0x00,0x00};
    sendToRadar(bytes);    Sleep(100);
    sendToRadar(bytes);    Sleep(100);
    sendToRadar(bytes);    Sleep(100);

    bytes[2] = 0x02;// = {0xaa,0xab,0x00,0x01,0x00,0x00,0x00};
    sendToRadar(bytes);    Sleep(100);
    sendToRadar(bytes);    Sleep(100);
    sendToRadar(bytes);    Sleep(100);


    bytes[2] = 0x03;
    sendToRadar(bytes);    Sleep(100);
    sendToRadar(bytes);    Sleep(100);
    sendToRadar(bytes);    Sleep(100);
    if(radar_state!=DISCONNECTED)
    {
        QFile logFile;
        QDateTime now = QDateTime::currentDateTime();
        if(!QDir("C:\\logs\\"+now.toString("\\dd.MM\\")).exists())
        {
            QDir().mkdir("C:\\logs\\"+now.toString("\\dd.MM\\"));
        }
        logFile.setFileName("C:\\logs\\"+now.toString("\\dd.MM\\")+now.toString("dd.MM-hh.mm.ss")+"_tx_off.log");
        logFile.open(QIODevice::WriteOnly);
        //logFile.p
        logFile.close();

    }
}

void Mainwindow::on_toolButton_filter2of3_clicked(bool checked)
{
    processing->radarData->filter2of3 = checked;
}




void Mainwindow::on_comboBox_radar_resolution_currentIndexChanged(int index)
{

}

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

void Mainwindow::on_toolButton_map_2_clicked()
{
    DrawMap();
}

void Mainwindow::on_comboBox_2_currentIndexChanged(int index)
{
    switch (index)
    {
    case 0:
        //sendToRadarHS("aaab030302000000");
        break;
    case 1:
        sendToRadarHS("aaab030100000000");
        Sleep(100);
        sendToRadarHS("aaab030100000000");
        Sleep(100);
        sendToRadarHS("aaab030100000000");
        Sleep(100);
        break;
    case 2:
        sendToRadarHS("aaab030200000000");
        Sleep(100);
        sendToRadarHS("aaab030200000000");
        Sleep(100);
        sendToRadarHS("aaab030200000000");
        Sleep(100);
        break;
    case 3:
        break;
    case 4 :
        break;
    default:
        break;
    }
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

     // Connect 2 buffer is fragment
     if(!m_CLocal.OnLinkBuff(szBuff, nLeng))
         return 0;

     if(!m_CLocal.GetTrackAIS(m_CLocal.m_Buff, m_CLocal.m_Leng, &nTkNew))
         return 0;
     for(short i = 0;i<m_trackList.size();i++)
     {
         if(m_trackList.at(i).CheckMMSI(nTkNew.m_MMSI))
         {
             m_trackList.at(i).Update(&nTkNew);
             nIndex = i;
             return true;
         }
     }
     if(nIndex<0)
     {
        m_trackList.push_back(nTkNew);
     }
     return true;
}





























void Mainwindow::on_toolButton_auto_detect_clicked()
{

}

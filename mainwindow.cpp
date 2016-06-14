#include "mainwindow.h"
#include "ui_mainwindow.h"

//#define mapWidth 2000
//#define mapWidth mapWidth
//#define mapHeight mapWidth
#define CONST_NM 1.825f
#define CONST_PI 3.141592654f
#define MAX_VIEW_RANGE_KM 50

#include <queue>

//static bool                 isAddingTarget=false;
static QPixmap              *pMap=NULL;
//static QImage               *sgn_img = new QImage(RADAR_MAX_RESOLUTION*2,RADAR_MAX_RESOLUTION*2,QImage::Format_RGB32);
dataProcessingThread *processing;
static Q_vnmap              vnmap;
QTimer*                     scrUpdateTimer,*readBuffTimer ;
QTimer*                     syncTimer1s ;
QTimer*                     dataPlaybackTimer ;
QThread*                    t,*t1 ;
bool displayAlpha = false;
//static short                currMaxRange = RADAR_MAX_RESOLUTION;
static short                currMaxAzi = MAX_AZIR,currMinAzi = 0;
static short                dxMax,dyMax;
static C_ARPA_data          arpa_data;
//static short                curAzir=-1;//,drawAzir=0;
static float                signsize,sn_scale;
static short                scrCtX, scrCtY, dx =0,dy=0,dxMap=0,dyMap=0;//mouseX,mouseY;
static short                mousePointerX,mousePointerY;
static char                 gridOff = false;
static char                 udpFailure = 0;//config file !!!
static bool                 isScreenUp2Date,isSettingUp2Date,isDraging = false;
static bool                 isScaleChanged =true;
static QStandardItemModel   modelTargetList;
enum drawModes{
    SGN_DIRECT_DRAW,SGN_IMG_DRAW,NOTERR_DRAW
}drawMode = SGN_IMG_DRAW;
short range = 1;

//typedef struct {
//    unsigned char        bytes[8];
//}
//Command_Control;
//typedef std::queue<Command_Control> CommandList;
//static CommandList command_queue;
bool isDrawSubTg = true;
//static unsigned short cur_object_index = 0;
int char2int( char input)
{
  if(input >= '0' && input <= '9')
    return input - '0';
  if(input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if(input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  return 0;
}
void hex2bin(const char* src,unsigned char* target)
{
  while(*src && src[1])
  {
    *(target++) = char2int(*src)*16 + char2int(src[1]);
    src += 2;
  }
  *(target++)=0;
}
void bin2hex(unsigned char byte, char* str)
{
    switch (byte>>4) {
    case 0:
        *str = '0';
        break;
    case 1:
        *str = '1';
        break;
    case 2:
        *str = '2';
        break;
    case 3:
        *str = '3';
        break;
    case 4:
        *str = '4';
        break;
    case 5:
        *str = '5';
        break;
    case 6:
        *str = '6';
        break;
    case 7:
        *str = '7';
        break;
    case 8:
        *str = '8';
        break;
    case 9:
        *str = '9';
        break;
    case 10:
        *str = 'A';
        break;
    case 11:
        *str = 'B';
        break;
    case 12:
        *str = 'C';
        break;
    case 13:
        *str = 'D';
        break;
    case 14:
        *str = 'E';
        break;
    case 15:
        *str = 'F';
        break;
    default:
        break;
    }
    switch (byte&(0x0F)) {
    case 0:
        *(str+1) = '0';
        break;
    case 1:
        *(str+1) = '1';
        break;
    case 2:
        *(str+1) = '2';
        break;
    case 3:
        *(str+1) = '3';
        break;
    case 4:
        *(str+1) = '4';
        break;
    case 5:
        *(str+1) = '5';
        break;
    case 6:
        *(str+1) = '6';
        break;
    case 7:
        *(str+1) = '7';
        break;
    case 8:
        *(str+1) = '8';
        break;
    case 9:
        *(str+1) = '9';
        break;
    case 10:
        *(str+1) = 'A';
        break;
    case 11:
        *(str+1) = 'B';
        break;
    case 12:
        *(str+1) = 'C';
        break;
    case 13:
        *(str+1) = 'D';
        break;
    case 14:
        *(str+1) = 'E';
        break;
    case 15:
        *(str+1) = 'F';
        break;
    default:
        break;
    }

}

void Mainwindow::sendToRadar(const char* hexdata)
{
    short len = strlen(hexdata)/2+1;
    unsigned char* sendBuff = new unsigned char[len];
    hex2bin(hexdata,sendBuff);
    udpSendSocket->writeDatagram((char*)sendBuff,8,QHostAddress("192.168.0.44"),2572);
    delete[] sendBuff;

}
void Mainwindow::sendToRadar(unsigned char* hexdata)
{

    udpSendSocket->writeDatagram((char*)hexdata,8,QHostAddress("192.168.0.44"),2572);
    //delete[] sendBuff;

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
    processing->radarData->updateZoomRect(mousePointerX - scrCtX+dx,mousePointerY - scrCtY+dy);
    DrawMap();
    isScreenUp2Date = false;
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
    //if(!isDraging)return;
    //mouseX = (event->x());
    //mouseY = (event->y());
    if(isDraging&&(event->buttons() & Qt::LeftButton)) {

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
        dx+= mousePointerX-event->x();
        dy+= mousePointerY-event->y();
        dxMap += mousePointerX-event->x();
        dyMap += mousePointerY-event->y();
        mousePointerX=event->x();
        mousePointerY=event->y();
        isScreenUp2Date = false;
    }
}
void Mainwindow::mousePressEvent(QMouseEvent *event)
{
    if(event->x()>height())return;
    mousePointerX = (event->x());
    mousePointerY = (event->y());

    //ui->frame_RadarViewOptions->hide();
    if(event->buttons() & Qt::LeftButton) {
        isDraging = true;
        mousePointerX=event->x();
        mousePointerY=event->y();
        //printf("mouseX %d\n",mouseX);
    }
    else if(event->buttons() & Qt::RightButton)
    {

        float xRadar = (mousePointerX - scrCtX+dx)/scale ;//coordinates in  radar xy system
        float yRadar = -(mousePointerY - scrCtY+dy)/scale;
        processing->radarData->addTrackManual(xRadar,yRadar);
        isScreenUp2Date = false;
        return;
    }

//    if(selectobject) {


//    }

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
    //QRectF signRect(RAD_M_PULSE_RES-(scrCtX-dx),RAD_M_PULSE_RES-(scrCtY-dy),width(),height());
    //QRectF screen(0,0,width(),height());

    //p->drawImage(screen,*processing->radarData->img_ppi,signRect,Qt::AutoColor);
    switch(drawMode)
    {
        case SGN_IMG_DRAW:
        {
            QRectF signRect(DISPLAY_RES-(scrCtX-dx),DISPLAY_RES-(scrCtY-dy),width(),height());
            QRectF screen(0,0,width(),height());
            p->drawImage(screen,*processing->radarData->img_ppi,signRect,Qt::AutoColor);

            break;
        }
        case SGN_DIRECT_DRAW:
    {/*
        // _________direct drawing(without Qimage)
        QPen pensgn(Qt::green);
        QPen pentrr(QColor(0,50,60));
        //pentrr.setWidth(((short)signsize)+1);
        //pensgn.setWidth(((short)signsize)+1);
        //QMutexLocker locker(&processing->mutex);
        short ctX=scrCtX-dx;
        short ctY=scrCtY-dy;
        for(short azi = 1;azi<MAX_AZIR;++azi)
        {
            for(unsigned short range = 0;range < RAD_M_PULSE_RES;range++)
            {
                if(processing->radarData->signal_map.frame[azi].raw_map[range].displaylevel)
                {
                    pensgn.setColor(QColor(processing->radarData->signal_map.frame[azi].raw_map[range].displaylevel,255,0));
                    p->setPen(pensgn);
                    p->drawPoint((processing->radarData->signal_map.frame[azi].raw_map[range].x-RAD_M_PULSE_RES)*signsize+ctX,
                                         (processing->radarData->signal_map.frame[azi].raw_map[range].y-RAD_M_PULSE_RES)*signsize+ctY);
                }
                else if(processing->radarData->signal_map.frame[azi].raw_map[range].vet)
                {
                    p->setPen(pentrr);
                    p->drawPoint((processing->radarData->signal_map.frame[azi].raw_map[range].x - RAD_M_PULSE_RES)*signsize+ctX,
                                         (processing->radarData->signal_map.frame[azi].raw_map[range].y - RAD_M_PULSE_RES)*signsize+ctY);
                }

            }
        }*/


    }
    default:
        break;
    }
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
    pMap->fill(QColor(0,0,0,255));
    //pMap->fill(Qt::transparent);
    if(ui->toolButton_map->isChecked()==false)return;
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
        //printf("vnmap.layers[i].size()%d\n",vnmap.layers[i].size());
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
        }else if(false)
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
    //DrawGrid(&p,centerX,centerY);
    //draw text
    pen.setColor(QColor(150,130,110,150));
    pen.setWidth(2);
    pen.setStyle(Qt::SolidLine);
    p.setPen(pen);
    //QPen pen2(Qt::red);
    //pen2.setWidth(2);
    for(uint i = 0; i < vnmap.placeList.size(); i++) {
            QPoint int_point;
            float x,y;
            vnmap.ConvDegToScr(&x,&y,&vnmap.placeList[i].m_Long,&vnmap.placeList[i].m_Lat);
            int_point.setX((int)(x*scale)+centerX);
            int_point.setY((int)(y*scale)+centerY);
            //p.drawEllipse(int_point,2,2);
            p.drawText(int_point.x()+5,int_point.y(),QString::fromWCharArray(vnmap.placeList[i].text.c_str()));
            //printf("toa do hien tai lat %f long %f\n",m_textList[i].m_Lat,m_textList[i].m_Long);
    }

}
void Mainwindow::DrawGrid(QPainter* p,short centerX,short centerY)
{
    //return;
    QPen pen(QColor(0x7f,0x7f,0x7f,0xff));
    pen.setWidth(3);
    pen.setStyle(Qt::SolidLine);
    p->setBrush(QBrush(Qt::NoBrush));
    p->setPen(pen);
    p->drawLine(centerX-5,centerY,centerX+5,centerY);
    p->drawLine(centerX,centerY-5,centerX,centerY+5);
    //pen.setColor(QColor(30,90,150,120));
    pen.setWidth(2);
    p->setPen(pen);

    p->drawEllipse(QPoint(centerX,centerY),
                  (short)(10.0*CONST_NM*scale),
                  (short)(10.0*CONST_NM*scale));
    p->drawEllipse(QPoint(centerX,centerY),
                  (short)(20.0*CONST_NM*scale),
                  (short)(20.0*CONST_NM*scale));
    p->drawEllipse(QPoint(centerX,centerY),
                  (short)(30.0*CONST_NM*scale),
                  (short)(30.0*CONST_NM*scale));
    p->drawEllipse(QPoint(centerX,centerY),
                  (short)(40.0*CONST_NM*scale),
                  (short)(40.0*CONST_NM*scale));
    p->drawEllipse(QPoint(centerX,centerY),
                  (short)(50.0*CONST_NM*scale),
                  (short)(50.0*CONST_NM*scale));
    p->drawEllipse(QPoint(centerX,centerY),
                  (short)(60.0*CONST_NM*scale),
                  (short)(60.0*CONST_NM*scale));
    p->drawEllipse(QPoint(centerX,centerY),
                  (short)(70.0*CONST_NM*scale),
                  (short)(70.0*CONST_NM*scale));
    return;
    if(gridOff == 0)//with frame
    {
        //p->drawLine(0,centerY,width(),centerY);
        //p->drawLine(centerX,0,centerX,height());
    }
    else
    {
        //p.drawEllipse(QPoint(centerX,centerY),(int)(20*CONST_NM*scale),(int)(20*CONST_NM*scale));
        //p.drawEllipse(QPoint(centerX,centerY),(int)(5*CONST_NM*scale),(short)(5*CONST_NM*scale));
        //pen.setWidth(1);
        //p->setPen(pen);
        short theta;
        short gridR = height()*0.7;
        for(theta=0;theta<360;theta+=90){
            QPoint point1,point2;
                short dx = gridR*cosf(CONST_PI*theta/180);
                short dy = gridR*sinf(CONST_PI*theta/180);
                point1.setX(centerX+0.33*dx);
                point1.setY(centerY+0.33*dy);
                point2.setX(centerX+dx);
                point2.setY(centerY+dy);
                p->drawLine(point1,point2);

        }
        for(theta=0;theta<360;theta+=10){
            QPoint point1,point2;
                short dx = gridR*cosf(CONST_PI*theta/180);
                short dy = gridR*sinf(CONST_PI*theta/180);
                point1.setX(centerX+0.95*dx);
                point1.setY(centerY+0.95*dy);
                point2.setX( centerX+dx);
                point2.setY(centerY+dy);
                p->drawLine(point1,point2);
                point2.setX(centerX+dx*1.02-9);
                point2.setY(centerY+dy*1.02+5);
                if(theta<270)p->drawText(point2,QString::number(theta+90));
                else p->drawText(point2,QString::number(theta-270));

        }
        for(theta=0;theta<360;theta+=2){
            QPoint point1,point2;
                short dx = gridR*cosf(CONST_PI*theta/180);
                short dy = gridR*sinf(CONST_PI*theta/180);
                point1.setX(centerX+0.98*dx);
                point1.setY(centerY+0.98*dy);
                point2.setX(centerX+dx);
                point2.setY(centerY+dy);
                p->drawLine(point1,point2);

        }
        //end grid
    }


}

void Mainwindow::DrawTarget(QPainter* p)
{
    //draw radar  target:
    QPen penTargetRed(Qt::red);
    penTargetRed.setWidth(3);
    //QPen penARPATarget(Qt::yellow);
    //penARPATarget.setWidth(3);
    QPen penTargetBlue(Qt::darkBlue);
    penTargetBlue.setWidth(2);
    //penTargetBlue.setStyle(Qt::DashLine);
    QPen penTrack(Qt::darkRed);
    //QPen penARPATrack(Qt::darkYellow);
    //draw radar targets
    short x,y;
    if(true)
    {
        for(uint i=0;i<processing->radarData->mTrackList.size();i++)
        {
            if(processing->radarData->mTrackList[i].state<TRACK_STABLE_STATE)continue;



                p->setPen(penTrack);
                short j;
                //draw track:
                for(j=0;j<((short)processing->radarData->mTrackList[i].object_list.size());j++)
                {
                    x = (processing->radarData->mTrackList[i].object_list[j].x + RAD_M_PULSE_RES)*signsize - (RAD_M_PULSE_RES*signsize-scrCtX)-dx;
                    y = (RAD_M_PULSE_RES - processing->radarData->mTrackList[i].object_list[j].y)*signsize - (RAD_M_PULSE_RES*signsize-scrCtY)-dy;
                    if(processing->radarData->mTrackList[i].tclass==RED_OBJ)p->drawPoint(x,y);
                }
                j--;
                if(j<0)continue;
                //printf("red");
                if(processing->radarData->mTrackList[i].tclass==RED_OBJ)
                {
                    p->setPen(penTargetRed);

                }
                else
                {
                    p->setPen(penTargetBlue);
                }
                p->drawRect(x-6,y-6,12,12);
                p->drawText(x-30,y-20,100,40,0,QString::number(i+1),0);
                /*if(false)//processing->radarData->mTrackList[i].isMoving) // moving obj
                {

                    QPolygon poly;
                    QPoint   point,point2;
                    point2.setX(x+processing->radarData->mTrackList[i].velocity*500*sinf(processing->radarData->mTrackList[i].course));
                    point2.setY(y-processing->radarData->mTrackList[i].velocity*500*cosf(processing->radarData->mTrackList[i].course));

                    point.setX(x+10*sinf(processing->radarData->mTrackList[i].course));
                    point.setY(y-10*cosf(processing->radarData->mTrackList[i].course));
                    p->setPen(penTargetBlue);
                    p->drawLine(point,point2);
                    poly<<point;
                    point.setX(x+10*sinf(processing->radarData->mTrackList[i].course+2.3562));
                    point.setY(y-10*cosf(processing->radarData->mTrackList[i].course+2.3562));
                    poly<<point;
                    point.setX(x);
                    point.setY(y);
                    poly<<point;
                    point.setX(x+10*sinf(processing->radarData->mTrackList[i].course-2.3562));
                    point.setY(y-10*cosf(processing->radarData->mTrackList[i].course-2.3562));
                    poly<<point;
                    point.setX(x+10*sinf(processing->radarData->mTrackList[i].course));
                    point.setY(y-10*cosf(processing->radarData->mTrackList[i].course));
                    poly<<point;
                    p->setPen(penTargetRed);
                    p->drawPolygon(poly);
                    p->drawText(x-30,y-20,100,40,0,QString::number(i+1),0);
                }else*/

            /*}
            else if(processing->radarData->mTrackList[i].tclass==BLUE_OBJ)
            {
                p->setPen(penTargetBlue);
                //printf("b");
                p->drawRect(x-6,y-6,12,12);
                p->drawText(x-30,y-20,100,40,0,QString::number(i+1),0);
                p->drawLine(x,
                            y,
                            x+processing->radarData->mTrackList[i].velocity*500*sinf(processing->radarData->mTrackList[i].course),
                            y-processing->radarData->mTrackList[i].velocity*500*cosf(processing->radarData->mTrackList[i].course));

            }*/
        }
    }
    /*else for(uint i=0;i<processing->radarData->mTrackList.size();i++)
    {
        if(!processing->radarData->mTrackList[i].state)continue;
        short x,y;
        p->setPen(penTrack);
        short j;
        //draw track:
        for(j=0;j<((short)processing->radarData->mTrackList[i].object_list.size());j++)
        {
            x = (processing->radarData->mTrackList[i].object_list[j].x + RADAR_MAX_RESOLUTION)*signsize - (RADAR_MAX_RESOLUTION*signsize-scrCtX)-dx;
            y = (RADAR_MAX_RESOLUTION - processing->radarData->mTrackList[i].object_list[j].y)*signsize - (RADAR_MAX_RESOLUTION*signsize-scrCtY)-dy;
            p->drawPoint(x,y);
        }
        j--;
        if(j<0)continue;


            if(processing->radarData->mTrackList[i].isMoving) // moving obj
            {

                QPolygon poly;
                QPoint   point,point2;
                point2.setX(x+processing->radarData->mTrackList[i].velocity*500*sinf(processing->radarData->mTrackList[i].course));
                point2.setY(y-processing->radarData->mTrackList[i].velocity*500*cosf(processing->radarData->mTrackList[i].course));

                point.setX(x+10*sinf(processing->radarData->mTrackList[i].course));
                point.setY(y-10*cosf(processing->radarData->mTrackList[i].course));
                p->setPen(penTargetSub);
                p->drawLine(point,point2);
                poly<<point;
                point.setX(x+10*sinf(processing->radarData->mTrackList[i].course+2.3562));
                point.setY(y-10*cosf(processing->radarData->mTrackList[i].course+2.3562));
                poly<<point;
                point.setX(x);
                point.setY(y);
                poly<<point;
                point.setX(x+10*sinf(processing->radarData->mTrackList[i].course-2.3562));
                point.setY(y-10*cosf(processing->radarData->mTrackList[i].course-2.3562));
                poly<<point;
                point.setX(x+10*sinf(processing->radarData->mTrackList[i].course));
                point.setY(y-10*cosf(processing->radarData->mTrackList[i].course));
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
    QPen penTargetRed(Qt::red);
    penTargetRed.setWidth(3);

    float fx,fy;
    for(uint i=0;i<arpa_data.ais_track_list.size();i++)
    {
            p->setPen(penTargetRed);
            short j;
            //draw track:
            for(j=0;j<((short)arpa_data.ais_track_list[i].object_list.size());j++)
            {
                vnmap.ConvDegToScr(&fx,&fy,&arpa_data.ais_track_list[i].object_list[j].mlong,&arpa_data.ais_track_list[i].object_list[j].mlat);
                short x = (fx*scale)+scrCtX-dx;
                short y = (fy*scale)+scrCtY-dy;
                p->drawPoint(x,y);
                //printf("\nj:%d,%d,%d,%f,%f",j,x,y,arpa_data.ais_track_list[i].object_list[j].mlong,arpa_data.ais_track_list[i].object_list[j].mlat);
            }
    }
}
void Mainwindow::paintEvent(QPaintEvent *event)
{
    event = event;
    isScreenUp2Date = true;
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
    //drawAisTarget(&p);
    //draw cursor
    QPen penmousePointer(QColor(0x50ffffff));
    /*if(!isDraging)
    {
        pencurPos.setWidth(1);
        p.setPen(pencurPos);
        p.drawLine(mouseX-10,mouseY,mouseX+10,mouseY);
        p.drawLine(mouseX,mouseY-10,mouseX,mouseY+10);
    }*/
    penmousePointer.setWidth(2);
    p.setPen(penmousePointer);
    p.drawLine(mousePointerX-15,mousePointerY,mousePointerX-10,mousePointerY);
    p.drawLine(mousePointerX+15,mousePointerY,mousePointerX+10,mousePointerY);
    p.drawLine(mousePointerX,mousePointerY-10,mousePointerX,mousePointerY-15);
    p.drawLine(mousePointerX,mousePointerY+10,mousePointerX,mousePointerY+15);
    float azi,range;
    processing->radarData->getPolar((mousePointerX - scrCtX+dx)/scale,-(mousePointerY - scrCtY+dy)/scale,&azi,&range);
    if(azi<0)azi+=PI_NHAN2;
    azi = azi/CONST_PI*180.0;
    range = range/CONST_NM;
    p.drawText(mousePointerX+5,mousePointerY+5,100,20,0,QString::number(range,'g',4)+"|"+QString::number(azi,'g',4),0);
    //draw zooom

    //draw frame

    DrawViewFrame(&p);
    if(ui->tabWidget_2->currentIndex()==2)
    {
        QRect rect = ui->tabWidget_2->geometry();
        //QRect zoomRect(DISPLAY_RES-100,DISPLAY_RES-100,200,200);
        rect.adjust(4,30,-5,-5);
        p.setPen(QPen(Qt::black));
        p.setBrush(QBrush(Qt::black));
        p.drawRect(rect);
        p.drawImage(rect,*processing->radarData->img_zoom_ppi,processing->radarData->img_zoom_ppi->rect());
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
    QRect rec = QApplication::desktop()->screenGeometry(0);
    setFixedSize(1920, 1080);
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
    mousePointerX = width()/2;
    mousePointerY = height()/2;
    dxMax = height()/4-10;
    dyMax = height()/4-10;
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
    range = 6; UpdateScale();
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





        scrCtX = height()/2+50;//+ ui->toolBar_Main->width()+20;//ENVDEP
        scrCtY = height()/2;


    UpdateScale();
    isSettingUp2Date = true;

}
void Mainwindow::UpdateSetting()
{
    isSettingUp2Date = false;
}

void Mainwindow::DrawViewFrame(QPainter* p)
{
    //draw grid

    if(ui->toolButton_grid->isChecked())
    {
        DrawGrid(p,scrCtX-dx,scrCtY-dy);
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

    QPen pengrid(QColor(128,128,0,255));
    pengrid.setWidth(4);
    p->setPen(pengrid);
    p->drawEllipse(scrCtX-scrCtY+25,25,d,d);
    pengrid.setWidth(2);
    p->setPen(pengrid);
    QFont font = p->font() ;
    font.setPointSize(6);
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
    isScreenUp2Date = false;
}
void Mainwindow::UpdateRadarData()
{

    //SendCommandControl();
    if(!isSettingUp2Date)
    {
        ReloadSetting();
    }

    if(!processing->getIsDrawn())
    {
        if(processing->radarData->isClkAdcChanged)
        {
            ui->comboBox_radar_resolution->setCurrentIndex(processing->radarData->clk_adc);
            processing->radarData->setScalePPI(scale);
            this->UpdateScale();
            printf("\nsetScale:%d",processing->radarData->clk_adc);
            processing->radarData->isClkAdcChanged = false;
        }
        processing->radarData->redrawImg();
        update();
        if(radar_state==DISCONNECTED)setRadarState(CONNECTED);
    }
    else if(!isScreenUp2Date)
    {
        update();
        isScreenUp2Date = true;
    }




    /*QStandardItemModel* model = new QStandardItemModel(processing->radarData->mTrackList.size(), 5);
    for (int row = 0; row < processing->radarData->mTrackList.size(); ++row)
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
    readBuffTimer = new QTimer();
    dataPlaybackTimer = new QTimer();
    t = new QThread();
    t1 = new QThread();
//    connect(fullScreenTimer, SIGNAL(timeout()), this, SLOT(UpdateSetting()));
//    fullScreenTimer->setSingleShot(true);
//    fullScreenTimer->start(1000);
    connect(syncTimer1s, SIGNAL(timeout()), this, SLOT(sync1()));
    syncTimer1s->start(1000);
    syncTimer1s->moveToThread(t);
    //
    connect(readBuffTimer,SIGNAL(timeout()),this,SLOT(readBuffer()));
    readBuffTimer->start(50);
    readBuffTimer->moveToThread(t1);
    //
    connect(scrUpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateRadarData()));

    scrUpdateTimer->start(30);//ENVDEP
    scrUpdateTimer->moveToThread(t);
    processing = new dataProcessingThread();
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

        udpSendSocket = new QUdpSocket(this);
        if(!udpSendSocket->bind(8900))
        {
            if(!udpSendSocket->bind(8901))
            {
                udpSendSocket->bind(8902);
            }
        }
        udpSendSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 10);

//    udpARPA = new QUdpSocket(this);
//    udpARPA->bind(1990,QUdpSocket::ShareAddress);
//    connect(udpARPA, SIGNAL(readyRead()),
//            this, SLOT(processARPA()));

}
void Mainwindow::processARPA()
{
    /*
    while (udpARPA->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpARPA->pendingDatagramSize());
        udpARPA->readDatagram(datagram.data(), datagram.size());
        //printf(datagram.data());
		QString str(datagram.data());
        QStringList list = str.split(",");
        if(*list.begin()=="$RATTM")
        {
            short tNum = (*(list.begin()+1)).toInt();
            float tDistance = (*(list.begin()+2)).toFloat();
            float tRange = (*(list.begin()+3)).toFloat();
            arpa_data.addARPA(tNum,tDistance,tRange);
        }
    }
    */
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
    //QProcess::startDetached("shutdown -s -f -t 0");
#else
    //system("/sbin/halt -p");
#endif
}

void Mainwindow::on_actionConnect_triggered()
{

}
void Mainwindow::sync1()//period 1 second
{
    // display radar temperature:
    ui->label_temp->setText(QString::number(processing->radarData->tempType)+"|"+QString::number(processing->radarData->temp)+"\260C");
//    int n = 32*256.0f/((processing->radarData->noise_level[0]*256 + processing->radarData->noise_level[1]));
//    int m = 256.0f*((processing->radarData->noise_level[2]*256 + processing->radarData->noise_level[3]))
//            /((processing->radarData->noise_level[4]*256 + processing->radarData->noise_level[5]));
    QByteArray array(processing->radarData->getFeedback(), 8);
    ui->label_command->setText(QString(array.toHex()));
    //display target list:
    /*for(uint i=0;i<processing->radarData->mTrackList.size();i++)
    {
        if(processing->radarData->mTrackList[i].state==0)
        {
            QList<QListWidgetItem *> items = (ui->listTargetWidget->findItems(QString::number(i+1),Qt::MatchStartsWith));
            if(items.size())delete items[0];
            continue;
        }
        QList<QListWidgetItem *> items = (ui->listTargetWidget->findItems(QString::number(i+1),Qt::MatchStartsWith));
        QString str;
        float targetSpeed = processing->radarData->mTrackList[i].velocity*3600*signsize/scale/CONST_NM;//mile per hours
        // check track parameters

        if(targetSpeed>TARGET_MAX_SPEED)
        {
            //processing->radarData->deleteTrack(i);
            continue;
        }//
        if(processing->radarData->mTrackList[i].tclass==RED_OBJ)
        {
            str.append(QString::number(i+1)+":");
            str.append(QString::number(processing->radarData->mTrackList[i].estR*signsize/scale/CONST_NM,'g',3)+" | ");
            str.append(QString::number((short)(processing->radarData->mTrackList[i].estA*57.2957795f),'g',3)+" | ");
            str.append(QString::number((short)(targetSpeed),'g',4)+" | ");
            str.append(QString::number((short)(processing->radarData->mTrackList[i].course*57.2957795f),'g',3)+" | ");
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
    //display radar state
    switch(radar_state)
    {
    case DISCONNECTED:

        break;
    case CONNECTED:
        ui->label_status->setText(QString::number(processing->radarData->overload));
        if(processing->radarData->rotation_speed)
        {
            ui->toolButton_scan->setChecked(true);

            if(processing->radarData->rotation_speed==1)ui->label_speed->setText("9 v/p");
            else if(processing->radarData->rotation_speed==2)ui->label_speed->setText("12 v/p");
        }
        else
        {
            ui->toolButton_scan->setChecked(false);
            ui->label_speed->setText("0 v/p");
        }
        switch((processing->radarData->sn_stat>>8))
        {
        case 0 :
            ui->label_sn_type->setText(QString::fromUtf8("Xung đơn"));
            break;
        case 1 :
            ui->label_sn_type->setText(QString::fromUtf8("Mã Baker"));
            break;
        case 2 :
            ui->label_sn_type->setText(QString::fromUtf8("Mã M"));
            break;
        case 3 :
            ui->label_sn_type->setText(QString::fromUtf8("Mã NN"));
            break;

        default:
            ui->label_sn_type->setText(QString::fromUtf8("Mã DTTT"));
            break;
            //ui->label_sn_type->setText(QString::number(processing->radarData->sn_stat&0x07));

        }

        ui->label_sn_param->setText(QString::number((processing->radarData->sn_stat)&0xff));
        break;
    case CONNECTED_ROTATE12_TXON:
        ui->label_status->setText(QString::fromUtf8("Phát 12v/p"));
        break;
    case CONNECTED_ROTATE12_TXOFF:
        ui->label_status->setText(QString::fromUtf8("Quay 12v/p"));
        break;
    }
}
void Mainwindow::setRadarState(radarSate radarState)
{
    if(radarState != radar_state)
    {
        radar_state = radarState;
        switch(radar_state)
        {
        case DISCONNECTED:
            ui->label_status->setText(QString::fromUtf8("Chưa kết nối"));
            ui->toolButton_tx->hide();
            ui->toolButton_scan->hide();
            break;
        case CONNECTED:
            ui->label_status->setText(QString::number(processing->radarData->overload));
            ui->label_sn_type->setText(QString::number(processing->radarData->sn_stat&0x07));
            ui->label_sn_param->setText(QString::number((processing->radarData->sn_stat>>3)&0x07));
            ui->toolButton_tx->show();
            ui->toolButton_scan->show();
            break;
        case CONNECTED_ROTATE12_TXON:
            ui->label_status->setText(QString::fromUtf8("Phát 12v/p"));
            break;
        case CONNECTED_ROTATE12_TXOFF:
            ui->label_status->setText(QString::fromUtf8("Quay 12v/p"));
            break;
        }
    }
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
    /*QDateTime time = QDateTime::currentDateTime();
    QString text = time.toString("hh:mm:ss");
    ui->label_date->setText(text);
    text = time.toString("dd/MM/yy");
    ui->label_time->setText(text);*/
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
    isScreenUp2Date = false;
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
    //float oldScale = scale;
    switch(range)

    {
    case 0:
        scale = (height()/2-5)/(CONST_NM*1.5f );
        ui->label_range->setText("1.5 NM");
        break;
    case 1:
        scale = (height()/2-5)/(CONST_NM*4 );
        ui->label_range->setText("4 NM");
        break;
    case 2:
        scale = (height()/2-5)/(CONST_NM*8 );
        ui->label_range->setText("8 NM");
        break;
    case 3:
        scale = (height()/2-5)/(CONST_NM*16 );
        ui->label_range->setText("16 NM");
        break;
    case 4:
        scale = (height()/2-5)/(CONST_NM*24 );
        ui->label_range->setText("24 NM");
        break;
    case 5:
        scale = (height()/2-5)/(CONST_NM*48 );
        ui->label_range->setText("48 NM");
        break;
    case 6:
        scale = (height()/2-5)/(CONST_NM*64 );
        ui->label_range->setText("64 NM");
        break;
    case 7:
        scale = (height()/2-5)/(CONST_NM*96 );
        ui->label_range->setText("96 NM");
        break;
    default:
        scale = (height()/2-5)/(90 );
        ui->label_range->setText("48 NM");
        break;
    }
    isScaleChanged = true;
    isScreenUp2Date = false;

    //dx /=short(scale/oldScale);
    //dy /=short(scale/oldScale);
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

void Mainwindow::on_toolButton_scan_clicked()
{

}

void Mainwindow::on_toolButton_tx_toggled(bool checked)
{

    if(checked)

    {   //0xaa,0xab,0x00,0x01,0x00,0x00,0x00
        unsigned char        bytes[8] = {0xaa,0xab,0x02,0x01,0x00,0x00,0x00};
        udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
        bytes[2] = 0x00;//{0xaa,0xab,0x00,0x01,0x00,0x00,0x00};
        udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
        //ui->toolButton_tx->setChecked(false);
    }
    else
    {

        unsigned char        bytes[8] = {0xaa,0xab,0x02,0x00,0x00,0x00,0x00};
        udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
        bytes[2] = 0x00;// = {0xaa,0xab,0x00,0x01,0x00,0x00,0x00};
        udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
        //ui->toolButton_tx->setChecked(true);
    }

}

void Mainwindow::on_toolButton_scan_toggled(bool checked)
{
    if(checked)
    {
        unsigned char        bytes[8] = {0xaa,0xab,0x03,0x02,0x00,0x00,0x00};
        udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);

    }
    else
    {
        unsigned char        bytes[8] = {0xaa,0xab,0x03,0x00,0x00,0x00,0x00};
        udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);

    }

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



void Mainwindow::on_toolButton_alphaView_toggled(bool checked)
{
    displayAlpha = checked;
    processing->radarData->isDisplayAlpha = checked;
}




void Mainwindow::on_toolButton_centerView_clicked()
{
    dx = 0;
    dy = 0;
    DrawMap();
    isScreenUp2Date = false;
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
    unsigned char        bytes[8];
    hex2bin(ui->lineEdit_byte_1->text().toStdString().data(),&bytes[0]);
    hex2bin(ui->lineEdit_byte_2->text().toStdString().data(),&bytes[1]);
    hex2bin(ui->lineEdit_byte_3->text().toStdString().data(),&bytes[2]);
    hex2bin(ui->lineEdit_byte_4->text().toStdString().data(),&bytes[3]);
    hex2bin(ui->lineEdit_byte_5->text().toStdString().data(),&bytes[4]);
    hex2bin(ui->lineEdit_byte_6->text().toStdString().data(),&bytes[5]);
    hex2bin(ui->lineEdit_byte_7->text().toStdString().data(),&bytes[6]);
    hex2bin(ui->lineEdit_byte_8->text().toStdString().data(),&bytes[7]);
    udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
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
    if(range<7)range++;
    UpdateScale();
    DrawMap();
}

void Mainwindow::on_toolButton_reset_clicked()
{
    processing->radarData->resetData();
}

void Mainwindow::on_toolButton_send_command_2_clicked()
{
    unsigned char        bytes[8] = {0xaa,0xab,0x02,0x02,0x0a,0,0,0};
    udpSendSocket->writeDatagram((char*)&bytes[0],8,QHostAddress("192.168.0.44"),2572);
//    bytes[0] = 0xaa;
//    bytes[1] = 0xab;
//    bytes[2] = 0x02;
//    bytes[3] = 0x02;
//    bytes[4] = 0x0a;
//    bytes[5] = 0x00;
//    bytes[6] = 0x00;
//    bytes[7] = 0x00;

}

void Mainwindow::SetGPS(float mlat,float mlong)
{
    config.m_config.m_lat = mlat;
    config.m_config.m_long = mlong;
    ui->text_latInput_2->setText(QString::number(mlat));
    ui->text_longInput_2->setText(QString::number(mlong));
    vnmap.setUp(config.m_config.m_lat, config.m_config.m_long, 200,config.m_config.mapFilename.data());
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
    vnmap.setUp(config.m_config.m_lat, config.m_config.m_long, 200,config.m_config.mapFilename.data());//100km  max range
    DrawMap();
}

void Mainwindow::on_dial_valueChanged(int value)
{
    float heading = value/100.0f;
    ui->textEdit_heading->setText(QString::number(heading));

}

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

void Mainwindow::on_toolButton_centerZoom_clicked()
{
    processing->radarData->updateZoomRect(mousePointerX - scrCtX+dx,mousePointerY - scrCtY+dy);
}

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
        processing->radarData->filter = true;
    }
    else
    {
        processing->radarData->filter = false;
    }
}

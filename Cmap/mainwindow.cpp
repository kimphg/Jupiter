#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPaintEvent>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent) :dxMap(0),dyMap(0),
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    map = new CMap(this);
    map->setCenterPos(21.036264,105.774866);
    mScale = 5;
    map->setPath("C:/HR2D/MapData/ThunderForest");
    this->setGeometry(100,100,1024,768);
    isPressed = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent * e)
{
    QPainter p(this);
    ui->label_scale->setText("OSM scale factor:" + QString::number(map->getScaleRatio()));
    QPixmap pix = map->getImage(mScale);
    p.drawPixmap((width()/2.0-pix.width()/2.0)+dxMap,
                 (height()/2.0-pix.height()/2.0)+dyMap,
                 pix.width(),pix.height(),pix
                 );
    // draw the reference 1km line in top left of the map
    p.drawLine(25,10,25+this->mScale,10);
    p.drawText(rect(), Qt::AlignTop|Qt::TextWordWrap,"1 km");
    // draw the crosshair mark in the center
    int scrCtx = width()/2;
    int scrCty = height()/2;
    drawCrossHairMark(scrCtx,scrCty,&p);
}
void MainWindow::drawCrossHairMark(int x,int y,QPainter* p)
{
    p->drawLine(x-10,y,x-5,y);
    p->drawLine(x+10,y,x+5,y);
    p->drawLine(x,y-10,x,y-5);
    p->drawLine(x,y+10,x,y+5);
}
void MainWindow::resizeEvent(QResizeEvent *)
{
    map->setImgSize(width(), height());

}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
//    if (e->key() != Qt::Key_Z )
//    {
//        return;
//    }
//    else{
//        int zoom = (int) map->getScaleRatio();
//        switch(zoom)
//        {
//        case 14: map->setScaleRatio(15);
//            break;
//        case 15: map->setScaleRatio(16);
//            break;
//        case 16: map->setScaleRatio(14);
//            break;
//        default: map->setScaleRatio(15);
//            break;
//        }

//        map->invalidate();
//        map->UpdateImage();
//    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    if(isPressed&&(e->buttons() & Qt::LeftButton)) {
        dxMap = e->x()-pressPos.x();
        dyMap = e->y()-pressPos.y();
        update();
    }
    else
    {
        //show cursor lat-lon
        short   x = this->mapFromGlobal(QCursor::pos()).x() - width()/2;
        short   y = this->mapFromGlobal(QCursor::pos()).y() - height()/2;
        double lat,lon;
        map->ConvKmToWGS(x/mScale,-y/mScale,&lon,&lat);
        ui->lineEdit_cursor_lat->setText(QString::number(lat));
        ui->lineEdit_cursor_lon->setText(QString::number(lon));
    }
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    if(e->button() != Qt::LeftButton)
    {
        return;
    }
    isPressed = true;
    pressPos = e->pos();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    // change center lat-lon of the map
    double mouseLat,mouseLon;
    map->ConvKmToWGS(-(double)dxMap/mScale,(double)dyMap/mScale,&mouseLon,&mouseLat);
    map->setCenterPos(mouseLat,mouseLon);
    ui->lineEdit_lat->setText(QString::number(mouseLat,'g',10));
    ui->lineEdit_lon->setText(QString::number(mouseLon,'g',10));
    dxMap = 0;
    dyMap = 0;
    update();
}


void MainWindow::on_lineEdit_returnPressed()
{
    map->setPath(ui->lineEdit->text());
    update();
}
void MainWindow::wheelEvent(QWheelEvent* event)
{
    if(event->delta()>0)mScale*=1.2;
    if(event->delta()<0)mScale/=1.2;
    if(mScale>4000)mScale = 4000;
    if(mScale<0.01)mScale = 0.01;
    update();
    repaint();
}

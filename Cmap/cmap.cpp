#include <math.h>
#include <QtGui>
//#include <QtNetwork>

#include "cmap.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


uint qHash(const QPoint& p)
{
    return p.x() * 17 ^ p.y();
}
// tile size in pixels
const int tdim = 256;

CMap::CMap(QObject *parent): mScale(10),QObject(parent)
{
    //khoi tao mot vung pixmap co kich thuoc 256 X 256 va boi day no voi mau lightGray
    m_emptyTile = QPixmap(tdim, tdim);
    m_emptyTile.fill(Qt::darkGray);
    mMapWidth = 1024;
    mMapHeight = 1024;
    mapImage = 0;
    this->setPath("D:/HR2D/MapData/GM/" );
    //SetType(0);
}
void CMap::SetType(int type)
{
    switch (type)
    {
    case 0:
        this->setPath("D:/HR2D/MapData/GM/" );
        break;
    case 1:
        this->setPath("D:/HR2D/MapData/ThunderForest/" );
        break;
    case 2:
        this->setPath("D:/HR2D/MapData/GS/" );
        break;
    default:
        break;
    }
    OpenMIF("D:/HR2D/MIF/HP_TEXT.MIF");
    Repaint();
}

void CMap::OpenMIF(const char *fileName)
{

}
CMap::~CMap()
{

}

int CMap::getScaleRatio()
{
    return mScale;
}


void CMap::Repaint()
{
    invalidate();
    UpdateImage();

}


void CMap::setPath(QString path)
{
    mPath = path + "/%1/%2/%3";
    mPathraw = path + "/gs_%1_%2_%3";
    scaleMin = 20;
    scaleMax = 1;
    for(int i=1;i<20;i++)
    {
        QString str = path+"/"+QString::number(i);
        if(QDir(str).exists())
        {
            if(scaleMin>i)scaleMin = i;
            if(scaleMax<i)scaleMax = i;
        }
    }
    m_tilePixmaps.clear();

}


void CMap::setCenterPos(double lat, double lon)
{
    mCenterLat = lat;
    mCenterLon = lon;
    Repaint();
}


bool CMap::setScaleRatio(int scale)
{
    mScale = scale;
    if(mScale>scaleMax){mScale = scaleMax;return false;}
    if(mScale<scaleMin){mScale = scaleMin;return false;}

    return true;

}


void CMap::setImgSize(int width, int height)
{
    if(mapImage)delete mapImage;
    mMapWidth = width*1.25;
    mMapHeight = height*1.25;
    // why 1.25??? - mapImage will be rescaled before sending to user
    //with scale ratio from 0.8 to 1.6 original size
    mapImage = new QPixmap(mMapWidth,mMapHeight);
}

//ham lay ti le m tren pixel
double CMap::getScaleKm()
{
    //The distance represented by one pixel (S) is given by
    //S=C*cos(y)/2^(z+8)
    //where...
    //C is the (equatorial) circumference of the Earth
    //z is the zoom level
    //y is the latitude of where you're interested in the scale.

    //double metersPerPixel = 156543.03392 * cos(mCenterLat * M_PI / 180) / pow(2, mScale);
    return (double)(1<<mScale)/156.54303392 / cos(mCenterLat * M_PI / 180.0) ;

}



// xac dinh manh ban do tu dau ra dau vao
QPointF tileForCoordinate(qreal lat, qreal lng, int zoom)
{
    qreal zn = static_cast<qreal>(1 << zoom);
    qreal tx = (lng + 180.0) / 360.0;
    qreal ty = (1.0 - log(tan(lat * M_PI / 180.0) +
                          1.0 / cos(lat * M_PI / 180.0)) / M_PI) / 2.0;
    return QPointF(tx * zn, ty * zn);
}

//tra ve kinh do tu manh ban do
qreal longitudeFromTile(qreal tx, int zoom)
{
    qreal zn = static_cast<qreal>(1 << zoom);
    qreal lat = tx / zn * 360.0 - 180.0;
    return lat;
}

//tra ve vi do tu manh ban do
qreal latitudeFromTile(qreal ty, int zoom)
{
    qreal zn = static_cast<qreal>(1 << zoom);
    qreal n = M_PI - 2 * M_PI * ty / zn;
    qreal lng = 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
    return lng;
}

//ham kien tao lai cac thong so khiu thay doi
void CMap::invalidate()
{
    if (mMapWidth <= 0 || mMapHeight <= 0)
        return;

    // xac dinh diem ban do
    QPointF ct = tileForCoordinate(mCenterLat, mCenterLon, mScale);
    qreal tx = ct.x();
    qreal ty = ct.y();


    // top-left corner of the center tile
    //diem tren cung ben trai cua manh ban do trung tam
    int xp = mMapWidth / 2 - (tx - floor(tx)) * tdim;
    int yp = mMapHeight / 2 - (ty - floor(ty)) * tdim;

    // first tile vertical and horizontal
    // manh ban do dau tien theo chieu doc va ngang
    int xa = (xp + tdim - 1) / tdim;
    int ya = (yp + tdim - 1) / tdim;
    int xs = static_cast<int>(tx) - xa;
    int ys = static_cast<int>(ty) - ya;

    // offset for top-left tile
    // offset cho manh tren cung ben trai
    m_offset = QPoint(xp - xa * tdim, yp - ya * tdim);

    // last tile vertical and horizontal
    // manh cuoi cung doc va ngang
    int xe = static_cast<int>(tx) + (mMapWidth - xp - 1) / tdim;
    int ye = static_cast<int>(ty) + (mMapHeight - yp - 1) / tdim;

    // build a rect
    // xay dung ban do
    m_tilesRect = QRect(xs, ys, xe - xs + 1, ye - ys + 1);

    //neu duong link ko rong thi thuc hien dơnload
    if (m_url.isEmpty())
        LoadMap();//download();

    emit updated(QRect(0, 0, mMapWidth, mMapHeight));
}

//ham viet du lieu hien thi tren su kien Painter
void CMap::render(QPainter *p, const QRect &rect)
{
    for (int x = 0; x <= m_tilesRect.width(); ++x)
        for (int y = 0; y <= m_tilesRect.height(); ++y) {
            QPoint tp(x + m_tilesRect.left(), y + m_tilesRect.top());
            QRect box = tileRect(tp);
            if (rect.intersects(box)) {
                if (m_tilePixmaps.contains(tp))
                    p->drawPixmap(box, m_tilePixmaps.value(tp));
                else
                    p->drawPixmap(box, m_emptyTile);
            }
        }
}

void CMap::UpdateImage()
{
    if(mapImage)
    {
    QPainter p(mapImage);
    render(&p,mapImage->rect());
    }
}

QPixmap CMap::getImage(double scale)
{
    // recursive algorithm to get value of zoomRatio between 0.8 and 1.6
    double curScale = this->getScaleKm();
    double zoomRatio = scale/curScale;
    if(zoomRatio<0.8)
    {
        if(this->setScaleRatio(getScaleRatio()-1))return getImage(scale);

    }
    else if(zoomRatio>=1.6)
    {
        if(this->setScaleRatio(getScaleRatio()+1))return getImage(scale);

    }
    // repaint, rescale and return mapImage
    if(!mapImage)
    {
        printf("\nMap image size have not set.");
        return m_emptyTile;
    }
    Repaint();
    return mapImage->scaled(mapImage->width()*zoomRatio,mapImage->height()*zoomRatio,Qt::IgnoreAspectRatio,Qt::SmoothTransformation );
}
void CMap::ConvWGSToKm(double* x, double *y, double m_Long,double m_Lat)
{
    double refLat = (mCenterLat + (m_Lat))*0.00872664625997;//pi/360
    *x	= (((m_Long)-mCenterLon) * 111.31949079327357)*cos(refLat);// 3.14159265358979324/180.0*6378.137);//deg*pi/180*rEarth
    *y	= ((mCenterLat - (m_Lat)) * 111.132954);
    //tinh toa do xy KM so voi diem center khi biet lat-lon
}
void CMap::ConvKmToWGS(double x, double y, double *m_Long, double *m_Lat)
{
    *m_Lat  = mCenterLat +  (y)/(111.132954);
    double refLat = (mCenterLat +(*m_Lat))*0.00872664625997;//3.14159265358979324/180.0/2;
    *m_Long = (x)/(111.31949079327357*cos(refLat))+ mCenterLon;
    //tinh toa do lat-lon khi biet xy km (truong hop coi trai dat hinh cau)
}
void CMap::ConvKmToWGS_precise(double x, double y, double *m_Long, double *m_Lat)
{
    *m_Lat  = mCenterLat +  (y)/(111.132954);
    double refLat = (mCenterLat +(*m_Lat))*0.00872664625997;//3.14159265358979324/180.0/2;
    *m_Long = (x)/(0.01745329252*6378.137*cos(atan(6356.7523142/6378.1370*tan(refLat))))+ mCenterLon;
    *m_Lat  = mCenterLat +  (y)/(111.132954-0.559822*cos(2.0*refLat)+0.001175*cos(4.0*refLat));
    //tinh toa do lat-lon khi biet xy km (truong hop coi trai dat hinh ellipsoid)
}
void CMap::LoadMap()
{
    QPoint grab(0, 0);
    for (int x = 0; x <= m_tilesRect.width(); ++x)
        for (int y = 0; y <= m_tilesRect.height(); ++y) {
            QPoint tp = m_tilesRect.topLeft() + QPoint(x, y);
            if (!m_tilePixmaps.contains(tp)) {
                grab = tp;
                break;
            }
        }
    if (grab == QPoint(0, 0)) {
        m_url = QUrl();
        return;
    }
    //QString path = "C:/Users/LamPT/Desktop/mapData/%1/%2_%3_%4.png" ;
    QString imageMapPath = mPath.arg(mScale).arg(grab.x()).arg(grab.y());
//    if(QFile::exists(imageMapPath+".png"))imageMapPath =imageMapPath+".png";
//    else if(QFile::exists(imageMapPath+".jpg"))imageMapPath+=".jpg";
//    else
//    {
//        QString imageMapPathraw = mPathraw.arg(grab.x()).arg(grab.y()).arg(mScale)+".jpg";
//        imageMapPath+=".jpg";
//        if(QFile::exists(imageMapPathraw))
//            QFile::copy(imageMapPathraw,imageMapPath);
//    }
    QImage img(imageMapPath+".png");
    if (img.isNull())
    {
        img = QImage(imageMapPath+".jpg");
    }
    if (img.isNull())
    {
        QString imageMapPathraw = mPathraw.arg(grab.x()).arg(grab.y()).arg(mScale)+".jpg";
        imageMapPath+=".jpg";
        if(QFile::exists(imageMapPathraw))
            QFile::copy(imageMapPathraw,imageMapPath);
    }


    m_tilePixmaps[grab] = QPixmap::fromImage(img);
    if (img.isNull())
        m_tilePixmaps[grab] = m_emptyTile;
    emit updated(tileRect(grab));

    // purge unused spaces
    QRect bound = m_tilesRect.adjusted(-2, -2, 2, 2);
    foreach(QPoint grab, m_tilePixmaps.keys())
    if (!bound.contains(grab))
        m_tilePixmaps.remove(grab);

    LoadMap();

}

// ham xac dinh vi tri manh ban do voi diem top-left cua no la diem tp
QRect CMap::tileRect(const QPoint &tp)
{
    QPoint t = tp - m_tilesRect.topLeft();
    int x = t.x() * tdim + m_offset.x();
    int y = t.y() * tdim + m_offset.y();
    return QRect(x, y, tdim, tdim);
}
double CMap::getLat()
{
    return mCenterLat;
}
double CMap::getLon()
{
    return mCenterLon;
}

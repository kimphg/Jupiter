#ifndef CMAP_H
#define CMAP_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtGui>
#include <QPixmap>
#include <QUrl>
//#include "mitab/mitab/mitab.h"
class QNetworkReply;
class QPainter;

class CMap: public QObject
{
    Q_OBJECT

public:
    CMap(QObject *parent = 0);
    virtual ~CMap();
    void setCenterPos(double lat, double lon);
    void setImgSize(int width, int height);
    void setPath(QString path);
    QPixmap getImage(double scale);
    void ConvKmToWGS_precise(double x, double y, double *m_Long, double *m_Lat);
    void ConvKmToWGS(double x, double y, double *m_Long, double *m_Lat);
    void ConvWGSToKm(double *x, double *y, double m_Long, double m_Lat);
    int getScaleRatio();
    double getLat();
    double getLon();
    void SetType(int type);

signals:
    void updated(const QRect &rect);
private slots:
    void LoadMap();
protected:
    QRect tileRect(const QPoint &tp);
private:
    void OpenMIF(const char* fileName);
    void Repaint();
    void render(QPainter *p, const QRect &rect);
    void invalidate();
    bool setScaleRatio(int scale);
    double getScaleKm();
    void UpdateImage();
    QPoint m_offset;
    QRect m_tilesRect;
    QPixmap m_emptyTile;
    QHash<QPoint, QPixmap> m_tilePixmaps;
    QUrl m_url;
    QString mPath,mPathraw;
    QPixmap *mapImage;//pkp
    int scaleMin,scaleMax;
    int mMapWidth;
    int mMapHeight;
    int mScale;
    double mCenterLat;
    double mCenterLon;
    //double metersPerPixel;
};

#endif // CMAP_H

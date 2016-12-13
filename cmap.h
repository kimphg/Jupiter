#ifndef CMAP_H
#define CMAP_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtGui>
#include <QPixmap>
#include <QUrl>

class QNetworkReply;
class QPainter;

class CMap: public QObject
{
    Q_OBJECT

public:
    CMap(QObject *parent = 0);
    virtual ~CMap();

    void Repaint();
    void setCenterPos(double lat, double lon);
    void setScaleRatio(int scale);
    void setWidthHeight(int width, int height);
    void setPath(QString path);
    double getScaleRatio();

    void pan(const QPoint &delta);
    void invalidate();
    void render(QPainter *p, const QRect &rect);
    QPixmap getImage(double scale);
    void UpdateImage();
    double getScaleKm();
    double pixelHeight(double km);
    double pixelWidth(double km);

signals:
    void updated(const QRect &rect);
private slots:
    void LoadMap();

protected:
    QRect tileRect(const QPoint &tp);

private:
    QPoint m_offset;
    QRect m_tilesRect;
    QPixmap m_emptyTile;
    QHash<QPoint, QPixmap> m_tilePixmaps;
    QUrl m_url;
    QString mPath;
    QPixmap *mapImage;//pkp
    int mMapWidth;
    int mMapHeight;
    int mScale;
    double mCenterLat;
    double mCenterLon;
    //double metersPerPixel;
};

#endif // CMAP_H

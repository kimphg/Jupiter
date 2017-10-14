#ifndef C_ARPA_DATA_H
#define C_ARPA_DATA_H
#include <vector>
#include <QDateTime>
#include <QStringList>
#ifndef CONST_NM
#define CONST_NM 1.825f
#endif
typedef struct  {
    float               centerA,centerR;//
    float               centerX,centerY;//in km
    float               course ;
    float               velocity;
    qint64              time;
}ARPA_object_t;
class AIS_object_t {
public:
    AIS_object_t();
    QString printData();
    QString              mName,mDst;
    int                  mMMSI;
    int                  mImo;
    int                  mNavStat;
    int                  mType;
    int                  mBow,mStern,mStarboard,mPort;
    double               mLat;//in deg
    double               mLong;//in deg
    double               mCog;//course
    double               mSog;//speed
    qint64               mLut;//last update time
    bool                isNewest;
};
typedef std::vector<ARPA_object_t> ArpaobjectList;
class ARPA_track
{
public:
    ARPA_track():id(0),selected(false){}
    ArpaobjectList object_list;
    void addObject(ARPA_object_t*newobj)
    {
        object_list.push_back(*newobj);
        centerX =   newobj->centerX;
        centerY =   newobj->centerY;
        centerA =   newobj->centerA;
        centerR =   newobj->centerR;
        course = newobj->course ;
        velocity =  newobj->velocity;
        lives = 20;
    }

    float          centerX,centerY;
    float          centerA,centerR;
    float          course ;
    float          velocity;
    short           id;
    bool            selected;
    unsigned short            lives;
};


class C_ARPA_data
{
public:
    C_ARPA_data();
    ~C_ARPA_data();
    void SortTrack();
    void processData(char *data, unsigned short len);
    std::vector<ARPA_track> track_list;

    void addARPA(short id, float r, float a, float course, float velocity);
    void addAIS(std::string id, float mlat, float mlong, float course, float velocity);
};
#endif // C_ARPA_DATA_H

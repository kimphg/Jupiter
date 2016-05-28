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
typedef struct  {
    float               mlat;//
    float               mlong;//in km
    float               course ;
    float               velocity;
    qint64              time;
}AIS_object_t;
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
class AIS_track
{
public:
    AIS_track(){}
    std::vector<AIS_object_t> object_list;
    void addObject(AIS_object_t*newobj)
    {
        object_list.push_back(*newobj);
        mlat = newobj->mlat;
        mlong = newobj->mlong;
        course = newobj->course ;
        velocity =  newobj->velocity;
        lives = 20;
    }
    float mlat,mlong;
    float          course ;
    float          velocity;
    std::string           id;
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
    std::vector<AIS_track> ais_track_list;
    void addARPA(short id, float r, float a, float course, float velocity);
    void addAIS(std::string id, float mlat, float mlong, float course, float velocity);
};
#endif // C_ARPA_DATA_H

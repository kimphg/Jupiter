#include "c_arpa_data.h"
#include <math.h>
#define PI 3.141592654f
C_ARPA_data::C_ARPA_data()
{

}

C_ARPA_data::~C_ARPA_data()
{

}
void C_ARPA_data::SortTrack()
{
    for (unsigned short i=0;i<track_list.size()-1;i++)
    {
        for(unsigned short j=i+1;j<track_list.size();j++)
        {
            if(track_list[i].id>track_list[j].id)
            {
                ARPA_track temp= track_list[i];
                track_list[i] = track_list[j];
                track_list[j] = temp;
            }
        }
    }
}
void C_ARPA_data::processData(char* data, unsigned short len)
{
    for(short i=0;i<len;i++)
    {
        if(*data==0)(*data)++;
    }

    QString str(data);
    QStringList strList = str.split(",");
    if((*strList.begin()).contains("TTM"))
    {
        short tNum = (*(strList.begin()+1)).toInt();
        float tDistance = (*(strList.begin()+2)).toFloat()*CONST_NM;
        float tazi = (*(strList.begin()+3)).toFloat();// in degrees
        if(tazi<0)tazi += 360;
        float velocity = (*(strList.begin()+5)).toFloat();
        float course = (*(strList.begin()+6)).toFloat();

        addARPA(tNum,tDistance,tazi,course,velocity);
    }

}
void C_ARPA_data::addARPA(short id,float r,float a,float course,float velocity)
{
    //printf("\n time:%I64u",QDateTime::currentMSecsSinceEpoch());

    ARPA_object_t newobj;
    newobj.centerA = PI*a/180;
    newobj.centerR = r;//in km
    newobj.centerX = newobj.centerR*sinf(newobj.centerA);
    newobj.centerY = - newobj.centerR*cosf(newobj.centerA);
    newobj.course = PI*course/180;
    newobj.velocity = velocity*18;
    newobj.time = QDateTime::currentMSecsSinceEpoch();
    for(unsigned short i=0;i<track_list.size();i++)
    {
        if(track_list[i].id==id)
        {
            track_list[i].addObject(&newobj);

            return;
        }
    }
    ARPA_track newtrack;
    newtrack.addObject(&newobj);
    newtrack.id=id;
    track_list.push_back(newtrack);
    SortTrack();
}
void C_ARPA_data::addAIS(std::string id, float mlat, float mlong, float course, float velocity)
{
    //printf("\n time:%I64u",QDateTime::currentMSecsSinceEpoch());

    AIS_object_t newobj;
    newobj.mlat = mlat;
    newobj.mlong = mlong;
    newobj.course = course;
    newobj.velocity = velocity;
    newobj.time = QDateTime::currentMSecsSinceEpoch();
    for(unsigned short i=0;i<ais_track_list.size();i++)
    {
        if(ais_track_list[i].id==id)
        {
            ais_track_list[i].addObject(&newobj);

            return;
        }
    }
    AIS_track newtrack;
    newtrack.addObject(&newobj);
    newtrack.id=id;
    ais_track_list.push_back(newtrack);

}

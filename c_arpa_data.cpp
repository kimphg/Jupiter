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
//void C_ARPA_data::addAIS(std::string id, float mlat, float mlong, float course, float velocity)
//{
//    //printf("\n time:%I64u",QDateTime::currentMSecsSinceEpoch());

//    AIS_object_t newobj;
//    newobj.mlat = mlat;
//    newobj.mlong = mlong;
//    newobj.course = course;
//    newobj.velocity = velocity;
//    newobj.time = QDateTime::currentMSecsSinceEpoch();
//    for(unsigned short i=0;i<ais_track_list.size();i++)
//    {
//        if(ais_track_list[i].id==id)
//        {
//            ais_track_list[i].addObject(&newobj);

//            return;
//        }
//    }
//    AIS_track newtrack;
//    newtrack.addObject(&newobj);
//    newtrack.id=id;
//    ais_track_list.push_back(newtrack);

//}

AIS_object_t::AIS_object_t()
{

}

QString AIS_object_t::printData()
{
    QString output;
    output.append(QString::fromUtf8("MMSI:")+QString::number(mMMSI)+"\n");
    output.append(QString::fromUtf8("IMO:")+QString::number(mImo)+"\n");
    output.append(QString::fromUtf8("Tên:")+mName+"\n");
    output.append(QString::fromUtf8("Loại:"));
    switch(mType/10)
    {
    case 2:
        output.append(QString::fromUtf8("Thủy phi cơ"));
        break;
    case 3:
        if(mType==30)output.append(QString::fromUtf8("Tàu cá"));
        else if(mType==31||mType==32)output.append(QString::fromUtf8("Tàu kéo"));
        else output.append(QString::fromUtf8("Thuyền"));
        break;
    case 4:
        output.append(QString::fromUtf8("Tàu cao tốc"));
        break;
    case 5:
        output.append(QString::fromUtf8("Tàu chuyên dụng"));
        break;
    case 6:
        output.append(QString::fromUtf8("Tàu chở khách"));
        break;
    case 7:
        output.append(QString::fromUtf8("Tàu hàng"));
        break;
    case 8:
        output.append(QString::fromUtf8("Tàu chở dầu"));
        break;
    default:
        output.append(QString::fromUtf8("Không xác định"));
        break;
    }
    output.append("("+ QString::number(mType)+")");output.append("\n");
    output.append(QString::fromUtf8("Kinh độ:")
                  +QString::number(mLong)
                  +"\n");
    output.append(QString::fromUtf8("Vĩ độ:")
                  +QString::number(mLat)
                  +"\n");
    output.append(QString::fromUtf8("Hướng di chuyển:")
                  + QString::number(mCog)
                  + "\n");
    output.append(QString::fromUtf8("Tốc độ:")
                  + QString::number(mSog)
                  +"\n");
    output.append(QString::fromUtf8("Chiều dài:")
                  + QString::number(mBow+mStern)
                  + "\n");
    output.append(QString::fromUtf8("Chiều rộng:")
                  + QString::number(mPort+mStarboard)
                  +"\n");
    output.append(QString::fromUtf8("Điểm đến:")
                  + mDst
                  +"\n");
    return output;
}

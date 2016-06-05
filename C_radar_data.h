#ifndef C_RAW_DATA_H
#define C_RAW_DATA_H

#define ARMA_USE_LAPACK
#define ARMA_USE_BLAS
#define ARMA_BLAS_UNDERSCORE


#define TRACK_STABLE_STATE          6
#define MIN_TERRAIN                 10
#define TRACK_CONFIRMED_SIZE        3
#define TRACK_INIT_STATE            3
#define PI_NHAN2                    6.2831853072f
#define PI_CHIA2                    1.5707963268f
#define PI                          3.141592654f
#define MAX_TRACK_LEN               400
#define MAX_TRACKS                  199
#define MAX_AZIR                    2048
#define MAX_AZIR_DRAW               6144
#define RAD_M_PULSE_RES             1536
#define RAD_S_PULSE_RES             256
#define DISPLAY_RES                 768
#define RAD_FULL_RES                1792
#define SIGNAL_SCALE_0      0.16104f//0.1464f//0.094523077
#define SIGNAL_SCALE_1      0.12892f//0.094523077
#define SIGNAL_SCALE_2      0.09669f//0.063015385
#define SIGNAL_SCALE_3      0.06446f//0.031507692
#define SIGNAL_SCALE_4      0.03278f
#define SIGNAL_SCALE_5      0.01606f
#define TERRAIN_GAIN        0.9f
#define TERRAIN_GAIN_1      0.1f
#define TERRAIN_THRESH      0.5f
#define TARGET_MIN_SPEED      3
#define TARGET_MAX_SPEED      50
#include <vector>
#include <math.h>
#include <QImage>
#include <QDateTime>
#include <QDebug> //REMLATER
/*#ifdef _WIN32
#include <armadilloWin32/armadillo>
#else
#include <armadilloLinux/armadillo>
#endif
using namespace arma;*/
//#include <list>
using namespace std;
/*typedef struct {
    short x,y;
    unsigned char level;
    unsigned char displaylevel;
    unsigned char vet;
    unsigned char dopler;
    float terrain;
    short markIndex;
}raw_point_t;

typedef struct {
    raw_point_t raw_map[RAD_FULL_RES];

}frame_t;
*/
typedef struct  {
    unsigned char level_m [MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char sled[MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char dopler_m[MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char level_s [MAX_AZIR][RAD_S_PULSE_RES];
    unsigned char dopler_s[MAX_AZIR][RAD_S_PULSE_RES];
    unsigned char display [DISPLAY_RES][3];//0 - signal, 1- dopler, 2 - sled;
    unsigned char display_mask [DISPLAY_RES*2+1][DISPLAY_RES*2+1];
    short x[MAX_AZIR_DRAW][DISPLAY_RES+1];
    short y[MAX_AZIR_DRAW][DISPLAY_RES+1];
} signal_map_t;

typedef struct  {
    short maxA,minA;
    double sumA,sumR;
    //unsigned short maxR,minR;
    float sumTer;
    double size;
    //bool isProcessed;
} mark_t;
typedef struct  {
    float          az ,rg;
    short          size;
    //short           markNum;
    float          x,y;
    float          p;
    float          terrain;
}object_t;
typedef std::vector<mark_t> markList;
typedef std::vector<object_t> objectList;
enum ObjectClass{
    WHITE_OBJ,BLUE_OBJ,RED_OBJ
};
//______________________________________//
class track_t {
public:
    track_t()
    {

    }
    qint64 currentTimeMs;
//    fmat q1,q2,h,p,x;
    objectList suspect_list,object_list;
    float deltaAzi;
    float estX ,estY;
    float estA, estR;
    float course, velocity;
    char state;
    float dTime;
    QDateTime time;
    bool isProcessed;
    bool isUpdated;
    ObjectClass tclass;
    bool isMoving;//,confirmed;
    bool isManeuvering;
    void updateTime()
    {

        qint64 newTimeMs = time.currentMSecsSinceEpoch();
        dTime = (((long long int)newTimeMs - (long long int)currentTimeMs))/1000.f;
        currentTimeMs = newTimeMs;

        //dTime = 5;

    }
    void init(float mesA,float mesR, char initState)
    {/*

        currentTimeMs = time.toMSecsSinceEpoch();
        q1<<  0 <<  0 <<  0 <<  0 <<endr
          <<  0 <<  0 <<  0 <<  0 <<endr
          <<  0 <<  0 <<  4 <<  0 <<endr
          <<  0 <<  0 <<  0 <<  4 <<endr;

        q2<<  0 <<  0 <<  0 <<  0 <<endr
          <<  0 <<  0 <<  0 <<  0 <<endr
          <<  0 <<  0 <<  12 <<  0 <<endr
          <<  0 <<  0 <<  0 <<  12 <<endr;

        h <<  1 <<  0 <<  0 <<  0 <<endr
          <<  0 <<  1 <<  0 <<  0 <<endr;

        p << 100 <<  0 <<  0 <<  0 <<endr
          <<  0 << 100 <<  0 <<  0 <<endr
          <<  0 <<  0 <<  200 <<  0 <<endr
          <<  0 <<  0 <<  0 <<  200 <<endr;//ENVDEP

        x <<  0 <<endr
          <<  0 <<endr
          <<  0 <<endr
          <<  0 <<endr;
        //

       isManeuvering = false;
       isProcessed= true;
       isUpdated = false;
       estA = mesA;
       estR = mesR;
       deltaAzi = 0;
       velocity = 0;
       state = initState;
       if(object_list.size())object_list.clear();
       if(suspect_list.size())suspect_list.clear();
       isMoving = false;
       tclass = WHITE_OBJ;
       predict();*/
    }
    void updateTrackParam()
    {
        if(tclass !=WHITE_OBJ)
        {
            if(tclass == BLUE_OBJ)
            {

                if(object_list.size()>20)
                {

                    float sx,sy;
                        sx = object_list[object_list.size()-1].x-object_list[1].x;
                        sy = object_list[object_list.size()-1].y-object_list[1].y;
                    if(sx+sy>10)
                    {
                        tclass = RED_OBJ;

                    }
                }
            }
            if(object_list.size()>MAX_TRACK_LEN)// lich su qui dao toi da
            {
                objectList new_ojb_list;
                new_ojb_list.swap(object_list);
                for(unsigned short i = 20; i < new_ojb_list.size() ; i++)//cut off 20 first elements
                {
                    object_list.push_back(new_ojb_list[i]);
                }
            }
            float dx,dy;
            if(object_list.size()>12)
            {
                dx = object_list[object_list.size()-1].x-object_list[object_list.size()-12].x;
                dy = object_list[object_list.size()-1].y-object_list[object_list.size()-12].y;
                float dist = dx*dx+dy*dy;
                velocity += 0.5f*(sqrtf(dist)/11.0f/(dTime)-velocity);
            }
            else if(object_list.size()>6)
            {
                dx = object_list[object_list.size()-1].x-object_list[object_list.size()-6].x;
                dy = object_list[object_list.size()-1].y-object_list[object_list.size()-6].y;
                float dist = dx*dx+dy*dy;
                velocity  += 0.5f*(sqrtf(dist)/5.0f/(dTime)-velocity);//= sqrtf(dist)/5.0f/(dt);
            }
//            else if(object_list.size()>3)
//            {
//                dx = object_list[object_list.size()-1].x-object_list[object_list.size()-2].x;
//                dy = object_list[object_list.size()-1].y-object_list[object_list.size()-2].y;
//                float dist = dx*dx+dy*dy;
//                velocity  += 0.5f*(sqrtf(dist)/(dTime)-velocity);//= sqrtf(dist)/dt;
//            }

            course = atanf(dx/dy);
            if(dy<0)course+=PI;
            if(course<0)course+=PI_NHAN2;
            /*if(velocity>0.05f)//pixel/s
            {
                isMoving = true;
            }
            else
            {
                isMoving = false;
            }*/
        }
        else if(object_list.size()>TRACK_CONFIRMED_SIZE)
        {
            tclass = BLUE_OBJ;
            //printf("blue");
        }
    }
    void update()
    {/*
        float mesA;
        float mesR;
        //dTime = 5;
        updateTime();
        float pmax = 0;
        short k=-1;
        for(unsigned short i=0;i<suspect_list.size();i++)
        {
            if(pmax<suspect_list[i].p)
            {
                k=i;
                pmax=suspect_list[i].p;
            }
        }
        if(k>=0)
        {
            mesA = suspect_list[k].az;
            mesR = suspect_list[k].rg;
            object_list.push_back(suspect_list[k]);
            isUpdated = true;// !!!
            suspect_list.clear();
        }
        else
        {
            isUpdated = false;
        }

        //
        //______________________________________//
        //Bayesian EKF tracking algorithm written by Phung Kim Phuong
        //______________________________________//

        if(isUpdated)
        {
            if(state<10)state++;
            //measurement :
            float cc = mesR*cosf(mesA-estA)-estR;
            float dd = mesR*tanf(mesA-estA);
            fmat z;
            z<<cc<<endr<<dd<<endr;
            //z.print("\nZ:\n");return;//!!!
            //measurement error matrix r:
            //rr = estR*estR*0.0003// azi error for HR2D is about 1 deg -> (2*pi/360)^2=0.0003
            //range error is about 1.5-> 1.5^2=2
            fmat r;
            //r<<(object_list[object_list.size()-1].drg+1)*(object_list[object_list.size()-1].drg+1)<<0<<endr
            // <<0<<(object_list[object_list.size()-1].daz+1)*(object_list[object_list.size()-1].daz+1)/100.0<<endr;//NIM!!!
            r<<2<<0<<endr
             <<0<<estR*estR*0.0002<<endr;//NIM!!!
//            printf("object_list[object_list.size()-1].daz:%f",object_list[object_list.size()-1].daz);
//            r.print("\nr:------------\n");
            //Kalman gain:
            fmat k;
            k = p*h.t()*inv(h*p*h.t() + r);
            //k.ones();
            //k.print("\nk:--------\n");
            //correct estimation:
            //x.print("\nXP:");
            x = x+k*(z-h*x);
            //x.print("\nX:");
            deltaAzi = atanf(x.at(3,0)/estR);
            estA += deltaAzi;
            estR += x.at(2,0);//delta range = x.at(3,1);
            object_list[object_list.size()-1].x = ((sinf(estA)))*estR;
            object_list[object_list.size()-1].y = ((cosf(estA)))*estR;
            //correct error covariance:
            p = p - k*h*p;
            if(tclass !=WHITE_OBJ)predict();
        }else//miss:
        {
            if(state)state--;
            deltaAzi = atan(x.at(3,0)/estR);
            estA += deltaAzi;
            estR += x.at(2,0);
            object_list[object_list.size()-1].x = ((sinf(estA)))*estR;
            object_list[object_list.size()-1].y = ((cosf(estA)))*estR;
            if(tclass !=WHITE_OBJ)predict();
        }
        updateTrackParam();*/
    }
    void predict()
    {/*
        float aa = cos(deltaAzi);
        float bb = sin(deltaAzi);//NIM
        isManeuvering = (deltaAzi>0.001);
        //printf("\n delta azi:%f",deltaAzi);
        fmat a;// jacobian matrix
        a <<  0 <<  0 <<  aa<<  bb<<endr
          <<  0 <<  0 << -bb<<  aa<<endr
          <<  0 <<  0 <<  aa<<  bb<<endr
          <<  0 <<  0 << -bb<<  aa<<endr;
        x = a*x;
        //predict error covariance:
        if(isManeuvering)p = a*p*a.t()+q2;
        else p = a*p*a.t()+q1;*/
    }
    bool checkProb(object_t* object)
    {
        float dA = object->az - estA;
        if(dA>PI) dA-=PI_NHAN2;
        else if(dA<-PI)dA+=PI_NHAN2;//----------------
        float dR = object->rg - estR;
        dA*=dA;
        dR*=dR;
        if((tclass!=WHITE_OBJ)&&(state>TRACK_STABLE_STATE))
        {
            if(dR>=4 || dA>=0.0007f)return false;//0.5 do = 0.009rad;(0.009*3)^2 = 0.0007
            object->p = (1.0f-dR/9.0f)*(1.0f-dA/0.0007f);
        }else
        {
            if(dR>=12 || dA>=0.0021f)return false;//0.5 do = 0.009rad;(0.009*3)^2 = 0.0007
            object->p = (1.0f-dR/27.0f)*(1.0f-dA/0.0021f);
        }
        return true;
    }


};
typedef std::vector<track_t> trackList;
//______________________________________//
enum imgDrawMode
{
    VALUE_ORANGE_BLUE = 0,
    VALUE_YELLOW_SHADES = 1,
    VALUE_ORANGE_CYAN = 2,
    DOPLER_3_COLOR = 3,
    DOPLER_4_COLOR = 4
};
enum DataOverLay { m_only, s_m_200, s_m_150 , max_s_m_200, max_s_m_150};
class C_radar_data {
public:

    C_radar_data();
    ~C_radar_data();
    float k_vet;// !!!!
    trackList               mTrackList;
    markList                mark_list;
    signal_map_t            signal_map;
    unsigned char           size_thresh,overload, terrain_init_time, clk_adc;
    float                   viewScale;
    unsigned short          sn_stat;
    bool                    avtodetect,xl_nguong,isClkAdcChanged;
    float                   krain,kgain,ksea,brightness;
    float                   temp;
    float                   trueN;
    DataOverLay             dataOver;
    bool                    isDisplayAlpha;
    unsigned char           noise_level[8];
    unsigned char           tempType,rotation_speed;
    unsigned short          range_max;
    QImage                  *img_ppi,*img_alpha;
    imgDrawMode             imgMode;
    void deleteTrack(short trackNum);
    //______________________________________//
    void        GetDataHR(unsigned char *data, unsigned short dataLen);
    void        redrawImg();
    void        ProcessDataFrame();
    void        GetData(unsigned short azi);
    void        raw_map_init();
    void        drawAzi(short azi);
    void        drawBlackAzi(short azi_draw);
    void        drawDopler(short azi);
    void        blackLine(short x0, short y0, short x1, short y1);
    void        addTrackManual(float x, float y);
    void        addTrack(float azi, float range, short state);
    void        getPolar(float x,float y,float *azi,float *range);
    void        setTrueN(float trueN_deg){

        while(trueN_deg<0)trueN_deg+=360;
        while(trueN_deg>=360)trueN_deg-=360;
        trueN =(trueN_deg/360.0f*PI_NHAN2);
        raw_map_init();
    }
    void        setViewScale(float scale);
    void        resetData();
    void        setProcessing(bool onOff);
    //bool        getDataOverload(){if(isDataTooLarge) {isDataTooLarge =false;return true;} else return false;}
//    bool        checkFeedback(unsigned char* command)
//    {
//        for (short i=0;i<8;i++)
//        {if(command[i]!=command_feedback[i])return false;}
//        memset(&command_feedback[0],0,8);
//        return true;
//    }
     char* getFeedback()
        {

            return (char*)&command_feedback[0];
        }
    void resetTrack();
private:
    void        setSnLevel(unsigned short azi, unsigned  short range, unsigned char level);
    void        setDoplerLevel(unsigned short azi, unsigned  short range, unsigned char level);
    void        drawVet(short azi, short r_pos);
    void        drawSgn(short azi_draw, short r_pos);
    //void        drawZero(short azi, short r_pos);
    //void        fadeSignPix(short px, short py);
    unsigned char command_feedback[8];

    //bool        isDataTooLarge;
    void        polarToXY(float *x, float *y, float azi, float range);
    bool        isProcessing;
    short       curAzir;
    float       noiseAverage,rainLevel,noiseVar;
    void        getNoiseLevel();
    void        procPix(short proc_azi,short range);
    void        procTracks(unsigned short curA);
    void        procMark(mark_t* pMark);
    void        procObject(object_t* pObject);

    //void status_start();
    //FILE *pFile;
};

//extern C_radar_data radarData;

#endif

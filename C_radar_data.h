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
#define ZOOM_SIZE           550
#define DISPLAY_RES_ZOOM            4096
#define DISPLAY_SCALE_ZOOM           4
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
    //processing data
    unsigned char level [MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char level_disp [MAX_AZIR][RAD_M_PULSE_RES];
    bool          detect[MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char dopler[MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char dopler_old[MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char dopler_old2[MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char sled[MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char hot[MAX_AZIR][RAD_M_PULSE_RES];
    short         plotIndex[MAX_AZIR][RAD_M_PULSE_RES];
    //display data
    unsigned char display_ray [DISPLAY_RES][3];//0 - signal, 1- dopler, 2 - sled;
    unsigned char display_ray_zoom [DISPLAY_RES_ZOOM][3];
    unsigned char display_mask [DISPLAY_RES*2+1][DISPLAY_RES*2+1];
    unsigned char display_mask_zoom [DISPLAY_RES_ZOOM*2+1][DISPLAY_RES_ZOOM*2+1];
    short x[MAX_AZIR_DRAW][DISPLAY_RES+1];
    short y[MAX_AZIR_DRAW][DISPLAY_RES+1];
    short xzoom[MAX_AZIR_DRAW][DISPLAY_RES_ZOOM];
    short yzoom[MAX_AZIR_DRAW][DISPLAY_RES_ZOOM];
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
    char           dopler;
    float          x,y;
    float          p;
    float          terrain;
}object_t;
typedef std::vector<mark_t> markList;
typedef std::vector<object_t> objectList;

//______________________________________//
class track_t {
public:
    track_t()
    {

    }
    qint64 currentTimeMs;
    bool confirmed;
    objectList suspect_list,object_list;
    float deltaAzi;
    float estX ,estY;
    float estA, estR;
    float course, velocity;
    char state;
    float dTime;
    //QDateTime time;
    bool isProcessed;
    bool isUpdated;
    void updateTime()
    {

//        qint64 newTimeMs = time.currentMSecsSinceEpoch();
//        dTime = (((long long int)newTimeMs - (long long int)currentTimeMs))/1000.f;
//        currentTimeMs = newTimeMs;

    }
    void init(object_t *object)
    {
        this->object_list.push_back(*object);
        estA = object->az;
        estR = object->rg;
        estX = ((sinf(estA)))*estR;
        estY = ((cosf(estA)))*estR;
        velocity = 0;
        confirmed = false;
        state = 1;
        dTime = 5;
    }
    void update()
    {
        float mesA;
        float mesR;

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
            isUpdated = true;
            state++;
            suspect_list.clear();
        }
        else
        {
            isUpdated = false;
            state--;
        }
        if(object_list.size()>10)
        {
            confirmed = true;
        }
        if(isUpdated)
        {
            float mesX = ((sinf(mesA)))*mesR;
            float mesY = ((cosf(mesA)))*mesR;
            float dx = mesX - estX;
            float dy = mesY - estY;
            estX+=(mesX-estX)/2;
            estY+=(mesY-estY)/2;
            float new_course = atanf(dx/dy);
            if(dy<0)new_course+=PI;
            if(new_course<0)new_course += PI_NHAN2;
            float new_velo = sqrt(dx*dx+dy*dy);
            if(!course)course = new_course;else course+= (new_course-course)/2;
            if(!velocity)velocity = new_velo;else velocity+= (new_velo-velocity)/2;
        }
        predict();
    }
    void predict()
    {
        estX += ((sinf(course)))*velocity;
        estY += ((cosf(course)))*velocity;
        estA = atanf(estX/estY);
        if(estY<0)estA += PI;
        if(estA<0)estA += PI_NHAN2;
        estR = estX*estX + estY*estY;
    }
    bool checkProb(object_t* object)
    {
        float dA = object->az - estA;
        if(dA>PI) dA-=PI_NHAN2;
        else if(dA<-PI)dA+=PI_NHAN2;//----------------
        float dR = object->rg - estR;
        dA*=dA;
        dR*=dR;
        if(state>TRACK_STABLE_STATE)
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
    DOPLER_3_COLOR = 2,
};
enum DataOverLay { m_only, s_m_200, s_m_150 , max_s_m_200, max_s_m_150};
class C_radar_data {
public:

    C_radar_data();
    ~C_radar_data();
    float k_vet;// !!!!
    trackList               mTrackList;
    markList                suspect_list;
    signal_map_t            data_mem;
    unsigned char           size_thresh,overload, terrain_init_time, clk_adc;
    float                   scale_ppi,scale_zoom;

    void                    updateZoomRect(float ctx, float cty);
    unsigned short          sn_stat;
    bool                    avtodetect,isClkAdcChanged,xl_dopler,cut_thresh;
    bool                    isFilting,rgs_auto,bo_bang_0;
    float                   krain,kgain,ksea,brightness;
    float                   krain_auto,kgain_auto,ksea_auto;
    void setAutorgs( bool aut)
    {
        if(aut)
        {
            rgs_auto = true;
            krain_auto = 0.5;
            kgain_auto  = 3;
            ksea_auto = 0;

        }else
        {
            rgs_auto = false;
        }

    }
    float                   temp;
    float                   trueN;
    DataOverLay             dataOver;
    bool                    isDisplayAlpha;
    unsigned char           noise_level[8];
    unsigned char           tempType,rotation_speed;
    unsigned short          range_max;
    QImage                  *img_ppi,*img_alpha,*img_zoom_ppi;
    imgDrawMode             imgMode;
    void deleteTrack(short trackNum);
    //______________________________________//
    void        GetDataHR(unsigned char *data, unsigned short dataLen);
    void        redrawImg();
    void        ProcessDataFrame();
    void        ProcessData(unsigned short azi);
    void        raw_map_init();
    void        raw_map_init_zoom();
    void        drawAzi(short azi);
    void        drawBlackAzi(short azi_draw);
    void        DrawZoom(short azi_draw, short r_pos);
//    void        blackLine(short x0, short y0, short x1, short y1);
    void        addTrackManual(float x, float y);
    void        addTrack(object_t *mObject);
    void        getPolar(float x,float y,float *azi,float *range);
    void        setTrueN(float trueN_deg){

        while(trueN_deg<0)trueN_deg+=360;
        while(trueN_deg>=360)trueN_deg-=360;
        trueN =(trueN_deg/360.0f*PI_NHAN2);
        raw_map_init();
    }
    void        setScalePPI(float scale);
    void        setScaleZoom(float scale);
    void        resetData();
    void        resetSled();
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

    void        setDoplerLevel(unsigned short azi, unsigned  short range, unsigned char level);
    uint        getColor(unsigned char pvalue, unsigned char dopler, unsigned char sled);
    void        drawSgn(short azi_draw, short r_pos);
    void        drawSgnZoom(short azi_draw, short r_pos);
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
    void        procPLot(mark_t* pMark);
    void        procObject(object_t* pObject);

    //void status_start();
    //FILE *pFile;
};

//extern C_radar_data radarData;

#endif

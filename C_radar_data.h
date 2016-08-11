#ifndef C_RAW_DATA_H
#define C_RAW_DATA_H
//----------------------------------------------------------//
//HR2D signal processing class and EKF tracking algorithm   //
//First release: November 2015                              //
//Project:https://github.com/kimphg/Jupiter                 //
//Last update: August 2016                                  //
//Author: Phung Kim Phuong                                  //
//----------------------------------------------------------//
#define ARMA_USE_LAPACK
#define ARMA_USE_BLAS
#define ARMA_BLAS_UNDERSCORE


#define TRACK_STABLE_STATE          5
#define MIN_TERRAIN                 10
#define TRACK_CONFIRMED_SIZE        3
#define TRACK_INIT_STATE            3
#define PI_NHAN2                    6.2831853072f
#define PI_CHIA2                    1.5707963268f
#ifndef PI
   #define PI                          3.141592654f
#endif
#define MAX_TRACK_LEN               400
#define MAX_TRACKS                  199
#define MAX_AZIR                    2048
#define MAX_AZIR_DRAW               6144
#define RAD_M_PULSE_RES             1536
#define RAD_S_PULSE_RES             256
#define DISPLAY_RES                 768
#define RAD_FULL_RES                1792
#define SIGNAL_SCALE_7      0.21538461538461538f //215.38461538461538461538461538461
#define SIGNAL_SCALE_6      0.18461538461538462f //184.61538461538461538461538461538
#define SIGNAL_SCALE_5      0.15384615384615385f //153.84615384615384615384615384615
#define SIGNAL_SCALE_4      0.12307692307692308f // 123.07692307692307692307692307692
#define SIGNAL_SCALE_3      0.09230769230769231f //92.307692307692307692307692307692
#define SIGNAL_SCALE_2      0.06153846153846154f //61.538461538461538461538461538462
#define SIGNAL_SCALE_1      0.03076923076923077f //30.769230769230769230769230769231
#define SIGNAL_SCALE_0      0.01538461538461538f //15.384615384615384615384615384615
#define TERRAIN_GAIN        0.9f
#define TERRAIN_GAIN_1      0.1f
#define TERRAIN_THRESH      0.5f
#define TARGET_MIN_SPEED      3
#define TARGET_MAX_SPEED      50
#define ZOOM_SIZE           550
#define DISPLAY_RES_ZOOM            5120
#define DISPLAY_SCALE_ZOOM           4
#include <vector>
#include <QImage>
#include <QDateTime>
#include <QFile>
#include <Eigen/Dense>


//#include <QDebug> //REMLATER
//#ifdef _WIN32
//#include <armadillo>
//#else
//#include <armadilloLinux/armadillo>
//#endif
//using namespace arma;
//#include <list>
//using namespace std;
using namespace Eigen;
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
    unsigned int sumTer,sumA,sumR;
    short maxA,minA,ctA;
    unsigned short maxR,minR,ctR;
    unsigned short size;
    unsigned char maxLevel,dopler;
    //bool isProcessed;
} plot_t;
typedef struct  {
    float          az ,rg,x,y;
    short          azMin,azMax,rMin,rMax;
    short          size;
    char           dopler;
    bool           isManual;
    float          p;
    float          terrain;
}object_t;
typedef std::vector<plot_t> plotList;
typedef std::vector<object_t> objectList;
using Eigen::MatrixXf;
//class matrix_t
//{
//public:
//    float *data;
//    short row,col;
//    matrix_t()
//    {
//        data = 0;
//        row = 0;
//        col = 0;
//    }
//    void init(short irow,short icol) {

//        data = new float[irow*icol];
//        memset(data,0,irow*icol);
//        row = irow;
//        col = icol;

//    }
//    ~matrix_t() {
//        if(data)delete[] data;
//    }
//    float* at(short r,short c)
//    {
//        if(r>row||c>col) return 0;
//        return &data[col*r+c];
//    }
//    matrix_t mul(matrix_t *mat)
//    {
//        short r1 = this->row;
//        short c1 = this->col;
//        short c2 = mat->col;
//        matrix_t m;
//        m.init(r1,c2);
//        if(c1!=mat->row)return m;

//        for(short i=0; i<r1; ++i)
//        for(short j=0; j<c2; ++j)
//        for(short k=0; k<c1; ++k)
//        *(m.at(i,j)) += (*this->at(i,k))*(*mat->at(k,j));
//        return m;
//    }
//};
//______________________________________//
class track_t {
public:
    track_t()
    {

    }

    //
    MatrixXf q1;
    MatrixXf q2;
    MatrixXf h;
    MatrixXf p;
    MatrixXf x;
    //
    //qint64 currentTimeMs;
    bool confirmed;
    objectList suspect_list,object_list;
    char terrain;
    float deltaAzi;
    float estX ,estY;
    float estA, estR;
    float mesA;
    float mesR;
    float speed;
    float course;
    char state;
    float dTime;
    bool isTracking,isManual;
    bool isManeuvering;
    char dopler;
    //QDateTime time;
    bool isProcessed;
    bool isUpdated;
    float heading;
    void updateTime()
    {
    }
    void init(object_t *object)
    {
        q1.resize(4,4);
        q1<<    0 ,  0 ,  0 ,  0 ,
                0 ,  0 ,  0 ,  0 ,
                0 ,  0 ,  4 ,  0 ,
                0 ,  0 ,  0 ,  4 ;

        q2.resize(4,4);
        q2<<    0 ,  0 ,  0 ,  0 ,
                0 ,  0 ,  0 ,  0 ,
                0 ,  0 ,  12,  0 ,
                0 ,  0 ,  0 , 12 ;
        h.resize(2,4);
        h <<    1 ,  0 ,  0 ,  0 ,
                0 ,  1 ,  0 ,  0 ,

        p.resize(4,4);
        p <<   100,  0 ,  0 ,  0 ,
                0 , 100,  0 ,  0 ,
                0 ,  0 , 200,  0 ,
                0 ,  0 ,  0 , 200;

        x.resize(4,1);
        x<< 0,0,0,0;
        object_list.clear();
        suspect_list.clear();
        this->object_list.push_back(*object);
        this->dopler = object->dopler;
        estA = object->az;
        estR = object->rg;
        deltaAzi = 0;
        velocity = 0;
        speed = 0;
        heading = 0;
        course = 0;
        confirmed = false;
        isProcessed = true;
        isTracking = false;
        state = 3;
        dTime = 5;

        if( object->isManual)
        {
            //printf("debug new man\n");
            isManual = true;
            confirmed  = true;
            state = 5;
        }
        else
        {
            isManual = false;
        }
    }
    void update()
    {

        isTracking = true;
        float pmax = 0;
        short k=-1;
        for(unsigned short i=0;i<suspect_list.size();i++)
        {
            if(suspect_list.at(i).isManual)
            {
                isManual = true;
                confirmed  = true;
                state = 5;
                k=i;
                break;
            }
            if(pmax<suspect_list.at(i).p)
            {
                k=i;
                pmax=suspect_list.at(i).p;
            }
        }
        if(k>=0)
        {
            mesA = suspect_list[k].az;
            mesR = suspect_list[k].rg;
            object_list.push_back(suspect_list[k]);
            dopler = suspect_list[k].dopler;
            terrain = suspect_list[k].terrain;
            isUpdated = true;
            if(state<12)
            {
                if(confirmed)state+=3;
                else state+=2;
            }
            suspect_list.clear();
        }
        else
        {
            isUpdated = false;
            if(state)state--;
        }
        if(!confirmed)
        {
            if(object_list.size()>10)
            {
                if(state>TRACK_STABLE_STATE)confirmed = true;
            }
        }

        if(isUpdated)
        {

//          thuat toan loc Kalman
            float cc = mesR*cosf(mesA-estA)-estR;//DR
            float dd = mesR*tanf(mesA-estA);     //
            MatrixXf z(2,1);// vector gia tri do
            z<<cc,dd;
            Matrix2f r(2,2);
            r<< 2, 0 ,0, estR*estR ; // ma tran hiep bien do
            MatrixXf k(4,2) ;
            k = p*h.transpose()*((h*p*h.transpose() + r).inverse());
            x = x+k*(z-h*x);
            deltaAzi = atanf(x(3,0)/estR);
            estA += deltaAzi;
            estR += x(2,0);
            estX = object_list[object_list.size()-1].x = ((sinf(estA)))*estR;
            estY = object_list[object_list.size()-1].y = ((cosf(estA)))*estR;
            p = p - k*h*p;
            predict();
        }
        else
        {
            if(state)state--;
            deltaAzi = atan(x(3,0)/estR);
            estA += deltaAzi;
            estR += x(2,0);
            estX = object_list[object_list.size()-1].x = ((sinf(estA)))*estR;
            estY = object_list[object_list.size()-1].y = ((cosf(estA)))*estR;
            predict();
        }
        if(object_list.size()>15)
        {
            float dxt = object_list.at(object_list.size()-1).x - object_list.at(object_list.size()-15).x;
            float dyt = object_list.at(object_list.size()-1).y - object_list.at(object_list.size()-15).y;
            if(dyt)
            {
                float nspeed = sqrt(dxt*dxt + dyt*dyt)*SIGNAL_SCALE_3/70.0f*3600/1.852;
                speed=500;//!!!
                velocity = sqrt(dxt*dxt + dyt*dyt)/14.0f;
                heading = atanf(dxt/dyt);
                if(dyt<0)heading+=PI;
                if(heading<0)heading += PI_NHAN2;
            }
        }

    }
    void predict()
    {
        float aa = cos(deltaAzi);
        float bb = sin(deltaAzi);//NIM
        isManeuvering = (deltaAzi>0.001);
        //printf("\n delta azi:%f",deltaAzi);
        MatrixXf a(4,4);// jacobian matrix
        a <<  0 ,  0 ,  aa,  bb,
              0 ,  0 , -bb,  aa,
              0 ,  0 ,  aa,  bb,
              0 ,  0 , -bb,  aa;
        x = a*x;
        //predict error covariance:
        if(isManeuvering)p = a*p*a.transpose()+q2;
        else p = a*p*a.transpose()+q1;
//        if(object_list.size()>2)
//        {
//            float dx = ((sinf(course)))*velocity;
//            float dy = ((cosf(course)))*velocity;
//            estX+=dx;
//            estY+=dy;
//            if(estY!=0)
//            {
//                estA = atanf(estX/estY);
//                if(estY<0 )
//                {
//                    estA+=PI;
//                    if(estA>PI_NHAN2)estA-=PI_NHAN2;
//                }
//                estR = sqrt(estX*estX + estY*estY);
//            }
//        }
//        else
//        {
//            oldA = estA;
//            oldR = estR;
//            estA = (mesA+oldA)/2;
//            estR = (mesR+oldR)/2;
//        }
//        return;
//        estX += ((sinf(course)))*velocity;
//        estY += ((cosf(course)))*velocity;
//        estA = atanf(estX/estY);
//        if(estY<0)estA += PI;
//        if(estA<0)estA += PI_NHAN2;
//        estR = sqrt(estX*estX + estY*estY);
    }
    bool checkProb(object_t* object)
    {
        float dA = object->az - estA;
        if(dA>PI) dA-=PI_NHAN2;
        else if(dA<-PI)dA+=PI_NHAN2;//----------------
        float dR = object->rg - estR;
        if(dopler!=17){
            if(object->dopler!=17)
            {
                short doplerVar = abs(dopler - object->dopler);
                if(doplerVar>8)doplerVar = 16-doplerVar;
                if(doplerVar>1)return false;
            }
        }
        dA*=dA;
        dR*=dR;
        if(!isManual)
        {
            if(state>TRACK_STABLE_STATE)
            {
                if(dR>=9 || dA>=0.0007f)return false;//0.5 do = 0.009rad;(0.009*3)^2 = 0.0007
                object->p = 4/dR*0.0007f/dA;
            }else if(!confirmed)
            {
                if(dR>=12 || dA>=0.0009f)return false;//0.5 do = 0.009rad;(0.009*3)^2 = 0.0007
                object->p = 12/dR*0.0010f/dA;
            }
            else
            {
                if(dR>=16 || dA>=0.0012f)return false;//0.5 do = 0.009rad;(0.009*3)^2 = 0.0007
                object->p = 12/dR*0.0012f/dA;
            }
        }else
        {
            if(state<10)
            {
                if(dR>=16 || dA>=0.0012f)return false;//0.5 do = 0.009rad;(0.009*3)^2 = 0.0007
                object->p = 25/dR*0.0012f/dA;
            }
            else
            {
                if(dR>=12 || dA>=0.0009f)return false;//0.5 do = 0.009rad;(0.009*3)^2 = 0.0007
                object->p = 12/dR*0.0010f/dA;
            }

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
    plotList                plot_list;

    unsigned char           size_thresh,overload, terrain_init_time, clk_adc;
    float                   scale_ppi,scale_zoom;
    short                   curAzir;
    void                    updateZoomRect(float ctx, float cty);
    unsigned short          sn_stat;
    bool                    isClkAdcChanged,xl_dopler,cut_thresh,isSled,filter2of3;
    bool                    isManualTune,rgs_auto,bo_bang_0,data_export;
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
    void setAvtoDetect(bool arg)
    {
        terrain_init_time = 2;
        avtodetect = arg;
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
        resetTrack();
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
    void        resetTrack();
private:
    bool        avtodetect;
    uint        getColor(unsigned char pvalue, unsigned char dopler, unsigned char sled);
    void        drawSgn(short azi_draw, short r_pos);
    void        drawSgnZoom(short azi_draw, short r_pos);
    unsigned char command_feedback[8];
    void        polarToXY(float *x, float *y, float azi, float range);
    bool        isProcessing;

    float       noiseAverage,rainLevel,noiseVar;
    void        getNoiseLevel();
    void        procPix(short proc_azi,short range);
    void        procTracks(unsigned short curA);
    void        procPLot(plot_t* mPlot);
    bool procObjectAvto(object_t* pObject);
    bool procObjectManual(object_t* pObject);
    //void status_start();
    //FILE *pFile;
};

//extern C_radar_data radarData;

#endif

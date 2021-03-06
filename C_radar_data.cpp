#define PI 3.141592654f
#include "c_radar_data.h"
#include <QElapsedTimer>
#include <cmath>
//#include <QtDebug>

#define PLOT_MAX_SIZE 80
#define PLOT_MIN_SIZE 5
#define PLOT_MAX_DR 10
#define RANGE_MIN 50
#define TERRAIN_MAX 40
#define TERRAIN_INIT 20
#define RADAR_COMMAND_FEEDBACK  6
#define RADAR_DATA_HEADER_MAX   62
#define RADAR_DATA_SPECTRE      22
#define RADAR_DATA_MAX_SIZE     2688
#define RADAR_
short waitForData = 0;
short headerLen = RADAR_DATA_HEADER_MAX;
unsigned char curFrameId;
unsigned char dataBuff[RADAR_DATA_HEADER_MAX + RADAR_DATA_MAX_SIZE];
QFile *exp_file = NULL;

typedef std::queue<int> aziQueue;
aziQueue aziToProcess;//hàng chờ các frame cần xử lý

typedef struct  {
    //processing dataaziQueue
    unsigned char level [MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char level_disp [MAX_AZIR][RAD_M_PULSE_RES];
    bool          detect[MAX_AZIR][RAD_M_PULSE_RES];
    //unsigned char rainLevel[MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char dopler[MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char terrain[MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char dopler_old[MAX_AZIR][RAD_M_PULSE_RES];
    //    unsigned char dopler_old2[MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char sled[MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char hot[MAX_AZIR][RAD_M_PULSE_RES];
    unsigned char hot_disp[MAX_AZIR][RAD_M_PULSE_RES];
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
double sn_scale = SIGNAL_SCALE_0;
short curIdCount = 1;
qint64 cur_timeMSecs = 0;//QDateTime::currentMSecsSinceEpoch();
signal_map_t data_mem;
float                   rot_period_sec =0;
short histogram[256];
//static period_t                curPeriod;
//static std::queue<period_t>    period_cache;
//static unsigned short cur_mark_index = 0;

// -------------------Radar tracking class-------------
void track_t::init(object_t *object)
{
    q1.resize(4,4);
    q1<<    0 ,  0 ,  0 ,  0 ,
            0 ,  0 ,  0 ,  0 ,
            0 ,  0 ,  1 ,  0 ,
            0 ,  0 ,  0 ,  1 ;

    q2.resize(4,4);
    q2<<    0 ,  0 ,  0 ,  0 ,
            0 ,  0 ,  0 ,  0 ,
            0 ,  0 ,  2,  0 ,
            0 ,  0 ,  0 , 2 ;
    h.resize(2,4);
    h <<    1 ,  0 ,  0 ,  0 ,
            0 ,  1 ,  0 ,  0 ,

            p.resize(4,4);
    p <<   50 ,  0 ,  0 ,  0 ,
            0 ,  50,  0 ,  0 ,
            0 ,  0 , 50 ,  0 ,
            0 ,  0 ,  0 ,  50;

    x.resize(4,1);
    x<< 0,0,0,0;
    object_list.clear();
    suspect_list.clear();
    this->object_list.push_back(*object);
    this->dopler = object->dopler;
    estA = object->az;
    estR = object->rg;
    estX = object->x;
    estY = object->y;
    rotA_r = 0;
    speed = 0;
    heading = 0;
    isConfirmed = false;
    isProcessed = true;
    isTracking = false;
    isLost = false;
    state = 3;
    setManual(object->isManual);
}
void track_t::stateUpdate(bool isNewPlot)
{
    if(!isNewPlot)
    {
        if(state)
        {
            state--;
        }
        else
        {
            isConfirmed = false;
            if(isManual)
            {
                setManual(false);
                isLost = true;
            }
        }
    }
    else
    {
        state+=2;
        if(state>12)
        {
            state = 12;
        }
    }
}
void track_t::update()
{
    isTracking = true;
    float pmax = 0;
    // tim index cua object gan nhat
    short ip=-1;
    for(unsigned short i=0;i<suspect_list.size();i++)
    {
        if(suspect_list.at(i).isManual)
        {
            setManual(true);
            ip = i;
            break;
        }
        if(pmax<suspect_list.at(i).p)
        {
            ip = i;
            pmax=suspect_list.at(i).p;
        }
    }
    if(ip>=0)
    {
        mesA = suspect_list[ip].az;
        mesR = suspect_list[ip].rg;
        object_list.push_back(suspect_list[ip]);
        dopler = suspect_list[ip].dopler;
        terrain = suspect_list[ip].terrain;
        isUpdated = true;
        suspect_list.clear();
    }
    else
    {
        isUpdated = false;
    }
    suspect_list.clear();
    stateUpdate(isUpdated);
    if(!isConfirmed)
    {
        if(trackLen>10)
        {
            if(state>TRACK_STABLE_STATE)isConfirmed = true;
        }
    }
    trackLen = object_list.size();
    if(isUpdated)
    {
        //          thuat toan loc Kalman
        double cc = mesR*cos(mesA-estA)-estR;//DR
        double dd = mesR*tan(mesA-estA);     //
        MatrixXd z(2,1);// vector gia tri do
        z<<cc,dd;
        Matrix2d r(2,2);
        r<< 9, 0 ,0, estR*estR*0.0016 ;
        // ma tran hiep bien do
        MatrixXd k(4,2) ;

        Matrix2d tmp;
        tmp = (h*p*h.transpose() + r).inverse();
        k = p*h.transpose()*(tmp);
        //        if(isManual)
        //        {
        //            printf("\n%f %f \n %f %f \n %f %f \n%f %f\n\n",k(0,0),k(0,1),k(1,0),k(1,1),k(2,0),k(2,1),k(3,0),k(3,1));
        //        }
        //            if(isManual)
        //            {
        //                int a = 8;
        //                float x0 = x(0,0);
        //                float x1 = x(1,0);
        //                float x2 = x(2,0);
        //                float x3 = x(3,0);;
        //                float h1 = k(0,0);
        //                float h2 = k(1,1);
        //                float h3 = k(0,1);
        //                float h4 = k(1,0);
        //                h1=h2;
        //            }
        MatrixXd xx = x+k*(z-h*x);
        x = xx;
        Matrix4d pp ;
        pp = p - k*h*p;
        p = pp;

    }
    float dxt = 0 ;
    float dyt = 0 ;
    if(trackLen>1)
    {
        predict();
        if(trackLen>7)//smoothing the track history
        {
            object_list.at(trackLen-4).x = (object_list.at(trackLen-6).x + object_list.at(trackLen-5).x
                                            + object_list.at(trackLen-3).x + object_list.at(trackLen-2).x)/4.0;
            object_list.at(trackLen-4).y = (object_list.at(trackLen-6).y + object_list.at(trackLen-5).y
                                            + object_list.at(trackLen-3).y + object_list.at(trackLen-2).y)/4.0;;

        }
        if(trackLen<=10)
        {
            dxt = (object_list.at(trackLen-1).x - object_list.at(0).x)/(trackLen-1);
            dyt = (object_list.at(trackLen-1).y - object_list.at(0).y)/(trackLen-1);
        }
        if(trackLen>10)
        {
            dxt = (object_list.at(trackLen-1).x - object_list.at(trackLen-10).x)/9.0;
            dyt = (object_list.at(trackLen-1).y - object_list.at(trackLen-10).y)/9.0;

        }
        if(dyt)
        {

            if(rot_period_sec)
            {
                double nspeed = sqrt(dxt*dxt + dyt*dyt)*sn_scale/rot_period_sec*3600.0/1.852;
                speed+=(nspeed-speed)*0.3f;
            }
            heading = atanf(dxt/dyt);
            if(dyt < 0) heading += PI;
            if(heading<0)heading += PI_NHAN2;
        }
    }
}

void track_t::predict()
{

    estR += x(2,0);
    rotA_r = atan(x(3,0)/estR);
    estA += rotA_r;


    estX = ((sin(estA)))*estR;
    estY =  ((cos(estA)))*estR;
    object_list.at(trackLen-1).x = estX;
    object_list.at(trackLen-1).y = estY;
    double aa = cos(rotA_r);
    double bb = sin(rotA_r);//NIM

    MatrixXd a(4,4);// jacobian matrix
    a <<  0 ,  0 ,  aa,  bb,
            0 ,  0 , -bb,  aa,
            0 ,  0 ,  aa,  bb,
            0 ,  0 , -bb,  aa;
    x = a*x;
    //update error covariance:
    if((rotA_r>0.005f))
        p = a*p*a.transpose()+q2;
    else
        p = a*p*a.transpose()+q1;
    //        if(trackLen>2)
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
bool track_t::checkProb(object_t* object)
{
    float dA = object->az - estA;
    if(dA>PI) dA-=PI_NHAN2;
    else if(dA<-PI)dA+=PI_NHAN2;//----------------
    float dR = object->rg - estR;
    short doplerVar = 0;
    if(dopler!=17){
        if(object->dopler!=17)
        {
            doplerVar = abs(dopler - object->dopler);
            if(doplerVar>8)doplerVar = 16-doplerVar;
            if(doplerVar>1)return false;
        }
    }
    dA*=dA;
    dR*=dR;
    float maxDr = (3+0.04/sn_scale)*3;
    maxDr*=maxDr;
    float maxDa = (0.009f +atanf(0.04/estR/sn_scale))*3;
    maxDa*=maxDa;
    if(dR>=maxDr || dA>=maxDa)
    {
        return false;//0.5 do = 0.009rad;(0.009*3)^2 = 0.0007
    }
    float err = dR+dA;
    if(err)
    {

        object->p = (maxDr+maxDa)/(err);//  abs(maxDr/dR+maxDa/dA);
    }
    else
    {
        object->p = 1000000;
    }
    p /= (doplerVar+1);
    return true;
}

void track_t::setManual(bool isMan)
{
    this->isManual = isMan;
    if(isMan)
    {
        isConfirmed  = true;
        this->idCount = curIdCount;
        curIdCount++;
    }
}


// ---------------Data processing class------------------------
//QImage *imgAR;

C_radar_data::C_radar_data()
{
    mJammingDetected =false;
    mHsTapMax = 60;
    imgMode = VALUE_ORANGE_BLUE;
    brightness = 1.5;
    for(int i=0;i<255;i++)
    {

        colorTable.push_back((getColor(i,0,0)));
    }
    mHsTap = 0;
    //he_so_tap_recv=new unsigned short[MAX_AZIR];
    img_histogram=new QImage(257,101,QImage::Format_Mono);
    img_histogram->fill(0);
    img_ppi = new QImage(DISPLAY_RES*2+1,DISPLAY_RES*2+1,QImage::Format_ARGB32);
    img_RAmp = new QImage(RAD_M_PULSE_RES,256,QImage::Format_ARGB32);
    img_spectre = new QImage(16,256,QImage::Format_Mono);
    img_spectre->fill(0);
    img_zoom_ppi = new QImage(ZOOM_SIZE+1,ZOOM_SIZE+1,QImage::Format_ARGB32);
    img_zoom_ar = NULL;
    tb_tap_k = 1;
    setZoomRectAR(10,10,1.852,10);
    mEncoderAzi = 0;
    //img_zoom_ar->setColorTable(colorTable);
    img_ppi->fill(Qt::transparent);
    isSelfRotation = false;
    isProcessing = true;
    isEncoderAzi  =false;
    isManualTune = false;
    isVtorih = true;
    rgs_auto = false;
    doubleFilter = false;
    rotation_per_min = 0;
    bo_bang_0 = false;
    data_export = false;
    xl_dopler = false;
    is_normalize_signal = false;
    filter2of3 = false;
    is_do_bup_song = false;
    clk_adc = 0;
    noiseAverage = 30;
    noiseVar = 0;
    krain_auto = 0.4;
    kgain_auto  = 2.1;
    ksea_auto = 0;
    kgain = 1;
    krain  = ksea = 0;
    brightness = 1;
    avtodetect = true;
    isClkAdcChanged = true;
    isSled = false;
    init_time = 3;
    dataOver = max_s_m_200;
    curAzir = 0;
    arcMaxAzi = 0;
    arcMinAzi = 0;
    isSharpEye = false;
    rotDir=Right;
    raw_map_init();
    raw_map_init_zoom();
    setTrueN(0);
    setScalePPI(1);
    resetData();
    setScaleZoom(4);
    //setZoomRectAR(0,0);
}
C_radar_data::~C_radar_data()
{
    delete img_ppi;
    //if(pFile)fclose(pFile);
    //    if(pScr_map)
    //    {
    //        delete[] pScr_map;
    //    }
}

double C_radar_data::getArcMaxAziRad() const
{
    double result = (trueN+(double)arcMaxAzi/(double)MAX_AZIR*PI_NHAN2);
    if(result>PI_NHAN2)result-=PI_NHAN2;
    return ( result);
}
double C_radar_data::getArcMinAziRad() const
{
    double result = (trueN+(double)arcMinAzi/(double)MAX_AZIR*PI_NHAN2);
    if(result>PI_NHAN2)result-=PI_NHAN2;
    return (result );
}

float C_radar_data::getHsTapAverage() const
{
    return mHsTapAverage;
}

void C_radar_data::setHsTapMax(float hsTapMax)
{
    mHsTapMax = hsTapMax;
}

double C_radar_data::getSelfRotationAzi() const
{
    return selfRotationAzi;
}

void C_radar_data::setSelfRotationAzi(int value)
{
    selfRotationAzi = value;

}
double C_radar_data::getCurAziRad() const
{
    if(isEncoderAzi)
    {
        double result = (trueN+(double)selfRotationAzi/(double)MAX_AZIR*PI_NHAN2);
        if(result>PI_NHAN2)result-=PI_NHAN2;

        return ( result);
    }
    double result = (trueN+(double)curAzir/(double)MAX_AZIR*PI_NHAN2);
    if(result>PI_NHAN2)result-=PI_NHAN2;
    return ( result);
}
bool C_radar_data::getIsVtorih() const
{
    return isVtorih;
}

void C_radar_data::setIsVtorih(bool value)
{
    isVtorih = value;
}
void C_radar_data::setProcessing(bool onOff)
{
    if(onOff)
    {

        //initData(true);
        isProcessing = true;
        printf("\nSecondary processing mode - on.");
    }
    else
    {
        isProcessing = false;
        printf("\nSecondary processing mode - off.");
    }
}

bool C_radar_data::checkFeedback(unsigned char *command)
{
    for (short i=0;i<8;i++)
    {if(command[i]!=command_feedback[i])return false;}
    memset(&command_feedback[0],0,8);
    return true;
}


void C_radar_data::drawSgn(short azi_draw, short r_pos)
{

    unsigned char value = data_mem.display_ray[r_pos][0];
    unsigned char dopler    = data_mem.display_ray[r_pos][1];
    unsigned char sled     = data_mem.display_ray[r_pos][2];
    short px = data_mem.x[azi_draw][r_pos];
    short py = data_mem.y[azi_draw][r_pos];
    if(px<=0||py<=0)return;
    short pSize = 1;

    //if(pSize>2)pSize = 2;
    if((px<pSize)||(py<pSize)||(px>=img_ppi->width()-pSize)||(py>=img_ppi->height()-pSize))return;
    for(short x = -pSize;x <= pSize;x++)
    {
        for(short y = -pSize;y <= pSize;y++)
        {
            double k ;
            switch(short(x*x+y*y))
            {

            case 0:
                k=1;
                break;
            case 1:
                if(data_mem.display_mask[px+x][py+y])k=0.85f;
                else k=1;
                break;
            case 2:
                if(data_mem.display_mask[px+x][py+y])k=0.5;
                else k=1;
                break;
            default:
                if(data_mem.display_mask[px+x][py+y])continue;

                k=0.2;
                break;
            }

            unsigned char pvalue = value*k;

            if( data_mem.display_mask[px+x][py+y] <= pvalue)
            {
                data_mem.display_mask[px+x][py+y] = pvalue;
                img_ppi->setPixel(px+x,py+y,getColor(pvalue,dopler,sled));//todo: set color table
            }
            else if(!value){img_ppi->setPixel(px+x,py+y,0x80ff8000);continue;}// may hoi

        }
    }



}

void C_radar_data::drawBlackAzi(short azi_draw)
{
    for (short r_pos = 1;r_pos < DISPLAY_RES;r_pos++)
    {

        short px = data_mem.x[azi_draw][r_pos];
        short py = data_mem.y[azi_draw][r_pos];
        if(px<0||py<0)continue;
        short pSize = 1;

        if((px<pSize)||(py<pSize)||(px>=img_ppi->width()-pSize)||(py>=img_ppi->height()-pSize))continue;

        for(short x = -pSize;x <= pSize;x++)
        {
            for(short y = -pSize;y <= pSize;y++)
            {

                data_mem.display_mask[px+x][py+y] = 0;
            }
        }
    }
    for (short r_pos = 1;r_pos<DISPLAY_RES_ZOOM;r_pos++)
    {

        short px = data_mem.xzoom[azi_draw][r_pos];
        short py = data_mem.yzoom[azi_draw][r_pos];
        if(px<0||py<0)continue;
        short pSize = 1;
        if((px<pSize)||(py<pSize)||(px>=img_zoom_ppi->width()-pSize)||(py>=img_zoom_ppi->height()-pSize))continue;

        for(short x = -pSize;x <= pSize;x++)
        {
            for(short y = -pSize;y <= pSize;y++)
            {

                data_mem.display_mask_zoom[px+x][py+y] = 0;
            }
        }
    }
}
void C_radar_data::drawAzi(short azi)
{


    if(rotDir==Left)
    {
        //reset the display masks
        short prev_azi = azi - 20;
        if(prev_azi<0)prev_azi += MAX_AZIR;
        drawBlackAzi(prev_azi*3+2);
        drawBlackAzi(prev_azi*3+1);
        drawBlackAzi(prev_azi*3);
        //reset the drawing ray
        memset(&data_mem.display_ray[0][0],0,DISPLAY_RES*3);
        //memset(&signal_map.display_zoom[0][0],0,DISPLAY_RES_ZOOM*3);
        //set data to the drawing ray

    }
    else
    {
        //reset the display masks
        short prev_azi = azi + 20;
        if(prev_azi>=MAX_AZIR)prev_azi -= MAX_AZIR;
        drawBlackAzi(prev_azi*3);
        drawBlackAzi(prev_azi*3+1);
        drawBlackAzi(prev_azi*3+2);
        //reset the drawing ray
        memset(&data_mem.display_ray[0][0],0,DISPLAY_RES*3);

    }


    unsigned short  lastDisplayPos =0;
    for (short r_pos = 0;r_pos<range_max-1;r_pos++)
    {
        unsigned short value = data_mem.level_disp[azi][r_pos];
        unsigned short dopler = data_mem.dopler[azi][r_pos];
        if(DrawZoomAR(azi,r_pos,
                      value,
                      dopler,
                      data_mem.sled[azi][r_pos]))
        {
            value+=50;
            if(value>255)value=255;
        }
        //zoom to view scale
        short display_pos = r_pos*scale_ppi;
        short display_pos_next = (r_pos+1)*scale_ppi;
        for(;;)
        {
            if(display_pos>=DISPLAY_RES)break;
            if(data_mem.display_ray[display_pos][0]<value)
            {
                data_mem.display_ray[display_pos][0] = value;
                data_mem.display_ray[display_pos][1] = dopler;

            }
            if(data_mem.display_ray[display_pos][2] < data_mem.sled[azi][r_pos])
            {
                data_mem.display_ray[display_pos][2] = data_mem.sled[azi][r_pos];
            }
            display_pos++;
            if(display_pos>=display_pos_next)break;
        }
        if(lastDisplayPos<display_pos_next)lastDisplayPos = display_pos_next;
        //zoom to zoom scale !
        short display_pos_zoom = r_pos*scale_zoom_ppi;
        short display_pos_next_zoom  = (r_pos+1)*scale_zoom_ppi;
        for(;;)
        {
            if(display_pos_zoom>=DISPLAY_RES_ZOOM)break;
            if(true)
            {
                data_mem.display_ray_zoom[display_pos_zoom][0] += (value-data_mem.display_ray_zoom[display_pos_zoom][0])/1.4;
                data_mem.display_ray_zoom[display_pos_zoom][1] = dopler;

            }
            if(true)//signal_map.display_zoom[display_pos_zoom][2] < signal_map.sled[azi][r_pos])
            {
                data_mem.display_ray_zoom[display_pos_zoom][2] = data_mem.sled[azi][r_pos];
            }
            display_pos_zoom++;
            if(display_pos_zoom>=display_pos_next_zoom)break;
        }

    }
    if (lastDisplayPos<DISPLAY_RES)
    {
        for(;lastDisplayPos<DISPLAY_RES;lastDisplayPos++)
        {

            data_mem.display_ray[lastDisplayPos][0] = 0;
            data_mem.display_ray[lastDisplayPos][1] = 0;
            data_mem.display_ray[lastDisplayPos][2] = 0;
        }
    }
    //smooothing the image
    float k  = scale_ppi/2;
    //printf("\nviewScale:%f",viewScale);
    for(short display_pos = 1;display_pos<DISPLAY_RES_ZOOM; display_pos++)
    {
        data_mem.display_ray_zoom[display_pos][0] = data_mem.display_ray_zoom[display_pos-1][0] + ((float)data_mem.display_ray_zoom[display_pos][0]-(float)data_mem.display_ray_zoom[display_pos-1][0])/2;
        //signal_map.display_zoom[display_pos][1] = signal_map.display_zoom[display_pos-1][1] + ((float)signal_map.display_zoom[display_pos][1]-(float)signal_map.display_zoom[display_pos-1][1])/3;
        drawSgnZoom(azi*3,display_pos);
        drawSgnZoom(azi*3+1,display_pos);
        drawSgnZoom(azi*3+2,display_pos);
    }


    if(k<=2)
    {
        for(short display_pos = 1;display_pos<DISPLAY_RES;display_pos++)
        {
            drawSgn(azi*3,display_pos);
            drawSgn(azi*3+1,display_pos);
            drawSgn(azi*3+2,display_pos);
        }

    }
    else
    {
        for(short display_pos = 1;display_pos<DISPLAY_RES;display_pos++)
        {
            data_mem.display_ray[display_pos][0] = data_mem.display_ray[display_pos-1][0] + ((float)data_mem.display_ray[display_pos][0]-(float)data_mem.display_ray[display_pos-1][0])/k;
            //signal_map.display[display_pos][1] = signal_map.display[display_pos-1][1] + ((float)signal_map.display[display_pos][1]-(float)signal_map.display[display_pos-1][1])/k;
            drawSgn(azi*3,display_pos);
            drawSgn(azi*3+1,display_pos);
            drawSgn(azi*3+2,display_pos);

        }

    }
    //drawingDone = true;

}

void  C_radar_data::processSector32azi()
{
    //kiem tra he so tap, phat hien nhieu tich cuc
    mHsTapAverage = mHsTap/32;
    if(mHsTapAverage>mHsTapMax)mJammingDetected = true;
    mHsTap = 0;
    int sumvar = 0;
    int n = 0;
    memset(histogram,0,256);
    int historgram_pos = range_max-50;

    for(short azi=curAzir;n<500;azi--)
    {
        n++;
        if(azi<0)azi+=MAX_AZIR;
        sumvar+= abs(data_mem.level[azi][historgram_pos]-data_mem.level[azi][historgram_pos-5]);
        unsigned char value = data_mem.level[azi][historgram_pos];
        if(value>5&&value<250)
        {
            histogram[value-3]+=1;
            histogram[value-2]+=2;
            histogram[value-1]+=3;
            histogram[value  ]+=1;
            histogram[value+1]+=3;
            histogram[value+2]+=2;
            histogram[value+3]+=1;
        }

    }

    short histogram_max_val=0;
    short histogram_max_pos;
    noiseVar+=(sumvar/float(n)-noiseVar)/2.0f;
    if(noiseVar<5)noiseVar = 5;

    for(short i = 0;i<250;i++)
    {
        if(histogram[i]>histogram_max_val)
        {
            histogram_max_val = histogram[i];
            histogram_max_pos = i;
        }

    }

    noiseAverage += (histogram_max_pos-noiseAverage)/2.0f;

    img_histogram->fill(0);
    for(short i = 0;i<256;i++)
    {
        histogram[i] = histogram[i]*100/histogram_max_val;
        img_histogram->setPixel(i,100-histogram[i],1);
    }
    short thresh = noiseAverage+(short)noiseVar*kgain;
    if(thresh<0)thresh=0;
    for(short j = 99;j>100-histogram[histogram_max_pos];j--)
    {
        img_histogram->setPixel(histogram_max_pos,j,1);
        if(j>50)img_histogram->setPixel(thresh,j,1);
    }
    //printf("\ntrung binh tap:%f, var:%f",noiseAverage,noiseVar);

}
void C_radar_data::SetHeaderLen( short len)
{
    headerLen = len;
}
bool C_radar_data::getDoubleFilter() const
{
    return doubleFilter;
}

void C_radar_data::setDoubleFilter(bool value)
{
    doubleFilter = value;
}
void C_radar_data::decodeData(int azi)
{
    //read spectre
    memcpy((char*)&spectre,(char*)&dataBuff[RADAR_DATA_HEADER_MAX],16);
    img_spectre->fill(0);
    //draw spectre

    for(short i=0;i<16;i++)
    {
        for(short j=255;j>255-spectre[i];j--)
        {
            img_spectre->setPixel(i,j,1);
        }
    }
    // sharpyeye dopler procession

    short i_m  = headerLen;
    short i_s  = i_m + range_max;
    short i_md = i_s + RAD_S_PULSE_RES;
    short i_sd = i_md+ range_max/2;
    for(short r_pos = 0;r_pos<range_max;r_pos++)
    {
        //data_mem.dopler_old[azi][r_pos] = data_mem.dopler[azi][r_pos];
        if(r_pos<RAD_S_PULSE_RES)
        {
            switch (dataOver) {
            case m_only:
                data_mem.level[azi][r_pos] = dataBuff[i_m+r_pos];
                data_mem.dopler[azi][r_pos] = dataBuff[i_md+r_pos/2];
                break;
            case s_m_200:
                if(r_pos<200)
                {
                    data_mem.level[azi][r_pos] = dataBuff[i_s+r_pos];
                    data_mem.dopler[azi][r_pos] = dataBuff[i_sd+r_pos/2];
                }
                else
                {
                    data_mem.level[azi][r_pos] = dataBuff[i_m+r_pos];
                    data_mem.dopler[azi][r_pos] = dataBuff[i_md+r_pos/2];
                }
                break;
            case max_s_m_200:
                if(r_pos<200&&(dataBuff[i_s+r_pos]>dataBuff[i_m+r_pos]))
                {
                    data_mem.level[azi][r_pos]  = dataBuff[i_s  + r_pos];
                    data_mem.dopler[azi][r_pos] = dataBuff[i_sd + r_pos/2];
                }
                else
                {
                    data_mem.level[azi][r_pos] = dataBuff[i_m+r_pos];
                    data_mem.dopler[azi][r_pos] = dataBuff[i_md+r_pos/2];
                }
                break;
            default:
                break;

            }
        }
        else
        {
            data_mem.level[azi][r_pos] = dataBuff[i_m+r_pos];
            data_mem.dopler[azi][r_pos] = dataBuff[i_md+r_pos/2];
        }
        //unzip the dopler data
        if(!(r_pos&0x01))
        {

            data_mem.dopler[azi][r_pos] = 0x0f&data_mem.dopler[azi][r_pos];

        }
        else
        {
            data_mem.dopler[azi][r_pos] = data_mem.dopler[azi][r_pos]>>4;
        }
    }
}
void C_radar_data::ProcessData(unsigned short azi)
{
    short thresh[RAD_M_PULSE_RES];
    if(isSharpEye)
    {
        for(short r_pos=5;r_pos<range_max;r_pos++)
        {

            int a = (int)data_mem.level[azi][r_pos];
            //if(!a) break;
            //printf("Innitial:%d\n",a);
            if(data_mem.dopler[azi][r_pos]==data_mem.dopler[azi][r_pos-1])
            {
                a = a*1.5;

            }
            if(data_mem.dopler[azi][r_pos]==data_mem.dopler[azi][r_pos-2])
            {
                a = a*1.2;
            }
            if(a>255)a=255;
            //printf("Result a:%d\n",a);
            data_mem.level[azi][r_pos] = (unsigned char)a;
            //                data_mem.level[azi][r_pos] +
            //                int k = 0;
            //                for(int i=-4;i<=5;i++)
            //                {
            //                    if(data_mem.dopler[azi][r_pos+i]==data_mem.dopler[azi][r_pos+i+1])k++;
            //                }

        }
    }
    // apply the  threshholding algorithm manualy
    memset(&thresh[0],0,RAD_M_PULSE_RES*2);
    //do bup song
    /*if(is_do_bup_song)
    {
        int r_pos = (double)(he_so_tap_recv[azi]*he_so_tap_recv[azi])/tb_tap_k;
        for(short pos=0;pos<range_max;pos++)
        {
            if(pos==r_pos)
            data_mem.level[azi][pos] =255;
            else
            data_mem.level[azi][pos] = 0;
        }

    }*/
    if(doubleFilter)// bo loc nguong rain 2 chieu
    {
        rainLevel = noiseAverage;
        for(short r_pos=range_max-1;r_pos>=0;r_pos--)
        {
            // RGS threshold
            rainLevel += krain*(data_mem.level[azi][r_pos]-rainLevel);
            if(rainLevel>(noiseAverage+6*noiseVar))rainLevel = noiseAverage + 6*noiseVar;
            thresh[r_pos] = rainLevel + noiseVar*kgain;
        }
    }
    rainLevel = noiseAverage ;

    for(short r_pos=0;r_pos<range_max;r_pos++)
    {
        if(isManualTune&&(!rgs_auto))
        {

            bool cutoff = false;

            rainLevel += krain*(data_mem.level[azi][r_pos]-rainLevel);
            if(rainLevel>(noiseAverage+6*noiseVar))rainLevel = noiseAverage + 6*noiseVar;
            short nthresh = rainLevel + noiseVar*kgain;
            thresh[r_pos] = thresh[r_pos]<nthresh?nthresh:thresh[r_pos];
            //            }

            if(!cutoff)cutoff = (data_mem.level[azi][r_pos]<=thresh[r_pos]);
            if(bo_bang_0)
            {
                if((data_mem.dopler[azi][r_pos]==0)
                        ||(data_mem.dopler[azi][r_pos]==1)
                        ||(data_mem.dopler[azi][r_pos]==15))
                {
                    cutoff = true;
                    //signal_map.hot[azi][r_pos]=0;
                }
            }
            if(xl_dopler)
            {
                //dopler
                if(!cutoff)
                {
                    cutoff = (data_mem.dopler[azi][r_pos]!=data_mem.dopler_old[azi][r_pos]);
                }


            }
            //update hot value
            if(!cutoff)
            {
                if(data_mem.hot_disp[azi][r_pos]<3)
                {
                    data_mem.hot_disp[azi][r_pos]++;
                }
            }
            else
            {
                if(data_mem.hot_disp[azi][r_pos])
                {
                    data_mem.hot_disp[azi][r_pos]--;
                }

            }
            if(filter2of3)cutoff = data_mem.hot_disp[azi][r_pos]<2;
            //            if(cutoff)
            //            {
            //                data_mem.sled[azi][r_pos]-= (data_mem.sled[azi][r_pos])/100.0f;

            //            }else
            //            {
            //                data_mem.sled[azi][r_pos] += (255 - data_mem.sled[azi][r_pos])/10.0f;
            //            }
            data_mem.level_disp[azi][r_pos] = cutoff?0:data_mem.level[azi][r_pos];

        }
        else
        {
            if(is_normalize_signal)
            {

                if(!r_pos)
                {
                    rainLevel = noiseAverage;
                }
                else
                    rainLevel += 0.2*(data_mem.level[azi][r_pos]-rainLevel);
                if(rainLevel>(noiseAverage+16*noiseVar))rainLevel = noiseAverage + 16*noiseVar;
                thresh[r_pos] = rainLevel - noiseVar*4;
                if(data_mem.level[azi][r_pos]>(thresh[r_pos]))
                    data_mem.level_disp[azi][r_pos] = (data_mem.level[azi][r_pos] - (thresh[r_pos]))*(thresh[r_pos]/255.0f+1.0f);
                else data_mem.level_disp[azi][r_pos] = 1;
            }
            else
            {
                data_mem.level_disp[azi][r_pos]=data_mem.level[azi][r_pos];
            }


        }

        //
    }
    //auto threshold

    memset(&thresh[0],0,RAD_M_PULSE_RES*2);

    if(doubleFilter)
    {
        rainLevel = noiseAverage;
        for(short r_pos=range_max-1;r_pos>0;r_pos--)
        {
            // RGS threshold
            rainLevel += krain_auto*(data_mem.level[azi][r_pos]-rainLevel);
            if(rainLevel>(noiseAverage+9*noiseVar))rainLevel = noiseAverage + 9*noiseVar;
            thresh[r_pos] = rainLevel + noiseVar*kgain_auto;//kgain = 4
        }
    }
    rainLevel = noiseAverage ;

    for(short r_pos=0;r_pos<range_max;r_pos++)
    {
        // RGS threshold
        rainLevel += krain_auto*(data_mem.level[azi][r_pos]-rainLevel);
        if(rainLevel>(noiseAverage+9*noiseVar))rainLevel = noiseAverage + 9*noiseVar;
        short nthresh = rainLevel + noiseVar*kgain_auto;//kgain = 4
        thresh[r_pos] = thresh[r_pos]<nthresh?nthresh:thresh[r_pos];
        bool cutoff = data_mem.level[azi][r_pos]<thresh[r_pos];
        //            short dvar;
        //            dvar = abs(data_mem.dopler[azi][r_pos]-data_mem.dopler_old[azi][r_pos]);
        //            if(dvar>8)dvar = 16-dvar;
        if(cutoff)//&&(dvar<2))
        {
            if(data_mem.hot[azi][r_pos])
            {
                data_mem.hot[azi][r_pos]--;
            }

        }
        else
        {
            if(data_mem.hot[azi][r_pos]<3)
            {
                data_mem.hot[azi][r_pos]++;
            }
        }
        if(this->filter2of3)
        {
            if(!cutoff)
            {
                if((data_mem.hot[azi][r_pos+1])<2
                        &&data_mem.hot[azi][r_pos-1]<2
                        &&data_mem.hot[azi][r_pos]<2)
                {
                    cutoff = true;
                }
            }
        }
        if(cutoff)
        {
            if(isManualTune&&rgs_auto)data_mem.level_disp[azi][r_pos]= 1;
            data_mem.sled[azi][r_pos] -= (data_mem.sled[azi][r_pos])/20.0f;
        }
        else
        {
            data_mem.sled[azi][r_pos] = 255;
        }
        if(r_pos>RANGE_MIN)
        {
            data_mem.detect[azi][r_pos] = !cutoff;
            if(data_mem.detect[azi][r_pos]&&(!init_time))
            {
                procPix(azi,r_pos);
                //if(data_mem.terrain[azi][r_pos]<TERRAIN_MAX)data_mem.terrain[azi][r_pos]++;
            }
            else
            {
                //if(data_mem.terrain[azi][r_pos])data_mem.terrain[azi][r_pos]--;
            }

        }
    }


    memcpy(&data_mem.dopler_old[azi][0],&data_mem.dopler[azi][0],range_max);
    if(data_export)
    {
        if(!exp_file)
        {
            exp_file = new QFile();
            exp_file->setFileName("export_data_dopler.dat");
            exp_file->open(QIODevice::WriteOnly);
        }
        if(azi==550)
        {
            exp_file->write((char*)&data_mem.dopler_old[azi][0],RAD_M_PULSE_RES);
            memset((char*)&data_mem.level_disp[azi][0],0xff,RAD_M_PULSE_RES);
        }
    }
    else
    {
        if(exp_file)
        {
            exp_file->close();
            delete exp_file;
            exp_file = NULL;
        }
    }
    return ;
}
void C_radar_data::SelfRotationOn( double dazi)
{
    isSelfRotation = true;
    printf("\nself rotation");
    SelfRotationReset();
    selfRotationDazi = dazi;
}
void C_radar_data::SelfRotationReset()
{
    //selfRotationAzi = 0;
    selfRotationAzi = 0;
}
void C_radar_data::SelfRotationOff()
{
    isSelfRotation = false;
}

int C_radar_data::getNewAzi()
{
    int newAzi;
    if(isEncoderAzi)
    {

        selfRotationAzi-=selfRotationDazi;
        if(selfRotationAzi>=MAX_AZIR)selfRotationAzi -= MAX_AZIR;
        if(selfRotationAzi<0)selfRotationAzi += MAX_AZIR;
        newAzi = selfRotationAzi;
    }
    else if(isSelfRotation)
    {
        selfRotationAzi-=selfRotationDazi;
        if(selfRotationAzi>=MAX_AZIR)selfRotationAzi = 0;
        if(selfRotationAzi<0)selfRotationAzi += MAX_AZIR;
        newAzi = selfRotationAzi;
    }
    else
    {
        newAzi = (0xfff & (dataBuff[4] << 8 | dataBuff[5]))>>1;
    }
    if(newAzi>MAX_AZIR||newAzi<0)
        return 0;
    return newAzi;
}
void C_radar_data::ProcessDataFrame()
{
    int newAzi = getNewAzi();

    int leftAzi = curAzir-1;if(leftAzi<0)leftAzi+=MAX_AZIR;
    int rightAzi = curAzir +1; if(rightAzi>=MAX_AZIR)rightAzi-=MAX_AZIR;
    if(newAzi == leftAzi )
    {
        if(rotDir==Right)
        {
            rotDir  = Left;
            arcMaxAzi = curAzir;
            init_time =2;

        }
    }
    else if(newAzi == rightAzi) {
        if(rotDir==Left)
        {
            rotDir = Right;
            arcMinAzi = curAzir;
            init_time =2;
        }
    }
    else if(newAzi ==curAzir)
    {
        //printf("\ncurAzir:%d",curAzir);
        return;
    }
    else
    {
        //clearPPI();
    }
    rotation_speed = dataBuff[1];
    overload = dataBuff[4]>>7;
    unsigned char n_clk_adc = (dataBuff[4]&(0xe0))>>5;
    if(clk_adc != n_clk_adc)
    {
        // clock adc

        clk_adc = n_clk_adc;
        isClkAdcChanged = true;
        resetData();
        clearPPI();

    }
    moduleVal = dataBuff[3];//
    tempType = dataBuff[2]&0x0f;
    if(tempType>4)printf("Wrong temperature\n");
    sn_stat = dataBuff[14]<<8|dataBuff[15];
    chu_ky = dataBuff[16]<<8|dataBuff[17];
    // he so tap may thu (phat hien nhieu tich cuc)

    //if(newHsTap>mHsTap*2)if(!isClkAdcChanged)mJammingDetected = true;
    mHsTap += ((dataBuff[18]<<8)|dataBuff[19]);
    //kiem tra lenh phan hoi
    memcpy(command_feedback,&dataBuff[RADAR_COMMAND_FEEDBACK],8);
    //memcpy(noise_level,&dataBuff[RADAR_COMMAND_FEEDBACK+8],8);
    curAzir = newAzi;
    aziToProcess.push(curAzir);
    decodeData(curAzir);
    if(!((unsigned char)(curAzir<<3))){
        procTracks(curAzir);
        processSector32azi();
    }
    if(curAzir==0)
    {
        if(cur_timeMSecs)
        {
            qint64 newtime = QDateTime::currentMSecsSinceEpoch();
            qint64 dtime = newtime - cur_timeMSecs;
            if(dtime<20000)
            {
                if(!rot_period_sec)
                {
                    rot_period_sec = (dtime/1000.0);
                }
                else
                {
                    rot_period_sec += ((dtime/1000.0)-rot_period_sec);
                }
                rotation_per_min = 60.0/rot_period_sec;
            }
            cur_timeMSecs = newtime;
        }
        else
        {
            cur_timeMSecs = QDateTime::currentMSecsSinceEpoch();
        }

        if(init_time)init_time--;
    }
}
short drawnazi = 0;
void C_radar_data::clearPPI()
{
    img_ppi->fill(0);
}
void C_radar_data::UpdateData()
{
    while(aziToProcess.size())
    {
        ProcessData(aziToProcess.front());
        drawAzi(aziToProcess.front());
        //if(aziToDraw.size<200)aziToDraw.push(curAzir);
        aziToProcess.pop();
    }

}
void C_radar_data::assembleDataFrame(unsigned char* data,unsigned short dataLen)
{

    if((dataLen<headerLen)){printf("Too short data.1\n");return;}
    char dataId = data[0]&0x0f;
    if(dataId==1)
    {
        //printf("%x-",data[0]);
        curFrameId = (data[0]&0xf0)>>4;
        range_max = (dataLen - headerLen)*4/3 - RAD_S_PULSE_RES;
        //printf("range_max:%d\n",range_max);
        if(range_max < RAD_S_PULSE_RES){printf("Too short data.2\n");return;}
        if(range_max > RAD_M_PULSE_RES){printf("Too long data.3\n");return;}
        memcpy(dataBuff,data,dataLen);
        waitForData = dataLen;
    }
    else if(dataId==2)
    {
        //check if we are waiting for second half data frame
        if(!waitForData){printf("First frame is mising\n");return;}
        //check if frame ID is the one that we are expecting
        short secondFrameId = (data[0]&0xf0)>>4;
        if(curFrameId!=secondFrameId){
            printf("\nWrong data.-%d-%d-%d",secondFrameId,curFrameId,dataLen);
            printf("\nWrong:%x\n",data[0]);
            //return;
        }
        // check if the data size is correct
        if(dataLen!=waitForData){printf("Wrong data.6\n");return;}
        //load data to buffer
        memcpy(dataBuff + waitForData,data + headerLen,dataLen-headerLen);
        //process data
        ProcessDataFrame();
        waitForData = 0;
    }
    else{
        printf("\nWrong data id. ID = %d",dataId);
    }
    //if(!dopler){frameId = data[0]>>4; }else {if(frameId =! (data[0]>>4))return;}//check id of dopler data

    /*
    short azi = 0xfff & (buff[ADDR_AZI_H] << 8 | buff[ADDR_AZI_L]);
    if(curAzir==azi) return GetData();
    curAzir = azi;
    if(curAzir==4095){
        curPeriodIndex++;
        procTracks();
    }
    for(short r = 1; r < 1023; r++)
    {
        short i = (r>>3);
        short j = (r&0x0007);
        if((buff[VIDEO_RAW_LENGTH+i]>>j & 0x1))
        {


            //signal_map.frame[azi].raw_map[r].level = signal_map.frame[azi].raw_map[r].level<<
            //if(signal_map.frame[azi].raw_map[r].level<80)
            signal_map.frame[azi].raw_map[r].displaylevel  = 1;
            signal_map.frame[azi].raw_map[r].level = buff[r];
            signal_map.frame[azi].raw_map[r].vet = float(signal_map.frame[azi].raw_map[r].vet*0.95 + 0.05);//255*0.125;

            procPix(azi,r);
        }
        else
        {
            signal_map.frame[azi].raw_map[r].displaylevel  = 0;
            signal_map.frame[azi].raw_map[r].level = buff[r];
            signal_map.frame[azi].raw_map[r].vet = float(signal_map.frame[azi].raw_map[r].vet*0.95);
            //signal_map.frame[azi].raw_map[r].level = 0;
        }

    }
    delete[] buff;
    return azi;*/

}
void C_radar_data::procPLot(plot_t* mPlot)
{
    if(init_time)return;
    if(mPlot->size>PLOT_MIN_SIZE)
    {
        object_t newobject;
        newobject.isManual = false;
        float ctA = (float)mPlot->sumA/(float)mPlot->size;// /MAX_AZIR*PI_NHAN2+trueN;
        float ctR = (float)mPlot->sumR/(float)mPlot->size;
        if(ctA >= MAX_AZIR)ctA -= MAX_AZIR;
        newobject.size = mPlot->size;
        newobject.azMax = mPlot->lastA;
        newobject.azMin = mPlot->firstA;
        //newobject.rMax = mPlot->maxR;
        //newobject.rMin = mPlot->minR;
        //short dr = mPlot->maxR-mPlot->minR;
        //short da =
        //            if(dr>PLOT_MAX_DR)
        //            {
        //                return;
        //            }
        //        float ctR = ((float)mPlot->sumR/(float)mPlot->size);//(mPlot->maxR+mPlot->minR)/2.0f;
        if(ctA<0||ctA>MAX_AZIR|| ctR>=RAD_M_PULSE_RES)
        {
            return;
        }
        //check dopler
        //            if(dr>2)
        //            {
        //                if(data_mem.dopler[short(ctA)][short(ctR)]!=data_mem.dopler[short(ctA)][short(ctR+1)])return;
        //            }

        newobject.dopler = mPlot->dopler;
        //newobject.terrain = data_mem.terrain[short(ctA)][short(ctR)];
        newobject.az   = ctA/MAX_AZIR*PI_NHAN2+trueN;
        if(newobject.az>PI_NHAN2)newobject.az-=PI_NHAN2;
        newobject.rg   = ctR;
        newobject.p   = -1;
        /*for(short i = mPlot->minA;i!=mPlot->maxA;i++)
            {
                if(i>=MAX_AZIR)i-=MAX_AZIR;
                for(short j = mPlot->minR;i!=mPlot->maxR;i++)
                {
                    if(data_mem.detect[i][j]&&data_mem.plotIndex[i][j]==data_mem.plotIndex[short(ctA)][short(ctR)])
                    {
                        data_mem.dopler_old[i][j] = newobject.dopler;
                    }
                }
            }*/
        if(!procObjectManual(&newobject))//check existing confirmed tracks
        {
            if(newobject.dopler!=0)
            {
                if(!procObjectAvto(&newobject))
                {
                    if(avtodetect)addTrack(&newobject);
                }
            }
        }



    }


}
void C_radar_data::procTracks(unsigned short curA)
{
    //process all marks
    short pr_curA = curA-1;
    if(pr_curA<0)pr_curA+=MAX_AZIR;
    for(unsigned short i = 0;i<plot_list.size();++i)
    {
        if(plot_list.at(i).size)
        {
            if((plot_list.at(i).lastA!=curA)&&(plot_list.at(i).lastA!=pr_curA))
            {
                procPLot(&plot_list.at(i));
                plot_list.at(i).size =0;
            }

        }
    }

    //proc track
    float azi = (float)curA/MAX_AZIR*PI_NHAN2+trueN;
    for(unsigned short i=0;i<mTrackList.size();i++)
    {
        if(!mTrackList.at(i).state)continue;
        float dA = azi - mTrackList.at(i).estA;
        if(dA>PI) dA-=PI_NHAN2;
        else if(dA<-PI)dA+=PI_NHAN2;
        if(mTrackList.at(i).isProcessed)
        {
            if((abs(dA)<0.35f)&&(mTrackList.at(i).isTracking))//20 deg
            {
                mTrackList.at(i).isProcessed = false;
            }
            if(!mTrackList.at(i).isTracking)
            {
                if(abs(dA)>0.35f)//20 deg
                {
                    mTrackList.at(i).isProcessed = true;
                    mTrackList.at(i).update();
                }
            }
        }
        else
        {
            if(abs(dA)>0.35f)//20 deg
            {
                mTrackList.at(i).isProcessed = true;
                mTrackList.at(i).update();
            }
        }
    }


}
void C_radar_data::kmxyToPolarDeg(double x, double y, double *azi, double *range)
{
    if(!y)
    {
        *azi = x>0? PI_CHIA2:(PI_NHAN2-PI_CHIA2);
        *azi = *azi*DEG_RAD;
        *range = abs(x);
    }
    else
    {
        *azi = atanf(x/y);
        if(y<0)*azi+=PI;
        if(*azi<0)*azi += PI_NHAN2;
        *range = sqrt(x*x+y*y);
        *azi = *azi*DEG_RAD;
    }

}
void C_radar_data::addTrackManual(double x,double y)
{
    float azi,range;
    if(!y)
    {
        azi = x>0? PI_CHIA2:(PI_NHAN2-PI_CHIA2);
        range = abs(x);
    }
    else
    {
         azi = atanf(x/y);//tinh azi,range
        if(y<0)azi+=PI;
        if(azi<0)azi += PI_NHAN2;
         range = sqrt(x*x+y*y);
    }
    object_t newobj;
    newobj.az = azi;
    newobj.rg = range;
    newobj.dopler = 17;
    newobj.isManual= true;
    newobj.x = x;
    newobj.y = y;
    //
    bool newtrack=true;
    short trackId = -1;
    ushort max_length = 0;
    for(unsigned short i=0;i<mTrackList.size();i++)
    {
        if(mTrackList.at(i).state>5)
        {
            if(mTrackList.at(i).checkProb(&newobj)){
                if(max_length<mTrackList.at(i).object_list.size())
                {
                    max_length = mTrackList.at(i).object_list.size();
                    trackId = i;
                    newtrack = false;
                }
            }
        }
    }
    if(newtrack)
    {
        addTrack( &newobj);
        //printf("newtrack ");
    }
    else
    {
        mTrackList.at(trackId).suspect_list.push_back(newobj);
    }


}
void C_radar_data::addTrack(object_t* mObject)
{
    //add new track
    //printf("new track \n");
    for(unsigned short i=0;i<mTrackList.size();i++)
    {
        if(!mTrackList.at(i).state)
        {
            mTrackList.at(i).init(mObject);
            return;
        }
    }
    if(mTrackList.size()<500)
    {
        track_t newTrack;
        newTrack.init(mObject);
        mTrackList.push_back(newTrack);
    }
}
void C_radar_data::deleteTrack(ushort trackNum)
{
    if(mTrackList.size()>trackNum)
    {
        mTrackList[trackNum].state = 0;
        if(mTrackList[trackNum].object_list.size())mTrackList[trackNum].object_list.clear();
        if(mTrackList[trackNum].suspect_list.size())mTrackList[trackNum].suspect_list.clear();
    }
}

void C_radar_data::drawRamp()
{
    img_RAmp->fill(Qt::black);
    for (short r_pos = 0;r_pos<RAD_M_PULSE_RES;r_pos++)
    {
        unsigned char value = data_mem.level[curAzir][r_pos];
        char dopler = data_mem.dopler[curAzir][r_pos];

        uint color ;
        if(dopler==0)
        {
            color = 0xffff00;
        }else
        {
            char dDopler = dopler-1;
            if(dDopler>7)dDopler = 15-dDopler;
            color = 0x00ff00 | ((dDopler<<5));
        }
        color = color|(0xff000000);

        if(((r_pos%100)==0)||(((r_pos+1)%100)==0)||(((r_pos-1)%100)==0))
        {
            for(short i=0;i<128;i++)
            {
                img_RAmp->setPixel(r_pos,i,0xffffffff);
            }
        }
        for(short i=255;i>255 - value;i--)
        {
            img_RAmp->setPixel(r_pos,i,color);
        }
    }

}

void C_radar_data::drawRamp(double azi)
{
    img_RAmp->fill(Qt::black);
    //newobject.az   = ctA/MAX_AZIR*PI_NHAN2+trueN;
    azi/=DEG_RAD;
    azi-=trueN;
    if(azi<0)azi+=PI_NHAN2;
    int az = azi/PI_NHAN2*MAX_AZIR;
    for (short r_pos = 0;r_pos<RAD_M_PULSE_RES;r_pos++)
    {
        unsigned char value = data_mem.level[az][r_pos];
        char dopler = data_mem.dopler[az][r_pos];

        uint color ;
        if(dopler==0)
        {
            color = 0xffff00;
        }else
        {
            char dDopler = dopler-1;
            if(dDopler>7)dDopler = 15-dDopler;
            color = 0x00ff00 | ((dDopler<<5));
        }
        color = color|(0xff000000);

        if(((r_pos%100)==0)||(((r_pos+1)%100)==0)||(((r_pos-1)%100)==0))
        {
            for(short i=0;i<128;i++)
            {
                img_RAmp->setPixel(r_pos,i,0xffffffff);
            }
        }
        for(short i=255;i>255 - value;i--)
        {
            img_RAmp->setPixel(r_pos,i,color);
        }

    }

}
bool C_radar_data::procObjectAvto(object_t* pObject)
{
    bool newtrack = true;
    short trackId = -1;
    short max_length = 0;
    for(unsigned short i=0;i<mTrackList.size();i++)
    {
        if(mTrackList.at(i).isManual)continue;
        if(mTrackList.at(i).state&&(! mTrackList.at(i).isProcessed))
        {
            if(mTrackList.at(i).checkProb(pObject)){
                if(max_length<mTrackList.at(i).object_list.size())
                {
                    max_length=mTrackList.at(i).object_list.size();
                    trackId = i;
                    newtrack = false;
                }
            }
        }
    }
    if(!newtrack)
    {
        //add object to a processing track
        mTrackList.at(trackId).suspect_list.push_back(*pObject);
        return true;
    }
    else
    {

        return false;
    }


}
bool C_radar_data::procObjectManual(object_t* pObject)// !!!
{

    short trackId = -1;
    ushort max_length = 0;
    for(unsigned short i=0;i<mTrackList.size();i++)
    {
        if(!mTrackList.at(i).isManual)continue;
        if(mTrackList.at(i).state&&(! mTrackList.at(i).isProcessed))
        {
            if(mTrackList.at(i).checkProb(pObject)){
                if(max_length<mTrackList.at(i).object_list.size())
                {
                    max_length = mTrackList.at(i).object_list.size();
                    trackId = i;
                }
            }
        }
    }
    if(trackId>=0)
    {
        //add object to a processing track
        mTrackList.at(trackId).suspect_list.push_back(*pObject);
        return true;
    }
    else return false;

}
void C_radar_data::procPix(short proc_azi,short range)//_______signal detected, check 4 last neighbour points for nearby mark_______________//
{
    if(init_time)return;

    short pr_proc_azi = proc_azi-1;
    if(pr_proc_azi<0)pr_proc_azi+=MAX_AZIR;
    short plotIndex =-1;
    char dopler_0 = data_mem.dopler[proc_azi][range];
    char dopler_1 = dopler_0 +1;
    if(dopler_1>15)dopler_1-=16;
    char dopler_2 = dopler_0 - 1;
    if(dopler_2<0)dopler_2+=16;
    if(data_mem.detect[pr_proc_azi][range]
            &&(data_mem.dopler[pr_proc_azi][range]==dopler_0
               ||data_mem.dopler[pr_proc_azi][range]==dopler_1
               ||data_mem.dopler[pr_proc_azi][range]==dopler_2)
            )
    {
        plotIndex = data_mem.plotIndex[pr_proc_azi][range];

    }else if(data_mem.detect[proc_azi][range-1]
             &&(data_mem.dopler[proc_azi][range-1]==dopler_0
                ||data_mem.dopler[proc_azi][range-1]==dopler_1
                ||data_mem.dopler[proc_azi][range-1]==dopler_2)
             )
    {
        plotIndex = data_mem.plotIndex[proc_azi][range-1];
    }
    else if(data_mem.detect[pr_proc_azi][range-1]
            &&(data_mem.dopler[pr_proc_azi][range-1]==dopler_0
               ||data_mem.dopler[pr_proc_azi][range-1]==dopler_1
               ||data_mem.dopler[pr_proc_azi][range-1]==dopler_2)
            )
    {
        plotIndex = data_mem.plotIndex[pr_proc_azi][range-1];

    }
    else if(data_mem.detect[pr_proc_azi][range+1]
            &&(data_mem.dopler[pr_proc_azi][range+1]==dopler_0
               ||data_mem.dopler[pr_proc_azi][range+1]==dopler_1
               ||data_mem.dopler[pr_proc_azi][range+1]==dopler_2)
            )
    {
        plotIndex = data_mem.plotIndex[pr_proc_azi][range+1];
    }
    if(plotIndex!=-1)// add to existing marker
    {
        if(!((plotIndex<plot_list.size())&&(plot_list.at(plotIndex).size)))
        {
            return;
        }
        data_mem.plotIndex[proc_azi][range] = plotIndex;
        plot_list.at(plotIndex).size++;
        if(proc_azi<plot_list.at(plotIndex).firstA){
            plot_list.at(plotIndex).sumA    +=  proc_azi + MAX_AZIR;
            plot_list.at(plotIndex).lastA    =  proc_azi ;
        }else
        {
            plot_list.at(plotIndex).sumA    +=  proc_azi;
            plot_list.at(plotIndex).lastA    =  proc_azi;
        }
        if(plot_list.at(plotIndex).maxR<range)plot_list.at(plotIndex).maxR=range;
        if(plot_list.at(plotIndex).minR>range)plot_list.at(plotIndex).minR=range;
        plot_list.at(plotIndex).sumR    +=  range;
        //plot_list.at(plotIndex).sumTer  +=  data_mem.terrain[proc_azi][range];
        // get max dopler and max level of this plot
        if(plot_list.at(plotIndex).maxLevel<data_mem.level[proc_azi][range])
        {
            plot_list.at(plotIndex).maxLevel = data_mem.level[proc_azi][range];
            plot_list.at(plotIndex).dopler = data_mem.dopler[proc_azi][range];
        }
    }
    else//_________new plot found_____________//
    {

        plot_t         new_plot;
        new_plot.lastA =  new_plot.firstA  = proc_azi;
        //new_plot.ctA = proc_azi;
        //new_plot.ctR = range;
        new_plot.maxLevel = data_mem.level[proc_azi][range];
        new_plot.dopler = data_mem.dopler[proc_azi][range];
        //new_mark.minR = new_mark.maxR = range;
        new_plot.size =  1;

        //.sumTer = data_mem.terrain[proc_azi][range];
        new_plot.sumA =  proc_azi;
        new_plot.sumR =  range;
        new_plot.maxR = range;
        new_plot.minR = range;
        bool listFull = true;

        for(unsigned short i = 0;i<plot_list.size();++i)
        {
            //  overwrite
            if(!plot_list.at(i).size)
            {
                data_mem.plotIndex[proc_azi][range] =  i;
                plot_list.at(i).sumA=0;
                plot_list.at(i).sumR=0;
                plot_list.at(i) = new_plot;
                listFull = false;
                break;
            }
        }
        if(listFull)
        {
            plot_list.push_back(new_plot);
            data_mem.plotIndex[proc_azi][range]  = plot_list.size()-1;
        }

    }

}
/*void C_radar_data::polarToSnXY(short *xsn, short *ysn, short azi, short range)
{
    *xsn = signal_map.frame[azi].raw_map[range].x;
    *ysn = signal_map.frame[azi].raw_map[range].y;
}
//static short ctX=0,ctY=0;
//static float dr = 0;
*/
void C_radar_data::polarToXY(float *x, float *y, float azi, float range)
{

    *x = ((sinf(azi)))*range;
    *y = ((cosf(azi)))*range;
}

float C_radar_data::getNoiseAverage() const
{
    return noiseAverage;
}

void C_radar_data::setNoiseAverage(float value)
{
    noiseAverage = value;
}

bool C_radar_data::getIsSharpEye() const
{
    return isSharpEye;
}

void C_radar_data::setIsSharpEye(bool value)
{
    isSharpEye = value;
}
short zoomXmax,zoomYmax,zoomXmin,zoomYmin;
short zoomCenterX=DISPLAY_RES,zoomCenterY=DISPLAY_RES;
void C_radar_data::setZoomRectXY(float ctx, float cty)
{
    zoomXmax = ctx*4.0/scale_ppi+ZOOM_SIZE/2;
    zoomYmax = cty*4.0/scale_ppi+ZOOM_SIZE/2;
    zoomXmin = ctx*4.0/scale_ppi-ZOOM_SIZE/2;
    zoomYmin = cty*4.0/scale_ppi-ZOOM_SIZE/2;
    raw_map_init_zoom();
}


int C_radar_data::getHsTapAverage(){

    //mHsTap += ((he_so_tap_recv[curAzir])-mHsTap)/5.0;
    return int(mHsTapAverage);
}

void C_radar_data::setTb_tap_k(double value)
{
    tb_tap_k = value;
    if(tb_tap_k<=0)tb_tap_k=1;
}
void C_radar_data::setZoomRectAR(float ctx, float cty,double sizeKM,double sizeDeg)// unit km
{
    double cta,ctr = sqrt(ctx*ctx+cty*cty);
    if(cty==0)
    {
        if(ctx>0)cta = PI_CHIA2;
        else cta = -PI_CHIA2;
    }
    else cta = atan(ctx/cty)-trueN;
    if(cty<0)cta+=PI;
    if(cta<0)cta += PI_NHAN2;
    if(cta>PI_NHAN2)cta-=PI_NHAN2;
    cta=cta/PI_NHAN2*MAX_AZIR;
    ctr/=sn_scale;
    zoom_ar_size_a = MAX_AZIR/360.0*sizeDeg;
    zoom_ar_size_r = sizeKM/sn_scale;
    zoom_ar_a0 = cta-zoom_ar_size_a/2.0;
    zoom_ar_a1 = zoom_ar_a0+zoom_ar_size_a;
    //if(zoom_ar_a0<0)zoom_ar_a0+=MAX_AZIR;
    zoom_ar_r0 = ctr-zoom_ar_size_r/2.0;
    if(zoom_ar_r0 <0)zoom_ar_r0=0;
    zoom_ar_r1 = zoom_ar_r0+zoom_ar_size_r;
    img_zoom_ar = new QImage(zoom_ar_size_r+1,zoom_ar_size_a+1,QImage::Format_ARGB32);
    //img_zoom_ar->// toto:resize
    //drawZoomAR(a0,r0);
    
}
bool C_radar_data::DrawZoomAR(int a,int r,short val,short dopler,short sled)
{
    //return true if point is on the edges of the zone
    //if(a<zoom_ar_size_a)a+=MAX_AZIR;
    if(!img_zoom_ar)return false;
    int pa= a-zoom_ar_a0;
    if(pa>=MAX_AZIR)pa-=MAX_AZIR;
    if(pa>zoom_ar_size_a)return false;
    if(pa<0)return false;
    int pr = r-zoom_ar_r0;
    if(pr>zoom_ar_size_r)return false;
    if(pr<0)return false;
    img_zoom_ar->setPixel(pr,pa,getColor(val,dopler,sled));
    if(pa==zoom_ar_size_a)return true;
    if(pr==zoom_ar_size_r)return true;
    if(pa==0)return true;
    if(pr==0)return true;
    return false;

}
void C_radar_data::setAutorgs(bool aut)
{
    if(aut)
    {
        rgs_auto = true;
        krain_auto = 0.3;
        kgain_auto  = 3;
        ksea_auto = 0;
    }else
    {
        rgs_auto = false;
    }

}
void C_radar_data::raw_map_init()
{
    float theta=trueN;
    float dTheta = 2*PI/MAX_AZIR_DRAW;
    for(short azir = 0; azir < MAX_AZIR_DRAW; azir++)
    {
        float cost = cosf(theta);
        float sint = sinf(theta);
        for(short range = 0;range<DISPLAY_RES;range++)
        {
            data_mem.x[azir][range]     =  short(sint*(range+1))+DISPLAY_RES;
            data_mem.y[azir][range]    =  -short(cost*(range+1))+DISPLAY_RES;
            if(data_mem.x[azir][range]<0||data_mem.x[azir][range]>=img_ppi->width()||data_mem.y[azir][range]<0||data_mem.y[azir][range]>=img_ppi->height())
            {
                data_mem.x[azir][range] = 0;
                data_mem.y[azir][range] = 0;
            }
        }
        theta+=dTheta;
    }
}
void C_radar_data::raw_map_init_zoom()
{
    img_zoom_ppi->fill(Qt::black);
    float theta=trueN;
    float dTheta = 2*PI/MAX_AZIR_DRAW;
    for(short azir = 0; azir < MAX_AZIR_DRAW; azir++)
    {

        float cost = cosf(theta);
        float sint = sinf(theta);
        for(short range = 0;range<DISPLAY_RES_ZOOM;range++)
        {
            data_mem.xzoom[azir][range]     =  short(sint*(range+1)) - zoomXmin;
            data_mem.yzoom[azir][range]    =  -short(cost*(range+1)) - zoomYmin;
            if(data_mem.xzoom[azir][range]<0||
                    data_mem.yzoom[azir][range]<0||
                    data_mem.xzoom[azir][range]>ZOOM_SIZE||
                    data_mem.yzoom[azir][range]>ZOOM_SIZE)
            {
                data_mem.xzoom[azir][range] = 0;
                data_mem.yzoom[azir][range] = 0;
            }
        }
        theta += dTheta;
    }
}
void C_radar_data::resetData()
{
    int dataLen = RAD_M_PULSE_RES*MAX_AZIR;
    memset(data_mem.level,      0,dataLen);
    memset(data_mem.dopler,     0,dataLen);
    memset(data_mem.detect,     0,dataLen);
    memset(data_mem.plotIndex,  0,dataLen);
    memset(data_mem.hot,        0,dataLen);
    memset(data_mem.hot_disp,   0,dataLen);
    //memset(data_mem.terrain,    TERRAIN_INIT,dataLen);
    //memset(data_mem.rainLevel,  0,dataLen);
    //noiseAverage = 30;
    resetSled();
    init_time = 3;

}
void C_radar_data::resetSled()
{
    memset(data_mem.sled,0,RAD_M_PULSE_RES*MAX_AZIR);
}
void C_radar_data::setScalePPI(float scale)
{

    switch(clk_adc)
    {
    case 0:
        sn_scale = SIGNAL_SCALE_0;
        break;
    case 1:
        sn_scale = SIGNAL_SCALE_1;//printf("1");
        break;
    case 2:
        sn_scale = SIGNAL_SCALE_2;//printf("2");
        break;
    case 3:
        sn_scale = SIGNAL_SCALE_3;//printf("2");
        break;
    case 4:
        sn_scale = SIGNAL_SCALE_4;//printf("2");
        break;
    case 5:
        sn_scale = SIGNAL_SCALE_5;//printf("2");
        break;
    case 6:
        sn_scale = SIGNAL_SCALE_6;//printf("2");
        break;
    case 7:
        sn_scale = SIGNAL_SCALE_7;//printf("2");
        break;
    default:
        sn_scale = SIGNAL_SCALE_0;
    }
    scale_ppi = sn_scale*scale;
    //updateZoomRect();
}
void C_radar_data::setScaleZoom(float scale)
{

    scale_zoom_ppi = SIGNAL_SCALE_0*scale/scale_ppi;
    //updateZoomRect();
}

//void C_radar_data::drawZoomAR()
//{

//      //memcpy(imgAR->bits(),(unsigned char *)&data_mem.level[0][0],MAX_AZIR*RAD_M_PULSE_RES);
//      QImage* imgAR = new QImage((unsigned char *)&data_mem.level[0][0],RAD_M_PULSE_RES,MAX_AZIR,QImage::Format_Indexed8);
//        imgAR->setColorTable(colorTable);
//      QRect rect(zoom_ar_r0, zoom_ar_a0, 225, 225);
//      *img_zoom_ar = imgAR->copy(rect);//.convertToFormat(QImage::Format_Indexed8,colorTable,Qt::ColorOnly);

//}
void C_radar_data::drawSgnZoom(short azi_draw, short r_pos)
{
    unsigned char value    = data_mem.display_ray_zoom[r_pos][0];
    unsigned char dopler    = data_mem.display_ray_zoom[r_pos][1];
    unsigned char sled     = data_mem.display_ray_zoom[r_pos][2];

    short px = data_mem.xzoom[azi_draw][r_pos];
    short py = data_mem.yzoom[azi_draw][r_pos];
    if(px<=0||py<=0)return;
    short pSize = 1;

    //if(pSize>2)pSize = 2;
    if((px<pSize)||(py<pSize)||(px>=img_zoom_ppi->width()-pSize)||(py>=img_zoom_ppi->height()-pSize))return;
    for(short x = -pSize;x <= pSize;x++)
    {
        for(short y = -pSize;y <= pSize;y++)
        {
            float k ;
            switch(short(x*x+y*y))
            {
            case 0:
                k=1;
                break;
            case 1:
                if(data_mem.display_mask_zoom[px+x][py+y])k=0.95f;
                else k=1;
                break;
            case 2:
                if(data_mem.display_mask_zoom[px+x][py+y])k=0.7f;
                else k=1;

            default:
                if(data_mem.display_mask_zoom[px+x][py+y])continue;
                k=0.7f;
                break;
            }
            //            if(value<6){img_ppi->setPixel(px+x,py+y,0x80ff8000);continue;}
            unsigned char pvalue = value*k;
            if( data_mem.display_mask_zoom[px+x][py+y] <= pvalue)
            {
                data_mem.display_mask_zoom[px+x][py+y] = pvalue;
                img_zoom_ppi->setPixel(px+x,py+y,getColor(pvalue,dopler,sled));
                //DrawZoom(px,py,pvalue);
            }else if(!value){img_zoom_ppi->setPixel(px+x,py+y,0x80ff8000);continue;}

        }
    }

}
uint C_radar_data::getColor(unsigned char pvalue,unsigned char dopler,unsigned char sled)
{
    //if(!pvalue)return 0x0fffff0f;
    unsigned short value = ((unsigned short)pvalue)*brightness;
    if(!isSled)sled = 0;
    else
        if(sled>=8)sled = 0xff; else sled*=32;
    if(value>0xff)
    {
        value = 0xff;
    }
    unsigned char alpha;
    unsigned char red   = 0;
    unsigned char green = 0;
    unsigned char blue  = 0;
    unsigned char gradation = value<<2;
    uint color;
    switch(imgMode)
    {
    case DOPLER_3_COLOR:
        if(pvalue>1)
        {
            if(dopler==0)
            {
                color = 0xffff00;
            }else
            {
                char dDopler = dopler-1;
                if(dDopler>7)dDopler = 15-dDopler;
                color = 0x00ff00 | ((dDopler<<5));
            }
            alpha = value;//0xff - ((0xff - value)*0.75);
            color = color|(alpha<<24);
        }
        else
        {
            color = (sled<<24)|(0xff);
        }
        //

        break;

    case VALUE_ORANGE_BLUE:
        if(pvalue>1)
        {
            alpha = 0xff - ((0xff - value)*0.75);
            //pvalue-=(pvalue/10);
            switch(value>>6)
            {
            case 3:
                red = 0xff;
                green = 0xff - gradation;
                break;
            case 2:
                red = gradation;
                green = 0xff;
                break;
            case 1:
                green = 0xff ;
                blue = 0xff - gradation;
                break;
            case 0:
                green = gradation ;
                blue = 0xff;
                break;
            }
            color = (alpha<<24)|(red<<16)|(green<<8)|blue;
        }
        else
        {
            color = (sled<<24)|(0xff);
        }

        break;
    case VALUE_YELLOW_SHADES:
        if(pvalue>1)
        {
            alpha = value;//0xff - ((0xff - pvalue)*0.75);
            color = (value<<24)|(0xff<<16)|(0xff<<8);
        }
        else
        {
            color = (sled<<24)|(0xff);
        }
        break;
    default:
        break;
    }
    return color;
}
void C_radar_data::resetTrack()
{
    init_time = 3;
    curIdCount = 1;
    mTrackList.clear();
    //    for(unsigned short i=0;i<mTrackList.size();i++)
    //    {
    //        if(mTrackList.at(i).state)
    //        {
    //            mTrackList.at(i).state = 0;
    //        }
    //    }
}

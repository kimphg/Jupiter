#define PI 3.141592654f
#include "C_radar_data.h"
#include <math.h>
#include <QtDebug>

//static period_t                curPeriod;
//static std::queue<period_t>    period_cache;
//static unsigned short cur_mark_index = 0;
C_radar_data::C_radar_data()
{
    img_ppi = new QImage(DISPLAY_RES*2+1,DISPLAY_RES*2+1,QImage::Format_ARGB32);
    img_alpha = new QImage(RAD_M_PULSE_RES,256,QImage::Format_Mono);
    img_zoom_ppi = new QImage(ZOOM_SIZE+1,ZOOM_SIZE+1,QImage::Format_ARGB32);

    isDisplayAlpha = false;
    size_thresh = 4;
    isProcessing = true;
    imgMode = VALUE_ORANGE_BLUE;
    isFilting = false;
    rgs_auto = false;
    bo_bang_0 = false;
    xl_dopler = false;
    cut_thresh = false;
    clk_adc = 0;
    noiseAverage = 0;
    noiseVar = 0;

    kgain = 1;
    krain  = ksea = 0;
    brightness = 1;
    avtodetect = false;
    isClkAdcChanged = true;
    dataOver = max_s_m_200;
    raw_map_init();
    raw_map_init_zoom();
    setTrueN(0);
    setScalePPI(1);
    resetData();
    setScaleZoom(4);
    updateZoomRect(0,0);
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
            float k ;
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
                if(data_mem.display_mask[px+x][py+y])k=0.5f;
                else k=1;

            default:
                if(data_mem.display_mask[px+x][py+y])continue;

                k=0.7f;
                break;
            }
            unsigned char pvalue = value*k;
            if( data_mem.display_mask[px+x][py+y] <= pvalue)
            {
                data_mem.display_mask[px+x][py+y] = pvalue;
                img_ppi->setPixel(px+x,py+y,getColor(pvalue,dopler,sled));

            }
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
    img_alpha->fill(0);
    //reset the display masks
    short prev_azi = azi + 200;
    if(prev_azi>=MAX_AZIR)prev_azi -= MAX_AZIR;
    drawBlackAzi(prev_azi*3);
    drawBlackAzi(prev_azi*3+1);
    drawBlackAzi(prev_azi*3+2);
    //reset the drawing ray
    memset(&data_mem.display_ray[0][0],0,DISPLAY_RES*3);
    //memset(&signal_map.display_zoom[0][0],0,DISPLAY_RES_ZOOM*3);
    //set data to the drawing ray


    unsigned short  lastDisplayPos =0;
    for (short r_pos = 0;r_pos<range_max-1;r_pos++)
    {

            unsigned short value = data_mem.level_disp[azi][r_pos];
            unsigned short dopler = data_mem.dopler[azi][r_pos];

            //xu ly nguong

            //display alpha graph
            if(isDisplayAlpha)
            {
                for(short i=255;i>255 - value;i--)
                {
                    img_alpha->setPixel(r_pos,i,1);
                }
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
            short display_pos_zoom = r_pos*DISPLAY_SCALE_ZOOM;
            short display_pos_next_zoom  = (r_pos+1)*DISPLAY_SCALE_ZOOM;
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
//            drawSgn(azi*2,display_pos);
//            drawSgn(azi*2+1,display_pos);

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
//                        drawSgn(azi*2,display_pos);
//                        drawSgn(azi*2+1,display_pos);
//            if(isDisplayAlpha)
//            {

//                {   //memset(img_alpha->bits()+r_pos*img_alpha->width()/8-value/8,1,value);
//                    for(short i=255;i>255-signal_map.display[display_pos][0];i--)
//                    {
//                        img_alpha->setPixel(display_pos,i,1);
//                    }

//                }
//            }
        }

    }
    //drawingDone = true;

}

 void  C_radar_data::getNoiseLevel()
{
//     float var = signal_map.level_m[azi][range_max-50]-noiseAverage;
//     noiseAverage += var/60.0f;
//     arar += (var-noiseVar)/60;
     printf("\nnoise Level:%f",noiseAverage);
     int sum=0;
     int sumvar = 0;
     int n = 0;
     for(short azi=0;azi<MAX_AZIR;azi++)
     {
         sum += data_mem.level[azi][range_max-50];
         if(data_mem.level[azi][range_max-50])n++;
         sumvar += abs(data_mem.level[azi][range_max-50]-data_mem.level[azi][range_max-100]);
     }
     if(noiseAverage==0)noiseAverage = sum/float(n);else
     {noiseAverage+=(sum/float(n)-noiseAverage)/2;}

     if(noiseVar==0)noiseVar = sumvar/float(n);else
     {noiseVar+=(sumvar/float(n)-noiseVar)/2;}
     printf("\nnoise var:%f",noiseVar);


}

 void  C_radar_data::setDoplerLevel(unsigned short azi,unsigned  short range, unsigned char level)
 {/*
     signal_map.frame[azi].raw_map[range].dopler = level;
*/
 }
#define RADAR_COMMAND_FEEDBACK  6
#define RADAR_DATA_HEADER       22
#define RADAR_DATA_MAX_SIZE     2688

short waitForData = 0;
unsigned char curFrameId;
unsigned char dataBuff[RADAR_DATA_HEADER + RADAR_DATA_MAX_SIZE];
void C_radar_data::ProcessData(unsigned short azi)
{
    short i_m  = RADAR_DATA_HEADER;
    short i_s  = i_m+range_max;
    short i_md = i_s+RAD_S_PULSE_RES;
    short i_sd = i_md+range_max/2;
    for (short r_pos = 0;r_pos<range_max;r_pos++)
    {
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
            if(r_pos&0x01)
            {

                data_mem.dopler[azi][r_pos] = 0x0f&data_mem.dopler[azi][r_pos];

            }
            else
            {
                data_mem.dopler[azi][r_pos] = data_mem.dopler[azi][r_pos]>>4;
            }
    }
    for(short r_pos=0;r_pos<range_max;r_pos++)
    {
        // apply the  threshholding algorithm
        bool cutoff = false;
        short thresh = 0;
        if(avtodetect)
        {
            rgs_auto = true;
            bo_bang_0 = false;
            xl_dopler = false;
        }
        if(rgs_auto)
        {
            //RGS thresh
            if(r_pos<4)
            {
                rainLevel = noiseAverage;
            }
            else
            rainLevel += krain_auto*(data_mem.level[azi][r_pos]-rainLevel);
            if(rainLevel>(noiseAverage+6*noiseVar))rainLevel = noiseAverage + 6*noiseVar;
            thresh = rainLevel + noiseVar*kgain_auto;
        }
        else
        {
            //RGS thresh
            if(r_pos<4)
            {
                rainLevel = noiseAverage;
            }
            else
            rainLevel += krain*(data_mem.level[azi][r_pos]-rainLevel);
            if(rainLevel>(noiseAverage+6*noiseVar))rainLevel = noiseAverage + 6*noiseVar;
            thresh = rainLevel + noiseVar*kgain;
        }

        if(!cutoff)cutoff = (data_mem.level[azi][r_pos]<=thresh);
        if(bo_bang_0)
        {
            if((data_mem.dopler[azi][r_pos]==0)
                    ||(data_mem.dopler[azi][r_pos]==0)
                    ||(data_mem.dopler[azi][r_pos]==0))
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
                short doplerVar =0;
                short var;
                var = abs(data_mem.dopler[azi][r_pos]-data_mem.dopler_old[azi][r_pos]);
                if(var>8)
                    var = 16-var;
                doplerVar+=var;
                var = abs(data_mem.dopler_old[azi][r_pos]-data_mem.dopler_old2[azi][r_pos]);
                if(var>8)
                    var = 16-var;
                doplerVar+=var;
                var = abs(data_mem.dopler[azi][r_pos]-data_mem.dopler_old2[azi][r_pos]);
                if(var>8)
                    var = 16-var;
                doplerVar+=var;
                cutoff = (doplerVar>2);
            }
            data_mem.dopler_old2[azi][r_pos] = data_mem.dopler_old[azi][r_pos];
            data_mem.dopler_old[azi][r_pos] = data_mem.dopler[azi][r_pos];
        }
        //update hot value
        if(cutoff)
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
        if(data_mem.hot[azi][r_pos]<2)
        {
            data_mem.detect[azi][r_pos] = false;
            data_mem.sled[azi][r_pos]-= (data_mem.sled[azi][r_pos])/100.0f;
        }
        else
        {
            data_mem.detect[azi][r_pos] = true;
            procPix(azi,r_pos);
            data_mem.sled[azi][r_pos] += (255 - data_mem.sled[azi][r_pos])/10.0f;
        }
        if(isFilting)
        {

            // check hot condition
            if(data_mem.hot[azi][r_pos]<2)
            {
                data_mem.level_disp[azi][r_pos] = 0;

            }
            else
            {

                data_mem.level_disp[azi][r_pos] = data_mem.level[azi][r_pos];// - (thresh);

            }
        }
        else
        {

            if(cut_thresh)
            {
                short thresh = 0;
                if(r_pos<4)
                {
                    rainLevel = noiseAverage;
                }
                else
                rainLevel += 0.4*(data_mem.level[azi][r_pos]-rainLevel);
                if(rainLevel>(noiseAverage+6*noiseVar))rainLevel = noiseAverage + 6*noiseVar;
                thresh = rainLevel - noiseVar*6;
                if(data_mem.level[azi][r_pos]>(thresh))data_mem.level_disp[azi][r_pos] = data_mem.level[azi][r_pos] - (thresh);
                else data_mem.level_disp[azi][r_pos] = 0;
            }
            else
            {
                data_mem.level_disp[azi][r_pos]=data_mem.level[azi][r_pos];
            }


        }

        //
    }


    return ;

}
//int testAzi = 0;
//int oldAzi =0;
void C_radar_data::ProcessDataFrame()
{

    short azi = (0xfff & (dataBuff[4] << 8 | dataBuff[5]))/2;

    if(azi==curAzir)return;
    //printf("azi:%d\n",azi);
    rotation_speed = dataBuff[1];
    overload = dataBuff[4]>>7;
    unsigned char n_clk_adc = (dataBuff[4]&(0xe0))>>5;
    if(clk_adc != n_clk_adc)
    {
        // clock adc
        if((n_clk_adc>=0)&&(n_clk_adc<=5))
        {
            clk_adc = n_clk_adc;
            isClkAdcChanged = true;
            resetData();
        }else
        {
            printf("\nClock adc is wrong.");
            return;
        }
    }
    temp = dataBuff[3]/4.0f;//
    tempType = dataBuff[2];
    sn_stat = dataBuff[14]<<8|dataBuff[15];

    memcpy(command_feedback,&dataBuff[RADAR_COMMAND_FEEDBACK],8);

    memcpy(noise_level,&dataBuff[RADAR_COMMAND_FEEDBACK+8],8);

    short lastazi=azi-1;//,nextazi=azi+1;
    if(lastazi<0)lastazi+=MAX_AZIR;
    if((lastazi!=curAzir)&&(azi!=curAzir))
    {
        ProcessData(lastazi);
        //printf("Data lost:%d at azi = %d\n",lastazi-curAzir,curAzir);
        lastazi-=1;

        if(lastazi<0)lastazi+=MAX_AZIR;
        if(lastazi!=curAzir)
        {
            ProcessData(lastazi);
            printf("Data lost:%d at azi = %d\n",lastazi-curAzir,curAzir);
            /*lastazi-=1;
            if(lastazi<0)lastazi+=MAX_AZIR;
            if(lastazi!=curAzir)
            {
                GetData(lastazi);
                printf("Data lost:%d at azi = %d\n",lastazi-curAzir,curAzir);
            }*/
        }
    }
    curAzir = azi;

    ProcessData(azi);
    if(azi == 0) {if(terrain_init_time) terrain_init_time--;}



}
short drawnazi = 0;
void C_radar_data::redrawImg()
{

    while(drawnazi!=curAzir)
    {
        drawnazi++;
        if(drawnazi>=MAX_AZIR)drawnazi=0;
        drawAzi(drawnazi);
        if(!((unsigned char)(drawnazi<<4))){
            procTracks(drawnazi);
        }
        if(drawnazi==0)getNoiseLevel();
    }
}
void C_radar_data::GetDataHR(unsigned char* data,unsigned short dataLen)
{

    if((dataLen<RADAR_DATA_HEADER)){printf("Wrong data.1\n");return;}
    char dataId = data[0]&0x0f;
    if(dataId==1)
    {
        //printf("%x-",data[0]);
        curFrameId = (data[0]&0xf0)>>4;
        range_max = (dataLen - RADAR_DATA_HEADER)*4/3 - RAD_S_PULSE_RES;
        //printf("range_max:%d\n",range_max);
        if(range_max < RAD_S_PULSE_RES){printf("Wrong data.2\n");return;}
        if(range_max > RAD_M_PULSE_RES){printf("Wrong data.3\n");return;}
        memcpy(dataBuff,data,dataLen);
        waitForData = dataLen;

    }
    else if(dataId==2)
    {
        //check if we are waiting for second half data frame
        if(!waitForData){printf("Wrong data.4\n");return;}
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
        memcpy(dataBuff + waitForData,data + RADAR_DATA_HEADER,dataLen-RADAR_DATA_HEADER);
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
void C_radar_data::procPLot(mark_t* pMark)
{

    if((pMark->size>size_thresh))//ENVDEP
    {
            object_t newobject;
            float ctA = pMark->sumA/pMark->size/MAX_AZIR*PI_NHAN2+trueN;
            float ctR = pMark->sumR/pMark->size;
            if(ctA >= PI_NHAN2)ctA -= PI_NHAN2;

            //check dopler
            newobject.dopler = data_mem.dopler[short(ctA)][short(ctR)];
            newobject.terrain = pMark->sumTer/(float)(pMark->size);
            newobject.az   = ctA;
            newobject.rg   = ctR;
            procObject(&newobject);
    }

}
void C_radar_data::procTracks(unsigned short curA)
{
    //process all marks
    short pr_curA = curA-1;
    if(pr_curA<0)pr_curA+=MAX_AZIR;
    for(unsigned short i = 0;i<plot_list.size();++i)
    {
        if(plot_list[i].size)
        {
            if((plot_list[i].maxA!=curA)&&(plot_list[i].maxA!=pr_curA))
            {
                procPLot(&plot_list[i]);
                plot_list[i].size =0;
            }

        }
    }
    //proc track
    float azi = (float)curA/MAX_AZIR*PI_NHAN2+trueN;
    for(unsigned short i=0;i<mTrackList.size();i++)
    {
        if(!mTrackList[i].state)continue;
        float dA = azi - mTrackList[i].estA;
        if(dA>PI) dA-=PI_NHAN2;
        else if(dA<-PI)dA+=PI_NHAN2;
        if(mTrackList[i].isProcessed)
        {
            if(abs(dA)<0.35f)//20 deg
            {
                mTrackList[i].isProcessed = false;
            }
        }
        else
        {
            if(abs(dA)>0.35f)//20 deg
            {
                mTrackList[i].isProcessed = true;
                mTrackList[i].update();
            }
        }
    }


}
void C_radar_data::getPolar(float x,float y,float *azi,float *range)
{
    //x*=scale_ppi
    *azi = atanf(x/y);//tinh azi theo chuan bac thuan kim dong ho
    if(y<0)*azi+=PI;
    if(azi<0)*azi += PI_NHAN2;
    *range = sqrt(x*x+y*y);
}
void C_radar_data::addTrackManual(float x,float y)
{
    float azi = atanf(x/y);//tinh azi,range
    if(y<0)azi+=PI;
    if(azi<0)azi += PI_NHAN2;
    float range = sqrt(x*x+y*y);
    //addTrack( azi,  range, 4);
}
void C_radar_data::addTrack(object_t* mObject)
{
    //add new track
    for(unsigned short i=0;i<mTrackList.size();i++)
    {
        if(!mTrackList[i].state)
        {
            mTrackList[i].init(mObject);
            return;
        }
    }
    track_t newTrack;
    newTrack.init(mObject);
    mTrackList.push_back(newTrack);
}
void C_radar_data::deleteTrack(short trackNum)
{
    if(mTrackList.size()>trackNum)
    {
        mTrackList[trackNum].state = 0;
        if(mTrackList[trackNum].object_list.size())mTrackList[trackNum].object_list.clear();
        if(mTrackList[trackNum].suspect_list.size())mTrackList[trackNum].suspect_list.clear();
    }
}
void C_radar_data::procObject(object_t* pObject)
{
        if(pObject->rg<80) return;
        if(avtodetect)
        {
            if(pObject->terrain<TERRAIN_THRESH)
            {
                for(unsigned short i=0;i<mTrackList.size();i++)
                {
                    if(mTrackList[i].state&&(! mTrackList[i].isProcessed))
                    {
                        if(mTrackList[i].checkProb(pObject)){
                            mTrackList[i].suspect_list.push_back(*pObject);
                            return;//add object to a processing track
                        }
                    }
                }

                for(unsigned short i=0;i<mTrackList.size();i++)
                {
                    if(!mTrackList[i].state)
                    {

                        mTrackList[i].init(pObject);

                        return;//add object to a empty track
                    }
                }
                //create new track
                if(mTrackList.size()<MAX_TRACKS)
                {

                    addTrack(pObject);

                }

            }
        }
        else
        {
            for(unsigned short i=0;i<mTrackList.size();i++)
            {
                if(mTrackList[i].state&&(! mTrackList[i].isProcessed))
                {
                    if(mTrackList[i].checkProb(pObject)){
                        mTrackList[i].suspect_list.push_back(*pObject);
                        return;//add object to a processing track
                    }
                }
            }
        }

}
void C_radar_data::procPix(short proc_azi,short range)//_______signal detected, check 4 last neighbour points for nearby mark_______________//
{

    short pr_proc_azi = proc_azi-1;
    if(pr_proc_azi<0)pr_proc_azi+=MAX_AZIR;
    short plotIndex =-1;
    if(data_mem.detect[pr_proc_azi][range])
    {
         plotIndex = data_mem.plotIndex[pr_proc_azi][range];

    }else if(data_mem.detect[proc_azi][range-1])
    {
         plotIndex = data_mem.plotIndex[proc_azi][range-1];

    }
    else if(data_mem.detect[pr_proc_azi][range-1])
    {
         plotIndex = data_mem.plotIndex[pr_proc_azi][range-1];

    }
    else if(data_mem.detect[pr_proc_azi][range+1])
    {
         plotIndex = data_mem.plotIndex[pr_proc_azi][range+1];
    }
    if(plotIndex!=-1)// add to existing marker
    {
        data_mem.plotIndex[proc_azi][range] = plotIndex;
        plot_list[plotIndex].size++;
        if(proc_azi<plot_list[plotIndex].minA){
            plot_list[plotIndex].sumA    +=  proc_azi + MAX_AZIR;
            plot_list[plotIndex].maxA    =  proc_azi + MAX_AZIR;
        }else
        {
            plot_list[plotIndex].sumA    +=  proc_azi;
            plot_list[plotIndex].maxA    =  proc_azi;
        }

        plot_list[plotIndex].sumR    +=  range;
        plot_list[plotIndex].sumTer  +=  data_mem.sled[proc_azi][range];

    }
    else//_________new plot found_____________//
    {

        mark_t         new_plot;
        new_plot.maxA =  new_plot.minA  = proc_azi;
        //new_mark.minR = new_mark.maxR = range;
        new_plot.size =  1;
        new_plot.sumTer = data_mem.sled[proc_azi][range];
        new_plot.sumA =  proc_azi;
        new_plot.sumR =  range;
        bool listFull = true;

        for(unsigned short i = 0;i<plot_list.size();++i)
        {
            //  overwrite
            if(!plot_list[i].size)
            {
                data_mem.plotIndex[proc_azi][range] =  i;
                plot_list[i] = new_plot;
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
short zoomXmax,zoomYmax,zoomXmin,zoomYmin;
short zoomCenterX=DISPLAY_RES,zoomCenterY=DISPLAY_RES;
void C_radar_data::updateZoomRect(float ctx, float cty)
{
    ctx*=4/scale_ppi;
    cty*=4/scale_ppi;
    zoomXmax = ctx+ZOOM_SIZE/2;
    zoomYmax = cty+ZOOM_SIZE/2;
    zoomXmin = ctx-ZOOM_SIZE/2;
    zoomYmin = cty-ZOOM_SIZE/2;
    raw_map_init_zoom();

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
        theta+=dTheta;
    }
}
void C_radar_data::resetData()
{
    terrain_init_time = 20;

    memset(data_mem.level,0,RAD_M_PULSE_RES*MAX_AZIR);
    memset(data_mem.dopler,0,RAD_M_PULSE_RES*MAX_AZIR);
    memset(data_mem.detect,0,RAD_M_PULSE_RES*MAX_AZIR);
    memset(data_mem.plotIndex,0,RAD_M_PULSE_RES*MAX_AZIR);
    memset(data_mem.hot,0,RAD_M_PULSE_RES*MAX_AZIR);
   resetSled();

}
void C_radar_data::resetSled()
{
        memset(data_mem.sled,0,RAD_M_PULSE_RES*MAX_AZIR);
}
void C_radar_data::setScalePPI(float scale)
{
    float sn_scale;
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
    default:
        sn_scale = SIGNAL_SCALE_0;
    }
    scale_ppi = sn_scale*scale;
    //updateZoomRect();
}
void C_radar_data::setScaleZoom(float scale)
{
    float sn_scale;
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
    default:
        sn_scale = SIGNAL_SCALE_0;
    }
    sn_scale = SIGNAL_SCALE_0;
    scale_zoom = sn_scale*scale/scale_ppi;
    //updateZoomRect();
}

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
            unsigned char pvalue = value*k;
            if( data_mem.display_mask_zoom[px+x][py+y] <= pvalue)
            {
                data_mem.display_mask_zoom[px+x][py+y] = pvalue;
                img_zoom_ppi->setPixel(px+x,py+y,getColor(pvalue,dopler,sled));
                //DrawZoom(px,py,pvalue);
            }
        }
    }

}
uint C_radar_data::getColor(unsigned char pvalue,unsigned char dopler,unsigned char sled)
{

    unsigned short value = ((unsigned short)pvalue)*brightness;
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
        if(dopler==0||dopler==1||dopler==15)
        {

            color = 0xffff00;
        }else
            if(dopler==2||dopler==3||dopler==13||dopler==14)
            {
                color = 0x00ff00;
            }
            else if(dopler==4||dopler==5||dopler==11||dopler==12)
            {
                color = 0x00ffff;
            }
            else
            {
                color = 0x00ffff;
            }
        alpha = value;//0xff - ((0xff - value)*0.75);
        color = color|(alpha<<24);
        break;

    case VALUE_ORANGE_BLUE:
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
    for(unsigned short i=0;i<mTrackList.size();i++)
    {
        if(mTrackList[i].state)
        {
            mTrackList[i].state = 0;
        }
    }
}

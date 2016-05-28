#define PI 3.14159265358979323846
#include "C_raymarine_data.h"
#include <math.h>


static period_t                curPeriod;
//static std::queue<period_t>    period_cache;
static unsigned short cur_mark_index = 0;
static unsigned short cur_object_index = 0;
//_______Kalman filter_____//
//double KM_EstimatePredicition(double dt)
//{
//    return m_x[0] + (dt * m_x[1]);
//}
//double KM_Variance () { return m_p[0]; } // positional variance.
//double KM_Value() { return m_x[0]; } // latest position.
//void KM_Reset(
//            double qx, // Measurement to position state minimal variance.
//            double qv, // Measurement to velocity state minimal variance.
//            double r, // Measurement covariance (sets minimal gain).
//            double pd, // Initial variance.
//            double ix) // Initial position
//{
//    m_q[0] = qx; m_q[1] = qv;
//    m_r = r;
//    m_p[0] = m_p[3] = pd;
//    m_p[1] = m_p[2] = 0;
//    m_x[0] = ix;
//    m_x[1] = 0;
//}
//void
//______________________________________//
C_raymarine_data::C_raymarine_data()
{
//    pScr_map = NULL;
    //OpenFile("rawData.001");
    thresh = 3;
    curPeriodIndex = 0;
    //status = init;
    pFile = NULL;
    procdata = &curPeriod;
}
C_raymarine_data::~C_raymarine_data()
{
	if(pFile)fclose(pFile);
//    if(pScr_map)
//    {
//        delete[] pScr_map;
//    }
}

int C_raymarine_data::OpenFile(const char* fileName)
{
    //printf(fileName);
	pFile = fopen(fileName,"rb");
	if (pFile==NULL)
	{
        printf( "Cannot open binary file ");return false;
	}

	fseek(pFile,0,SEEK_SET);
	//data_full=0;
    return true;
}
frame_t* C_raymarine_data::GetData(char* data,short dataLen)
{
    for(short i = 0; i < dataLen; i++)
    {
        if((data[i]==0x01))
        {
            if((!(data[i+1]||data[i+2]))&&(data[i+3]==0x28))
            {

                i+=8;
                short azi = 0x7ff & (data[i+1] << 8 | data[i]);
                curAzir = azi;
                if(curAzir==0x7ff){
                    curPeriodIndex++;
                    procTracks();
                }
                i+=72;
                int r_pos=0;
                while(r_pos<MAX_RANGE_RM)
                {
                    if((data[i]==0x5c)&&(!data[i+2])&&(!data[i+3]))
                    {
                        short null_end = r_pos + 2*data[i+1];
                        for(;r_pos<null_end;r_pos++)
                        {
                            signal_map.frame[azi].raw_map[r_pos].level = 0;
                        }
                        i+=4;
                    }else
                    {
                        signal_map.frame[azi].raw_map[r_pos].level=data[i];
                        r_pos++;
                        i++;
                    }
                }

            }
        }
//            signal_map.frame[azi].raw_map[r].level = buff[r];
//            signal_map.frame[azi].raw_map[r].terrain = float(signal_map.frame[azi].raw_map[r].terrain*0.95 + 0.05);//255*0.125;

    }

    return &signal_map.frame[curAzir];

}
void C_raymarine_data::procMark(mark_t* pMark)
{

    if(cur_mark_index >= MAX_MARK_RM)          cur_mark_index -= MAX_MARK_RM;
    short realsize = (double)pMark->size*(pMark->minR/1024.0);
    double ctA = pMark->sumA/pMark->size;
    double ctR = pMark->sumR/pMark->size;
    if(ctA >= 4096)ctA -= 4096;
    if((realsize>thresh)&&
       (signal_map.frame[int(ctA)].raw_map[int(ctR)].terrain < NGUONG_DIA_VAT_RM))
        //kiem tra kich thuoc va dia vat de tao object moi_________________________________
    {
//        if(curPeriod.object_list.size()>= MAX_OBJECTS)
//        {
//            cur_object_index++;
//            if(cur_object_index >= MAX_OBJECTS) cur_object_index -= MAX_OBJECTS;
//            procObject(&curPeriod.object_list[cur_object_index]);//____________kiem tra track____________

//            curPeriod.object_list[cur_object_index].centerA   = ctA;
//            curPeriod.object_list[cur_object_index].centerR   = ctR;
//            //_________convert center AR to XY_______________//
//            polarToXY(&curPeriod.object_list[cur_object_index].centerX,
//                      &curPeriod.object_list[cur_object_index].centerY,
//                      curPeriod.object_list[cur_object_index].centerA,
//                      curPeriod.object_list[cur_object_index].centerR  );
//            curPeriod.object_list[cur_object_index].selected = false;
//        }
//        else
//        {
//            cur_object_index = curPeriod.object_list.size();
            object_t newobject;
            newobject.centerA   = ctA;
            newobject.centerR   = ctR;
            //_________convert center AR to XY_______________//
            polarToXY(&newobject.centerX,
                      &newobject.centerY,
                      newobject.centerA,
                      newobject.centerR);
            newobject.selected = false;
            procObject(&newobject);
//            curPeriod.object_list.push_back(newobject);
//        }
    }

}
void C_raymarine_data::procTracks()
{
    if(!trackList.empty())
        for(int i=0;i<trackList.size();i++)
        {
            //printf("\ntrackList[i].state%d",trackList[i].state);
            if(trackList[i].state==0)continue;
            if(trackList[i].suspect_list.size())
            {

                trackList[i].object_list.push_back(trackList[i].suspect_list[0]);
                trackList[i].suspect_list.clear();
                trackList[i].curPeriod = curPeriodIndex;
                if(trackList[i].state<10)trackList[i].state++;
                trackList[i].estimate();
            }
            else trackList[i].miss();
        }

}
void C_raymarine_data::procObject(object_t* pObject)
{

    //if(curPeriodIndex<10) return;

    if(!trackList.empty())//tim qui dao tuong ung voi object
    {
        short trackIndex=-1;
        char trackstat=0;
        for(int i=0;i<trackList.size();i++)
        {

            if((abs(pObject->centerX-trackList[i].estX)<trackList[i].probR)&&
               (abs(pObject->centerY-trackList[i].estY)<trackList[i].probR))
            {
                //printf("\nnew suspect");
                //trackList[i].suspect_list.push_back(*pObject);
                if(trackstat<trackList[i].state)
                {
                    trackIndex=i;
                    trackstat=trackList[i].state;
                }
            }

        }
        if(trackIndex!=-1)trackList[trackIndex].suspect_list.push_back(*pObject);
    }
    //add new track
    if(trackList.size()<MAX_TRACKS_RM)
    {
        track_t newTrack;
        newTrack.suspect_list.push_back(*pObject);
        newTrack.curPeriod = curPeriodIndex;
        trackList.push_back(newTrack);
    }
    else
    {
        for(int i=0;i<trackList.size();i++)
        {
            if(trackList[i].state==0)
            {
                trackList[i].suspect_list.push_back(*pObject);
                trackList[i].curPeriod = curPeriodIndex;
                trackList[i].state=1;
                return;
            }
        }
    }

}
void C_raymarine_data::scan_nearby(short proc_azi,short range)//_______signal detected, check 4 last neighbour points for nearby mark_______________//
{

    short pr_proc_azi = proc_azi-1;
    if(pr_proc_azi<0)pr_proc_azi+=4096;
    unsigned short markIndex =0;
    if(signal_map.frame[pr_proc_azi].raw_map[range].cfar)
    {
         markIndex = signal_map.frame[pr_proc_azi].raw_map[range].markIndex;

    }else if(signal_map.frame[proc_azi].raw_map[range-1].cfar)
    {
         markIndex = signal_map.frame[proc_azi].raw_map[range-1].markIndex;

    }
    else if(signal_map.frame[pr_proc_azi].raw_map[range-1].cfar)
    {
         markIndex = signal_map.frame[pr_proc_azi].raw_map[range-1].markIndex;

    }
    else if(signal_map.frame[pr_proc_azi].raw_map[range+1].cfar)
    {
        markIndex = signal_map.frame[pr_proc_azi].raw_map[range+1].markIndex;

    }
    if(markIndex)
    {
        signal_map.frame[proc_azi].raw_map[range].markIndex = markIndex;
        curPeriod.mark_list[markIndex].size++;
        if(proc_azi<curPeriod.mark_list[markIndex].minA){
            curPeriod.mark_list[markIndex].sumA    +=  proc_azi+4096;
        }else curPeriod.mark_list[markIndex].sumA    +=  proc_azi;
        curPeriod.mark_list[markIndex].sumR    +=  range;
        curPeriod.mark_list[markIndex].maxA    =  proc_azi;
        if(range>curPeriod.mark_list[markIndex].maxR )
            curPeriod.mark_list[markIndex].maxR   =  range;
        else
            if(range<curPeriod.mark_list[markIndex].minR )
            curPeriod.mark_list[markIndex].minR   =  range;
    }
    else//_________new marker found_____________//
    {
        if(curPeriod.mark_list.size()>= MAX_MARK_RM)//_____buffer full, check mark for object and overwrite old marks________
        {
            cur_mark_index++;
            if(cur_mark_index >= MAX_MARK_RM) cur_mark_index -= MAX_MARK_RM;
            procMark(&curPeriod.mark_list[cur_mark_index]);
            signal_map.frame[proc_azi].raw_map[range].markIndex =  cur_mark_index;//___overwrite___
            curPeriod.mark_list[cur_mark_index].maxA    =  proc_azi;
            curPeriod.mark_list[cur_mark_index].minA    =  proc_azi;
            curPeriod.mark_list[cur_mark_index].maxR    =  range;
            curPeriod.mark_list[cur_mark_index].minR    =  range;
            curPeriod.mark_list[cur_mark_index].size    =  1;
            curPeriod.mark_list[cur_mark_index].sumA    =  proc_azi;
            curPeriod.mark_list[cur_mark_index].sumR    =  range;
        }
        else//fill the buffer
        {
            cur_mark_index                                      = curPeriod.mark_list.size();
            signal_map.frame[proc_azi].raw_map[range].markIndex =  cur_mark_index;
            mark_t              m_mark;
            m_mark.maxA =  proc_azi;
            m_mark.minA =  proc_azi;
            m_mark.minR =  range;
            m_mark.maxR =  range;
            m_mark.size    =  1;
            m_mark.sumA    =  proc_azi;
            m_mark.sumR    =  range;
            curPeriod.mark_list.push_back(m_mark);
        }
    }
            //______________________________________//




}
void C_raymarine_data::polarToXY(short *x, short *y, short azi, short range)
{
    *x = signal_map.frame[azi].raw_map[range].x;
    *y = signal_map.frame[azi].raw_map[range].y;
}
static short ctX=0,ctY=0;
static float dr = 0;
void C_raymarine_data::polarToXY(double *x, double *y, double azi, double range)
{

    float theta = azi*2*PI/MAX_AZIR_RM-PI/2;
    *x = ctX+((dr*cosf(theta)))*range;
    *y = ctY+((dr*sinf(theta)))*range;
}
void C_raymarine_data::raw_map_init(short max_X,short max_Y, float scale)
{
//    if(pScr_map)
//    {
//        delete[] pScr_map;
//    }
//    pScr_map = new raw_point_t[max_X][max_Y];

    float theta=-PI/2;
    float dTheta = 2*PI/MAX_AZIR_RM;
	int azir,range;
    float r = scale*MAX_RANGE_KM_RM;
    dr = r/MAX_RANGE_RM;
    ctX = max_X/2;
    ctY = max_Y/2;
    for(azir = 0;azir< MAX_AZIR_RM;azir++)
	{

		float cost = cosf(theta);
		float sint = sinf(theta);

        for(range = 0;range<MAX_RANGE_RM;range++)
		{

			//signal->ray[nRay].pulse[nPulse].level = *(data->raw+nPulse);
            signal_map.frame[azir].raw_map[range].x     = ctX+short((((float)dr*(float)cost))*range);
            signal_map.frame[azir].raw_map[range].y     = ctY+short((((float)dr*(float)sint))*range);
            signal_map.frame[azir].raw_map[range].cfar  = 0;
            signal_map.frame[azir].raw_map[range].level  = 0;
		}
		theta+=dTheta;
	}
}

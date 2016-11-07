
#include "vnmap.h"
#include <math.h>
bool Q_vnmap::checkInside(Point_t  *point)
{
        if((point->m_Lat)<(brPos.m_Lat)) return 0;
        if((point->m_Long)<(ulPos.m_Long)) return 0;
        if((point->m_Lat)>(ulPos.m_Lat)) return 0;
        if((point->m_Long)>(brPos.m_Long)) return 0;
        return 1;
}
//void Q_vnmap::getElement(short nlayer, short npoly, short npoint, Point_t *point)
//{

//    ConvDegToScr(point,layers[nlayer][npoly][npoint].m_Lat,layers[nlayer][npoly][npoint].m_Long);
//    // !!! check inside
//    return ;
//}
/*
void Q_vnmap::OpenShpFile(const char* filename, short layerIndex)
{

    if(layerIndex<0||layerIndex>=N_LAYER)return;

    SHPHandle hSHP = SHPOpen(filename,"rb");
    if(hSHP == NULL) return;

    SHPObject *psShape;

    for(int i=0;i<hSHP->nRecords;i++)
    {
        psShape = SHPReadObject(hSHP, i);
        Point_t pPoint;
        C2_Poly polygon;
        int currpart = 1;
        for(int j=0;j<psShape->nVertices;j++)
        {
            if((j==psShape->panPartStart[currpart])&&(currpart< psShape->nParts)){
                currpart++;
                layers[layerIndex].push_back(polygon);
                polygon.clear();
            }
            float fX = float (psShape->padfX[j]);//+0.006);
            float fY = float(psShape->padfY[j]);//+0.002);
            pPoint.m_Long   =   fX;
            pPoint.m_Lat    =   fY;

            polygon.push_back(pPoint);
        }

        layers[layerIndex].push_back(polygon);

    }

}*/
Q_vnmap::Q_vnmap()
{

}
void Q_vnmap::setUp(float ctLat, float ctLong, unsigned short size, const char* fileName)
{
    float degSize = size/100.0f;
    if(ctLat)currPos.m_Lat = ctLat;
    if(ctLong)currPos.m_Long =  ctLong;
    ulPos.m_Lat = currPos.m_Lat + degSize;//0.9 degrees approximately equal to 100km
    ulPos.m_Long = currPos.m_Long - degSize;
    brPos.m_Lat = currPos.m_Lat - degSize;
    brPos.m_Long = currPos.m_Long + degSize;
	
	if(fileName){
        //CString filePlaceName;
		//LoadPlaces(filePlaceName);
		
        LoadBinFile(fileName);
        //LoadPlaces(fileName);
	}
}
void Q_vnmap::ClearData()
{
    for(short i=0;i<N_LAYER;i++)
    {
        layers[i].clear();
    }
    placeList.clear();
}
void Q_vnmap::LoadBinFile(const char* fileName)
{
    FILE *pFile= fopen(fileName,"rb");
    if (!pFile)
    {
        printf( "cannot open file VNMap\n");return;
    }
    ClearData();
    LoadPlaces(fileName);
    unsigned char*      szBuff;// Buffer for data
    unsigned long		fPos;					// position in File
    unsigned long       i;				// position in Buff
    unsigned long       fileSize;
    unsigned char		nType;
    unsigned short int 	nElement, j;
    fseek (pFile, 0, SEEK_END);
    fileSize = ftell(pFile);
    fPos = 0;
    fseek(pFile,0,SEEK_SET);

    szBuff = new unsigned char[fileSize];
    if(szBuff==0) return;
    unsigned long nread = fread(szBuff,sizeof(unsigned char),fileSize,pFile);
    fclose(pFile);
    //printf("\nfilesize-%d",nread);
    //C2_PolyList layer;

    for(;;){
        if (fPos >= nread) break;
        i = 0;

        nType = szBuff[fPos];
        if(nType>=N_LAYER) return;
        nElement =(szBuff[fPos+2] << 8) | szBuff[fPos+1];
        i+=3;

        //printf("\nElement- %d",nElement);
        C2_Poly	pPoly;
        bool inside = false;
        for (j = 0; j < nElement; j ++)
        {

            Point_t pPoint;
            pPoint.m_Lat = (float)((unsigned short) ((szBuff[fPos+i  ] ) |(szBuff[fPos+i+1])<<8))/3276.8f +5.0f;// 3276.8 = 2^16/20
            i+=2;
            pPoint.m_Long = (float)((unsigned short) ((szBuff[fPos+i  ]  ) |(szBuff[fPos+i+1])<<8))/3276.8f +100.0f;
            i+=2;
            pPoly.push_back(pPoint);
            if(!inside)
            {
                if(checkInside(&pPoint)){
                    inside=true;
                }
            }
        }
        if(inside)layers[nType].push_back(pPoly);
        fPos += i;
    }

    delete [] szBuff;

}

void Q_vnmap::LoadPlaces(const char* binfileName)
{

    int len = strlen(binfileName);
	char *fileName = new char[len+1]; 
	strncpy(fileName,binfileName,len);
	fileName[len-3]='t';
	fileName[len-2]='x';
	fileName[len-1]='t';
	fileName[len]=0;
    FILE *pFile= fopen(fileName,"r");
	delete[] fileName;

    if (!pFile) return;

	int lSize;
	
	fseek (pFile , 0 , SEEK_END);
	lSize = ftell (pFile);
	fseek(pFile,0,SEEK_SET);
	lSize/=sizeof(wchar_t);
    wchar_t* buffer = new wchar_t[lSize];//(char*) malloc (sizeof(char)*lSize);
	fread(buffer,sizeof(wchar_t),lSize,pFile);
	fclose(pFile);
	std::wstring str(buffer);
	delete[] buffer;
	size_t pos, posend;
	//read COM prt 
	size_t i=0;
	placeList.clear();
	while(i<str.size()-8)
	{
		Place_t place;
		pos = str.find(13, i);//13 = LF line feed
		
		if((str[pos+1]==10)&&(str[pos+2]==13)&&(str[pos+3]==10))
		{
			posend = str.find('\n', pos+4);
			place.text = str.substr(pos+4,posend-pos-3);
			if(str[pos+4]=='#') break;
			pos = posend;
			//posend = str.find(' ', pos+1);
            Point_t pPoint;
			pPoint.m_Lat  =	stof(str.substr(pos+1,2)) + 
							stof(str.substr(pos+4,2))/60.0 + 
							stof(str.substr(pos+7,2))/3600.0;
			pPoint.m_Long =	stof(str.substr(pos+10,3)) + 
							stof(str.substr(pos+14,2))/60.0 + 
							stof(str.substr(pos+17,2))/3600.0;
			i = pos + 19;
            //ConvDegToScr(&pPoint.m_Long,&pPoint.m_Lat,&pPoint.m_Long,&pPoint.m_Lat);
			place.m_Lat  = pPoint.m_Lat;
			place.m_Long = pPoint.m_Long;
			placeList.push_back(place);
		}
		else i=pos+1;
		
	
    }


}
void Q_vnmap::ConvWGSToKmXY(double* x, double *y, double m_Long,double m_Lat)
{
    float refLat = (currPos.m_Lat + (m_Lat))*0.00872664625997;//pi/360
    *x	= (((m_Long)-currPos.m_Long) * 111.31949079327357f)*cosf(refLat);// 3.14159265358979324/180.0*6378.137);//deg*pi/180*rEarth
    *y	= ((currPos.m_Lat - (m_Lat)) * 111.31949079327357f);
    //toa do tinh bang KM so voi diem currPos
}
void Q_vnmap::ConvKmXYToWGS(double x, double y, double *m_Long, double *m_Lat)
{
    double refLat = (currPos.m_Lat + (*m_Lat))*0.00872664625997;//pi/360
    *m_Long = (x)/111.31949079327357f/cos(refLat)+currPos.m_Long;
    *m_Lat  = currPos.m_Lat +  (y)/111.31949079327357f;
    //toa do lat long
}

Q_vnmap::~Q_vnmap()
{

}


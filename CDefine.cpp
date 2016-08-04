//=============================================================================
//
// CDefine.cpp: implementation of the C2_Data class.
//	
//	Updated: 10-10-2007
//=============================================================================

//#include "StdAfx.h"
#include "CDefine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

//-----------------------------------------------------------------------------
//	Encode/Decode of string: leng max = 256 byte
//-----------------------------------------------------------------------------
int MakeStrCode(BYTE *szStr, int nLeng)
{
    BYTE	szKey[16] = {'C','o','n','g',' ','t','y',' ','A','I','C',' ','B','o','Q','P'};
    unsigned char *	szTmp = new unsigned char[10];
	if (!szTmp)
		return 0;
	
	for (int i = 0; i < nLeng; i ++)
	{
		szTmp[i] = 0xFF		^ szKey[i%16];
		szTmp[i] = szStr[i] ^ szTmp[i];
	}	
	memcpy(szStr, szTmp, nLeng);
	
	delete []szTmp;
	return nLeng;
}

//-----------------------------------------------------------------------------
int ConvStrChar(QString szStr, char *szBuff)
{
    int	nLen = szStr.size();
	
	nLen = (nLen < 254)? nLen : 254;		// Get min(nLeng,nSize)	- 254 = max buff	
	for (int i = 0; i < nLen; i ++)		
        szBuff[i] = szStr.toAscii().at(i);

	szBuff[nLen] = 0x00;
	return nLen;
} 

int ConvStrChar(QString szStr, char *szBuff, int nMax)
{
    int	nLen = szStr.size();

	nLen = (nLen < nMax)? nLen : nMax;		// Get min(nLeng,nSize)	- 254 = max buff
	for (int i = 0; i < nLen; i ++)
        szBuff[i] = szStr.toAscii().at(i);
	szBuff[nLen] = 0x00;
	return nLen;
} 


//-----------------------------------------------------------------------------
int ConvStrBuff(QString szStr, BYTE *szBuff)
{
    int	nLen = szStr.size();

	nLen = (nLen < 254) ? nLen : 254;		// max name = 128 byte	
	for (int i = 0; i < nLen; i ++)
        szBuff[i] = szStr.toAscii().at(i);

	szBuff[nLen] = 0x00;
	return nLen;
}

int ConvStrBuff(QString szStr, BYTE *szBuff, int nMax)
{
    int	nLen = szStr.size();

	nLen = (nLen < nMax)? nLen : nMax;		// max name = 254 byte	
	for (int i = 0; i < nLen; i ++)
        szBuff[i] = szStr.toAscii().at(i);

	szBuff[nLen] = 0x00;
	return nLen;
} 

//QString GetStrTime(int nTime)
//{
//	BYTE	nHour, nMin, nSec;

//	nHour	= BYTE(nTime/3600);
//	nTime  %= 3600;
//	nMin	= BYTE(nTime/60);
//	nSec	= BYTE(nTime%60);

//    QString szStr;
//	szStr.Format(_T("%02d:%02d:%02d"), nHour, nMin, nSec);
//	return szStr;
//}

//-----------------------------------------------------------------------------
//QString GetStrTime(long nTime)
//{
//	BYTE	nHour, nMin, nSec;
//	long	nTmp = (nTime >> 7);

//	nHour	= BYTE(nTmp/3600);
//	nTmp   %= 3600;
//	nMin	= BYTE(nTmp/60);
//	nSec	= BYTE(nTmp%60);

//    QString szStr;
//	szStr.Format(_T("%02d:%02d:%02d"), nHour, nMin, nSec);
//	return szStr;
//}
/*
int GetIntTime(QString szTime)
{
	int		nDiv = 0;		// [number of ':']
	int		i, nLen;

	nLen = szTime.GetLength();
	i	 = 0;
	while (i < nLen)
	{
		if (szTime[i] == ' ')
		{
			szTime.Delete(i, 1);
			nLen = szTime.GetLength();
			continue;
		}

		if ((szTime[i] >= 0x30) && (szTime[i] <= 0x39))
		{}
		else 
		if ( szTime[i] == ':')
			nDiv ++;		
		else
			return -1;			
		i ++;
	}

	if (szTime[0] == ':')	// Letter 1 is Digital
		return -1;

	if (nDiv != 2)			// 2 letter ':'
		return -1;

	int		nHour, nMin, nSec;
	int		nTemp;

	i		= 0;
	nDiv	= 0;
	nTemp	= 0;	// Time temp
	while (i < nLen)
	{
		if (szTime[i] == ':')
		{
			if (nDiv == 0)
				nHour = nTemp;
			else 
				if (nDiv == 1)
					nMin  = nTemp;	

			nDiv ++;
			nTemp = 0;
		}
		else
		{
			nTemp *= 10;
			nTemp += (szTime[i] - 0x30);
		}
		i ++;
	}
	nSec = nTemp;

	if ((nMin > 60) || (nSec > 60))
		return -1;
	else
		return (nHour*3600 + nMin*60 + nSec);
}
*/
/*
int GetIntSpeed(QString szStr)
{
	int		nDiv = 0;		// [number of '.']
	int		i, nLen;
	char	szTmp[32];

	nLen = szStr.GetLength();
	i	 = 0;
	while (i < nLen)
	{
		if (szStr[i] == ' ')
		{
			szStr.Delete(i, 1);
			nLen --;
			continue;
		}

		if ((szStr[i] >= '0') && (szStr[i] <= '9'))
			szTmp[i] = (char)szStr[i];
		else 
		if ((szStr[i] == '.') || (szStr[i] == ','))
		{
			szTmp[i] = '.';
			nDiv ++;
		}
		else
			return -1;

		i ++;		
	}
	szTmp[i] = 0x00;

	if ((szTmp[0] == '.') || (szTmp[0] == ','))	// Letter 1 is Digital
		return -1;

	if (nDiv > 1)			// 0/1 letter '.'
		return -1;

	return int(10*atof(szTmp));
}
*/

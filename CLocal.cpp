//=============================================================================
//
// CLocal.cpp: implementation of the C2_Local class.
//
//	Update 23-06-2011 by Dang Quang Hieu
//	AIC Corporation
//=============================================================================

#include "StdAfx.h"
#include "CLocal.h"
#include <math.h>


//=============================================================================
// C2_Host Class
//=============================================================================
C2_Host::C2_Host()
{
	m_Uid	  = 0;
	m_Status  = 0;
	m_szName  = "";
	m_LinkTyp = 0;		// Ethernet / Dialup

	memset(m_RmtAddr, 0, 4);
	memset(m_LocAddr, 0, 4);

	m_RmtPort = 0;
	m_LocPort = 0;
}

C2_Host::~C2_Host()
{}

BYTE C2_Host::Compare(C2_Host *pHost)
{
	BYTE	nResl = 0;

	if ((m_LinkTyp & 1) != (pHost->m_LinkTyp & 1))
		nResl |= 0x01;

	if (m_szName.Compare(pHost->m_szName) != 0)
		nResl |= 0x02;

	if ((m_RmtAddr[0] != pHost->m_RmtAddr[0]) ||
		(m_RmtAddr[1] != pHost->m_RmtAddr[1]) ||
		(m_RmtAddr[2] != pHost->m_RmtAddr[2]) ||
		(m_RmtAddr[3] != pHost->m_RmtAddr[3]))
		nResl |= 0x04;

	if ((m_LocAddr[0] != pHost->m_LocAddr[0]) ||
		(m_LocAddr[1] != pHost->m_LocAddr[1]) ||
		(m_LocAddr[2] != pHost->m_LocAddr[2]) ||
		(m_LocAddr[3] != pHost->m_LocAddr[3]))
		nResl |= 0x08;

	return nResl;
}

void C2_Host::Update(C2_Host *pHost)
{
	m_Uid	  = pHost->m_Uid	;
	m_Status  = pHost->m_Status ;
	m_szName  = pHost->m_szName ;
	m_LinkTyp = pHost->m_LinkTyp;

	memcpy(m_RmtAddr, pHost->m_RmtAddr, 4);	
	memcpy(m_LocAddr, pHost->m_LocAddr, 4);	

	m_RmtPort = pHost->m_RmtPort;
	m_LocPort = pHost->m_LocPort;
}

//-----------------------------------------------------------------------------
CString C2_Host::GetLocAddr()
{
	CString	szAddr;

	szAddr.Format(_T("%d.%d.%d.%d"), m_LocAddr[0], m_LocAddr[1], m_LocAddr[2], m_LocAddr[3]);
	return szAddr;
}

CString C2_Host::GetRmtAddr()
{
	CString	szAddr;

	szAddr.Format(_T("%d.%d.%d.%d"), m_RmtAddr[0], m_RmtAddr[1], m_RmtAddr[2], m_RmtAddr[3]);
	return szAddr;
}

//-----------------------------------------------------------------------------
CString C2_Host::GetStrAddr(BYTE *nAddr)
{
	CString	szAddr;

	szAddr.Format(_T("%d.%d.%d.%d"), nAddr[0], nAddr[1], nAddr[2], nAddr[3]);
	return szAddr;
}

//-----------------------------------------------------------------------------
bool C2_Host::SetStrAddr(BYTE *nAddr, CString szAddr)
{
	int	nLen = szAddr.GetLength();
	int	nNum = 0, i;

	// Check valid of IP string
	for (i = 0; i < nLen; i++)
	{
		if (szAddr[i] == '.')
			nNum ++;
		else
			if ((szAddr[i] < 0x30) || (szAddr[i] > 0x39))
				return false;		
	}

	if (nNum != 3)
		return false;

	// Convert to BYTE
	nNum = 0;
	i	 = 0;	
	while (i < nLen)
	{
		if (szAddr[i] == '.')
			nNum ++;
		else
			nAddr[nNum] = nAddr[nNum]*10 + szAddr[i] - 0x30;
		i++;
	}
	return true;
}

//-----------------------------------------------------------------------------
CString C2_Host::GetStrStat()
{
	switch (m_Status)
	{
		case STAT_SESS_NONE: return _T("Không mở phiên ");
		case STAT_SESS_OPEN: return _T("Đã mở kết nối !");	
		case STAT_SESS_WAIT: return _T("Chờ... trả lời ");	
		case STAT_SESS_DROP: return _T("Chờ ngắt phiên ");
		case STAT_SESS_WARN: return _T("Bị lỗi kết nối ");
		case STAT_SESS_STOP: return _T("Sẽ kết nối lại ");
		case STAT_SESS_ERRO: return _T("Lỗi giao diện IP");
		default:			 return _T("...");
	}
}

//-----------------------------------------------------------------------------
CString C2_Host::GetStrLink()
{
	switch (m_Status)
	{
		case STAT_SESS_NONE: return _T("Không mở");
		case STAT_SESS_OPEN: return _T("Đang nối");	
		case STAT_SESS_WAIT: return _T("Chờ ... ");	
		case STAT_SESS_DROP: return _T("Chờ ngắt");
		case STAT_SESS_WARN: return _T("Cảnh báo");
		case STAT_SESS_STOP: return _T("Nối lại ");
		case STAT_SESS_ERRO: return _T("Lỗi mạng");
		default:			 return _T("...");
	}
}



//=============================================================================
//	C2_Local class
//=============================================================================
C2_Local::C2_Local()
{
	m_szRoot	= "";
	m_szFile	= "";
	m_szAccoun	= "";	
	m_szPasswd	= "";	

//	m_IsLinkOn	= false;
	m_LinkType	= 0;		// 0:Ethernet, 1:Modem
	m_szDialup	= "";
	m_IsCheck	= false;
	m_SelAdap	= 0;

	m_UserRole	= 0;
	m_szPwdTec	= "";
	m_szPwdSup	= "";

	m_AlTmPlan	= 30;
	m_AlTmShip	= 120;
	m_AlRgPlan	= 100;
	m_AlRgShip	= 50;
	m_AlTrPlan	= 0xFF;	
	m_AlTrShip	= 0xFF;
	m_AlarZone	= 0x0F;

	m_ReplType	= 0x0F;
	m_ReplIden	= 0x0F;
	m_RecdTime	= 600;
	m_ReplTAN	= 0;

	m_ComAIS    = 0;                 // Serial port of Rada ARPA
	m_ComARPA   = 0;                 // Serial port of AIS

	m_RadaPnt.m_Lat	 = long(20.827 * (1<<21)/45);
	m_RadaPnt.m_Long = long(106.72 * (1<<21)/45);

	m_preFrag = 0;
	m_NumFrag = 1;
	m_Leng	  = 0;

	// Session link
	m_HostRada.SetStrAddr(m_HostRada.m_RmtAddr, _T("192.168.1.90"));
	m_HostRada.m_RmtPort = PORT_SESS_RADA;
	
}

C2_Local::~C2_Local()
{}

CString C2_Local :: GetStrPort(BYTE nPort)
{
	if(nPort == 0)
		return _T(" --- ");

	CString szStr;
	szStr.Format(_T("COM %d"), nPort + 1);
	return szStr;
}

//-----------------------------------------------------------------------------
// by [1/128s]
long C2_Local::GetTimeUTC()
{
	SYSTEMTIME	nTime;

	GetLocalTime(&nTime);
	return (((nTime.wHour*3600 + nTime.wMinute*60 + nTime.wSecond) << 7) + 
			 (nTime.wMilliseconds << 4)/125);
}

//-----------------------------------------------------------------------------
//	Return Number of Adapters: 3 IP max
//-----------------------------------------------------------------------------
int C2_Local::GetAddrLocal(BYTE nMax, CString *szAddr)
{
	int		 nAdap;
	char	 szName[255];
	char	 szIP[16];
	HOSTENT *pHostEnt;
	LPSTR	 lpszIP;
	struct	 sockaddr_in sAddr;

	memset(szName, 0, 255);
	if (gethostname(szName, sizeof(szName)) == SOCKET_ERROR)
		return 0;

	if (0 == (pHostEnt = gethostbyname(szName)))
		return 0;

	nAdap = 0;
	while (pHostEnt->h_addr_list[nAdap] && (nAdap < nMax))
	{
		memcpy(&sAddr.sin_addr.s_addr, pHostEnt->h_addr_list[nAdap], pHostEnt->h_length);
		lpszIP = inet_ntoa(sAddr.sin_addr);

		if (lpszIP)
		{
			strcpy_s(szIP, 16, (char *)lpszIP);
			szAddr[nAdap] = szIP;			
		}
		nAdap ++; 
	}

	return nAdap;
}

//-----------------------------------------------------------------------------
// Get Name of PC
CString C2_Local::GetNameHost()
{
	TCHAR	szHost[255];
	CString	szName;
	DWORD	nSize;

	nSize = sizeof(szHost);
	if (GetComputerName((LPWSTR)szHost, &nSize))
		szName = szHost;
	else
		szName = "";

	return szName;
}


//-----------------------------------------------------------------------------
bool C2_Local::CheckPwd(BYTE nRole, CString szPwd)
{
	if (nRole == 1)	// Technical
	{
		if (szPwd.Compare(m_szPwdTec) == 0)
			return true ;
		else
			return false;
	}
	else
	if (nRole == 2)	// Supervisor
	{
		if (szPwd.Compare(m_szPwdSup) == 0)
			return true;
		else
			return false;
	}
	else			// Operator
		return true;
}

//-----------------------------------------------------------------------------
void C2_Local::ChangePwd(BYTE nRole, CString szNewPwd)
{
	if (nRole == 1)
		m_szPwdTec = szNewPwd;
	else 
	if (nRole == 2)
		m_szPwdSup = szNewPwd;

	SaveFile();
}
//
////-----------------------------------------------------------------------------
//void C2_Local::LoadNati()
//{
//	m_CNation.m_szRoot	= m_szRoot;
//	m_CNation.m_szFile	= _T("IdNation.txt");
//	m_CNation.LoadFile();
//}

//-----------------------------------------------------------------------------
//	Get file info
//-----------------------------------------------------------------------------
bool C2_Local::LoadFile()
{
	CFile	hFile ;				// Handle of file
	CString	szPath;

	szPath = m_szRoot + _T("\\") + m_szFile;
	if (!hFile.Open(szPath, CFile::modeRead))	// No file / Not load
		return false;

	BYTE	szBuff[512];		// Max = 512 byte
	BYTE	szTemp[ 64];
	int		nPos, nLen;
	bool	bOpen = true;		// Open file

	nLen = hFile.Read(szBuff, 512);
	hFile.Close();

	// 0: Check header: (8 byte = 0xFF)
	nPos = 0;
	while ((nPos < 8) && bOpen)
	{
		if (szBuff[nPos] != 0xFF)
			bOpen = false;		// Error of Header	
		else
			nPos ++;
	}

	// 1: Check CAT - LEN (1+2 byte) and (5 byte = 0x00)
	if (bOpen)
	{
		if (szBuff[nPos] != 5)	// CAT = 5: this File Info
			bOpen = false;		// Error of CAT
		else
			if (nLen != ((szBuff[nPos+1] << 8) | szBuff[nPos+2]))
				bOpen = false;	// Error of LEN

		nPos += 3;

		int	i = 0;
		while ((i < 5) && bOpen)
		{
			if (szBuff[nPos+i] != 0x00)
				bOpen = false;	// Error of End header
			else
				i ++;
		}
		nPos += 5;	
	}

	if (!bOpen)
		return false; 	

	::MakeStrCode(&szBuff[nPos], 256);

	// 1: Password of Login user	- 32 byte
	memcpy(szTemp, &szBuff[nPos], 16);		// Technical
	szTemp[16]	= 0x00;
	m_szPwdTec  = szTemp;
	nPos += 16;

	memcpy(szTemp, &szBuff[nPos], 16);		// Supervisor
	szTemp[16]	= 0x00;
	m_szPwdSup  = szTemp;
	nPos += 16;

	// 2: IP address of interface	- 40 byte
	memcpy(m_HostServ.m_LocAddr, &szBuff[nPos], 4);
	nPos += 4;
	memcpy(m_HostServ.m_RmtAddr, &szBuff[nPos], 4);
	nPos += 4;

	memcpy(szTemp, &szBuff[nPos], 16);		// Account  = 16 byte
	szTemp[16]	= 0;
	m_szAccoun	= szTemp;
	nPos += 16;

	memcpy(szTemp, &szBuff[nPos], 16);		// Password = 16 byte
	szTemp[16]	= 0;
	m_szPasswd	= szTemp;
	nPos += 16;

	// 3: Dial-up type		- 38 byte
	m_LinkType	= szBuff[nPos];
	nPos += 2;
	m_SelAdap	= m_LinkType & 1;
	memcpy(szTemp, &szBuff[nPos], 36);		// max leng = 36 byte
	szTemp[36]	= 0;
	m_szDialup	= szTemp;
	nPos += 36;

	// 3: Warning infor.	- 12 byte
	m_AlTmPlan	= (szBuff[nPos  ] << 8) | szBuff[nPos+1];
	m_AlTmShip	= (szBuff[nPos+2] << 8) | szBuff[nPos+3];
	nPos += 4;

	m_AlRgPlan	= (szBuff[nPos  ] << 8) | szBuff[nPos+1];
	m_AlRgShip	= (szBuff[nPos+2] << 8) | szBuff[nPos+3];
	nPos += 4;

	m_AlTrPlan	= szBuff[nPos  ];
	m_AlTrShip	= szBuff[nPos+1];
	m_AlarZone	= szBuff[nPos+2];
	nPos += 4;

	// 4: Replay type	- 4 byte
	m_ReplType	= szBuff[nPos  ];
	m_ReplIden	= szBuff[nPos+1];
	m_RecdTime	=(szBuff[nPos+2] << 8) | szBuff[nPos+3];
	nPos += 4;

	// 5: ComInfo		- 6 byte	
    for (int i = 0; i < 16; i++)
    {
		m_ComInfo[i].m_NumbPort	= i+1;
		m_ComInfo[i].m_IdBaudRat= szBuff[nPos+1];
		m_ComInfo[i].m_IdBitData= szBuff[nPos+2];
		m_ComInfo[i].m_IdBitStop= szBuff[nPos+3];
		m_ComInfo[i].m_IdParity	= szBuff[nPos+4];
		m_ComInfo[i].m_IdFlwCtrl= szBuff[nPos+5];
		nPos += 6;
    }

	// Connect to ARPA/AIS
	m_ComAIS	= szBuff[nPos++];	
	m_ComARPA	= szBuff[nPos++];
	
    // Tọa độ Rada
	m_RadaPnt.m_Lat	= (szBuff[nPos  ] <<16) | (szBuff[nPos+1] << 8) | szBuff[nPos+2];
	m_RadaPnt.m_Long= (szBuff[nPos+3] <<16) | (szBuff[nPos+4] << 8) | szBuff[nPos+5];
	nPos += 6;

	//6: Connect Rada by UDP
	memcpy(m_HostRada.m_RmtAddr, &szBuff[nPos], 4);
	nPos += 4;

	m_HostRada.m_RmtPort = (szBuff[nPos  ] << 8) | szBuff[nPos+1];
	nPos += 2;

	return true;
}

//-----------------------------------------------------------------------------
// Save file Info
//-----------------------------------------------------------------------------
bool C2_Local::SaveFile()
{
	CFile		hFile;
	CFileStatus	hStat;			// handle of file status
	CString		szPath;			//

	szPath = m_szRoot + _T("\\") + m_szFile;

	// Clear the Read Only 
	if (CFile::GetStatus(szPath, hStat))
	{
		hStat.m_attribute = 0x00;
		CFile::SetStatus(szPath, hStat);
	}

	if (!hFile.Open(szPath, CFile::modeCreate | CFile::modeWrite))
		return false;	

	// Read from file
	BYTE	szBuff[512];		// max = 512 byte
	int		nPos;

	memset(szBuff, 0, 512);

	// Header: 16 byte
	nPos = 0;
	memset(&szBuff[nPos], 0xFF, 8);
	nPos += 8;
	nPos += 3;	// CAT - LEN
	memset(&szBuff[nPos], 0x00, 5);
	nPos += 5;

	// 1: Save password:	- 32
	::ConvStrBuff(m_szPwdTec, &szBuff[nPos], 16);	// Technical
	nPos += 16;

	::ConvStrBuff(m_szPwdSup, &szBuff[nPos], 16);	// Supervisor
	nPos += 16;

	// 2: Save IP address of interface	- 40
	memcpy(&szBuff[nPos], m_HostServ.m_LocAddr, 4);
	nPos += 4;
	memcpy(&szBuff[nPos], m_HostServ.m_RmtAddr, 4);
	nPos += 4;

	::ConvStrBuff(m_szAccoun, &szBuff[nPos], 16);	// Account
	nPos += 16;

	::ConvStrBuff(m_szPasswd, &szBuff[nPos], 16);	// Password
	nPos += 16;

	// 3: Link dial-up	- 38
	szBuff[nPos  ]	= m_LinkType;
	nPos += 2;

	ConvStrBuff(m_szDialup, &szBuff[nPos], 36);
	nPos += 36;

	// 3: Warning infor.	- 12
	szBuff[nPos++]	= BYTE(m_AlTmPlan >> 8);
	szBuff[nPos++]	= BYTE(m_AlTmPlan	  );
	szBuff[nPos++]	= BYTE(m_AlTmShip >> 8);
	szBuff[nPos++]	= BYTE(m_AlTmShip	  );

	szBuff[nPos++]	= BYTE(m_AlRgPlan >> 8);
	szBuff[nPos++]	= BYTE(m_AlRgPlan	  );
	szBuff[nPos++]	= BYTE(m_AlRgShip >> 8);
	szBuff[nPos++]	= BYTE(m_AlRgShip	  );

	szBuff[nPos++]	= m_AlTrPlan;
	szBuff[nPos++]	= m_AlTrShip;
	szBuff[nPos++]	= m_AlarZone;
	szBuff[nPos++]	= 0;

	// 4: Replay type	- 4
	szBuff[nPos++]	= m_ReplType;
	szBuff[nPos++]	= m_ReplIden;
	szBuff[nPos++]	= BYTE(m_RecdTime >> 8);
	szBuff[nPos++]	= BYTE(m_RecdTime	  );

	// 5: ComInfo		- 6 byte	

	for (int i = 0; i < 16; i++)
	{
		szBuff[nPos  ]	= m_ComInfo[i].m_NumbPort ;	
		szBuff[nPos+1]	= m_ComInfo[i].m_IdBaudRat;
		szBuff[nPos+2]	= m_ComInfo[i].m_IdBitData;
		szBuff[nPos+3]	= m_ComInfo[i].m_IdBitStop;
		szBuff[nPos+4]	= m_ComInfo[i].m_IdParity ;
		szBuff[nPos+5]	= m_ComInfo[i].m_IdFlwCtrl;
		nPos += 6;
	}

	// Connect to ARPA/AIS - User ports
// 	memcpy(&szBuff[nPos], m_ComUsed, 2);
// 	nPos += 2;
	szBuff[nPos++] = m_ComAIS ;
	szBuff[nPos++] = m_ComARPA;	

	szBuff[nPos  ]	= BYTE(m_RadaPnt.m_Lat  >> 16);
	szBuff[nPos+1]	= BYTE(m_RadaPnt.m_Lat  >>  8);
	szBuff[nPos+2]	= BYTE(m_RadaPnt.m_Lat		 );
	szBuff[nPos+3]	= BYTE(m_RadaPnt.m_Long >> 16);
	szBuff[nPos+4]	= BYTE(m_RadaPnt.m_Long >>  8);
	szBuff[nPos+5]	= BYTE(m_RadaPnt.m_Long		 );
	nPos += 6;

	//6: Connect Rada by UDP
	memcpy(&szBuff[nPos], m_HostRada.m_RmtAddr, 4);
	nPos += 4;

	szBuff[nPos  ] = BYTE(m_HostRada.m_RmtPort >> 8);
	szBuff[nPos+1] = BYTE(m_HostRada.m_RmtPort     );
	nPos += 2;

	::MakeStrCode(&szBuff[32], 256);

	szBuff[ 8] = 5;					// CAT = 5 Area
	szBuff[ 9] = BYTE (nPos >> 8);	// LEN
	szBuff[10] = BYTE (nPos     );

	hFile.Write(szBuff, nPos);	
	hFile.Close();

	return true;
}

//-----------------------------------------------------------------------------
//	Data from ComPort: ($--TTM,xx,x.x,x.x,a,x.x,x.x,a,x.x,x.x,a,c--c,a,a*hh)
//  Hàm GetTrackRada để lọc tham số quỹ đạo mục tiêu của rada ARPA
//-----------------------------------------------------------------------------
bool C2_Local::GetTrackRada(BYTE *szBuff, int nLeng, C2_Track* pTrack)
{	
	// Get track from message - format:($--TTM ....<CR><LF>)
	CString szStr;	
	int nPos = 0;                       
	int nTmp;

	// 0: Get header : $RATMM
	szStr = "";
	while (nPos < nLeng)
	{ 
		if(szBuff[nPos]==',')
			break;
		szStr += szBuff[nPos++];
	}
	nPos ++;

	if(szStr.Compare(_T("$RATTM"))!= 0)              // Find $RATMM.
		return false;                                // Not TTM message

   //1: Track/ target number 00 - 99                 // Số hiệu mục tiêu
	szStr = "";
    while (nPos < nLeng)
    {
		if(szBuff[nPos]== ',')
			break;
		szStr += szBuff[nPos++];
    }
	nPos ++;
	pTrack -> m_TRN = (szStr[0] - 0x30)*10 + (szStr[1] - 0x30);
	pTrack->m_Fspec[0]	|= FRN_4;

	//2: Target Distance: X.x nm
	int nRge;  // 0.001nm
	UINT nAzi;

	szStr = "";
	while(nPos < nLeng)
	{
		if(szBuff[nPos]==',')
			break;
		szStr += szBuff[nPos++];
	}
	nPos ++;
	nRge = _wtof(szStr) * 1000;	
	//nRge = (szStr[0] - 0x30)*100 + (szStr[2] - 0x30)*10 + (szStr[3] - 0x30);	

	//3: Bearing /Head: X.x degree
	szStr = "";
	while (nPos < nLeng)
	{
		if(szBuff[nPos] == ',')
			break;
		szStr += szBuff[nPos++];
	}
	nAzi = _wtof(szStr) * (1<<13)/45;           // _wtof convert character string to a duble-precision.
	nPos++;

	m_RadaPnt.ConvPolToWGS(pTrack, nRge, nAzi); //Convert Polar(Range, Azimuth) to WGS(Lat, Long)
	pTrack->m_Fspec[1]	|= FRN_8;

	// 4: Unit of bearing: T/R (True/Relative)
	nPos +=2;
	 
	// 5: Track speed: X.x knot
	szStr = "";
	while(nPos<nLeng)
	{
		if(szBuff[nPos]==',')
			break;
		szStr += szBuff[nPos++];
	}
	nPos++;

	//nTmp = (szStr[0] - 0x30) + (szStr[2] - 0x30)*0.1 + (szStr[3] - 0x30)*0.01 ;
	pTrack->m_Speed = UINT(_wtof(szStr)*10);			// 0.1nm/hour
	pTrack->m_Fspec[1]	|= FRN_9;

	//6: Target Course: X.x degree
	szStr = "";
	while(nPos<nLeng)
	{
		if(szBuff[nPos]==',')
			break;
		szStr += szBuff[nPos++];
	}
	nPos++;

	pTrack->m_Head = int (_wtof(szStr)* (1<<13)/ 45);			// 2^16/360
	pTrack->m_Fspec[1]	|= FRN_9;

	// 7: Unit of Course: T/R (True/Relative)
	nPos += 2;

	// 8: Distance of CPA: X.x nm
	szStr = "";
	while (nPos < nLeng)
	{
		if (szBuff[nPos] == ',')
			break;
		szStr += szBuff[nPos++];
	}
	nPos ++;
	nTmp = (szStr[0] - 0x30) + (szStr[2] - 0x30)*0.1 + (szStr[3] - 0x30)*0.01;
	//nTmp = _wtof(szStr);

	// 9: Time until CPA: minute,  '-' = move away
	szStr = "";
	while (nPos < nLeng)
	{
		if (szBuff[nPos] == ',')
			break;
		szStr += szBuff[nPos++];
	}
	nPos ++;
	nTmp = int(_wtof(szStr));	

	// 10: Units of Speed/Distance: K/N/S 
	nPos += 2;

	// 11: Target name
	szStr = "";
	while (nPos < nLeng)
	{
		if (szBuff[nPos] == ',')
			break;
		szStr += szBuff[nPos++];
	}
	pTrack->m_szName = szStr;
	pTrack->m_Fspec[3]	|= FRN_23;
	nPos ++;	

	// 12: Target status: L/Q/T (Lost, Query, Tracking)
	pTrack->m_StMain = szBuff[nPos];
	pTrack->m_Fspec[0]	|= FRN_6;
	nPos += 2;

	// 13: Reference target = R

	// 14: Time of data (hhmmss.ss)

	// 15: Type of target acquisition A / M (Automatic / Manual)

	// 16: Checksum

	return true;
}

//-----------------------------------------------------------------------------
// OnLinkBuff  AIS: Xử lý gộp các gói tin nhiều mảnh
//-----------------------------------------------------------------------------
int C2_Local::OnLinkBuff(BYTE *szBuff, int nLeng)
{
	CString  szStr;
	int      nPos;
	int      nEndOfData;
	int      nNumFrag , nCurFrag;
	int      nComm = 0;                  // Số dấu phẩy ','

	//0: Get header : !AIVDM and !AIVDO
	szStr  = "";
	nPos   = 0 ;
	while(nPos < nLeng)
	{
		if(szBuff[nPos] == ',')
			break;
		szStr += szBuff[nPos++];
	}
	nComm++;
	nPos++;

	if((szStr.Compare(_T("!AIVDM")) != 0) && (szStr.Compare(_T("!AIVDO")) != 0))
		return 0;                       //Not AIVDM or !AIVDO

	//1: Count of fragment
	nNumFrag   = szBuff[nPos++] - 48;   // Tổng số mảnh
	m_NumFrag  = nNumFrag; 
	nComm++;
	nPos++;

	nCurFrag   = szBuff[nPos++] - 48;    // Số thứ tự mảnh
	nComm++;
	nPos++;

	// Find the begin of data payload
	while (nComm < 5)
	{
		while ((nPos < nLeng) && (szBuff[nPos] != ','))
			nPos++;
	
		nPos++;
		nComm++;
	}

 	if(nPos > nLeng)
		return 0;             // Error in buffer

	// Find the end of data payload
	nEndOfData = nPos;
	while((nEndOfData < nLeng) && (szBuff[nEndOfData] != ','))
		nEndOfData ++;

	if (m_NumFrag > 1)        // Tổng số mảnh nhiều hơn 1
	{
		if (nCurFrag == 1)    // Frag #1
		{
			memcpy(&m_Buff, &szBuff[nPos], nEndOfData - nPos);
			m_Leng = nEndOfData - nPos;
			m_preFrag = 1;
			return 0;
		}

		if(m_preFrag != 1)     // If msg is error: don't have Frag #1
			return 0;

		memcpy(&m_Buff[m_Leng], &szBuff[nPos], nEndOfData - nPos);
		m_Leng += nEndOfData - nPos;
		m_preFrag = 2;
	}
	else                        // Only 1 fragment
	{
		memcpy(&m_Buff, &szBuff[nPos], nEndOfData - nPos);  // Copy truong du lieu vao buff
		m_Leng = nEndOfData - nPos;
	}

	return 1;
}

//-----------------------------------------------------------------------------
//	Track from AIS: 
//	IdSrc | IdMsg | Class | MMSI  | IMO   | IdNati| VesTyp| DimA  | DimB  | Dim C | 
//	DimD  | Draug | ROT   | Speed | Head  | Long  | Lati  | NavSt | HeadTr| CallSg|
//	Name  | ETArr |	Desti | TimeRx| 
//-----------------------------------------------------------------------------
int C2_Local::GetTrackAIS(BYTE *szBuff, int nLeng, C2_Track* pTrack)
{
	int      nPos = 0;       //Read position in buffer  	 
	BYTE     nMssg = 0;      //Type of AIS message
	C2_Track nTrack;
 

	//1: Message ID: 1-2 byte (1,2,3,4,5,18,19)
	nMssg = Decode(szBuff[nPos++]);	 

	//Check type of AIS message
	if((nMssg != 1)&&(nMssg!= 2)&&(nMssg!= 3)&&(nMssg!= 4)&&(nMssg!=5)&&
	   (nMssg !=18)&&(nMssg!=19)&&(nMssg!=21)&&(nMssg!=24))
	 return 0;

	pTrack->m_TypAIS = nMssg;

	if ((nMssg == 1)|(nMssg == 2)|(nMssg == 3))	    // Position Report Class A
	{
		if (!GetTrkClassA(szBuff, nPos, &nTrack))
			return 0;		
		pTrack->Update(&nTrack);
		return 1;
	}

	if (nMssg == 4)                                 // Type 4: base station report
	{
		if (!GetTrkStatio(szBuff, nPos, &nTrack))
			return 0;

		pTrack->Update(&nTrack);
		return 1;
	}

	if (nMssg == 5)                                // Type 5: static and voyage data
	{
		if(!GetTrkStatic(szBuff, nPos, &nTrack))
			return 0;
		pTrack->Update(&nTrack);
		return 1;
	}

	if (nMssg == 18)		                       // Standard Class B CS Posittion Report
	{
		if (!GetTrkClassB(szBuff, nPos, &nTrack))
		 return 0;

		pTrack->Update(&nTrack);
		
		return 1;
	}

	if (nMssg == 19)                               // Extended Class B CS Position Report
	{
		if (!GetTrkClasEB(szBuff, nPos, &nTrack))
			return 0;

		pTrack->Update(&nTrack);
		
		return 1;
	}

	if (nMssg == 21)                               // Type = 21 Aid - to - Navigation Report
	{
		if (!GetTrkNaviga(szBuff, nPos, &nTrack))
			return 0;

		pTrack->Update(&nTrack);
		return 1;
	}

	if (nMssg == 24)                              // Type = 24 Static data report
	{
		if (!GetTrkType24(szBuff, nPos, &nTrack))
			return 0;

		pTrack->Update(&nTrack);
		return 1;
	}

	return 1;
}

BYTE C2_Local::Decode(BYTE nBuff)
{
	if (nBuff < 48)
		return nBuff;

	if ((nBuff - 48)>40)
		return (nBuff - 56);	

	return (nBuff - 48);
}

int	C2_Local::GetTrkClassA(BYTE* szBuff, int nPos, C2_Track* pTrack)
{
	CString szNum;
	int     nLen;

	// 1: Type AIS
	pTrack->m_Fspec[0] |= FRN_1;
	pTrack->m_Fspec[2] |= FRN_15;

	// 2: Repeat indicator: bit 6, 7
	int rep_ind;
	rep_ind = Decode(szBuff[nPos]) & 0x30;

	// 3: MMSI: bit 8-37
	long nMMSI;
	nMMSI = ((Decode(szBuff[nPos  ]) & 0x0F) << 26)|((Decode(szBuff[nPos+1]) & 0x3F) <<20)|
		    ((Decode(szBuff[nPos+2]) & 0x3F) << 14)|((Decode(szBuff[nPos+3]) & 0x3F) << 8)|
		    ((Decode(szBuff[nPos+4]) & 0x3F) <<  2)|((Decode(szBuff[nPos+5]) & 0x30) >> 4) ;
	
	nPos += 5;

	szNum.Format(_T("%d"), nMMSI);
	nLen = szNum.GetLength();

	if((nLen < 4) || (nLen > 9))
		return 0;
	::ConvStrBuff(szNum, pTrack->m_MMSI, 9);
	pTrack->m_Fspec[2] |= FRN_16;	

	if      (nLen == 9)
		pTrack->m_Type = 0x12;      // Auto + Vessel
	else if (nLen == 7)
		pTrack->m_Type = 0x13;      // Auto + Terrain
	else
		pTrack->m_Type = 0x10;      // Auto + Default

	// 4: Navigator status: bit 38 - 41
	BYTE nNaviSt = Decode(szBuff[nPos++]) & 0x0F;
	pTrack->m_NaviSt    = nNaviSt;
	pTrack->m_Fspec[1] |= FRN_12;

	// 5: Rate of turn: bit 42 - 49
	int nROT;
	nROT = ((Decode(szBuff[nPos]) & 0x3F) << 2)|
		   ((Decode(szBuff[nPos+1]) & 0x30)>>4);

	if (int(nROT) <= 126)				                    // [deg/min] - convert
		pTrack->m_ROT	= UINT((nROT/4.733)*(nROT/4.733));
	else
		if (int(nROT) >= 128)				                // No information
			pTrack->m_ROT	= 0;
		else
			if (int(nROT) == 127)				            // > 10deg / min
				pTrack->m_ROT	= 10;

	pTrack->m_Fspec[2]	   |= FRN_18;
	nPos ++;

	// 6: Speed over ground: 50 - 59
	pTrack->m_Speed = ((Decode(szBuff[nPos]) & 0x0F) << 6)|(Decode(szBuff[nPos+1]) & 0x3F); 
	pTrack->m_Fspec[1]	   |= FRN_9;
	nPos += 2;

	// 7: Position accuracy: bit 60
	bool bPos = ((Decode(szBuff[nPos]) & 0x20) >> 5) & 1;
	
	// 8: Long: 61 - 88
	long nLong = ((Decode(szBuff[nPos  ]) & 0x1F) << 23) | ((Decode(szBuff[nPos+1]) & 0x3F)<<17) |
				 ((Decode(szBuff[nPos+2]) & 0x3F) << 11) | ((Decode(szBuff[nPos+3]) & 0x3F)<< 5) |
				  (Decode(szBuff[nPos+4]) >>1)&0x1F;	
	pTrack->m_Long      = long(nLong / 421875.0 * (1 << 15));        // Minute
	pTrack->m_Fspec[1] |= FRN_8;
	nPos += 4; 

	// 9: Lat: 89 - 115
	long nLat;
	nLat = ((Decode(szBuff[nPos  ]) & 1   ) << 26) | ((Decode(szBuff[nPos+1]) & 0x3F)<<20) |
		   ((Decode(szBuff[nPos+2]) & 0x3F) << 14) | ((Decode(szBuff[nPos+3]) & 0x3F)<< 8) |
	       ((Decode(szBuff[nPos+4]) & 0x3F) <<  2) | ((Decode(szBuff[nPos+5]) & 0x30)>> 4);
	pTrack->m_Lat  =  long(nLat     / 421875.0 * (1 << 15));        // degree//* bit23/180;
	nPos += 5;

	// 10: Course over ground: 116 - 127 :10
	int nCOG;
	nCOG = ((Decode(szBuff[nPos  ]) & 0x0F) << 8) | ((Decode(szBuff[nPos+1]) & 0x3F) << 2)|
	       ((Decode(szBuff[nPos+2]) & 0x03) >> 4);
	nPos += 2;

	// 11: true heading: 128 - 136
	int nHeadTr;
	nHeadTr = ((Decode(szBuff[nPos ]) & 0x0F) << 5) | ((Decode(szBuff[nPos+1]) & 0x3E)/2);
	pTrack->m_HeadTr = nHeadTr * bit15/ 180;
	pTrack->m_Fspec[2] |= FRN_19;
	nPos ++;

	//12: Time Stamp: 137 - 142
	BYTE m_TimeStp;
	m_TimeStp = ((Decode(szBuff[nPos]) & 1) << 5) | ((Decode(szBuff[nPos+1]) & 0x3E )/2); 
	nPos ++;

	// 13: Maneuver indicator: 143 -144
	BYTE m_MaIndi;
	m_MaIndi  = ((Decode(szBuff[nPos]) & 1)*2) | ((Decode(szBuff[nPos+1]) & 0x20)>>5);
	nPos ++;

	// 14: Spare: 145 - 147
	BYTE m_Spare;
	m_Spare = (Decode(szBuff[nPos]) & 0x1C) >> 2;

	// 15:RAIM flag: bit 148
	bool bRAIM = ((Decode(szBuff[nPos]) & 0x02) >> 1) & 1;

	// 16: Radio status: 149 - 167
	long nRadiSt;
	nRadiSt = ((Decode(szBuff[nPos]) & 0x01) << 18) |((Decode(szBuff[nPos+1]) & 0x3F) << 12)|
	((Decode(szBuff[nPos+2]) & 0x3F) << 6)|(Decode(szBuff[nPos+3]) & 0x3F);
	nPos += 3;	
	
	return 1;

}
//-----------------------------------------------------------------------------
int C2_Local::GetTrkStatio(BYTE* szBuff, int nPos, C2_Track* pTrack)
{
	CString szNum;
	int      nLen;

	// 1: TypeAIS
	pTrack->m_Fspec[0] |= FRN_1;
	pTrack->m_Fspec[2] |= FRN_15;

	// 2: Repeat indicator: bit 6, 7
	int rep_ind;
	rep_ind = Decode(szBuff[nPos]) & 0x30;

	// 3: MMSI: bit 8-37
	long nMMSI;
	nMMSI = ((Decode(szBuff[nPos  ]) & 0x0F) << 26)|((Decode(szBuff[nPos+1]) & 0x3F)<<20)|
		    ((Decode(szBuff[nPos+2]) & 0x3F) << 14)|((Decode(szBuff[nPos+3]) & 0x3F)<<8) |
		    ((Decode(szBuff[nPos+4]) & 0x3F) <<  2)|((Decode(szBuff[nPos+5]) & 0x30)>>4);
	nPos += 5;

	szNum.Format(_T("%d"), nMMSI);
	nLen = szNum.GetLength();

	if ((nLen < 4)||(nLen > 9))
		return 0;

	::ConvStrBuff(szNum, pTrack->m_MMSI, 9);
	pTrack->m_Fspec[2] |= FRN_16;

	if		(nLen == 9)
		pTrack->m_Type	= 0x12;			// Auto + Vessel
	else if (nLen == 7)
		pTrack->m_Type	= 0x13;			// Auto + Terrain
	else
		pTrack->m_Type	= 0x10;			// Auto + Default	

	// 4: Year (UTC): bit 38-51
	int nYear;
	nYear = ((Decode(szBuff[nPos]) & 0x0F)<< 10)|((Decode(szBuff[nPos+1]) & 0x3F)<< 4)|
		    ((Decode(szBuff[nPos+2]) >> 2)& 0x0F);
	nPos += 2;

	// 5: Month(UTC): bit 52-55
	int nMonth;
	nMonth = ((Decode(szBuff[nPos]) & 0x03) << 2)|((Decode(szBuff[nPos+1]) & 0x30) >> 4); 
	nPos ++;

	// 6: Day (UTC): bit 56-60
	int nDay;
	nDay = ((Decode(szBuff[nPos]) & 0x0F) << 1)|((Decode(szBuff[nPos+1]) & 0x02) >> 5); 
	nPos ++;

	// 7: Hour(UTC): bit 61-65
	int nHour;
	nHour = Decode(szBuff[nPos]) & 0x1F;
	nPos++;

	// 8: Minute(UTC): Bit 66-71
	int nMin;
	nMin = Decode(szBuff[nPos]) & 0x3F;
	nPos++;

	// 9: Second (UTC): 72-77
	int nSecond;
	nSecond = Decode(szBuff[nPos]) & 0x3F;
	nPos++;

	// 10: fix quality: bit 78
	bool bQual = (Decode(szBuff[nPos]) >> 5) & 1;

	// 11: Longitude:bit 79-106
	long nLong = ((Decode(szBuff[nPos  ]) & 0x1F) << 23) | ((Decode(szBuff[nPos+1]) & 0x3F) <<17)  |
		         ((Decode(szBuff[nPos+2]) & 0x3F) << 11) | ((Decode(szBuff[nPos+3]) & 0x3F) << 5)|
		         ((Decode(szBuff[nPos+4]) & 0x3E)/2);	
	pTrack->m_Long = long(nLong / 421875.0 * (1 << 15));         // minute
	pTrack->m_Fspec[1] |= FRN_8;
	nPos += 4; 

	//12: Latitude: bit 107-133
	long nLat;
	nLat = ((Decode(szBuff[nPos]  ) &    1) << 26)|((Decode(szBuff[nPos+1]) & 0x3F) << 20) |
		   ((Decode(szBuff[nPos+2]) & 0x3F) << 14)|((Decode(szBuff[nPos+3]) & 0x3F) <<  8) |
		   ((Decode(szBuff[nPos+4]) & 0x3F) <<  2)|((Decode(szBuff[nPos+5]) & 0x30) <<  4) ;
    pTrack->m_Lat = long(nLong / 421875.0 * (1 << 15));       // degree//* bit23/180;
	nPos += 5;

	// 13: Type of EPFD:bit  134-137
	BYTE m_EPFD;
	m_EPFD = Decode(szBuff[nPos]) & 0x0F;
	nPos ++;

	// 14: Spare :bit 138-147
	BYTE m_Spare;
	m_Spare = ((Decode(szBuff[nPos])& 0x3F) << 4) | ((Decode(szBuff[nPos+1])>>2)& 0x0F);
	nPos++;

	// 15:RAIM flag: bit 148
	bool bRAIM = (Decode(szBuff[nPos]) >> 1) & 1;

	// 16: SOTDMA state: 149- 167
	long nRadiSt;
	nRadiSt = ((Decode(szBuff[nPos  ]) & 0x01) << 18) | ((Decode(szBuff[nPos+1]) & 0x3F) << 12)|
		      ((Decode(szBuff[nPos+2]) & 0x3F) <<  6) |  (Decode(szBuff[nPos+3]) & 0x3F);
	nPos += 3;	

	return 1;
}

int C2_Local::GetTrkStatic(BYTE* szBuff, int nPos, C2_Track* pTrack)
{
	CString	szNum;
	int     nLen;

	//1: Type AIS
	pTrack->m_Fspec[0]   |= FRN_1;
	pTrack->m_Fspec[2]   |= FRN_15;
    
	//2: Repeat indicator: bit 6,7
	int rep_ind;
	rep_ind = Decode(szBuff[nPos]) & 0x30;

	//3: MMSI: bit 8 - 37
	long nMMSI;
	nMMSI = ((Decode(szBuff[nPos]) & 0x0F)   << 26)| ((Decode(szBuff[nPos+1])&0x3F)<<20)|
		    ((Decode(szBuff[nPos+2]) & 0x3F) << 14)| ((Decode(szBuff[nPos+3])&0X3F)<<8) |
			((Decode(szBuff[nPos+4]) & 0X3F) << 2) | ((Decode(szBuff[nPos+5])&0x30)>>4)  ;
	nPos += 5;

	szNum.Format(_T("%d"), nMMSI);
	nLen = szNum.GetLength();

	if ((nLen < 4)||(nLen > 9))
		return 0;

	::ConvStrBuff(szNum, pTrack->m_MMSI, 9);
	pTrack->m_Fspec[2] |= FRN_16;

	if		(nLen == 9)
		pTrack->m_Type	= 0x12;			// Auto + Vessel
	else if (nLen == 7)
		pTrack->m_Type	= 0x13;			// Auto + Terrain
	else
		pTrack->m_Type	= 0x10;			// Auto + Default

	//4:AIS Version bit 38 - 39
	BYTE nVersion = (Decode(szBuff[nPos])>> 2) & 0x03;

	//5: IMO number bit 40 -69
	long nIMO;
	nIMO = ((Decode(szBuff[nPos  ])& 0x03) << 28)|((Decode(szBuff[nPos+1])& 0x3F) << 22)|
		   ((Decode(szBuff[nPos+2])& 0x3F) << 16)|((Decode(szBuff[nPos+3])& 0x3F) << 10)|
		   ((Decode(szBuff[nPos+4])& 0x3F) << 4) |((Decode(szBuff[nPos+5])>> 2) & 0x0F) ;
	nPos += 5;

	szNum.Format(_T("%d"), nIMO);
	if (szNum.GetLength() > 7)
		return 0;

	::ConvStrBuff(szNum, pTrack->m_IMO, 7);

	pTrack->m_Fspec[2]	  |= FRN_20;
	
	//6: Call Sign bit 70 - 111	  : Hô hiệu
	char	sCallsign[8];
	sCallsign[7] = 0x00 ;	
	char ch;
	for (int j =0; j < 7; j++)
	{
		ch = (((Decode(szBuff[nPos + j    ]) << 4) & 0x03 )|
			  ((Decode(szBuff[nPos + j + 1]) >> 2) & 0x0F ));
		if (ch == 0)
		{
         sCallsign[j] = 0x00;
		 break;
		}
		if (ch < 32)    
			ch += 64;
		sCallsign[j] = ch;
	}
	pTrack->m_szClSg     = sCallsign;
	pTrack->m_Fspec[3]	|= FRN_22;
	nPos += 7;

	//7: Vessel Name : bit 112-231	
	char	szName[21];	
	szName[20] = 0x00;
	for (int j = 0; j < 20; j++)
	{
		ch = (((Decode(szBuff[nPos + j    ]) << 4) & 0x30)|
			  ((Decode(szBuff[nPos + j + 1]) >>2 ) & 0x0F));
		if (ch == 0)	               // '@' - finish
		{
			szName[j] = 0x00;
			break;
		}
		if (ch == 32)
			if (szName[j-1] == 32)	   // 2 spaces
			{
				szName[j-1] = 0x00;
				break;
			}

		if (ch < 32)
			ch += 64;
		szName[j] = ch;
	}
	pTrack->m_szName = szName;
	pTrack->m_Fspec[3]	|= FRN_23;
	nPos += 20;

	//8: Ship Type: bit 232-239 
	BYTE nType;
	nType = ((Decode(szBuff[nPos])& 0x03) << 6) | (Decode(szBuff[nPos + 1])& 0x3F);
	pTrack->m_SbType  = ConvShipType(nType);
	pTrack->m_Fspec[1]	|= FRN_11; 
	nPos += 2;

	//9: Dimension to Bow :bit 240-248
	int		nDimA, nDimB;
	BYTE	nDimC, nDimD;

	nDimA = ((Decode(szBuff[nPos  ]) & 0x3F)<<3) |
		    ((Decode(szBuff[nPos+1]) >> 3) & 0x07);
	nPos ++;

	//10: Dimension to Stern : bit 249-257
	nDimB = ((Decode(szBuff[nPos ]) & 0x07) << 6) | (Decode(szBuff[nPos+1]) & 0x3F);
	pTrack->m_Length	= (nDimA + nDimB) * 10;	  // [0.1m]
	pTrack->m_Fspec[2]	|= FRN_21;                // Chiều dài của tàu
	nPos += 2;

	//11: Dimension to Port:bit 258-263
	nDimC = Decode(szBuff[nPos]) & 0x3F;
	nPos ++;

	//12: Dimension to Starboard: bit 264-269
	nDimD = Decode(szBuff[nPos])&0x3F;
	pTrack->m_Width = (nDimC + nDimD) * 5;	      // [0.2m]
	pTrack->m_Fspec[2]	|= FRN_21;                // Chiều rộng của tàu
	nPos ++;

	//14: ETA month (UTC)  :bit 274 - 277
	BYTE   nMonth, nDay;
	BYTE   nHour, nMin;

	nMonth = ((Decode(szBuff[nPos])& 0x03)<<2) | ((Decode(szBuff[nPos+1])>>4) & 0x03);
	nPos ++;

	//15: ETA day (UTC)  : bit 278- 282

	nDay   = ((Decode(szBuff[nPos])& 0x0F)<<1) | ((Decode(szBuff[nPos+1])>>5)&1);
	nPos ++;

	//16: ETA hour (UTC) : bit 283-287

	nHour  = Decode(szBuff[nPos])& 0x1F;
	nPos ++;

	//17: ETA minute (UTC) : bit 288 - 293

	nMin   = Decode(szBuff[nPos])& 0x3F;
	nPos ++;

	if (nMin > 59)
	{
		nMin -= 60;
		nHour ++;
	}
	if (nHour > 23)
	{
		nHour = 24;
		nDay ++;
	}

	pTrack->m_TimEta[0] = nHour;
	pTrack->m_TimEta[1] = nMin;
	pTrack->m_TimEta[2] = nDay;
	pTrack->m_TimEta[3] = nMonth;

	pTrack->m_Fspec[3]	|= FRN_25;

	//18: Draught : bit 294-301
	BYTE nDraught;
	nDraught = ((Decode(szBuff[nPos]) & 0x3F) << 2) | ((Decode(szBuff[nPos+1]) >> 4) & 0x03);
	pTrack->m_Draugh    = nDraught;	   //[0.1m]
	pTrack->m_Fspec[2] |= FRN_21;
	nPos ++;

	//19: Destination : bit 302 - 421 (120bit)
	char	szDes[21];	
	szDes[20] = 0x00;
	for (int j = 0; j < 20; j++)
	{
		ch = (((Decode(szBuff[nPos + j    ]) & 0x0F) << 2)|
			  ((Decode(szBuff[nPos + j + 1]) & 0x30) >> 4));
		if (ch == 0)	             // '@' - finish
		{
			szName[j] = 0x00;
			break;
		}

		if (ch == 32)
			if (szDes[j-1] == 32)	// 2 spaces
			{
				szDes[j-1] = 0x00;
				break;
			}

		if (ch < 32)
			ch += 64;
		szDes[j] = ch;
	}	
	pTrack->m_szDest = szDes;
	pTrack->m_Fspec[3]	|= FRN_24;
	nPos += 20;
		
	return 1;
}

int C2_Local::GetTrkClassB(BYTE* szBuff, int nPos, C2_Track* pTrack)
{
	CString	szNum;

	// 1: TypeAIS
	pTrack->m_Fspec[0] |= FRN_1;
	pTrack->m_Fspec[2] |= FRN_15;

	// 2: Repeat indicator: bit 6, 7
	int rep_ind;
	rep_ind = Decode(szBuff[nPos]) & 0x30;

	// 3: MMSI: bit 8-37
	long nMMSI;
	nMMSI = ((Decode(szBuff[nPos])   & 0x0F)<<26)|((Decode(szBuff[nPos+1]) & 0x3F)<<20)|
		    ((Decode(szBuff[nPos+2]) & 0x3F)<<14)|((Decode(szBuff[nPos+3]) & 0x3F)<<8) |
		    ((Decode(szBuff[nPos+4]) & 0x3F)<< 2)|((Decode(szBuff[nPos+5]) & 0x30)>> 4) ;
	nPos += 5;

	szNum.Format(_T("%d"), nMMSI);
	if((szNum.GetLength() < 4) || (szNum.GetLength() > 9))
		return 0;

	::ConvStrBuff(szNum, pTrack->m_MMSI, 9);
	pTrack->m_Fspec[2] |= FRN_16;	

	// 4: Regional reserved: 38-45: not used	
	nPos++;

	// 5: Speed over ground: 46 - 55
	pTrack->m_Speed = ((Decode(szBuff[nPos  ]) & 0x03) << 8) | ((Decode(szBuff[nPos+1]) & 0x3F) <<2)|
		              ((Decode(szBuff[nPos+2]) >> 4) & 0x03);
	pTrack->m_Fspec[1]	   |= FRN_9;
	nPos += 2;

	// 6: position accuracy: bit 56
	bool bPos = ((Decode(szBuff[nPos]) & 0x20) >> 3) & 1;

	// 8: Long: 57 - 84
	long nLong = ((Decode(szBuff[nPos  ]) & 0x07) << 25)|((Decode(szBuff[nPos+1]) & 0x3F) <<19)|
				 ((Decode(szBuff[nPos+2]) & 0x3F) << 13)|((Decode(szBuff[nPos+3]) & 0x3F) << 7)|
				 ((Decode(szBuff[nPos+4]) & 0x3F) <<  1)|((Decode(szBuff[nPos+5]) >> 5) & 1);	
	pTrack->m_Long = long(nLong / 421875.0 * (1 << 15));     // degree = 600000*bit23/180;
	pTrack->m_Fspec[1]   |=FRN_8;
	nPos += 5; 

	// 9: Lat: 85 - 111
	long nLat;
	nLat = ((Decode(szBuff[nPos])   & 0x1F) << 22)|((Decode(szBuff[nPos+1]) & 0x3F) << 16)|
		   ((Decode(szBuff[nPos+2]) & 0x3F) << 10)|((Decode(szBuff[nPos+3]) & 0x3F) << 4)|
	       ((Decode(szBuff[nPos+4]) >> 2)& 0x0F);
	pTrack->m_Lat  = long(nLat / 421875.0 * (1 << 15));		// degree
	nPos += 4;

	// 10: Course over ground: 112 - 123
	int nCOG;
	nCOG = ((Decode(szBuff[nPos  ]) & 0x03) << 10) | ((Decode(szBuff[nPos+1]) & 0x3F) << 4)|
	       ((Decode(szBuff[nPos+2]) >> 2 ) & 0x0F);
	nPos += 2;

	// 11: True heading: 124 - 132
	int nHeadTr;
	nHeadTr = ((Decode(szBuff[nPos]) & 0x03) << 7)|((Decode(szBuff[nPos+1]) & 0x3F)<<1)|
		      ((Decode(szBuff[nPos+2]) >> 5) & 1) ;
	pTrack->m_HeadTr = nHeadTr * bit15/180;
	pTrack->m_Fspec[2]  |= FRN_19;
	nPos += 2;

	//12: Time Stamp: 133 - 138
	BYTE m_TimeStp;
	m_TimeStp = ((Decode(szBuff[nPos]) & 0x1F) << 1) |((Decode(szBuff[nPos+1]) >> 5 ) & 1);
	nPos ++;

	// 13: Regional reserved: 139 - 140: uninterpreted
	
	// 14: CS Unit: bit 141

	//... More fields
	
	return 1;
}
int C2_Local::GetTrkClasEB(BYTE* szBuff, int nPos, C2_Track* pTrack)
{
	CString	szNum;

	// 1: TypeAIS
	pTrack->m_Fspec[0] |= FRN_1;
	pTrack->m_Fspec[2] |= FRN_15;

	// 2: Repeat indicator: bit 6, 7
	int rep_ind;
	rep_ind = Decode(szBuff[nPos]) & 0x30;

	// 3: MMSI: bit 8-37
	long nMMSI;
	nMMSI = ((Decode(szBuff[nPos  ]) & 0x0F)<<26)|((Decode(szBuff[nPos+1]) & 0x3F)<<20)|
		    ((Decode(szBuff[nPos+2]) & 0x3F)<<14)|((Decode(szBuff[nPos+3]) & 0x3F)<<8)|
		    ((Decode(szBuff[nPos+4]) & 0x3F)<< 2)|((Decode(szBuff[nPos+5]) & 0x30)>>4) ;
	nPos += 5;

	szNum.Format(_T("%d"), nMMSI);
	if((szNum.GetLength() < 4) || (szNum.GetLength() > 9))
		return 0;

	::ConvStrBuff(szNum, pTrack->m_MMSI, 9);
	
	pTrack->m_Fspec[2] |= FRN_16;	

	// 4: Regional reserved: 38-45: not used	
	nPos++;

	// 5: Speed over ground: 46 - 55
	pTrack->m_Speed = (((Decode(szBuff[nPos  ]) & 0x03) << 8)|((Decode(szBuff[nPos+1]) & 0x3F) <<2)|
		               ((Decode(szBuff[nPos+2]) >>4   ) & 0x03));
	pTrack->m_Fspec[1]     |= FRN_9;
	nPos += 2;

	// 6: position accuracy: bit 56
	bool bPos = ((Decode(szBuff[nPos]) & 0x20) >> 3) & 1;

	// 8: Long: 57 - 84
	long nLong = ((Decode(szBuff[nPos  ]) & 0x07) << 25)|((Decode(szBuff[nPos+1])& 0x3F)<<19)|
				 ((Decode(szBuff[nPos+2]) & 0x3F) << 13)|((Decode(szBuff[nPos+3])& 0x3F)<< 7)|
				 ((Decode(szBuff[nPos+4]) & 0x3F) <<  1)|((Decode(szBuff[nPos+5]) >> 5) & 1);	
	pTrack->m_Long = long(nLong / 421875.0 * (1 << 15));
	nPos += 5; 

	// 9: Lat: 85 - 111
	long nLat;
	nLat = ((Decode(szBuff[nPos  ]) & 0x1F) << 22)|((Decode(szBuff[nPos+1]) & 0x3F) << 16)|
		   ((Decode(szBuff[nPos+2]) & 0x3F) << 10)|((Decode(szBuff[nPos+3]) & 0x3F) << 4)|
	       ((Decode(szBuff[nPos+4]) >> 2)& 0x0F);
	pTrack->m_Lat	= long(nLat / 421875.0 * (1 << 15));		// degree//* bit23/180;
	nPos += 4;

	// 10: Course over ground: 112 - 123
	int nCOG;
	nCOG = ((Decode(szBuff[nPos  ]) & 0x03) << 10) | ((Decode(szBuff[nPos+1]) & 0x3F) << 4)|
		   ((Decode(szBuff[nPos+2]) >> 2 ) & 0x0F);
	nPos += 2;

	// 11: true heading: 124 - 132
	int nHeadTr;
	nHeadTr = ((Decode(szBuff[nPos  ]) &  0x03) << 7)|((Decode(szBuff[nPos+1]) & 0x3F)<<1)|
			  ((Decode(szBuff[nPos+2]) >> 5   ) & 1) ;
	pTrack->m_HeadTr = nHeadTr * bit15/180;
	pTrack->m_Fspec[2]   |= FRN_19;
	nPos += 2;

	//12: Time Stamp: 133 - 138
	BYTE m_TimeStp;
	m_TimeStp = ((Decode(szBuff[nPos]) & 0x1F) << 1) |((Decode(szBuff[nPos+1]) >> 5 ) & 1);
	nPos ++;

	//... More

	return 1;
}

//-----------------------------------------------------------------------------
int C2_Local::GetTrkNaviga(BYTE* szBuff, int nPos, C2_Track* pTrack)
{
	CString  szNum;
	char     szStr[24];
	char     ch;
	int      nRep, j, nLen;
	long     nLong;
	BYTE     nType;

	// 1: Type AIS
	pTrack->m_Fspec[2]  |= FRN_15;

	// 2: Repeat indicator: bit 6,7
	nRep = Decode(szBuff[nPos] & 0x30);

	// 3: MMSI: bit 8 - 37
	nLong = ((Decode(szBuff[nPos  ]) & 0x0F) << 26) | ((Decode(szBuff[nPos+1])& 0x3F)<<20) |
		((Decode(szBuff[nPos+2]) & 0x3F) << 14) | ((Decode(szBuff[nPos+3])& 0X3F)<< 8) |
		((Decode(szBuff[nPos+4]) & 0X3F) <<  2) | ((Decode(szBuff[nPos+5])& 0x30)>> 4);
	nPos += 5;

	szNum.Format(_T("%d"), nLong);
	nLen = szNum.GetLength();   

	if ((nLen < 4)||(nLen > 9))
		return 0;
	::ConvStrBuff(szNum, pTrack->m_MMSI, 9);
	pTrack->m_Fspec[2] |= FRN_16;

	if      (nLen == 9)
		pTrack->m_Type = 0x12;      // Auto + Vessel
	else if (nLen == 7)
		pTrack->m_Type = 0x13;      // Auto + Terrain
	else
		pTrack->m_Type = 0x10;      // Auto + default;

	// 4: Aid type: 38 - 42
	nType = ((Decode(szBuff[nPos]) & 0x0F) << 1) | ((Decode(szBuff[nPos+1]) & 0x20) >> 5);
	pTrack->m_SbType = ConvNaviStat(nType);

	// 5: Name: 43 -162: 120 bit
	szStr[20] = 0x00;
	for (j = 0; j < 20; j++)
	{
		ch = ((Decode(szBuff[nPos+j  ]) & 0x1F) << 1) |
			((Decode(szBuff[nPos+j+1]) & 0x20) >> 5);
		if (ch = 0)     // '@' - finish
		{
			szStr[j] = 0x00;
			break;
		}

		if (ch == 32)
			if (szStr[j-1] == 32)	// 2 spaces
			{
				szStr[j-1] = 0x00;
				break;
			}

			if (ch < 32)
				ch += 64;
			szStr[j] = ch;
	}
	pTrack->m_szName	= szStr;
	pTrack->m_Fspec[3] |= FRN_23;
	nPos += 20;

	// 6: Position accuracy: 163
	bool bAccur;

	bAccur = (Decode(szBuff[nPos]) >> 4) & 1;

	// 7: Long: 164-191: 28
	nLong = ((Decode(szBuff[nPos  ]) & 0x1F) << 2)|((Decode(szBuff[nPos+1]) & 0x30) >> 4)|
		((Decode(szBuff[nPos+1]) & 0x1F) << 2)|((Decode(szBuff[nPos+2]) & 0x30) >> 4)|
		((Decode(szBuff[nPos+2]) & 0x1F) << 2)|((Decode(szBuff[nPos+3]) & 0x30) >> 4)|
		((Decode(szBuff[nPos+3]) & 0x1F) << 2)|((Decode(szBuff[nPos+4]) & 0x30) >> 4);
	pTrack->m_Long		= long(nLong / 421875.0 * (1 << 15));
	pTrack->m_Fspec[1] |= FRN_8;
	nPos += 5;

	// 8: Lat: 192 - 218: 27
	nLong = 0;
	for (j = 0; j < 4; j++)
		nLong |= Decode(szBuff[nPos+j]);
	nLong |= (Decode(szBuff[nPos+4]) & 0x38) >> 3;

	pTrack->m_Lat = long(nLong / 421875.0 * (1 << 15));
	nPos += 4;

	int nDimA, nDimB, nDimC, nDimD;

	// 9: Dimension to bow: 219 - 227	
	nDimA = ((Decode(szBuff[nPos] & 0x07)) << 6)|(Decode(szBuff[nPos+1]));
	nPos += 2;

	// 10: Dimension to Stern: 228 - 236
	nDimB = (Decode(szBuff[nPos]) << 3)|((Decode(szBuff[nPos+1]) & 0x38) >> 3);
	nPos ++;

	pTrack->m_Length	= (nDimA + nDimB) * 10;	// [0.1m]
	pTrack->m_Fspec[2] |= FRN_21;

	// 11: Dimension to Port: 237 - 242
	nDimC = ((Decode(szBuff[nPos]) & 0x07) << 3)|((Decode(szBuff[nPos+1]) & 0x38) >> 3);
	nPos ++;

	// 12: Dimension to Starboard: 243 - 248
	nDimD = ((Decode(szBuff[nPos]) & 0x07) << 3)|((Decode(szBuff[nPos+1]) & 0x38) >> 3);
	nPos ++;

	pTrack->m_Width		= (nDimC + nDimD) * 5;	// [0.2m]
	pTrack->m_Fspec[2] |= FRN_21;

	// More
	return 1;

}

//-----------------------------------------------------------------------------
int C2_Local::GetTrkType24(BYTE* szBuff, int nPos, C2_Track* pTrack)
{
	CString	szNum;
	int		nRep, nLen;
	long	nLong;

	// 1: Type AIS	
	pTrack->m_Fspec[2]   |= FRN_15;

	// 2: Repeat indicator: bit 6,7	
	nRep = Decode(szBuff[nPos]) & 0x30;

	// 3: MMSI: bit 8 - 37
	nLong = ((Decode(szBuff[nPos  ]) & 0x0F) << 26) | ((Decode(szBuff[nPos+1])& 0x3F)<<20) |
		((Decode(szBuff[nPos+2]) & 0x3F) << 14) | ((Decode(szBuff[nPos+3])& 0X3F)<< 8) |
		((Decode(szBuff[nPos+4]) & 0X3F) <<  2) | ((Decode(szBuff[nPos+5])& 0x30)>> 4);
	nPos += 5;

	szNum.Format(_T("%d"), nLong);
	nLen = szNum.GetLength();		//5.12

	if ((nLen < 4)||(nLen > 9))
		return 0;

	::ConvStrBuff(szNum, pTrack->m_MMSI, 9);
	pTrack->m_Fspec[2] |= FRN_16;

	if		(nLen == 9)
		pTrack->m_Type	= 0x12;			// Auto + Vessel
	else if (nLen == 7)
		pTrack->m_Type	= 0x13;			// Auto + Terrain
	else
		pTrack->m_Type	= 0x10;			// Auto + Default	

	// 4: Part number bit 38 - 39
	BYTE	nPart = (Decode(szBuff[nPos])>> 2) & 0x03;
	if (nPart == 0)	// Part A		
	{	
		if (!GetTrkTyp24A(szBuff, nPos, pTrack))
			return 0;
	}
	else
		if (!GetTrkTyp24B(szBuff, nPos, pTrack))
			return 0;

	return 1;
}

//-----------------------------------------------------------------------------
int C2_Local::GetTrkTyp24A(BYTE* szBuff, int nPos, C2_Track* pTrack)
{	
	char	szStr[24];
	char	ch;
	int		j;	

	// 5: Vessel name: 40 -159
	szStr[20] = 0x00;
	for (j = 0; j < 20; j++)
	{
		ch = (((Decode(szBuff[nPos+j  ]) << 4) & 0x30) | 
			((Decode(szBuff[nPos+j+1]) >> 2) & 0x0F));
		if (ch == 0)	// '@' - finish
		{
			szStr[j] = 0x00;
			break;
		}

		if (ch == 32)
			if (szStr[j-1] == 32)	// 2 spaces
			{
				szStr[j-1] = 0x00;
				break;
			}

			if (ch < 32)
				ch += 64;
			szStr[j] = ch;
	}
	pTrack->m_szName	= szStr;
	pTrack->m_Fspec[3] |= FRN_23;
	nPos += 20;

	// 6: Spare: 160 - 167: not use
	nPos +=2;

	return 1;
}

//-----------------------------------------------------------------------------
int C2_Local::GetTrkTyp24B(BYTE* szBuff, int nPos, C2_Track* pTrack)
{
	CString	szNum;
	char	szStr[24];
	char	ch;
	int		j;
	long	nLong;

	//5: ship type: 40-47
	BYTE	nType;

	nType = ((Decode(szBuff[nPos])& 0x03) << 6) | (Decode(szBuff[nPos + 1])& 0x3F);
	pTrack->m_SbType	= ConvShipType(nType);	
	pTrack->m_Fspec[1] |= FRN_11; 
	nPos += 2;

	// 6: Vendor ID: 48 - 65: 18 bit
	CString szVendorId;	

	szStr[3] = 0x00;
	for (j = 0; j < 3; j++)
	{
		ch = Decode(szBuff[nPos+j]);
		if (ch == 0)	// '@' - finish
		{
			szStr[j] = 0x00;
			break;
		}

		if (ch == 32)
			if (szStr[j-1] == 32)	// 2 spaces
			{
				szStr[j-1] = 0x00;
				break;
			}

			if (ch < 32)
				ch += 64;
			szStr[j] = ch;
	}	
	szVendorId = szStr;
	nPos += 3;

	// 7: Unit Model Code: 66 - 69
	BYTE nCode;

	nCode = (Decode(szBuff[nPos]) >> 2) & 0x0F;

	// 8: Serial number: 70 - 89
	nLong = ((Decode(szBuff[nPos  ]) & 0x03) << 18) | (Decode(szBuff[nPos+1]) << 12)|
		(Decode(szBuff[nPos+2]) << 6)		    | (Decode(szBuff[nPos+3]));
	nPos += 4;

	// 9: CallSign: 90 - 131: 42bits
	szStr[7] = 0x00;
	for (j = 0; j < 7; j++)
	{
		ch = Decode(szBuff[nPos+j]);
		if (ch == 0)	// '@' - finish
		{
			szStr[j] = 0x00;
			break;
		}

		if (ch == 32)
			if (szStr[j-1] == 32)	// 2 spaces
			{
				szStr[j-1] = 0x00;
				break;
			}		

			if (ch < 32)
				ch += 64;
			szStr[j] = ch;
	}
	pTrack->m_szClSg	= szStr;
	pTrack->m_Fspec[3] |= FRN_22;	
	nPos += 7;

	// 10: Dimension to Bow: 132 - 140

	int		nDimA, nDimB;
	BYTE	nDimC, nDimD;

	nDimA = ((Decode(szBuff[nPos  ])& 0x3F) << 3) |
		((Decode(szBuff[nPos+1])  >> 3) & 0x07);	
	nPos ++;

	// 11: Dimension to Stern : 141 - 149
	nDimB = ((Decode(szBuff[nPos  ])& 0x07)<<6) | 
		(Decode(szBuff[nPos+1])& 0x3F);
	nPos += 2;

	pTrack->m_Length	= (nDimA + nDimB) * 10;	// [0.1m]
	pTrack->m_Fspec[2] |= FRN_21;

	// 12: Dimension to Port: 150 - 155
	nDimC = Decode(szBuff[nPos++]) & 0x3F;		

	// 13: Dimension to Starboard: 156 - 161
	nDimD = Decode(szBuff[nPos++])& 0x3F;

	pTrack->m_Width		= (nDimC + nDimD) * 5;	// [0.2m]
	pTrack->m_Fspec[2] |= FRN_21;

	// 14: Mother ship MMSI: 132 - 161
	nLong = ((Decode(szBuff[nPos  ]) & 0x0F) << 26) | ((Decode(szBuff[nPos+1])& 0x3F)<<20) |
		    ((Decode(szBuff[nPos+2]) & 0x3F) << 14) | ((Decode(szBuff[nPos+3])& 0X3F)<< 8) |
		    ((Decode(szBuff[nPos+4]) & 0X3F) <<  2) | ((Decode(szBuff[nPos+5])& 0x30)>> 4);
	nPos += 5;

	szNum.Format(_T("%d"), nLong);

	BYTE nMoMMSI[9];
	::ConvStrBuff(szNum, nMoMMSI, 9);	

	return 1;
}
//-----------------------------------------------------------------------------
BYTE C2_Local::ConvShipType(BYTE nType)
{
	BYTE	nTyp3x[10] = {30,31,31,33,34,35,36,37,14,14};
	BYTE	nTyp5x[10] = {50,51,52,53,54,55,56,57,14,59};
	BYTE	nTyp7x[10] = {70,72,72,72,74,70,70,70,70,70};

	if		(nType == 0)		// Not defined
		return 0;
	else if	(nType < 20)		// Reserved
		return 14;
	else if (nType < 30)		// Wing on ground
		return 20;
	else if (nType < 40)
		return nTyp3x[nType-30];
	else if (nType < 50)
		return 40;
	else if (nType < 60)
		return nTyp5x[nType-50];
	else if (nType < 70)
		return 60;
	else if (nType < 80)
		return 70;
	else if (nType < 90)
		return 80;
	else
		return 14;
}
//-----------------------------------------------------------------------------
BYTE C2_Local::ConvNaviStat(BYTE nIdNav)
{
	BYTE	nNaviSt[9] = {2, 1, 4, 3, 0, 5, 6, 7, 8};

	return (nIdNav < 9)? nNaviSt[nIdNav] : 0;	
}
//-----------------------------------------------------------------------------
// 2012-12-04 09:20:33.062
//-----------------------------------------------------------------------------
bool C2_Local::ConvEstaEcom (char *szBuff, int nLeng, C2_Track* pTrack)
{
	if (nLeng < 20)
		return false;

	char	szTmp[16];
	int		nTemp;
	int		nPos = 0;

	// 1: Date
	memcpy(szTmp, &szBuff[nPos], 4);		// Year
	szTmp[4] = 0x00;
	nTemp = atoi(szTmp);
	pTrack->m_TimEta[4] = BYTE(nTemp >> 8);
	pTrack->m_TimEta[5] = BYTE(nTemp     );
	nPos += 5;

	memcpy(szTmp, &szBuff[nPos], 2);		// Mon
	szTmp[2] = 0x00;
	nTemp = atoi(szTmp);
	pTrack->m_TimEta[3] = BYTE(nTemp);
	nPos += 3;

	memcpy(szTmp, &szBuff[nPos], 2);		// Day
	szTmp[2] = 0x00;
	nTemp = atoi(szTmp);
	pTrack->m_TimEta[2] = BYTE(nTemp);
	nPos += 3;

	// 2: Time
	memcpy(szTmp, &szBuff[nPos], 2);		// Hour
	szTmp[2] = 0x00;
	nTemp = atoi(szTmp);
	pTrack->m_TimEta[0] = BYTE(nTemp);
	nPos += 3;

	memcpy(szTmp, &szBuff[nPos], 2);		// Min
	szTmp[2] = 0x00;
	nTemp = atoi(szTmp);
	pTrack->m_TimEta[1] = BYTE(nTemp);
	nPos += 3;

/*	memcpy(szTmp, &szBuff[nPos], 2);		// Sec
	szTmp[2] = 0x00;
	pTrack->m_TimEta   += atoi(szTmp);
	nPos += 3;

	pTrack->m_TimEta   *= 128; */
	return true;
}

// 20121204092209
bool C2_Local::ConvTimeEcom(char *szBuff, int nLeng, C2_Track* pTrack)
{
	if (nLeng < 14)
		return false;

	char	szTmp[16];
	int		nTemp;
	int		nPos = 0;

	// 1: Date
	memcpy(szTmp, &szBuff[nPos], 4);		// Year
	szTmp[4] = 0x00;
	nTemp = atoi(szTmp);
	pTrack->m_DayLst[2] = BYTE(nTemp >> 8);
	pTrack->m_DayLst[3] = BYTE(nTemp     );
	nPos += 4;

	memcpy(szTmp, &szBuff[nPos], 2);		// Mon
	szTmp[2] = 0x00;
	nTemp = atoi(szTmp);
	pTrack->m_DayLst[1] = BYTE(nTemp);
	nPos += 2;

	memcpy(szTmp, &szBuff[nPos], 2);		// Day
	szTmp[2] = 0x00;
	nTemp = atoi(szTmp);
	pTrack->m_DayLst[0] = BYTE(nTemp);
	nPos += 2;

	// 2: Time
	memcpy(szTmp, &szBuff[nPos], 2);		// Hour
	szTmp[2] = 0x00;
	pTrack->m_TimLst	= atoi(szTmp)*3600;
	nPos += 2;

	memcpy(szTmp, &szBuff[nPos], 2);		// Min
	szTmp[2] = 0x00;
	pTrack->m_TimLst   += atoi(szTmp)*60;
	nPos += 2;

	memcpy(szTmp, &szBuff[nPos], 2);		// Sec
	szTmp[2] = 0x00;
	pTrack->m_TimLst   += atoi(szTmp);
	nPos += 2;

	pTrack->m_Time		= pTrack->m_TimLst*128;
	return true;
}


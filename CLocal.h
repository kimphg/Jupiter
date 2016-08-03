//=============================================================================
//
// CLocal.h: interface for the C2_Local class.
// File name	: CLocal.h - header file for the system information
//=============================================================================

#if !defined(AFX_C2LOCAL_H_)
#define AFX_C2LOCAL_H_

#pragma once
#include "CTrack.h"
#include "CDefine.h"
#include "CSerial.h"
#include "CNation.h"
#include "CLogfile.h"


//-----------------------------------------------------------------------------
//	Host site: Server/Source
//-----------------------------------------------------------------------------
class C2_Host  
{
public:	
	BYTE		m_Uid	;			// Open status
	BYTE		m_Status;			// Link status
	CString		m_szName;			// Name of Dialup/Host	

	BYTE		m_LinkTyp;			// Type of link: b0(0:Ether, 1:Modem), b4=1:IsDial 		
	BYTE		m_RmtAddr[4];		// Remote address
	BYTE		m_LocAddr[4];		// Local address
	UINT		m_RmtPort;			// Port for send/receive of remote
	UINT		m_LocPort;			// Port for send/receive of local

public:
	C2_Host();
	~C2_Host();

	BYTE		Compare	(C2_Host *);			// Compare different
	void		Update	(C2_Host *);

	CString		GetLocAddr	();
	CString		GetRmtAddr	();

	CString		GetStrAddr	(BYTE *);
	bool		SetStrAddr	(BYTE *, CString);

	CString		GetStrStat	();
	CString		GetStrLink	();
	//bool		GetTrackRada(BYTE *szBuff, int nLeng, C2_Track* pTrack); //sử lý tín hiệu ra đa nhận từ COM
};


//-----------------------------------------------------------------------------
//	All of system information
//-----------------------------------------------------------------------------
class C2_Local  
{
public:
	CString		m_szRoot;			// Root path of App.exe file
	CString		m_szFile;			// Name of information file
	CC_Nation	m_CNation;		    // For get name of Country

	C2_Host		m_HostServ;			// Server of system !
	C2_Host     m_HostRada;         // Address of Rada Arpa
	CString		m_szNamePC;			// Name of BaTe computer
	CString		m_szAccoun;	
	CString		m_szPasswd;	

//	CC_Nation	m_CNation;			// For get name of Country

//	bool		m_IsLinkOn;			// Turn On/Off link
	BYTE		m_LinkType;			// b0=(0:Ether, 1:Modem); b4:Auto dial
	CString		m_szDialup;			// Is check name of dial entry
	bool		m_IsCheck ;			// Is auto dial-up
	BYTE		m_SelAdap ;			// Temple in session

	BYTE		m_UserRole;			// User role in Server: 0=Oper, 1:Tech, 2:Root
	CString		m_szPwdTec;			// Password of Technical
	CString		m_szPwdSup;			// Password of Supervisor

	int			m_AlTmPlan;			// Min time for Air alarm  (minute)
	int			m_AlTmShip;			// Min time for Ship alarm (minute)
	int			m_AlRgPlan;			// Min range for Air alarm (miles)
	int			m_AlRgShip;			// Min range for Ship alarm(miles)
	BYTE		m_AlTrPlan;			// Type of track to alarm  (bits )
	BYTE		m_AlTrShip;
	BYTE		m_AlarZone;

	BYTE		m_ReplType;			// Replay type
	BYTE		m_ReplIden;
	int			m_ReplTAN ;
	int			m_RecdTime;			// Record time [min]

	BYTE		m_ComARPA;		    // Serial port of Radar	
	BYTE		m_ComAIS;		    // Serial port of AIS	

	//Port Com parameters
	//BYTE		m_ComUsed[ 2];	    // Com port for opening
	BYTE		m_ComStat[16];	    // Max = 16 port status
	C2_ComInfo	m_ComInfo[16];      // Information of all comports
	C2_Point	m_RadaPnt;			// Position of radar

	BYTE        m_preFrag;          // the previous fragment number
	int         m_NumFrag;          // total fragments
	BYTE        m_Buff[1024];       // Save temporary fragment
	int         m_Leng;

public:
	C2_Local();
	~C2_Local();

	int			GetAddrLocal(BYTE, CString *);
	CString		GetNameHost	();
	long		GetTimeUTC	();

	bool		CheckPwd	(BYTE, CString);		// Type, Password
	void		ChangePwd	(BYTE, CString);
	bool		LoadFile	();
	bool		SaveFile	();
//	void		LoadNati	();

	//void		Update	    (C2_Local *);        // for AIS + ARPA
	CString		GetStrPort	(BYTE);              // for AIS + ARPA

public:
	bool		GetTrackRada(BYTE*, int, C2_Track*);
	int			GetTrackAIS (BYTE*, int, C2_Track*);
	int         OnLinkBuff  (BYTE*, int);

private:
	BYTE		ConvShipType(BYTE);						// AIS - Sub type
	BYTE		ConvNaviStat(BYTE);						// AIS - Navi status
	bool		ConvEstaEcom(char*, int, C2_Track*);	// ELCOM AIS
	bool		ConvTimeEcom(char*, int, C2_Track*);
	BYTE		Decode(BYTE);
	int			GetTrkClassA(BYTE*, int, C2_Track*);	// type = 1, 2, 3
	int			GetTrkClassB(BYTE*, int, C2_Track*);	// type = 18
	int			GetTrkClasEB(BYTE*, int, C2_Track*);	// type = 19
	int			GetTrkStatio(BYTE*, int, C2_Track*);    // type = 4
	int         GetTrkStatic(BYTE*, int, C2_Track*);    // type = 5 Static and Voyage related data
	int			GetTrkNaviga(BYTE*, int, C2_Track*);	// type = 21 Aid - to - Navigation Report
	int         GetTrkType24(BYTE*, int, C2_Track*);    // type = 24 Static data report
	int			GetTrkTyp24A(BYTE*, int, C2_Track*);	// type 24 PartA
	int			GetTrkTyp24B(BYTE*, int, C2_Track*);	// type 24 PartB

};

//-----------------------------------------------------------------------------
#endif // !defined(AFX_C2LOCAL_H_)

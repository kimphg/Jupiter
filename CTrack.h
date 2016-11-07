//=============================================================================
//	File name: CTrack.h	
//	Object: header of the Track class
//	Compiler: Microsoft Visual C++ 6.0
//	Update:	24-06-2011
//	Copyright by Dang Quang Hieu - AIC Corp.
//=============================================================================

#if !defined(AFX_C2TRACK_H_)
#define AFX_C2TRACK_H_

//#pragma once

#include "CPoints.h"
//#include "CTrkSnd.h"

// define type of tracks
#define		TYPE_TRACK_NONE		0		// No source track
#define		TYPE_TRACK_RCCS		8		// Medium range radar
#define		TYPE_TRACK_RADA		2		// Coastal radar - ARPA
#define		TYPE_TRACK_ECOM		3		// AIS - ELCOM / TT 47
#define		TYPE_TRACK_TT47		4		// Radio location center
#define		TYPE_TRACK_FGPS		5		// Fishing boat by GPS
#define		TYPE_TRACK_VQ98	    6		// Air defense center
#define		TYPE_TRACK_SAIR		7		// VMP - Sea patrol craft
#define		TYPE_TRACK_VAIS		1		// AIS - VISHIPEL
#define		TYPE_TRACK_WEAT		9		// Weather center
#define		TYPE_TRACK_NCCS	   10		// Long range center

#define		TYPE_TRACK_SIMU	   20		// Simulation track
#define		TYPE_TRACK_SYST	   21		// System/Fusion track

#define		IDEN_TRACK_NOINFO	0		// Chua xac dinh
#define		IDEN_TRACK_UNKNOW	1		// Khong ro
#define		IDEN_TRACK_ENERMY	2		// Ke dich
#define		IDEN_TRACK_SUSPEC	3		// Nghi ngo
#define		IDEN_TRACK_FRIEND	4		// Quan su
#define		IDEN_TRACK_INTER	5		// Quoc te
#define		IDEN_TRACK_DOMEST	6		// Noi dia
#define		IDEN_TRACK_TRANSI	7		// Qua canh

#define		MAX_NUMB_TRACK		9999	// may be more	
#define		MAX_NUMB_HIST		8192	// Number of history
#define		NUM_TIME_HIST		240		// [s] Time to save history of track



//-----------------------------------------------------------------------------
//	C2_Track class: track information
//-----------------------------------------------------------------------------
class C2_Track : public C2_Place
{
public:	
	BYTE		m_Fspec[5];			// max
//	BYTE		m_Comm;				// 0:None; 1:New; 2:Upd; 3:Del; 5:Corr
	BYTE		m_Sac ;				// System area code: type of source
	BYTE		m_Sic ;				// Type of source: see define TYPE_TRACK_xxx
	BYTE		m_TypSer;			// Type of service: 2:Track, 3:AIS

	/* Data from Radar */
	int			m_TRN	;			// Track Ref. number
	long		m_Time	;			// Current time [1/128s]
	BYTE		m_StMain;			// Status #1: Simu, Del/New, Numb
	BYTE		m_StExt1;			// Status #2: Corr, IFF
	BYTE		m_StExt2;
	BYTE		m_StExt3;
	BYTE		m_Ages	;
	long		m_TmInit;			// Init of track [1/128s]

//	long		m_Lat	;			// [180/(2^23)]
//	long		m_Long	;			// [180/(2^23)]
	UINT		m_Speed	;			// [0.1 nm/h]
	UINT		m_Head	;			// [360/2^16]
    double      getHead();
	UINT		m_Alti	;			// [1 m]
//	BYTE		m_Type	;			// Type/Class: b0/3
	BYTE		m_SbType;			// Sub type
	BYTE		m_Iden	;			// Identify: b0/3 
	BYTE		m_NaviSt;			// Navigator status: b0/7
	BYTE		m_TQ	;			// [0..15]
    QString		m_szLabe;			// Track label
    QString		m_szNote;			// Track note

	BYTE		m_TN	;			// Aircraft: 
	int			m_Mode1 ;			// nMark + Mode1
	int			m_Mode2 ;			// 2 byte Mode2
	int			m_Mode3A;			// 2 byte Mode3A

	// Data from AIS - HQ
	BYTE		m_TypAIS;			// Type of AIS message
	BYTE		m_TypMsg;			// 1,2,3,5,...
	BYTE		m_MMSI[9]	;		// MMSI string - 9 byte
//	long		m_MMSI;
	int			m_IdNati;			// Identical of national
    QString		m_szNati;			// national of ship
	UINT		m_ROT	;			// [1deg/min]
	UINT		m_HeadTr;			// [360/2^16] True heading
	BYTE		m_IMO[7];			// IMO number - 7 byte
	UINT		m_Length;			// [0.1m]
	BYTE		m_Width ;			// [0.2m]
	BYTE		m_Draugh;			// [0.1m]
    QString		m_szClSg;			// Call sign of track
//	QString		m_szName;			// Name: max 20 byte
    QString		m_szDest;			// Destination
	BYTE		m_EsTime[6];		// Estimated time: Hour,min,day,mon,year
	
	long		m_TimLst;			// Last time receive
	BYTE		m_DayLst[4];		// Day.Mon.Year
	BYTE		m_TimEta[6];		// ETA time/date
public:		// User addition	
	int			m_TmUpd ;			// For predict of position [s]	
	long		m_TmEnd ;			// For replay

	BYTE		m_StWarn;			// Warn status: b7:IsLost, b0:Alarm, b1:Defense, b2:Fire, b3:Zone
	BYTE		m_bBlink;			// Blink flags: b7:IsDrop, b0:Alarm, b1:Defense, b2:Fire, b3:Zone
	bool		m_IsView;			// by check in Track list	
	BYTE		m_LvWarn;			// Warning level
	bool		m_OnHist;			// Is get history

	// Predict: Move, CPA, CTS
	BYTE		m_IsHisOn ;			// Turn-on history
	bool		m_IsPrdMov;			// Is predict of moving
	bool		m_IsPrdCPA;			// Is predict of Closest Point Approach
	bool		m_IsPrdCTS;			// Is predict of Course To Steer
	int			m_TimeMove;			// Time for predict of moving [sec]
	BYTE		m_TypeCTS ;			// 1:CTS, 2:CTT

private:
	C2_PntList	m_ListHist;			// History list

	int			m_TmHist;			// For make history	
	int			m_TmSave;			// For save/record

public:	
	C2_Track();	
	~C2_Track();
	
	bool		CheckMMSI	(BYTE *);			// Check same MMSI
	int			CompaMMSI	(BYTE*);			// Compare MMSI
	bool		CheckIMO	(BYTE *);			// Check same IMO
	void		PredicPos	();					// In
	void		SetNewTRN	(int);				// Set a TRN for track
	void		OnInitSimu	(BYTE);				// Default infor.

	void		Update		(C2_Track*);
	void		Update		(C2_Track*, BYTE);	// nSic

//    QString		GetStrTime	(long);				// nTime
//    QString		GetStrSource();
//    QString		GetStrSpeed	();
//    QString		GetStrHead	();
	int			GetIntSpeed	();
	int			GetIntHead	();

//    QString		GetStrTypAir(BYTE, BYTE&);		// Aircraft type
//    QString		GetStrTypCrf(BYTE, BYTE&);		// Aircraft type
//    QString		GetStrTypSuf(BYTE, BYTE&);		// Surface  type
//    QString		GetStrTypTer(BYTE, BYTE&);		// Terrain  type
//    QString     GetStrTypShp(BYTE);				// Ship		type
//    QString		GetStrClass	(BYTE);
//    QString		GetStrIden	(BYTE);
//    QString		GetStrNaviSt(BYTE);
	
//	BYTE		FindIdTypAir(BYTE);
//	BYTE		FindIdTypCrf(BYTE);
//	BYTE		FindIdTypSuf(BYTE);
//	BYTE		FindIdTypTer(BYTE);
	//BYTE        FindIdTypShp(BYTE);

//    QString		GetRegion(BYTE);		// MMSI to region

//    QString		GetEstTime	();
//    QString		GetEstDate	();
//    bool		ConvEstTime	(QString);
//    bool		ConvEstDate	(QString);
//	bool		CheckSave	(BYTE &);			// Check save record

public:	// For history points
	void		Clear	();
	int			Count	();	
	bool		IsEmpty	();

//	void		AddHead	(C2_Point *);			// Add a Point to head of list
//	void		AddTail	(C2_Point *);
//	C2_Point  *	GetHead	();						// Get a point from head of list

//	POSITION    GetPosH	();						// Get head position
//	C2_Point  *	GetNext	(POSITION &);			// Get next position
    void  *	RemvTail();

//	void		CopyHist(C2_Track *);
//	bool		MakeHist();						// Make history
};
typedef std::vector< const C2_Track > C2_TrackLst;


//-----------------------------------------------------------------------------
#endif // !defined(AFX_C2TRACK_H_)


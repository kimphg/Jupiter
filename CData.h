//=============================================================================
//
// CData.h: interface for the C2_Rada class.
//
//=============================================================================

#if !defined(AFX_C2DATA_H_)
#define AFX_C2DATA_H_

#include "CGraph.h"

#pragma once

//------------------------------------------
#define		TYPE_DATA_MARK		0x21		// Marker point
#define		TYPE_DATA_RADA		0x22		// Radar station
#define		TYPE_DATA_BATE		0x23		// Battery terminar
#define		TYPE_DATA_UNIT		0x24		// Military unit
#define		TYPE_DATA_HOME		0x25		// Building home 
#define		TYPE_DATA_ALARM		0x26		// Alarm zone

#define		MAX_DATA_MARK		250
#define		MAX_DATA_RADA		100
#define		MAX_DATA_BATE		100
#define		MAX_DATA_UNIT		100
#define		MAX_DATA_HOME		100
#define		MAX_DATA_ALARM		100

// ----- ID of Unit type: 0-255
#define		UNIT_TYPE_C2_QC		 0	// SCH Quan chung
#define 	UNIT_TYPE_C2_QD		 1	// SCH Quan doan - Binh chung
#define 	UNIT_TYPE_C2_BCH	 2	// Bo Chi huy quan su tinh
#define 	UNIT_TYPE_C2_F		 3	// SCH su doan
#define 	UNIT_TYPE_C2_L		 4	// SCH lu doan
#define 	UNIT_TYPE_C2_E		 5	// SCH trung doan
#define 	UNIT_TYPE_C2_EDP	 6	// SCH trung doan dia phuong

// Bo binh
#define 	UNIT_TYPE_BB_TANK	 7	// Bo binh: tang 
#define 	UNIT_TYPE_BB_TGIAP	 8	// Bo binh: xe thiet giap
#define 	UNIT_TYPE_BB_OTO	 9	// Bo binh: xe oto
#define 	UNIT_TYPE_PH_LUU122	10	// Phao luu 122mm
#define 	UNIT_TYPE_PH_LUU155	11	// Phao luu 155mm
#define 	UNIT_TYPE_PH_ND122	12	// Phao nong dai 122
#define 	UNIT_TYPE_PH_ND155	13	// Phao nong dai 155
#define 	UNIT_TYPE_PH_LND122	14	// Phao luu nong dai 122
#define 	UNIT_TYPE_PH_LND155	15	// Phao luu nong dai 155
#define 	UNIT_TYPE_BB_COI	16	// Tran dia sung coi 80
#define 	UNIT_TYPE_BB_TL		17	// Tran dia ten lua chien dich

// Thong tin
#define 	UNIT_TYPE_TT_RELAY	18	// Tram tiep suc
#define 	UNIT_TYPE_TT_VIBA	19	// Tram vi ba
#define 	UNIT_TYPE_TT_VSAT	20	// Tram VSAT
#define 	UNIT_TYPE_TT_COMM	21	// Tram vien thong
#define 	UNIT_TYPE_TT_VHF	22	// Tram vo tuyen song ngan

// TCDT
#define 	UNIT_TYPE_ESM_RADIO	23	// Trinh sat vo tuyen
#define 	UNIT_TYPE_ESM_HF	24	// Dinh vi song ngan
#define 	UNIT_TYPE_ESM_JAMM	25	// Dai gay nhieu song ngan
#define 	UNIT_TYPE_ESM_JRADA	26	// Dai gay nhieu rada

// Cong binh
#define 	UNIT_TYPE_CB_HAM	27	// Ham
#define 	UNIT_TYPE_CB_CAUBT	28	// Cau be tong - go
#define 	UNIT_TYPE_CB_CAUSAT	29	// Cau sat
#define 	UNIT_TYPE_CB_BENPHA	30	// Ben pha

// PKKQ
#define 	UNIT_TYPE_SAM_MAN	31	// Ten lua - vac vai
#define 	UNIT_TYPE_SAM_NEAR	32	// Ten lua - gan
#define 	UNIT_TYPE_SAM_MED	33	// Ten lua - trung
#define 	UNIT_TYPE_SAM_FAR	34	// Ten lua - xa

#define 	UNIT_TYPE_AAA_SMALL	35	// Phao cao xa - nho
#define 	UNIT_TYPE_AAA_MED	36	// Phao cao xa - trung
#define 	UNIT_TYPE_AAA_MOVE	37	// Phao phong khong tu hanh 
#define 	UNIT_TYPE_AAA_GUN	38	// Sung may phong khong

#define 	UNIT_TYPE_RADA_M	39	// Radar canh gioi song m
#define 	UNIT_TYPE_RADA_DM	40	// Radar canh gioi song dm
#define 	UNIT_TYPE_RADA_CM	41	// Radar canh gioi song cm
#define 	UNIT_TYPE_RADA_HIGH	42	// Radar do cao
#define 	UNIT_TYPE_RADAKQ_DD	43	// Radar dan duong KQ
#define 	UNIT_TYPE_RADAKQ_C2	44	// Tram chi huy dan duong KQ
#define 	UNIT_TYPE_RADA_SEA	45	// Radar bien song m

#define 	UNIT_TYPE_AIR_FIGHT	46	// May bay tiem kich
#define 	UNIT_TYPE_AIR_ATTAC	47	// May bay tiem kich bom
#define 	UNIT_TYPE_AIR_BOMBR	48	// May bay nem bom phan luc
#define 	UNIT_TYPE_AIR_VIEW	49	// May bay trinh sat
#define 	UNIT_TYPE_HEL_FIGHT	50	// Truc thang vu trang
#define 	UNIT_TYPE_HEL_TRANS	51	// Truc thang van tai

#define 	UNIT_TYPE_POS_MAN	52	// Dai quan sat mat
#define 	UNIT_TYPE_POS_NAV	53	// Dai dan duong KQ
#define 	UNIT_TYPE_AIRBASE_1	54	// San bay cap 1
#define 	UNIT_TYPE_AIRBASE_2	55	// San bay cap 2
#define 	UNIT_TYPE_AIRBASE_3	56	// San bay cap 3

// Hai quan
#define 	UNIT_TYPE_HQ_CANCU	57	// Can cu hai quan 2
#define		UNIT_TYPE_HQ_TAUSB	58	// Tau san bay
#define		UNIT_TYPE_HQ_TUANDG	59	// Tau tuan duong
#define		UNIT_TYPE_HQ_KHUTRC 60	// Tau Khu truc
#define		UNIT_TYPE_HQ_HOVE	61	// Tau Ho ve
#define		UNIT_TYPE_HQ_TPHAO	62	// Tau phao
#define		UNIT_TYPE_HQ_TTLUA	63	// Tau ten lua
#define		UNIT_TYPE_HQ_TTUANT	64	// Tau tuan tien
#define		UNIT_TYPE_HQ_DIEZEN	65	// Tau ngam diezen
#define		UNIT_TYPE_HQ_QULOI	66	// Tau Quet loi
#define		UNIT_TYPE_HQ_DOBO	67	// Tau Do bo
#define		UNIT_TYPE_HQ_VANTAI	68	// Van tai qs
#define		UNIT_TYPE_HQ_TAUCA	69	// Tau ca
#define		UNIT_TYPE_HQ_NEO	70	// Vi tri neo tau

// Loai khac
#define 	UNIT_TYPE_STORE		71	// Kho quan chung
#define 	UNIT_TYPE_NHAMAY	72	// Nha may
#define 	UNIT_TYPE_XUONG		73	// Xuong
#define 	UNIT_TYPE_BV_QC		74	// Benh vien quan chung
#define 	UNIT_TYPE_BV_F		75	// Benh vien su doan
#define 	UNIT_TYPE_BV_E		76	// Tram quan y
#define 	UNIT_TYPE_OIL_F		77	// Tram xang dau co dinh
#define 	UNIT_TYPE_OIL_M		78	// Tram xang dau da chien

// Diem danh dau
#define		MARK_TYPE_TEXT_X	 0	// Type X
#define		MARK_TYPE_TEXT_0	 1	// ..   O
#define		MARK_TYPE_TEXT_V	 2	//      V
#define		MARK_TYPE_PLUS		 3	//      +
#define		MARK_TYPE_THANG		 4	//      #

// Radar
#define 	RADA_TYPE_LONG		 0	// Radar tam xa
#define 	RADA_TYPE_MEDIUM	 1	// Radar tam trung
#define 	RADA_TYPE_SHORT		 2	// Radar ven bo
#define 	RADA_TYPE_PASSIVE	 3	// Radar thu dong
#define 	RADA_TYPE_RADASEA	 4	// Radar hang hai 

// Bate
#define		BATE_TYPE_SCH_L		 0	// So chi huy lu
#define		BATE_TYPE_SCH_E		 1	// So chi huy trung doan
#define		BATE_TYPE_SCH_D		 2	// Tieu doan
#define		BATE_TYPE_SAM_SEA	 3	// ten lua doi hai
#define		BATE_TYPE_GUN_SEA    4	// Phao bien 75-100
#define		BATE_TYPE_AIRBASE    5	// San bay quan su cap 3
#define		BATE_TYPE_DT_PHAO    6	// Doi tau phao
#define		BATE_TYPE_DT_HOVE    7	// Doi tau ho ve
#define		BATE_TYPE_DT_TLUA	 8	// Doi tau ten lua

// Cong trinh quan su
#define		HOME_TYPE_DANKH		 0	// Dan khoan dau
#define		HOME_TYPE_NHGIAN	 1	// Nha gian
#define		HOME_TYPE_NHBETG	 2	// Nha be tong
#define		HOME_TYPE_DOCNOI	 3	// Doc noi
#define		HOME_TYPE_PHAO		 4	// Phao bien
#define		HOME_TYPE_TRAMKH	 5	// Khoan tham do
#define		HOME_TYPE_KHITUONG	 6	// Khi tuong
#define		HOME_TYPE_DAOCHIM	 7	// Dao chim
#define		HOME_TYPE_DAONOI	 8	// Dao noi

//package
#define		TYPE_PACKET_ACCS		1
#define		TYPE_PACKET_RADA		2
#define		TYPE_PACKET_BATE		3
#define		TYPE_PACKET_DCOM		4


//-----------------------------------------------------------------------------
//	MARK point
//-----------------------------------------------------------------------------
class C2_Mark : public C2_Place
{
public:
	BYTE		m_TypSub;			// type of place (see #define)

public:
	C2_Mark(void);
	virtual ~C2_Mark(void);

	void		Copy	(C2_Mark *);

	CString		GetStrType	();
	CString		GetStrType	(int);
};
typedef CTypedPtrList<CPtrList, C2_Mark*> C2_MarkList;


//-----------------------------------------------------------------------------
//	RADAR
//-----------------------------------------------------------------------------
class C2_Rada : public C2_Sector  
{
public:
//	BYTE		m_Type;			// Type of radar: 0:Short; 1:Long; 2:Center
	BYTE		m_Status;

public:
	C2_Rada();
	virtual ~C2_Rada();

	void		Copy	(C2_Rada *);
	bool		CheckSelect	(C2_Point *, UINT);

	CString		GetStrType	();
	CString		GetStrType	(int);
};
typedef CTypedPtrList<CPtrList, C2_Rada*> C2_RadaList;


//-----------------------------------------------------------------------------
//	BATE
//-----------------------------------------------------------------------------
class C2_Bate : public C2_Rada  
{
public:
//	BYTE		m_Type;			// Type of radar: 0:Short; 1:Long; 2:Center
//	int			m_RgMax;		// [20m]
//	int			m_AzBeg;		// [2^16/360]
//	int			m_AzEnd;
//	BYTE		m_Color;

public:
	C2_Bate();
	virtual ~C2_Bate();

	void		Copy	(C2_Bate *);

	CString		GetStrType	();
	CString		GetStrType	(int);
};
typedef CTypedPtrList<CPtrList, C2_Bate *> C2_BateList;


//-----------------------------------------------------------------------------
//	UNIT 
//-----------------------------------------------------------------------------
class C2_Unit : public C2_Mark  
{
public:
//	BYTE		m_Type ;
//	BYTE		m_Color;	

public:
	C2_Unit();
	virtual ~C2_Unit();

	void		Copy	(C2_Unit *);

	CString		GetStrType	();
	CString		GetStrType	(int);
};
typedef CTypedPtrList<CPtrList, C2_Unit *> C2_UnitList;


//-----------------------------------------------------------------------------
//	HOME building/platform in sea
//-----------------------------------------------------------------------------
class C2_Home : public C2_Mark  
{
public:
//	BYTE		m_Type ;
//	BYTE		m_Color;	

public:
	C2_Home();
	virtual ~C2_Home();

	void		Copy	(C2_Home *);

	CString		GetStrType	();
	CString		GetStrType	(int);
};
typedef CTypedPtrList<CPtrList, C2_Home*> C2_HomeList;


//-----------------------------------------------------------------------------
//	Container of all data: Radar, Unit, Mark, Alarm
//-----------------------------------------------------------------------------
class CC_Data
{
public:
	CString		m_szRoot;

private:	
	C2_MarkList	m_ListMark;
	C2_RadaList	m_ListRada;		// Data list
	C2_BateList	m_ListBate;			
	C2_UnitList	m_ListUnit;			
	C2_HomeList	m_ListHome;

public:
	CC_Data();
	~CC_Data();

	void	   	Clear   ();
	void	   	Clear   (BYTE);
	int		  	Count   (BYTE);
	bool	   	IsEmpty (BYTE);
	BYTE		GetUid	(BYTE);

	BYTE		Command (BYTE, BYTE, void *, int &);	// Type, Comm, Data, Index
	BYTE		Command (BYTE, BYTE, void *);
	BYTE		Update  (BYTE, BYTE, void *, int &);	// Type, Comm, Data, Index
	BYTE		Update  (BYTE, BYTE, void *);

	BYTE		LoadFile();
	BYTE		SaveFile();	
	bool		LoadFile(BYTE);
	bool		SaveFile(BYTE);

public:
	void	  	AddHead (BYTE, void *);                 // Type, Data
	void	  	AddTail (BYTE, void *);                 // ...	

	void *		GetHead (BYTE);                         // Type, Index
	void *		GetTail (BYTE);                         // ...

	POSITION	GetPosH	(BYTE);
	void *		GetNext	(BYTE, POSITION &);	
	void *		GetAt   (BYTE, int);					// Type, Index

	void *		Find	(BYTE, BYTE, int &);			// Type, Uid, Index
	void *		Find	(BYTE, BYTE);
	POSITION	FindPos	(BYTE, int );	

	void		RemvHead(BYTE);
	void		RemvTail(BYTE);
	bool   		RemvAt  (BYTE, int);					// ...
	bool		RemvPos	(BYTE, POSITION);

private:
	bool   		LoadHead(BYTE, BYTE *);
	bool   		SaveHead(BYTE, BYTE *);

	bool		LoadMark(CString);
	bool		LoadRada(CString);
	bool		LoadBate(CString);
	bool		LoadUnit(CString);
	bool		LoadHome(CString);

	bool		SaveMark(CString);
	bool		SaveRada(CString);
	bool		SaveBate(CString);
	bool		SaveUnit(CString);
	bool		SaveHome(CString);
};

//-----------------------------------------------------------------------------
#endif // !defined(AFX_C2DATA_H_)

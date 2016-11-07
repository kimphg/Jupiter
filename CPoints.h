//=============================================================================
//	Ban quyen	: phong Phat trien He thong Thong tin - Cong ty AIC
//	Bat dau		: ../../2002
//	Cap nhat	: 12/03/2013; 
//	Tac gia		: Dang Quang Hieu, hieudq@aic.com.vn
//
//	Module		: C2_Point - Quan ly lop diem tren ban do dia ly
//=============================================================================

#if !defined(AIC_C2POINT_H_)
#define AIC_C2POINT_H_

//#pragma once

#include "CDefine.h"


//-------------------------------------
//	Define
//-------------------------------------
#if !defined(M_PI)
#define		M_PI			3.14159265358979324	// PI constant
#endif

#define		bit15			(1<<15)
#define		bit16			(1<<16)
#define		bit23			(1<<23)

//#define	R_EARTH			6378137.0		// The Earth's radius = [0.0001 km]
#define		R_EARTH			3443920.1		// The Earth's radius = [0.0001 mile]
#define		RAD_WGS			bit23/M_PI		// 
#define		WGS_RAD			M_PI/bit23		//
#define		LBS_WGS			(1<<21)/45.0	// LBS = (1<<24)/360
//#define	LBS_RGE			50				// LBS = 50m : for range/distance: PKKQ
#define		LBS_RGE			20				// LBS = 20m : for range/distance: QCHQ

#define		SEC_RAD			M_PI/(180*GG)
#define		RAD_SEC			(180*GG)/M_PI
#define		SEC				3600.0			// He so: 1 degree = 3600 second
#define		GG				3600.0			// 1 hour = 3600 second
#define		gg				60.0			// 1 minute = 60 second

#define		MAX_NUMB_PLACE	200	


//-----------------------------------------------------------------------------
//	Class point
//-----------------------------------------------------------------------------
class C2_Point  
{
private:
	long		m_Lat ;			// [180/(2^23)]
	long		m_Long;			// [180/(2^23)]
    double		mLat_dec ;			// double
    double		mLon_dec;			// double
	int			x, y;			// [pixel]

public:	
    void setLat(long lat);
    void setLon(long lon);
    double getLat();
    double getLon();
	C2_Point();
	virtual ~C2_Point();

	void		Update	(C2_Point *);


	void		ConvPolToWGS (C2_Point *, long  , UINT  );
	void		ConvWGSToPol (C2_Point *, long &, UINT &);

	void		ConvScrToWGS (C2_Point *, int);
	void		ConvScrToWGS (C2_Point *, int , int , int);

	void		ConvWGSToScr (C2_Point *, int);
	void		ConvWGSToScr (C2_Point *, long, long, int);


	int			GetDistLine	 (C2_Point *, C2_Point *);	// On screen
	int			GetDistPoint (C2_Point *);				// On screen
};
typedef QList<C2_Point *> C2_PntList;


//-----------------------------------------------------------------------------
//	Class Place
//-----------------------------------------------------------------------------
class C2_Place : public C2_Point  
{
public:
	BYTE		m_Uid	;		// Index of object: 1-255	
	BYTE		m_Type	;		// Type of place (see #define)
	BYTE		m_Comm  ;		// Command for update: 1,2,3
    QString		m_szName;		// Name of place
	BYTE		m_Color ;

	long		m_Lat0	;		// Position of Text
	long		m_Long0	;		// [180/(2^23)]
	int			m_Zmx;			// Position in Zoom
	int			m_Zmy;			// Position in Zoom

public:
	C2_Place();
	virtual ~C2_Place();

	void		Update	(C2_Place *);			// Copy all member

    QString		GetStrType	();					// Get type of Place
    QString		GetStrRange	(long);				// xx.yyy
    QString		GetStrAzimu	(UINT);				// xx.yyy
    QString		GetStrAzMin	(UINT);				// yyyy
	int			GetIntAzimu	(UINT);				// yyyy
    QString		GetStrColor	(int);
};
typedef QList< C2_Place *> C2_PlaceList;


//-----------------------------------------------------------------------------
//	Class PLACE container
//-----------------------------------------------------------------------------
class CC_Place
{
private:
    C2_PlaceList	m_ListPlace;				// List of points

public:
	CC_Place ();
	~CC_Place();

	void		Copy	(CC_Place *);
	void		Clear	();
	int			Count	();	
	bool		IsEmpty	();
	BYTE		GetUid	();

//	BYTE		Command	(BYTE, C2_Place *, int &);	// Comm, Data, Index
//	BYTE		Command	(BYTE, C2_Place *);
//	BYTE		Update	(BYTE, C2_Place *, int &);	// Comm, Data, Index
//	BYTE		Update	(BYTE, C2_Place *);

public:
	void		AddHead	(C2_Place *);				// Add a Point to head of list
	void		AddTail	(C2_Place *);				// ...            tail

//	C2_Place *	GetHead	();							// Get a point from head of list
//	C2_Place *	GetTail	();							//					tail
//	C2_Place *	GetAt	(int);						//                  index

//	POSITION    GetPosH	();							// Get head position
//	C2_Place *	GetNext	(POSITION &);				// Get next position
//	C2_Place *	GetPos	(POSITION  );				// Get at POS

//	C2_Place *	Find	(BYTE);						// Uid
//	C2_Place *	Find	(BYTE, int&);				// Uid, Index
//	POSITION	FindPos	(int);						// Find at index

    void *RemvHead();
    void *RemvTail();

//	void		RemvPos	(POSITION);					// Remove at POS
	void		RemvAt	(int);						// Remove at Index
};

//-----------------------------------------------------------------------------
#endif // !defined(AIC_C2POINT_H_)

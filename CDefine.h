//=============================================================================
//
// C2Define.h : header file
//
//=============================================================================

#if !defined(AFX_C2DEFFINE_H_)
#define	AFX_C2DEFFINE_H_
#include <QWidget>
typedef  unsigned char BYTE;
#ifndef UINT
typedef  unsigned int UINT;
#endif
//----------------------
#define		PORT_LIST_BATE		21200		// ...     BaTe listen
#define		PORT_SESS_BATE		21290		// Session port of BaTe session
#define     PORT_SESS_RADA      21210       // Session port of Rada session
#define     PORT_SESS_ARPA      10028       // Session port of BATE session by UDP connect ARPA
/*
#define		PORT_SYST_TRACK		20051		// Sys:Port in NCSS
#define		PORT_SYST_STATS		20008
#define		PORT_SYST_HIST		20084
#define		PORT_SYST_TIME		20076

#define		PORT_BROA_CONS		20090		// Port Console for broadcast
#define		PORT_BROA_SEBA		20100		// Port Server  for broadcast

#define		PORT_LIST_CONS		20200		// Port in SDDS
#define		PORT_SESS_CONS		20290		// Session port of ConsX
*/

#define		CAT_MSG_CONNEC		61			// Message
#define		CAT_MSG_TRACK		62			// Track
#define		CAT_MSG_STATUS		63			// Status
#define		CAT_MSG_STATS2		65			// Status / Simu

#define		ID_CAT_24			24			// For Console
#define		CAT_MSG_LOGIN		70			// Command Login
#define		CAT_MSG_SYSTEM		71			// System parameters
#define		CAT_MSG_GRAPH		73			// Graph update
#define		CAT_MSG_UNITS		74			// Data  update
#define		CAT_MSG_AREAS		75			// Area  update
#define		CAT_MSG_REPLAY		78			// Replay request


// CAT 61 - Type of message
#define		TYPE_MSG_REQ_CONNECT	0x11    // Request for connect
#define		TYPE_MSG_REQ_DISCONN	0x12    // Request for disconnect
#define		TYPE_MSG_ACK_CONNECT	0x13    // Acknow connect
#define		TYPE_MSG_REJ_CONNECT	0x14    // Reject connect
#define		TYPE_MSG_ACK_DISCONN	0x15    // Acknow disconnect
#define		TYPE_MSG_REJ_DISCONN	0x16    // Reject disconnect

#define		TYPE_MSG_DEF_SERVICE	0x21    // Definition service
#define		TYPE_MSG_TER_SERVICE	0x25    // Termination service
#define		TYPE_MSG_ACK_SERVICE	0x26    // Acknow service
#define		TYPE_MSG_REJ_SERVICE	0x27    // Reject service
#define		TYPE_MSG_ACK_TERSERV	0x2E    // Acknow termination of service
#define		TYPE_MSG_REJ_TERSERV	0x2D    // Reject termination of service

// CAT 63
#define		TYPE_MSG_STT_SERVER		0x31	// Server  status
#define		TYPE_MSG_STT_SERVICE	0x32	// Service status

#define		TYPE_MSG_REQUES_NETW	0x3E	// From ConsX: E=14
#define		TYPE_MSG_ANSWER_NETW	0x3F	// From ACom:  F=15

// CAT 71: CAT_MSG_SYSTEM
#define		TYPE_COMM_REQ_LOGIN		(1<<4) + 1
#define		TYPE_COMM_ANS_LOGIN		(1<<4) + 2
#define		TYPE_COMM_REQ_PASWD		(1<<4) + 3
#define		TYPE_COMM_ANS_PASWD		(1<<4) + 4

#define		TYPE_COMM_REQ_HISTOR	(2<<4) + 1
#define		TYPE_COMM_ANS_HISTOR	(2<<4) + 2

#define		TYPE_DATA_REQ_HISTOR	(0<<6) + 1
#define		TYPE_DATA_REQ_CODE99	(0<<6) + 2
#define		TYPE_DATA_REQ_CODESP	(0<<6) + 3
#define		TYPE_DATA_REQ_CODETN	(0<<6) + 4
#define		TYPE_DATA_REQ_CODE55	(0<<6) + 8
#define		TYPE_DATA_REQ_RADAR		(0<<6) + 9
#define		TYPE_DATA_REQ_ERADA		(0<<6) +10
#define		TYPE_DATA_REQ_UNIT		(0<<6) +12
#define		TYPE_DATA_REQ_SUMTRK	(0<<6) +15

#define		TYPE_DATA_ACK_HISTOR	(1<<6) + 1
#define		TYPE_DATA_ACK_CODE99	(1<<6) + 2
#define		TYPE_DATA_ACK_CODESP	(1<<6) + 3
#define		TYPE_DATA_ACK_CODETN	(1<<6) + 4
#define		TYPE_DATA_ACK_CODE55	(1<<6) + 8
#define		TYPE_DATA_ACK_RADAR		(1<<6) + 9
#define		TYPE_DATA_ACK_ERADA		(1<<6) +10
#define		TYPE_DATA_ACK_UNIT		(1<<6) +12
#define		TYPE_DATA_ACK_SUMTRK	(1<<6) +15

#define		TYPE_DATA_REJ_HISTOR	(2<<6) + 1
#define		TYPE_DATA_REJ_CODE99	(2<<6) + 2
#define		TYPE_DATA_REJ_CODESP	(2<<6) + 3
#define		TYPE_DATA_REJ_CODETN	(2<<6) + 4
#define		TYPE_DATA_REJ_CODE55	(2<<6) + 8
#define		TYPE_DATA_REJ_RADAR		(2<<6) + 9
#define		TYPE_DATA_REJ_ERADA		(2<<6) +10
#define		TYPE_DATA_REJ_UNIT		(2<<6) +12
#define		TYPE_DATA_REJ_SUMTRK	(2<<6) +15

// CAT 78: Replay
#define		TYPE_REPL_REQ_DATA		1		// ConsX
#define		TYPE_REPL_ACK_DATA		2		// SeBa
#define		TYPE_REPL_REJ_DATA		3		// SeBa
#define		TYPE_REPL_TER_DATA		4		// SeBa
#define		TYPE_REPL_SEND_DATA		5		// SeBa
#define		TYPE_REPL_RECV_DATA		6		// ConsX
#define		TYPE_REPL_REQ_STOP		7		// ConsX
#define		TYPE_REPL_REQ_PAUSE		8		// ConsX
#define		TYPE_REPL_REQ_CONTI		9		// ConsX

// Type of session
#define		STAT_SESS_NONE			0x00	// Close
#define		STAT_SESS_OPEN			0x01	// Open
#define		STAT_SESS_WAIT			0x02	// Wait Ack
#define		STAT_SESS_DROP			0x03	// Wait Dis
#define		STAT_SESS_WARN			0x04	// Warning by not receive
#define		STAT_SESS_STOP			0x05	// Will stop this link
#define		STAT_SESS_ERRO			0x0F	// Can not create socket

// Define type of source
#define		TYPE_LINK_NONE			0		// Nothing
#define		TYPE_LINK_CONS			1		// Console
#define		TYPE_LINK_BATE			2		// Battery Terminal
#define		TYPE_LINK_EXAG			3		// External Agency
#define		TYPE_LINK_USER			5		// User session
#define		TYPE_LINK_SEND			9		// RGate by Comport

#define		TYPE_LINK_PORT			10		// RGate link by Serial
#define		TYPE_LINK_RCCS			11		// Radar Command Control System
#define		TYPE_LINK_RADA			12		// Coastal radar sites
#define		TYPE_LINK_ECOM			13		// AIS - ELCOM / TT 47
#define		TYPE_LINK_TT47			14		// HF/VHF - Trung tam 47
#define		TYPE_LINK_FGPS			15		// Fishing boat by GPS
#define		TYPE_LINK_VQ98			16		// SCH VQ98 - PKKQ
#define		TYPE_LINK_SAIR			17		// Sea AIR craft center
#define		TYPE_LINK_VAIS			18		// VISHIPEL AIS system
#define		TYPE_LINK_WEAT			19		// Weather center
#define		TYPE_LINK_NCCS			20		// National Command Control System
#define     TYPE_LINK_AIS           21      // Link of AIS

//---- Value of Timer count (with 5s)
#define		TIME_WAIT   			 1   	// =   5s: wait Ack/Rej
#define		TIME_REPORT				 3   	// =  15s: Status of link
#define		TIME_DATA				12   	// =  60s: Send data
#define		TIME_STATUS				12   	// =  60s: send MsgStatus	
#define		TIME_WARN				24   	// = 120s: warn of link
#define		TIME_DROP				30   	// = 150s: drop of link
#define		TIME_TIMEOUT			36  	// = 180s: TimeOut
#define		TIME_ZERO		   24*3600		// = 1s


// define FRN
#define		FRN_1			128		//(1 << 7)
#define		FRN_2			64		//(1 << 6)
#define		FRN_3			32		//(1 << 5)
#define		FRN_4			16		//(1 << 4)
#define		FRN_5			8		//(1 << 3)
#define		FRN_6			4		//(1 << 2)
#define		FRN_7			2		//(1 << 1)
#define		FX				1		//(1 << 0)

#define		FRN_8			FRN_1
#define		FRN_9			FRN_2
#define		FRN_10			FRN_3
#define		FRN_11			FRN_4
#define		FRN_12			FRN_5
#define		FRN_13			FRN_6
#define		FRN_14			FRN_7

#define		FRN_15			FRN_1
#define		FRN_16			FRN_2
#define		FRN_17			FRN_3
#define		FRN_18			FRN_4
#define		FRN_19			FRN_5
#define		FRN_20			FRN_6
#define		FRN_21			FRN_7

#define		FRN_22			FRN_1
#define		FRN_23			FRN_2
#define		FRN_24			FRN_3
#define		FRN_25			FRN_4
#define		FRN_26			FRN_5
#define		FRN_27			FRN_6
#define		FRN_28			FRN_7

#define		FRN_29			FRN_1
#define		FRN_30			FRN_2
#define		FRN_31			FRN_3
#define		FRN_32			FRN_4
#define		FRN_33			FRN_5
#define		FRN_34			FRN_6
#define		FRN_35			FRN_7

#define		COMM_NON			0
#define		COMM_NEW			1
#define		COMM_UPD			2
#define		COMM_DEL			3
#define		COMM_INS			4

#define		MIN_SCALE		2			// [m/pixel]
#define		MAX_SCALE		650			// [m/pixel]


//-----------------------------------------------------------------------------
//	Global functions
//-----------------------------------------------------------------------------
int		ConvStrChar (QString, char *);
int		ConvStrChar	(QString, char *, int);
//int		ConvStrCHAR	(QString, WCHAR*);
//int		ConvStrBuff	(QString, BYTE *);
//int		ConvStrBuff	(QString, BYTE *, int);
//int		MakeStrCode	(		  BYTE *, int);


//-----------------------------------------------------------------------------
#endif // !defined(AFX_C2DEFFINE_H_)

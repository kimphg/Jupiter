//
// CSerial.h: interface for the C2_ComInfo class.
//
//=============================================================================

#if !defined(AFX_CSERIAL_H_)
#define AFX_CSERIAL_H_



#include "CData.h"


#define		WMS_COMM_BREAK		WM_USER+1	// A break was detected on input.
#define		WMS_COMM_CTS		WM_USER+2	// The CTS (clear-to-send) signal changed state. 
#define		WMS_COMM_DSR		WM_USER+3	// The DSR (data-set-ready) signal changed state. 
#define		WMS_COMM_ERR		WM_USER+4	// A line-status error occurred. Line-status errors are CE_FRAME, CE_OVERRUN, and CE_RXPARITY. 
#define		WMS_COMM_RING		WM_USER+5	// A ring indicator was detected. 
#define		WMS_COMM_RLSD		WM_USER+6	// The RLSD (receive-line-signal-detect) signal changed state. 
#define		WMS_COMM_RXCHAR		WM_USER+7	// A character was received and placed in the input buffer. 
#define		WMS_COMM_RXFLAG		WM_USER+8	// The event character was received and placed in the input buffer.  
#define		WMS_COMM_TXEMPTY	WM_USER+9	// The last character in the output buffer was sent.  
#define		WMS_COMM_RESET		WM_USER+10

#define		STAT_PORT_NONE		0x00		// Status of ComPort
#define		STAT_PORT_IDLE		0x01		// 0001
#define		STAT_PORT_SEND		0x02		// 0010
#define		STAT_PORT_RECV		0x04		// 0100
#define		STAT_PORT_ERRO		0x08		// 1000

#define		SIZE_PORT_SEND			2048	// max size of buffer for serial send/recv (old version: 8192)
#define		SIZE_PORT_RECV			2048
#define		MAX_NUMB_PORT		16			// max number of port in system


//-----------------------------------------------------------------------------
//	Port info
//-----------------------------------------------------------------------------

class C2_ComInfo
{
public:
	BYTE	m_NumbPort ;	// Name of COM port = 1,2,3...
	BYTE	m_IdBaudRat;	// [1200,2400,4800,9600,19200,38400,57600,115200]
	BYTE	m_IdBitData;	// [5,6,7,8]
	BYTE	m_IdBitStop;	// [1, 2]
	BYTE	m_IdParity ;	// [None, Odd, Even, Mark, Space]
	BYTE	m_IdFlwCtrl;	// [Xon/Xoff, Hardware, None ]
public:
	C2_ComInfo(void);
	~C2_ComInfo(void);

	DWORD	GetBaudRate	();
	BYTE	GetBitsStop	();
	BYTE	GetBitsData	();	
	BYTE	GetBitParity();
	CString GetFlwCtrl	();

	void	Update	(C2_ComInfo *);
	bool	Compare	(C2_ComInfo *);		// true = IsSame

};

//-----------------------------------------------------------------------------
//	ComPort class
//-----------------------------------------------------------------------------

class C2_ComPort
{
public:
 	bool		m_IsOpen;			// Is Open for link
 	bool		m_IsRecv;			// Is receiving data
 	bool		m_IsSend;			// Is sending data
	BYTE		m_Status;			// Status of port
	UINT		m_NumPort;			// Number of port
	BYTE		m_TypLink;			// see define
	int			m_TimeOut;			// TimeOut of Send status

private:
	int			m_nLenRx;			// Leng of RX buffer
	int			m_nLenTx;			// Leng of TX buffer
	int			m_nLenOn;			// Length of On-line buff 
	char		m_BuffRx[4096];     // Buffer for RECV = 8192 byte
	char		m_BuffTx[4096];     // Buffer for SEND
    char		m_BuffOn[4096];		// Buffer for Read from MainFrame

protected:
	CWinThread *m_pWndThread;		// Thread pointer	
	BOOL		m_IsThreadOn;		// Is Thread alive	
	CWnd	   *m_pOwner  ;			// Ownner of handle
	DWORD		m_nEvtPort;			// Com port events
	// Handles
	HANDLE		m_hComm;			// handle of communication link
	HANDLE		m_hEvtClose;		// Shutdown the port
	HANDLE		m_hEvtWrite;		// Write byte to port
	HANDLE		m_hEvtArray[3];
	OVERLAPPED	m_OverLap;			// Structures 
	DCB			m_dcb;				// Setting for serial communication device		
	CRITICAL_SECTION m_csCommSync;	// Synchronisation objects		
public:
	C2_ComPort(void);
	~C2_ComPort(void);

	// Port init/close
	bool	OnCreate	(C2_ComInfo *, CWnd *);
	void	OnClose		();
	void	OnReset		(C2_ComInfo *);		// Reset mode of port

	// There functions added by HieuDQ
	bool	CheckStatus	(BYTE);				// Status of local port
	bool	CheckRemote	();					// Status of Remote port
	void	SendBlock	(char *, int);		// Buff, Len
	int		ReadBlock	(char *, int);		// Buff, Len

private:
	bool	StartWatch	();					// Start/Stop comm watching	
	bool	ResetWatch	();
	bool	StopsWatch	();

protected:
	void	ProcMsgError(char *);

	static	UINT CommThread	(LPVOID);
	static	void CheckRecv	(C2_ComPort *);	// Check & send MSG
	static	void ReceiChar	(C2_ComPort *, COMSTAT *);
	static	void WriteChar	(C2_ComPort *);			
};
typedef CTypedPtrList<CPtrList, C2_ComPort *> C2_PortList;

#endif // !defined(AFX_CSERIAL_H_)

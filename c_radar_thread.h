#ifndef DATAPROCESSINGTHREAD_H
#define DATAPROCESSINGTHREAD_H
#include <QThread>
#include <queue>
#include <QTimer>
#include <QGeoPositionInfo>
#include "c_radar_data.h"
#include "c_arpa_data.h"
#include "AIS/AIS.h"
#include <vector>
#include <QFile>
#include <QUdpSocket>
#include <QStringList>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
#define MAX_COMMAND_QUEUE_SIZE 100
#define HAVE_REMOTE// for pcap
#include "pcap.h"
#ifndef CONST_NM
#define CONST_NM 1.852f

#endif
#define HR2D_UDP_PORT 5000
struct DataBuff// buffer for data frame
{
    short len;
    unsigned char data[1500];
};


struct  RadarCommand// radar control commmand
{
    unsigned char bytes[8];
};
typedef std::queue<RadarCommand> RadarCommandQueue;
class dataProcessingThread:public QThread
{
    Q_OBJECT
public:
    unsigned char    connect_timeout;
    //QMutex  mutex_data_change;
    unsigned short    playRate;
    DataBuff*   dataBuff;
    float   k_vet;
    void SetRadarPort( unsigned short portNumber);
    void SetARPAPort( unsigned short portNumber);
    ~dataProcessingThread();
    dataProcessingThread();
    QTimer commandSendTimer;
    QTimer readUdpBuffTimer;
    QTimer readSerialTimer;

    void PlaybackFile();
    void startRecord(QString fileName);
    void stopRecord();
    void stopThread();
    void radRequestTemp(char index);
    void radTxOn();
    void radTxOff();
    void sendCommand(unsigned char* sendBuff,short len);
    void loadRecordDataFile(QString fileName);
    void togglePlayPause(bool play);

    C_radar_data* mRadarData;
    C_ARPA_data* arpaData;
    void run();
    bool getIsDrawn();
    AIS aisMessageHandler;
    QList<AIS_object_t> m_aisList;
    bool isConnected()
    {
        return bool(connect_timeout);
    }
    void setIsDrawn(bool value);

    void setRotationSpeed(int index);
    bool getIsXuLyThuCap() const;
    void setIsXuLyThuCap(bool value);

    double getSelsynAzi() const;
    double newAzi;
    unsigned int mazi;
    unsigned int realazi1,realazi2;
    bool getPosition(double *lat, double *lon);
    double getHeading() const;

    bool   isHeadingAvaible;
signals:
    void HeadingDataReceived(double heading);
private:
    QSerialPort mEncoderPort;
    double mHeading ;
    unsigned char failureCount;
    bool  isDrawn;
    bool isXuLyThuCap;
    RadarCommandQueue radarComQ;
    bool isRecording;
    bool isPlaying;
    QFile signRepFile;
    QFile signRecFile;
    std::vector<QSerialPort*>     serialPorts;
    QUdpSocket      *radarSocket;
    QUdpSocket      *ARPADataSocket;
    double selsynEncoderAzi;
    void listenToRadar();
    void initSerialComm();
    void processSerialData(QByteArray inputData);
    //    bool ProcDataAIS(BYTE *szBuff, int nLeng);
    bool checkFeedback();
private slots:
    void ReadDataBuffer();
    void PushCommandQueue();
    void processRadarData();
    void processARPAData(QByteArray inputdata);
    void playbackRadarData();
    void SerialDataRead();
    void gpsupdate(QGeoPositionInfo geo);

public slots:
    void SerialEncoderRead();
    void StopProcessing();
};

#endif // DATAPROCESSINGTHREAD_H

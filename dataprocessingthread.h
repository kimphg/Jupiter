#ifndef DATAPROCESSINGTHREAD_H
#define DATAPROCESSINGTHREAD_H
#include <QThread>
#include <queue>
#include <QTimer>
#include "C_radar_data.h"
#include "c_arpa_data.h"
#include <vector>
#include <QFile>
#include <QUdpSocket>
#include <QStringList>
#define MAX_COMMAND_QUEUE_SIZE 50
#define HAVE_REMOTE
#include "pcap.h"
#ifndef CONST_NM
#define CONST_NM 1.825f

#endif
#define HR2D_UDP_PORT 5000
struct DataBuff
{
    short len;
    unsigned char data[1500];
};
struct  RadarCommand
{
    unsigned char bytes[8];
};
typedef std::queue<RadarCommand> RadarCommandQueue;
class dataProcessingThread:public QThread
{
    Q_OBJECT
public:
    bool    isDrawn;
    bool    isRunning;
    unsigned char    connect_timeout;
//    QMutex  mutex;
    unsigned short    playRate;
    DataBuff*   dataBuff;
    float   k_vet;
    void SetRadarPort( unsigned short portNumber);
    void SetARPAPort( unsigned short portNumber);
    void ReadDataBuffer();
    ~dataProcessingThread();
    dataProcessingThread();
    QTimer UpdateTimer;
    void PlaybackFile();
    void startRecord(QString fileName);
    void stopRecord();
    void stopThread();
    void radRequestTemp(char index);
    void radTxOn();
    void radTxOff();
    void sendCommand(unsigned char* sendBuff,short len);
    void startReplay(QString fileName);
    void togglePlayPause(bool play);
    C_radar_data* radarData;
    C_ARPA_data* arpaData;
    void run();
    bool getIsDrawn()
    {
       if(!isDrawn){isDrawn = true;return false;}
       else return true;
    }
    bool isConnected()
    {
        return bool(connect_timeout);
    }
private:

    RadarCommandQueue radarComQ;
    bool isRecording;
    bool isPlaying;
    QFile signRepFile;
    QFile signRecFile;

    QUdpSocket      *radarSocket;
    QUdpSocket      *ARPADataSocket;
    void listenToRadar();
public slots:
    void UpdateRadar();
    void processRadarData();
    void processARPAData();
    void playbackRadarData();
};

#endif // DATAPROCESSINGTHREAD_H

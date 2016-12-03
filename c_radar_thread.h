#ifndef DATAPROCESSINGTHREAD_H
#define DATAPROCESSINGTHREAD_H
#include <QThread>
#include <queue>
#include <QTimer>
#include "c_radar_data.h"
#include "c_arpa_data.h"
#include <vector>
#include <QFile>
#include <QUdpSocket>
#include <QStringList>
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
    QMutex  mutex_data_change;
    unsigned short    playRate;
    DataBuff*   dataBuff;
    float   k_vet;
    void SetRadarPort( unsigned short portNumber);
    void SetARPAPort( unsigned short portNumber);

    ~dataProcessingThread();
    dataProcessingThread();
    QTimer UpdateTimer;
    QTimer readBuffTimer;
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
    C_radar_data* radarData;
    C_ARPA_data* arpaData;
    void run();
    bool getIsDrawn();

    bool isConnected()
    {
        return bool(connect_timeout);
    }
    void setIsDrawn(bool value);

private:
    bool    isDrawn;
    RadarCommandQueue radarComQ;
    bool isRecording;
    bool isPlaying;
    QFile signRepFile;
    QFile signRecFile;

    QUdpSocket      *radarSocket;
    QUdpSocket      *ARPADataSocket;
    void listenToRadar();
private slots:
    void ReadDataBuffer();
    void PushCommandQueue();
    void processRadarData();
    void processARPAData();
    void playbackRadarData();
};

#endif // DATAPROCESSINGTHREAD_H

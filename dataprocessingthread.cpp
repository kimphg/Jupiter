#include "dataprocessingthread.h"
#define MAX_IREC 100

DataBuff dataB[MAX_IREC];
dataProcessingThread::~dataProcessingThread()
{
    delete radarData;
    delete arpaData;
}
void dataProcessingThread::ReadDataBuffer()
{
    //short nread = 0;
    while(iRec!=iRead)
    {
        //nread++;
        iRead++;
        if(iRead>=MAX_IREC)iRead=0;
        radarData->GetDataHR(&dataBuff[iRead].data[0],dataBuff[iRead].len);
    }
    //printf("\nnread:%d",nread);
}
dataProcessingThread::dataProcessingThread()
{
    dataBuff = &dataB[0];
    iRec=0;iRead=0;
//    udpSendSocket = new QUdpSocket(this);
//    udpSendSocket->bind(2000);
    playRate = 10;
    arpaData = new C_ARPA_data();
    isRecording = false;
    radarData = new C_radar_data();
    isPlaying = false;
    radarDataSocket = new QUdpSocket(this);
    radarDataSocket->bind(5000, QUdpSocket::ShareAddress);
//   arpaData = new C_ARPA_data();
//   isRecording = false;
//   radarData = new C_radar_data();
//   isPlaying = false;
//   radarDataSocket = new QUdpSocket(this);
   /*connect(radarDataSocket, SIGNAL(readyRead()),
           this, SLOT(processRadarData()));*/
   /*ARPADataSocket = new QUdpSocket(this);
   ARPADataSocket->bind(4001, QUdpSocket::ShareAddress);
   connect(ARPADataSocket, SIGNAL(readyRead()),
           this, SLOT(processARPAData()));*/
}

void dataProcessingThread::playbackRadarData()
{
    if(isPlaying) {
        isDrawn = false;
        unsigned short len;

        if(!signRepFile.isOpen())return;
        for(unsigned short i=0;i<playRate;i++)
        {
            QMutexLocker locker(&mutex);

            if(!signRepFile.read((char*)&len,2))
            {
                signRepFile.seek(0);
                //togglePlayPause(false);
                return;
            }
            QByteArray buff;
            buff.resize(len);
            signRepFile.read(buff.data(),len);
            radarData->GetDataHR((unsigned char*)buff.data(),buff.size());
            if(isRecording)
            {
                signRecFile.write((char*)&len,2);
                signRecFile.write(buff.data(),len);
            }
            if(playRate<10){togglePlayPause(false);return;}

        }
        return;
    }
}
void dataProcessingThread::SetRadarPort( unsigned short portNumber)
{
    radarDataSocket->bind(portNumber, QUdpSocket::ShareAddress);
}
void dataProcessingThread::SetARPAPort( unsigned short portNumber)
{
    ARPADataSocket->bind(portNumber, QUdpSocket::ShareAddress);
}

void dataProcessingThread::startReplay(QString fileName)//
{
    if(signRepFile.isOpen()) signRepFile.close();
    signRepFile.setFileName(fileName);
    signRepFile.open(QIODevice::ReadOnly);

    isPlaying = false;
}

void dataProcessingThread::togglePlayPause(bool play)
{
    isPlaying = play;

}
void dataProcessingThread::processARPAData()
{
    while (ARPADataSocket->hasPendingDatagrams()) {
        isDrawn = false;
        QByteArray datagram;
        unsigned short len = ARPADataSocket->pendingDatagramSize();
        datagram.resize(len);
        ARPADataSocket->readDatagram(datagram.data(), len);
        arpaData->processData(datagram.data(),len);
        if(isRecording)
        {
            signRecFile.write((char*)&len,2);
            signRecFile.write(datagram.data(),len);
        }

//        else if((*list.begin()).contains("CAMAZI"))
//        {
//            currCamAzi = (*(list.begin()+1)).toDouble()+config.m_config.trueNCam;
//            if(currCamAzi>360)currCamAzi-=360;
//            alpha = currCamAzi/CONST_RAD_TO_DEG;
//            //printf((*(list.begin()+1)).data());
//            //printf("\ncurrCamAzi:%f",currCamAzi);
//            update();
//        }
    }
    return;
}
void dataProcessingThread::processRadarData()
{

}
void dataProcessingThread::run()
{
    setPriority(QThread::TimeCriticalPriority);
    while  (true)
    {


        if(radarDataSocket->hasPendingDatagrams())
        {
            iRec++;
            if(iRec>=MAX_IREC)iRec = 0;
            dataBuff[iRec].len = radarDataSocket->pendingDatagramSize();
            radarDataSocket->readDatagram((char*)&dataBuff[iRec].data[0], dataBuff[iRec].len);
             isDrawn = false;
            if(isRecording)
            {
                signRecFile.write((char*)&dataBuff[iRec].len,2);
                signRecFile.write((char*)&dataBuff[iRec].data[0],dataBuff[iRec].len);

            }

        }
        else { usleep(100);}
    }
}
void dataProcessingThread::stopThread()
{

    terminate();
}

void dataProcessingThread::listenToRadar()
{
    for(;;)
    {
        short len = radarDataSocket->pendingDatagramSize();
        if(len<0)continue;
        QByteArray buff;
        buff.resize(len);
        printf("data:%d",len);
        radarDataSocket->readDatagram(buff.data(), len);
        /*unsigned short len = radarDataSocket->pendingDatagramSize();
        QByteArray buff;
        buff.resize(len);
        radarDataSocket->readDatagram(buff.data(), len);
        radarData->GetDataHR((unsigned char*)buff.data(),len);
        if(isRecording)
        {
            signRecFile.write((char*)&len,2);
            signRecFile.write(buff.data(),len);

            //udpSendSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 10);

            //udpSendSocket->writeDatagram(buff.data(),len,QHostAddress("192.168.0.5"),500);
        }*/
    }
}

void dataProcessingThread::startRecord(QString fileName)
{
    signRecFile.setFileName(fileName);
    signRecFile.open(QIODevice::WriteOnly);
    isRecording = true;
}
void dataProcessingThread::stopRecord()
{
    signRecFile.close();
    isRecording = false;
}

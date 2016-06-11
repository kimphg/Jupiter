#include "dataprocessingthread.h"
#define MAX_IREC 1000

DataBuff dataB[MAX_IREC];
short iRec=0,iRead=0;
bool *pIsDrawn;
bool *pIsPlaying;
//QTimer readDataBuff;
dataProcessingThread::~dataProcessingThread()
{
    delete radarData;
    delete arpaData;
}
void dataProcessingThread::ReadDataBuffer()
{
    while(iRec!=iRead)
    {
        //nread++;

        radarData->GetDataHR(&dataBuff[iRead].data[0],dataBuff[iRead].len);
        if(isRecording)
        {
            signRecFile.write((char*)&dataBuff[iRead].len,2);
            signRecFile.write((char*)&dataBuff[iRead].data[0],dataBuff[iRead].len);

        }
        iRead++;
        if(iRead>=MAX_IREC)iRead=0;
    }
}
dataProcessingThread::dataProcessingThread()
{
    dataBuff = &dataB[0];
    iRec=0;iRead=0;
    pIsDrawn = &isDrawn;
    pIsPlaying = &isPlaying;
//    udpSendSocket = new QUdpSocket(this);
//    udpSendSocket->bind(2000);
    playRate = 10;
    arpaData = new C_ARPA_data();
    isRecording = false;
    radarData = new C_radar_data();
    isPlaying = false;
    radarDataSocket = new QUdpSocket(this);
    radarDataSocket->bind(5000, QUdpSocket::ShareAddress);

//    connect(&readDataBuff, SIGNAL(timeout()), this, SLOT(ReadDataBuffer()));
//    readDataBuff.start(10);
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
            //QMutexLocker locker(&mutex);

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
#define UDP_HEADER_LEN 42
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
//    struct tm ltime;
//    char timestr[16];
//    time_t local_tv_sec;

    /*
     * unused variables
     */
    (VOID)(param);
    (VOID)(pkt_data);

    /* convert the timestamp to readable format */
//    local_tv_sec = header->ts.tv_sec;
//    localtime_s(&ltime, &local_tv_sec);
//    strftime( timestr, sizeof timestr, "%H:%M:%S", &ltime);

    if(*pIsPlaying)return;
    if(header->len<=500)return;
    if(((*(pkt_data+36)<<8)|(*(pkt_data+37)))!=HR2D_UDP_PORT)
    {
        //printf("\nport:%d",((*(pkt_data+36)<<8)|(*(pkt_data+37))));
        return;
    }
    dataB[iRec].len = header->len - UDP_HEADER_LEN;
    memcpy(&dataB[iRec].data[0],pkt_data+UDP_HEADER_LEN,dataB[iRec].len);
    iRec++;
    if(iRec>=MAX_IREC)iRec = 0;
    *pIsDrawn = false;
    //printf("nhan duoc:%x\n",dataB[iRec].data[0]);

    return;
    printf("len:%d\n", header->len);
    //printf("%.6d len:%d\n", header->ts.tv_usec, header->len);
    for(short i=0;i<dataB[iRec].len;i++)
    {
        printf("%x-",dataB[iRec].data[i]);
    }
    printf("\n");

}
void dataProcessingThread::run()
{
    pcap_if_t *alldevs;
    pcap_if_t *d;
    pcap_t *adhandle;
    char errbuf[PCAP_ERRBUF_SIZE];
    //
    /* Retrieve the device list on the local machine */
    if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
    {
        isRunning = false;
        printf( errbuf); return;
    }
    isRunning = true;
//    int i = 0;
    /* Print the list */
//    for(d=alldevs; d; d=d->next)
//    {
//        printf("%d. %s", ++i, d->name);
//        if (d->description)
//            printf(" (%s)\n", d->description);
//        else
//            printf(" (No description available)\n");
//    }
    d=alldevs;
    if ( (adhandle= pcap_open(d->name,          // name of the device
                                  65536,            // portion of the packet to capture
                                                    // 65536 guarantees that the whole packet will be captured on all the link layers
                                  PCAP_OPENFLAG_PROMISCUOUS,    // promiscuous mode
                                  1000,             // read timeout
                                  NULL,             // authentication on the remote machine
                                  errbuf            // error buffer
                                  ) ) == NULL)
        {
            /* Free the device list */
            pcap_freealldevs(alldevs);
            return ;
        }
    printf("\nlistening on %s...\n", d->description);

    /* start the capture */
    pcap_loop(adhandle, 0, packet_handler, NULL);
    return;
    //__________
    /*setPriority(QThread::TimeCriticalPriority);
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
    }*/
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

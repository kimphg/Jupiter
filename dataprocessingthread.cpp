#include "dataprocessingthread.h"
#define MAX_IREC 500

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
    if(iRec!=iRead)
    {
        connect_timeout = 4;
    }else
    {
        if(connect_timeout)connect_timeout--;
    }
    short nread = 0;
    while(iRec!=iRead)
    {
        nread++;
        DataBuff *pData = &dataBuff[iRead];
        if(nread>400)
        {
            radarData->resetData();
            break;
        }
        radarData->GetDataHR(&pData->data[0],pData->len);
        if(isRecording)
        {
            signRecFile.write((char*)&pData->len,2);
            signRecFile.write((char*)&pData->data[0],pData->len);
        }
        iRead++;
        if(iRead>=MAX_IREC)iRead=0;
    }
}
dataProcessingThread::dataProcessingThread()
{
    dataBuff = &dataB[0];
    iRec=0;iRead=0;
    connect_timeout = 0;
    pIsDrawn = &isDrawn;
    isDrawn = true;
    pIsPlaying = &isPlaying;
    playRate = 10;
    arpaData = new C_ARPA_data();
    isRecording = false;
    radarData = new C_radar_data();
    isPlaying = false;
    radarSocket = new QUdpSocket(this);
    radarSocket->bind(5555, QUdpSocket::ShareAddress);
    connect(&UpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateRadar()));
    UpdateTimer.start(200);
}
void dataProcessingThread::UpdateRadar()
{
    if(radarComQ.size())
    {

        // check if the radar has already recieved the command
//        if(radarData->checkFeedback(&radarComQ.front().bytes[0]))
//        {
//            radarComQ.pop();
//        }
//        if(radarComQ.size())
        radarSocket->writeDatagram((char*)&radarComQ.front().bytes[0],
                                    8,
                                    QHostAddress("192.168.0.44"),2572
                                    );
        radarComQ.pop();
    }
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
    radarSocket->bind(portNumber, QUdpSocket::ShareAddress);
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
//    (VOID)(param);
//    (VOID)(pkt_data);

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
//    printf("len:%d\n", header->len);
//    //printf("%.6d len:%d\n", header->ts.tv_usec, header->len);
//    for(short i=0;i<dataB[iRec].len;i++)
//    {
//        printf("%x-",dataB[iRec].data[i]);
//    }
//    printf("\n");

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
    int i = 0;
    /* Print the list */
    for(d=alldevs; d; d=d->next)
    {
        printf("%d. %s", ++i, d->name);
        if (d->description)
            printf(" (%s)\n", d->description);
        else
            printf(" (No description available)\n");
    }
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

void dataProcessingThread::radRequestTemp( char index)
{
    RadarCommand command;
    command.bytes[0] = 0xaa;
    command.bytes[1] = 0xab;
    command.bytes[2] = index;
    command.bytes[3] = 0xaa;
    command.bytes[4] = 0x00;
    command.bytes[5] = 0x00;
    command.bytes[6] = 0x00;
    command.bytes[7] = 0;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)
        radarComQ.push(command);
}

void dataProcessingThread::radTxOn()
{
    RadarCommand command;
    //rotation on
    command.bytes[0] = 0xaa;
    command.bytes[1] = 0xab;
    command.bytes[2] = 0x03;
    command.bytes[3] = 0x03;
    command.bytes[4] = 0x00;
    command.bytes[5] = 0x00;
    command.bytes[6] = 0x00;
    command.bytes[7] = 0x00;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //tx off
    command.bytes[0] = 0xaa;
    command.bytes[2] = 0x02;
    command.bytes[3] = 0x00;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //thich nghi
    command.bytes[0] = 0x1a;
    command.bytes[2] = 0x20;
    command.bytes[3] = 0x01;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //do trong
    command.bytes[0] = 0x14;
    command.bytes[2] = 0xff;
    command.bytes[3] = 0x01;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //set 1536
    command.bytes[0] = 0x04;
    command.bytes[2] = 0x00;
    command.bytes[3] = 0x06;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //dttt 256
    command.bytes[0] = 0x01;
    command.bytes[2] = 0x04;
    command.bytes[3] = 0x03;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //set resolution 60m
    command.bytes[0] = 0x08;
    command.bytes[2] = 0x02;
    command.bytes[3] = 0x00;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //tat thich nghi
    command.bytes[0] = 0x1a;
    command.bytes[2] = 0x20;
    command.bytes[3] = 0x00;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);

    //tx on 1
    command.bytes[0] = 0xaa;
    command.bytes[2] = 0x02;
    command.bytes[3] = 0x01;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //tx on 2
    command.bytes[2] = 0x00;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
//    if(1){
//        QFile logFile;
//        QDateTime now = QDateTime::currentDateTime();
//        if(!QDir("C:\\logs\\"+now.toString("\\dd.MM\\")).exists())
//        {
//            QDir().mkdir("C:\\logs\\"+now.toString("\\dd.MM\\"));
//        }
//        logFile.setFileName("C:\\logs\\"+now.toString("\\dd.MM\\")+now.toString("dd.MM-hh.mm.ss")+"_tx_on.log");

//        logFile.open(QIODevice::WriteOnly);
//        //logFile.p
//        logFile.close();
//    }


}

void dataProcessingThread::radTxOff()
{
    RadarCommand command;
    //rotation on
    command.bytes[0] = 0xaa;
    command.bytes[1] = 0xab;
    command.bytes[2] = 0x00;
    command.bytes[3] = 0x00;
    command.bytes[4] = 0x00;
    command.bytes[5] = 0x00;
    command.bytes[6] = 0x00;
    command.bytes[7] = 0x00;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    command.bytes[2] = 0x02;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    command.bytes[2] = 0x03;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
//    if(1)
//    {
//        QFile logFile;
//        QDateTime now = QDateTime::currentDateTime();
//        if(!QDir("C:\\logs\\"+now.toString("\\dd.MM\\")).exists())
//        {
//            QDir().mkdir("C:\\logs\\"+now.toString("\\dd.MM\\"));
//        }
//        logFile.setFileName("C:\\logs\\"+now.toString("\\dd.MM\\")+now.toString("dd.MM-hh.mm.ss")+"_tx_off.log");
//        logFile.open(QIODevice::WriteOnly);
//        //logFile.p
//        logFile.close();

    //    }
}

void dataProcessingThread::sendCommand(unsigned char *sendBuff, short len)
{
    RadarCommand command;
    memcpy(&command.bytes[0],sendBuff,7);
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
}

void dataProcessingThread::listenToRadar()
{

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

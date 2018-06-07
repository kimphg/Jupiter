#include "c_config.h"
#include "c_radar_thread.h"
#define MAX_IREC 500
//#include <QGeoCoordinate>
#include <QNmeaPositionInfoSource>
DataBuff dataB[MAX_IREC];
short iRec=0,iRead=0;
bool *pIsDrawn;
bool *pIsPlaying;
extern CConfig         mGlobbalConfig;
QNmeaPositionInfoSource *geoLocation = NULL;
//QTimer readDataBuff;
dataProcessingThread::~dataProcessingThread()
{
    delete mRadarData;
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
            mRadarData->resetData();
            break;
        }
        mRadarData->assembleDataFrame(&pData->data[0],pData->len);
        //read encoder
//        if(mRadarData->isEncoderAzi)
//        {
//            if(mEncoderPort.isOpen())
//            {
//                while(1)
//                {
//                    QString str(mEncoderPort.readLine());
//                    if(str.size()==0)break;
//                    int value = str.toInt();
//                    value = (value*MAX_AZIR)/5400;
//                    if(value>MAX_AZIR)value-=MAX_AZIR;
//                    mRadarData->mEncoderAzi = value;
//                }

//            }
//        }

        if(isRecording)
        {
            signRecFile.write((char*)&pData->len,2);
            signRecFile.write((char*)&pData->data[0],pData->len);
        }
        iRead++;
        if(iRead>=MAX_IREC)iRead=0;
    }
}
void dataProcessingThread::setRotationSpeed(int index)
{
    unsigned char command[7];
    command[0]=0xaa;
    command[1]=0xab;
    command[2]=0x03;
    command[3]=index;
    command[4]=0x00;
    command[5]=0x00;
    command[6]=0x00;
    sendCommand(&command[0],7);
    sendCommand(&command[0],7);
    sendCommand(&command[0],7);
}

bool dataProcessingThread::getIsXuLyThuCap() const
{
    return isXuLyThuCap;
}

void dataProcessingThread::setIsXuLyThuCap(bool value)
{
    isXuLyThuCap = value;
    mRadarData->setIsVtorih(isXuLyThuCap);
}

double dataProcessingThread::getSelsynAzi() const
{
    return selsynEncoderAzi;
}
dataProcessingThread::dataProcessingThread()
{
    failureCount = 0;
    isHeadingAvaible=false;
    selsynEncoderAzi = 0;
    isXuLyThuCap = false;
    dataBuff = &dataB[0];
    iRec=0;iRead=0;
    connect_timeout = 0;
    pIsDrawn = &isDrawn;
    isDrawn = true;
    pIsPlaying = &isPlaying;
    playRate = 10;
    arpaData = new C_ARPA_data();
    isRecording = false;
    mRadarData = new C_radar_data();
    isPlaying = false;
    radarSocket = new QUdpSocket(this);
    int port = 8000;
    while(port<9000)
    {
        if(radarSocket->bind(port))
        {
            break;
        }
        port++;
    }
    connect(&commandSendTimer, &QTimer::timeout, this, &dataProcessingThread::PushCommandQueue);
    commandSendTimer.start(100);
    connect(&readUdpBuffTimer, &QTimer::timeout, this, &dataProcessingThread::ReadDataBuffer);
    readUdpBuffTimer.start(10);
    connect(&readSerialTimer, &QTimer::timeout, this, &dataProcessingThread::SerialDataRead);
    readSerialTimer.start(20);
    initSerialComm();
    //processSerialData("!AIVDM,1,1,,A,13EoN=0P00NqIS<@6Od00?vN0D1F,0*5D");
}
/*QString str="";
void dataProcessingThread::SerialEncoderRead()
{
    if(!mEncoderPort.isOpen())return;
    QByteArray ba = mEncoderPort.readAll();
    if(!ba.size())return;
    if(!mRadarData->isEncoderAzi)return;
    str.append(ba);
    QStringList strList = str.split('$');
    if(strList.size()>2)
    {
        QStringList lsstrData = strList.at(1).split('*');
        if(lsstrData.size()==2)
        {

            int value = lsstrData.at(0).toInt();
            int value2 = lsstrData.at(1).toInt();
            if(value==value2)
            {
                if(value>0&&value<MAX_AZIR)mRadarData->setSelfRotationAzi(value);
            }
        }
        str="";
        //str.remove(0,strList.at(0).size()+strList.at(1).size()+1);
        //SerialEncoderRead();
    }
}*/
void dataProcessingThread::initSerialComm()
{
    //int serialBaud = 1000000;
    //baurate 1Mbps for highspeed encoder
    //QString SerialEncoder1Mb =  mGlobbalConfig.getString("serialEncoder1Mb");
    //mEncoderPort = new QSerialPort(this);
    //QString qstr = SerialEncoder1Mb;
    //mEncoderPort.setPortName(qstr);
    //mEncoderPort.setBaudRate(serialBaud);

    //mEncoderPort.setFlowControl(QSerialPort::NoFlowControl);
    /*if(mEncoderPort.open(QIODevice::ReadWrite))
    {
        mRadarData->isEncoderAzi = mGlobbalConfig.getInt("isSerialEncoderEnable");
        //connect(&mEncoderPort, &QSerialPort::readyRead, this, &dataProcessingThread::SerialEncoderRead);
        printf("\nencoder connected");
    }*/
    // baudrate at 38400 standart for sector encoder and ais
    int serialBaud = 38400;
    QList<QSerialPortInfo> portlist = QSerialPortInfo::availablePorts();
    for(int i = 0;i<portlist.size();i++)
    {
        if(!portlist.at(i).isBusy())
        {
            QSerialPort *newport = new QSerialPort(this);
            QString qstr = portlist.at(i).portName();
            newport->setPortName(qstr);
            newport->setBaudRate(serialBaud);
            newport->open(QIODevice::ReadWrite);
            connect(newport, &QSerialPort::readyRead, this, &dataProcessingThread::SerialDataRead);
            serialPorts.push_back(newport);
        }
    }
    printf("Serial available:%d\n",portlist.size());
}
bool  dataProcessingThread::getPosition(double *lat,double *lon)
{
    if(!geoLocation)return false;

    QGeoPositionInfo location = geoLocation->lastKnownPosition();
    *lat = location.coordinate().latitude();
    *lon = location.coordinate().longitude();
    return true;
}

double dataProcessingThread::getHeading() const
{
    return mHeading;
}

void dataProcessingThread::gpsupdate(QGeoPositionInfo geo)
{
    //geo.HorizontalAccuracy
    return;
}
void dataProcessingThread::SerialDataRead()
{
    for(std::vector<QSerialPort*>::iterator it = serialPorts.begin() ; it != serialPorts.end(); ++it)
    {
        QByteArray responseData = (*it)->readAll();
        if(!geoLocation)
        {
            if(responseData.contains("$GP")&&(!responseData.contains("HDT")))
            {
                geoLocation = new QNmeaPositionInfoSource(QNmeaPositionInfoSource::RealTimeMode,this);
                QString nameport( (*it)->portName() );
                (*it)->close();
                serialPorts.erase(it);
                QSerialPort *newport = new QSerialPort();
                newport->setPortName(nameport);
                newport->setBaudRate(9600);
                newport->open(QIODevice::ReadWrite);
                geoLocation->setDevice(newport);
                geoLocation->startUpdates();
                //                //connect(&geoLocation, QNmeaPositionInfoSource::positionUpdated(), this, &dataProcessingThread::gpsupdate);
                connect(geoLocation, SIGNAL(positionUpdated(QGeoPositionInfo)),
                        this, SLOT(gpsupdate(QGeoPositionInfo)));
                return;

            }
        }
        if(responseData.size())
        {
            processSerialData(responseData);
        }

    }
}
void dataProcessingThread::processSerialData(QByteArray inputData)
{

    unsigned short len = inputData.length();
    unsigned char* data = (unsigned char*)inputData.data();
    if(isRecording)
    {
        signRecFile.write((char*)&len,2);
        signRecFile.write((char*)data,len);
    }
    if(data[0]==0xff)//encoder data
    {
        mazi = (data[1]<<16) + (data[2]<<8)+(data[3]);
        realazi1 = (data[4]);
        realazi2 = (data[5]);
        newAzi = mazi*360.0/262144.0*3.0;
        while(newAzi>=360)newAzi-=360;
        selsynEncoderAzi = newAzi;
    }
    //    else if(data[0]==0x24)//NMEA
    //    {
    //         QString s_data = QString::fromLatin1(inputData.data());
    //         if(s_data.contains("HDT"))
    //         {
    //              mHeading = s_data.split(',').at(1).toDouble();
    //              isHeadingAvaible = true;
    //         }
    //    }
    else
    {

        processARPAData(inputData);
        //printf("arpa ais\n");

    }

}
bool dataProcessingThread::checkFeedback()
{
    unsigned char * pFeedBack = mRadarData->getFeedback();
    unsigned char * command = &radarComQ.front().bytes[0];
    if(   (pFeedBack[0]==command[0])
          &&(pFeedBack[1]==command[1])
          &&(pFeedBack[2]==command[2])
          &&(pFeedBack[3]==command[3])
          &&(pFeedBack[4]==command[4])
          &&(pFeedBack[5]==command[5])
          &&(pFeedBack[6]==command[6])
          )
    {
        return true;
    }
    else return false;
}
void dataProcessingThread::PushCommandQueue()
{
    if(radarComQ.size())
    {
        if(checkFeedback())
        {
            radarComQ.pop();
            failureCount = 0;
        }
        else
        {
            failureCount++;
            if(failureCount>3)
            {
                radarComQ.pop();
                failureCount = 0;
            }
        }
        radarSocket->writeDatagram((char*)&radarComQ.front().bytes[0],
                8,
                QHostAddress("192.168.0.44"),2572
                );

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
                mRadarData->SelfRotationReset();
                //togglePlayPause(false);
                return;
            }
            QByteArray buff;
            buff.resize(len);
            signRepFile.read(buff.data(),len);
            if(len>500)mRadarData->assembleDataFrame((unsigned char*)buff.data(),buff.size());
            else processSerialData(buff);
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

void dataProcessingThread::StopProcessing()
{

    deleteLater();
}
void dataProcessingThread::setIsDrawn(bool value)
{
    isDrawn = value;
}

void dataProcessingThread::SetRadarPort( unsigned short portNumber)
{
    radarSocket->bind(portNumber, QUdpSocket::ShareAddress);
}
void dataProcessingThread::SetARPAPort( unsigned short portNumber)
{
    ARPADataSocket->bind(portNumber, QUdpSocket::ShareAddress);
}

void dataProcessingThread::loadRecordDataFile(QString fileName)//
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
QString messageStringbuffer;
void dataProcessingThread::processARPAData(QByteArray inputdata)
{
    messageStringbuffer.append(QString::fromLatin1(inputdata));
    if(messageStringbuffer.size()>100)messageStringbuffer = "";
    //printf(inputdata.data());
    QStringList strlist = messageStringbuffer.split("\r\n");
    if(strlist.size() <= 1)return;

    for(int i = 0;i<strlist.size();i++)
        if(aisMessageHandler.ProcessNMEA(strlist.at(i)))
        {
            AIS_object_t newAisObj ;


            newAisObj.mMMSI = aisMessageHandler.get_mmsi();
//            if(574098083 == newAisObj.mMMSI)
//            {
//                newAisObj.mMMSI=newAisObj.mMMSI;
//            }//remove later
            if(aisMessageHandler.get_type()==AIS::AIS_MSG_24_STATIC_DATA_REPORT)
            {
                if(aisMessageHandler.get_partno()==0)
                {
                    newAisObj.mName = QString::fromLatin1(aisMessageHandler.get_shipname());
                    newAisObj.mType = 0;
                }
                else
                {
                    newAisObj.mType = aisMessageHandler.get_shiptype();

                }
            }
            else
            {
                newAisObj.mName = QString::fromLatin1(aisMessageHandler.get_shipname());
                newAisObj.mType = aisMessageHandler.get_shiptype();
            }
            newAisObj.mDst = QString(aisMessageHandler.get_destination());
            newAisObj.mImo = aisMessageHandler.get_imo();
            newAisObj.mNavStat = aisMessageHandler.get_navStatus();
            newAisObj.mBow = aisMessageHandler.get_to_bow();
            newAisObj.mStern = aisMessageHandler.get_to_stern();
            newAisObj.mStarboard = aisMessageHandler.get_to_starboard();
            newAisObj.mPort = aisMessageHandler.get_to_port();
            newAisObj.mSog = aisMessageHandler.get_SOG()/10.0;
            newAisObj.mCog = aisMessageHandler.get_COG()/10.0;
            newAisObj.mLat = aisMessageHandler.get_latitude()/600000.0;
            newAisObj.mLong = aisMessageHandler.get_longitude()/600000.0;
            newAisObj.mLut = QDateTime::currentMSecsSinceEpoch();
            newAisObj.isNewest = true;
            newAisObj.isSelected = false;
            newAisObj.mName.replace('@',"");
            QMutableListIterator<AIS_object_t> i(m_aisList);
            int elecount = 0;
            while (i.hasNext())
            {
                AIS_object_t obj = i.next();
                elecount++;
                if(elecount>3000){i.remove();continue;}
                if(obj.mMMSI==newAisObj.mMMSI)
                {
                    obj.isNewest = false;
                    newAisObj.isSelected = obj.isSelected;
                    if(newAisObj.mName.isEmpty()&&(!obj.mName.isEmpty()))
                        newAisObj.mName = obj.mName;
                    if(newAisObj.mLat==0)newAisObj.mLat = obj.mLat;
                    if(newAisObj.mLong==0)newAisObj.mLong = obj.mLong;
                    if(newAisObj.mDst.isEmpty())
                        newAisObj.mDst      = obj.mDst;
                    if(!newAisObj.mImo)
                        newAisObj.mImo      = obj.mImo;
                    if(!newAisObj.mType)
                        newAisObj.mType      = obj.mType;
                    if(!newAisObj.mBow)
                        newAisObj.mBow      = obj.mBow;
                    if(!newAisObj.mStern)
                        newAisObj.mStern      = obj.mStern;
                    if(!newAisObj.mStarboard)
                        newAisObj.mStarboard      = obj.mStarboard;
                    if(!newAisObj.mPort)
                        newAisObj.mPort      = obj.mPort;
                    if(!newAisObj.mSog)
                        newAisObj.mSog      = obj.mSog;
                    if(!newAisObj.mCog)
                        newAisObj.mCog      = obj.mCog;
                    i.setValue(obj);
                    break;
                }

            }
            m_aisList.push_front(newAisObj);

            //            mLONG = mLONG;

        }
    messageStringbuffer=strlist.at(strlist.size()-1);

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
    if(header->len<1000)return;
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
QTimer *timer_read_buffer;
void dataProcessingThread::run()
{

    //init();
    pcap_if_t *alldevs;
    pcap_if_t *d;
    pcap_t *adhandle;
    char errbuf[PCAP_ERRBUF_SIZE];
    //
    /* Retrieve the device list on the local machine */
    if (pcap_findalldevs_ex((char*)PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
    {
        //isRunning = false;
        printf( errbuf); return;
    }
    //isRunning = true;
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

bool dataProcessingThread::getIsDrawn()
{

    if(!isDrawn){isDrawn = true;return false;}
    else return true;

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
    unsigned char bytes[8]={0,0,0,0,0,0,0,0};
    //rotation on
    bytes[1] = 0xab;
    //setRotationSpeed(3);
    //    //tx off
    //    command.bytes[0] = 0xaa;
    //    command.bytes[2] = 0x02;
    //    command.bytes[3] = 0x00;
    //    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);

    //thich nghi
    bytes[0] = 0xaa;
    bytes[2] = 0x02;
    bytes[3] = 0x01;
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);
    //if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //do trong
    bytes[0] = 0xaa;
    bytes[2] = 0x00;
    bytes[3] = 0x07;
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);
    //if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //0x18 - 0xab - 0x01 - 0x0f
    //dttt 192
    bytes[0] = 0x18;
    bytes[2] = 0x01;
    bytes[3] = 0x0f;
    sendCommand(&bytes[0],7);

    //if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //dttt 192
    bytes[0] = 0x14;
    bytes[2] = 0xff;
    bytes[3] = 0x02;
    sendCommand(&bytes[0],7);
    //if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //dttt 192
    bytes[0] = 0x01;
    bytes[2] = 0x02;
    bytes[3] = 0x07;
    sendCommand(&bytes[0],7);
    //if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //dttt 192
    bytes[0] = 0x15;
    bytes[2] = 0x01;
    bytes[3] = 0x00;
    sendCommand(&bytes[0],7);
    //if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //set resolution 60m
    /*command.bytes[0] = 0x08;
    command.bytes[2] = 0x02;
    command.bytes[3] = 0x00;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);*/
    //    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    //    //tat thich nghi
    //    command.bytes[0] = 0x1a;
    //    command.bytes[2] = 0x20;
    //    command.bytes[3] = 0x00;
    //    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);
    /*
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
    //tx on tien khuech
    command.bytes[0] = 0x11;
    command.bytes[2] = 0xff;
    command.bytes[3] = 0x0f;
    command.bytes[4] = 0xff;
    command.bytes[5] = 0x05;
    if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);

*/
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
    //RadarCommand command;
    unsigned char bytes[8]={0xaa,0xab,0,0,0,0,0,0};

    //if(radarComQ.size()<MAX_COMMAND_QUEUE_SIZE)radarComQ.push(command);

    bytes[3] = 0x06;
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);
    bytes[2] = 0x02;
    bytes[3] = 0x00;
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);
    sendCommand(&bytes[0],7);
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
    if(len>8)return;
    command.bytes[7] = 0;
    memset(&command.bytes[0],0,8);
    memcpy(&command.bytes[0],sendBuff,len);
    for(int i=0;i<len-1;i++)
    {
        command.bytes[7]+=command.bytes[i];
    }
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

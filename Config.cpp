
#include "Config.h"

 
CConfig::CConfig(void)
{
    LoadConfigFile();
}

CConfig::~CConfig(void)
{
}

void CConfig::SaveToFile()
{
    QFile configFile(CFG_FILE);
    if(!configFile.open(QIODevice::WriteOnly))return;

    QTextStream outStream(&configFile);

    outStream<<m_config.mapFilename.data()<<"\n";
    outStream<<QString::number(m_config.m_lat)<<"\n";
    outStream<<QString::number(m_config.m_long)<<"\n";
    outStream<<QString::number(m_config.scale)<<"\n";
    outStream<<QString::number(m_config.trueN)<<"\n";
    outStream<<QString::number(m_config.dxView)<<"\n";
    outStream<<QString::number(m_config.dyView)<<"\n";
    outStream<<QString::number(m_config.mapEnabled)<<"\n";
    outStream<<QString::number(m_config.cfarThresh)<<"\n";
    outStream<<QString::number(m_config.codeType)<<"\n";
    //Close the file
    configFile.close();
}

int CConfig::setConfig(Config_t* config)
{
	m_config = *config;
	return 0;
}
Config_t* CConfig::getConfig()
{
	return &m_config;
}
void CConfig::setDefault()
{
	//m_config.comPort.Empty();
    //m_config.comRate    = 4800;
    m_config.m_lat      = DEFAULT_LAT;//20.8;20.705434, 106.785371
    m_config.m_long     = DEFAULT_LONG;//106.87;
    m_config.scale      = SCALE_MIN;
    m_config.trueN      = 0;
    m_config.dxView     = 0;
    m_config.dyView     = 0;
    m_config.mapEnabled = false;
    m_config.cfarThresh = 15;
    m_config.codeType   = 0;
    SaveToFile();
	//m_config.mapFilename.Empty();
	
}


bool CConfig::LoadConfigFile()
{

    QFile file(CFG_FILE);
    if(!file.open(QIODevice::ReadOnly))
    {
        setDefault();
        return false;
    }
    QTextStream in(&file);
    QString line = in.readLine();// map file
    m_config.mapFilename = line.toStdString();
    line = in.readLine();//coordinate lat
    m_config.m_lat = line.toDouble();
    line = in.readLine();//coordinate long
    m_config.m_long = line.toDouble();
    line = in.readLine();// last scale NIM
    m_config.scale = line.toDouble();
    line = in.readLine();// true north
    m_config.trueN = line.toDouble();
    line = in.readLine();// dx NIM
    m_config.dxView = line.toDouble();
    line = in.readLine();//dy NIM
    m_config.dyView = line.toDouble();
    line = in.readLine();// enable map
    m_config.mapEnabled = line.toInt();
    line = in.readLine();// CFAR thresh
    m_config.cfarThresh = line.toInt();
    if((m_config.cfarThresh>40)||(m_config.cfarThresh<0))
    {
        m_config.cfarThresh = 30;
    }
    line = in.readLine();// modulation code
    m_config.codeType = line.toInt();
    file.close();
    return true;
}

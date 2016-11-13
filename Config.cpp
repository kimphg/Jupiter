
#include "Config.h"

 
CConfig::CConfig(void)
{
    LoadFromFile();
}

CConfig::~CConfig(void)
{
}

void CConfig::SaveToFile()
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement * radar_config = doc.NewElement("radar_pk");//new tinyxml2::XMLElement("radar_pk" );
    doc.LinkEndChild( radar_config );
    radar_config->SetAttribute("mLat",mLat);
    radar_config->SetAttribute("mLong",mLon);
    radar_config->SetAttribute("scale",mLon);
    radar_config->SetAttribute("trueN",mLon);
    radar_config->SetAttribute("mapFilename",this->mapFilename.data());
    doc.SaveFile(CFG_FILE_NAME);
    /*QFile configFile(CFG_FILE);
    if(!configFile.open(QIODevice::WriteOnly))return;

    QTextStream outStream(&configFile);

    outStream<<mapFilename.data()<<"\n";
    outStream<<QString::number(mLat)<<"\n";
    outStream<<QString::number(mLong)<<"\n";
    outStream<<QString::number(scale)<<"\n";
    outStream<<QString::number(trueN)<<"\n";
    outStream<<QString::number(dxView)<<"\n";
    outStream<<QString::number(dyView)<<"\n";
    outStream<<QString::number(mapEnabled)<<"\n";
    outStream<<QString::number(cfarThresh)<<"\n";
    outStream<<QString::number(codeType)<<"\n";
    //Close the file
    configFile.close();*/

}


void CConfig::setDefault()
{
	//m_config.comPort.Empty();
    //m_config.comRate    = 4800;
    mLat      = DEFAULT_LAT;//20.8;20.705434, 106.785371
    mLon     = DEFAULT_LONG;//106.87;
    scale      = SCALE_MIN;
    trueN      = 0;
    dxView     = 0;
    dyView     = 0;
    mapEnabled = false;
    cfarThresh = 15;
    codeType   = 0;
    this->mapFilename = "";
    SaveToFile();
	//m_config.mapFilename.Empty();
	
}


bool CConfig::LoadFromFile()
{
    tinyxml2::XMLDocument xmlDoc;
    if(xmlDoc.LoadFile(CFG_FILE_NAME)==0)
    {
        //TiXmlHandle hDoc(&doc);
        tinyxml2::XMLElement  *pParm;
        pParm = xmlDoc.FirstChildElement("radar_pk");
        pParm->QueryDoubleAttribute("mLat",&mLat);
        pParm->QueryDoubleAttribute("mLong",&mLon);
        pParm->QueryDoubleAttribute("scale",&mLon);
        pParm->QueryDoubleAttribute("trueN",&mLon);
        const char *pName=pParm->Attribute("mapFilename");
        if(pName)mapFilename.append(pName);
        return true;
    }
    else
    {
        printf("Could not load config File.");
        SaveToFile();
        return false;
    }
    /*
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
    return true;*/
}

double CConfig::getLat() const
{
    return mLat;
}

void CConfig::setLat(double lat)
{
    mLat = lat;
}

double CConfig::getLon() const
{
    return mLon;
}

void CConfig::setLon(double lon)
{
    mLon = lon;
}

std::string CConfig::getMapFilename() const
{
    return mapFilename;
}

void CConfig::setMapFilename(const char *filename)
{
    mapFilename.clear();
    mapFilename.append(filename);
}

float CConfig::getTrueN() const
{
    return trueN;
}

void CConfig::setTrueN(float value)
{
    trueN = value;
}

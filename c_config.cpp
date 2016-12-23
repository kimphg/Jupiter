
#include "c_config.h"

 
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
    radar_config->SetAttribute("scale",scale);
    radar_config->SetAttribute("trueN",trueN);
    radar_config->SetAttribute("rangeView",rangeView);
    radar_config->SetAttribute("mapOpacity",mapOpacity);
    //printf("\nthis->mapFilename.data():");
    //printf(this->mapFilename.data());
    //radar_config->SetAttribute("mapFilename",this->mapFilename.data());
    radar_config->SetAttribute("socket_port_arpa",this->socket_port_arpa);
    radar_config->SetAttribute("socket_port_radar",this->socket_port_radar);
    doc.SaveFile(HR_CONFIG_FILE);
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
    rangeView = 5;
    mapOpacity = 0.4;
    mLat      = DEFAULT_LAT;//20.8;20.705434, 106.785371
    mLon      = DEFAULT_LONG;//106.87;
    scale      = SCALE_MIN;
    trueN      = 0;
    dxView     = 0;
    dyView     = 0;
    mapEnabled = false;
    cfarThresh = 15;
    codeType   = 0;
    socket_port_arpa = 8800;
    socket_port_radar =8900;
    SaveToFile();

}


bool CConfig::LoadFromFile()
{
    tinyxml2::XMLDocument xmlDoc;
    if(xmlDoc.LoadFile(HR_CONFIG_FILE)==0)
    {
        //TiXmlHandle hDoc(&doc);
        tinyxml2::XMLElement  *pParm;
        pParm = xmlDoc.FirstChildElement("radar_pk");
        pParm->QueryDoubleAttribute("mLat",&mLat);
        pParm->QueryDoubleAttribute("mLong",&mLon);
        pParm->QueryDoubleAttribute("scale",&scale);
        pParm->QueryDoubleAttribute("trueN",&trueN);
        pParm->QueryDoubleAttribute("mapOpacity",&mapOpacity);
        pParm->QueryIntAttribute("rangeView",&rangeView);
        return true;
    }
    else
    {
        printf("Could not load config File.");
        setDefault();
        return false;
    }

}

double CConfig::getLat() const
{
    return mLat;
}

void CConfig::setLat(double lat)
{
    mLat = lat;
    SaveToFile();
}

double CConfig::getLon() const
{
    return mLon;
}

void CConfig::setLon(double lon)
{
    mLon = lon;
    SaveToFile();
}



float CConfig::getTrueN() const
{
    return trueN;
}

void CConfig::setTrueN(float value)
{
    trueN = value;
    SaveToFile();
}

double CConfig::getMapOpacity() const
{
    return mapOpacity;
}

void CConfig::setMapOpacity(double value)
{
    mapOpacity = value;
    SaveToFile();
}

short CConfig::getRangeView() const
{
    return rangeView;

}

void CConfig::setRangeView(int value)
{
    rangeView = value;
    SaveToFile();
}

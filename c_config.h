
#ifndef CONFIG_H
#define CONFIG_H

#define SCALE_MAX 80
#define SCALE_MIN 5
#define HR_APP_PATH       "D:/HR2D/"
#define HR_DATA_REC_PATH  "D:/HR2D/RecordData/"
#define HR_CONFIG_FILE    "D:/HR2D/radar_config.xml"
#define DEFAULT_LAT		20.707f
#define DEFAULT_LONG	106.78f
#define LAT_MIN			5
#define LAT_MAX			25
#define LON_MIN		105
#define LON_MAX		125
#include <QFile>
#include <QTextStream>
#include <string>
#include <tinyxml/tinyxml2.h>
enum MeasuringUnit{NauticalMile, Kilometer};
class CConfig
{
public:
	
	CConfig(void);
	~CConfig(void);
	void SaveToFile();
	void setDefault();
    bool LoadFromFile();
    double getLat() const;
    void setLat(double getLat);
    MeasuringUnit measUnit;
    double getLon() const;
    void setLon(double getLon);

    //std::string getMapFilename() const;
    //void setMapFilename(const char* filename);

    float getTrueN() const;
    void setTrueN(float value, float value2);

    double getMapOpacity() const;
    void setMapOpacity(double value);

    short getRangeView() const;
    void setRangeView(int value);

    MeasuringUnit getMeasUnit();
    void setMeasUnit(const MeasuringUnit &value);

    double getTrueN2() const;

private:
    double mLat;
    double mLon;
    //std::string mapFilename;
    char mapEnabled;
    char cfarThresh;
    char codeType;
    double trueN,trueN2;
    double scale;
    double mapOpacity;
    int rangeView;
    int nmkm;
    short dxView ,dyView;
    short socket_port_radar;
    short socket_port_arpa;
};
#endif

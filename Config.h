
#ifndef CONFIG_H
#define CONFIG_H

#define SCALE_MAX 80
#define SCALE_MIN 5

#define CFG_LAT     "Vi do "
#define CFG_LONG    "Kinh do "
#define CFG_SCALE   "Ty le "
#define CFG_DXVIEW  "Dx "
#define CFG_DYVIEW  "Dy "
#define CFG_MAPFILE "Map "
#define CFG_FILE    "config.isc"
#define DEFAULT_LAT		20.707f
#define DEFAULT_LONG	106.78f
#define LAT_MIN			5
#define LAT_MAX			25
#define LON_MIN		105
#define LON_MAX		125
#include <QFile>
#include <QTextStream>
#include <string>
//using namespace std;
struct Config_t {
	double m_lat;
	double m_long;
	//char mapFilename[200];
	std::string mapFilename;
    char mapEnabled;
    char cfarThresh;
    char codeType;
    float trueN;
	float scale;
	short   dxView ,dyView;
};
class CConfig
{
public:
	Config_t m_config;
	
	CConfig(void);
	~CConfig(void);

	void SaveToFile();
	Config_t* getConfig();
	int setConfig(Config_t* config);
	void setDefault();
	bool checkValue(){
        if(m_config.m_lat<LAT_MIN||m_config.m_lat>LAT_MAX||m_config.m_long < LON_MIN||m_config.m_long > LON_MAX)
		{
			m_config.m_lat = DEFAULT_LAT;
			m_config.m_long = DEFAULT_LONG;
            return 0;
		}
        return 1;
	}
	bool LoadConfigFile();
};
#endif

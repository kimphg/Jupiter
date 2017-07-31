
#ifndef CONFIG_H
#define CONFIG_H

#define SCALE_MAX 80
#define SCALE_MIN 5
#define HR_APP_PATH       "D:/HR2D/"
#define HR_DATA_REC_PATH  "D:/HR2D/RecordData/"
#define HR_CONFIG_FILE    "D:/HR2D/radar_config.xml"
#define HR_CONFIG_FILE_DF "D:/HR2D/radar_config_default.xml"
#define XML_ELEM_NAME     "radar_config"
#define DEFAULT_LAT		20.707f
#define DEFAULT_LONG	106.78f
#include <QFile>
#include <QHash>
#include <QXmlStreamReader>
class CConfig
{
public:
    CConfig(void);
    ~CConfig(void);
    QHash<QString, QString> hashData;
    void    setValue(QString key, double value);
    void    setValue(QString key,QString value);
    double  getDouble(QString key);
    QString getString(QString key);
    int     getInt(QString key);
    void    setDefault();
private:
    QXmlStreamReader xml;

    void readFile();
    void SaveToFile();
};
#endif

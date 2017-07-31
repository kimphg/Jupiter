
#include "c_config.h"


void CConfig::setValue(QString key, double value)
{
    QString strValue = QString::number(value);
    hashData.insert(key, strValue);
    SaveToFile();
}

void CConfig::setValue(QString key, QString value)
{
    hashData.insert(key, value);
    SaveToFile();
}

double CConfig::getDouble(QString key)
{
    if(hashData.find(key)!=hashData.end())
    return hashData.value(key).toDouble();
    else return 0;
}
int CConfig::getInt(QString key)
{
    if(hashData.find(key)!=hashData.end())
    return hashData.value(key).toInt();
    else return 0;
}
QString CConfig::getString(QString key)
{
    if(hashData.find(key)!=hashData.end())
    return hashData.value(key);
    else return QString::number(0);
}

CConfig::CConfig(void)
{

    readFile();
}

CConfig::~CConfig(void)
{
}

void CConfig::SaveToFile()
{
    QHash<QString, QString>::const_iterator it = hashData.constBegin();
    QXmlStreamAttributes attr;
    while (it != hashData.constEnd()) {
        attr.append(it.key(),it.value());
        ++it;
    }
    QXmlStreamWriter writer;
    QFile xmlFile(HR_CONFIG_FILE);
    xmlFile.open(QIODevice::WriteOnly);
    writer.setDevice(&xmlFile);
    writer.writeEmptyElement(XML_ELEM_NAME);
    writer.writeAttributes(attr);
    writer.writeEndElement();
    xmlFile.close();

}

void CConfig::setDefault()
{
    if (QFile::exists(HR_CONFIG_FILE))
    {
        QFile::remove(HR_CONFIG_FILE);
    }

    QFile::copy(HR_CONFIG_FILE_DF, HR_CONFIG_FILE);

}

void CConfig::readFile() {
    QFile xmlFile(HR_CONFIG_FILE);
    xmlFile.open(QIODevice::ReadOnly);
    xml.setDevice(&xmlFile);

    while (xml.readNextStartElement())
    {
        if(xml.name()==XML_ELEM_NAME)
        {
           for (uint i=0;i<xml.attributes().size();i++)
           {
               QXmlStreamAttribute attr = xml.attributes().at(i);
               hashData.insert( attr.name().toString(),
                                attr.value().toString());
           }
        }
        if (xml.tokenType() == QXmlStreamReader::Invalid)
            xml.readNext();
        // readNextStartElement() leaves the stream in
        // an invalid state at the end. A single readNext()
        // will advance us to EndDocument.
        if (xml.hasError()) {
            continue;
        }
    }
    xmlFile.close();
}



#include <QCommandLineParser>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QString>
#include <QDataStream>
#include <zip.h>

#include "geomaps/TripKit.h"


GeoMaps::TripKit::TripKit(const QString& fileName)
    : m_zip(fileName)
{
    if (!m_zip.isValid())
    {
        m_error = QObject::tr("File %1 is not a valid zip file.").arg(fileName);
        return;
    }

    {
        auto json = m_zip.extract("toc.json");
        if (json.isNull())
        {
            m_error = QObject::tr("The zip archive %1 does not contain the required file 'toc.json'.").arg(fileName);
            return;
        }
        auto jDoc = QJsonDocument::fromJson(json);
        if (jDoc.isNull())
        {
            m_error = QObject::tr("The file 'toc.json' from the zip archive %1 cannot be interpreted.").arg(fileName);
            return;
        }
        auto rootObject = jDoc.object();
        m_name = rootObject["name"].toString();
    }

    {
        auto json = m_zip.extract("charts/charts_toc.json");
        if (json.isNull())
        {
            m_error = QObject::tr("The zip archive %1 does not contain the required file 'charts/charts_toc.json'.").arg(fileName);
            return;
        }
        auto jDoc = QJsonDocument::fromJson(json);
        if (jDoc.isNull())
        {
            m_error = QObject::tr("The file 'charts/charts_toc.json' from the zip archive %1 cannot be interpreted.").arg(fileName);
            return;
        }
        auto rootObject = jDoc.object();
        m_charts = rootObject["charts"].toArray();
        if (m_charts.isEmpty())
        {
            m_error = QObject::tr("The trip kit %1 does not contain any charts.").arg(fileName);
            return;
        }
    }


}



void GeoMaps::TripKit::extract()
{

    foreach (auto chart, m_charts)
    {
        auto name = chart.toObject()["name"].toString();

        auto path = chart.toObject()["filePath"].toString();
        QString ending;
        auto idx = path.lastIndexOf('.');
        if (idx != -1)
        {
            ending = path.mid(idx+1, -1);
        }

        auto top = chart.toObject()["geoCorners"].toObject()["upperLeft"].toObject()["latitude"].toDouble();
        auto left = chart.toObject()["geoCorners"].toObject()["upperLeft"].toObject()["longitude"].toDouble();
        auto bottom = chart.toObject()["geoCorners"].toObject()["lowerRight"].toObject()["latitude"].toDouble();
        auto right = chart.toObject()["geoCorners"].toObject()["lowerRight"].toObject()["longitude"].toDouble();

        auto newPath = u"%1-geo_%2_%3_%4_%5.%6"_qs
                           .arg(name)
                           .arg(left)
                           .arg(top)
                           .arg(right)
                           .arg(bottom)
                           .arg(ending);
        qWarning() << name << path;
        auto imageData = m_zip.extract("charts/"+name+"-geo."+ending);
        QFile outFile(newPath);
        outFile.open(QIODeviceBase::WriteOnly);
        outFile.write(imageData);
        outFile.close();
    }
}

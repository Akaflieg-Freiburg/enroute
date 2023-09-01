
#include <QEventLoop>
#include <QImage>
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



void GeoMaps::TripKit::extract(const QString& directoryPath, int index)
{
    auto chart = m_charts.at(index);


//    foreach (auto chart, m_charts)
    {
        auto name = chart.toObject()[u"name"_qs].toString();

        auto path = chart.toObject()[u"filePath"_qs].toString();
        QString ending;
        auto idx = path.lastIndexOf('.');
        if (idx != -1)
        {
            ending = path.mid(idx+1, -1);
        }

        auto top = chart.toObject()[u"geoCorners"_qs].toObject()[u"upperLeft"_qs].toObject()[u"latitude"_qs].toDouble();
        auto left = chart.toObject()[u"geoCorners"_qs].toObject()[u"upperLeft"_qs].toObject()[u"longitude"_qs].toDouble();
        auto bottom = chart.toObject()[u"geoCorners"_qs].toObject()[u"lowerRight"_qs].toObject()[u"latitude"_qs].toDouble();
        auto right = chart.toObject()[u"geoCorners"_qs].toObject()[u"lowerRight"_qs].toObject()[u"longitude"_qs].toDouble();

        auto newPath = u"%1/%2-geo_%3_%4_%5_%6.webp"_qs
                           .arg(directoryPath, name)
                           .arg(left)
                           .arg(top)
                           .arg(right)
                           .arg(bottom);

        auto imageData = m_zip.extract(path);
        if (imageData.isEmpty())
        {
            imageData = m_zip.extract("charts/"+name+"-geo."+ending);
        }

        if (ending == "webp")
        {
            QFile outFile(newPath);
            outFile.open(QIODeviceBase::WriteOnly);
            outFile.write(imageData);
            outFile.close();
        }
        else
        {
            QImage img(imageData);
            img.save(newPath);
        }

    }
}

/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
 *   stefan.kebekus@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QEventLoop>
#include <QGeoCoordinate>
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
        auto json = m_zip.extract(u"toc.json"_qs);
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
        m_name = rootObject[u"name"_qs].toString();
    }

    {
        auto json = m_zip.extract(u"charts/charts_toc.json"_qs);
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
        m_charts = rootObject[u"charts"_qs].toArray();
        if (m_charts.isEmpty())
        {
            m_error = QObject::tr("The trip kit %1 does not contain any charts.").arg(fileName);
            return;
        }
    }
}

auto GeoMaps::TripKit::extract(const QString &directoryPath, qsizetype index) -> QString
{
    if ((index < 0) || (index >= m_charts.size()))
    {
        return {};
    }
    auto chart = m_charts.at(index);
    auto name = chart.toObject()[u"name"_qs].toString();
    if (name.isEmpty())
    {
        return {};
    }
    auto path = chart.toObject()[u"filePath"_qs].toString();
    if (path.isEmpty())
    {
        return {};
    }
    QString ending;
    auto idx = path.lastIndexOf('.');
    if (idx == -1)
    {
        return {};
    }
    ending = path.mid(idx+1, -1);

    auto top = chart.toObject()[u"geoCorners"_qs].toObject()[u"upperLeft"_qs].toObject()[u"latitude"_qs].toDouble();
    auto left = chart.toObject()[u"geoCorners"_qs].toObject()[u"upperLeft"_qs].toObject()[u"longitude"_qs].toDouble();
    auto bottom = chart.toObject()[u"geoCorners"_qs].toObject()[u"lowerRight"_qs].toObject()[u"latitude"_qs].toDouble();
    auto right = chart.toObject()[u"geoCorners"_qs].toObject()[u"lowerRight"_qs].toObject()[u"longitude"_qs].toDouble();

    QGeoCoordinate const topLeft(top, left);
    if (!topLeft.isValid())
    {
        return {};
    }
    QGeoCoordinate const bottomRight(bottom, right);
    if (!bottomRight.isValid())
    {
        return {};
    }


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
    if (imageData.isEmpty())
    {
        return {};
    }

    if (ending == u"webp"_qs)
    {
        QFile outFile(newPath);
        if (!outFile.open(QIODeviceBase::WriteOnly))
        {
            return {};
        }
        if (outFile.write(imageData) != imageData.size())
        {
            outFile.close();
            outFile.remove();
            return {};
        }
        outFile.close();
    }
    else
    {
        QImage const img = QImage::fromData(imageData);
        if (!img.save(newPath))
        {
            QFile::remove(newPath);
            return {};
        }
    }
    return newPath;
}

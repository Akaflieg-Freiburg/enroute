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

#include <QFile>
#include <QGeoCoordinate>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>

#include "fileFormats/TripKit.h"


FileFormats::TripKit::TripKit(const QString& fileName)
    : m_zip(fileName)
{
    if (!m_zip.isValid())
    {
        setError(m_zip.error());
        return;
    }

    {
        auto json = m_zip.extract(u"toc.json"_qs);
        if (json.isNull())
        {
            setError(QObject::tr("The zip archive %1 does not contain the required file 'toc.json'.", "FileFormats::TripKit").arg(fileName));
            return;
        }
        auto jDoc = QJsonDocument::fromJson(json);
        if (jDoc.isNull())
        {
            setError(QObject::tr("The file 'toc.json' from the zip archive %1 cannot be interpreted.", "FileFormats::TripKit").arg(fileName));
            return;
        }
        auto rootObject = jDoc.object();
        m_name = rootObject[u"name"_qs].toString();
    }

    {
        auto json = m_zip.extract(u"charts/charts_toc.json"_qs);
        if (json.isNull())
        {
            setError(QObject::tr("The zip archive %1 does not contain the required file 'charts/charts_toc.json'.", "FileFormats::TripKit").arg(fileName));
            return;
        }
        auto jDoc = QJsonDocument::fromJson(json);
        if (jDoc.isNull())
        {
            setError(QObject::tr("The file 'charts/charts_toc.json' from the zip archive %1 cannot be interpreted.", "FileFormats::TripKit").arg(fileName));
            return;
        }
        auto rootObject = jDoc.object();
        m_charts = rootObject[u"charts"_qs].toArray();
        if (m_charts.isEmpty())
        {
            setError(QObject::tr("The trip kit %1 does not contain any charts.", "FileFormats::TripKit").arg(fileName));
            return;
        }
    }
}

bool FileFormats::TripKit::extract(const QString &directoryPath, qsizetype index)
{
    if ((index < 0) || (index >= m_charts.size()))
    {
        return false;
    }
    auto chart = m_charts.at(index);
    auto name = chart.toObject()[u"name"_qs].toString();
    if (name.isEmpty())
    {
        return false;
    }
    auto path = chart.toObject()[u"filePath"_qs].toString();
    if (path.isEmpty())
    {
        return false;
    }
    QString ending;
    auto idx = path.lastIndexOf('.');
    if (idx == -1)
    {
        return false;
    }
    ending = path.mid(idx+1, -1);

    auto top = chart.toObject()[u"geoCorners"_qs].toObject()[u"upperLeft"_qs].toObject()[u"latitude"_qs].toDouble();
    auto left = chart.toObject()[u"geoCorners"_qs].toObject()[u"upperLeft"_qs].toObject()[u"longitude"_qs].toDouble();
    auto bottom = chart.toObject()[u"geoCorners"_qs].toObject()[u"lowerRight"_qs].toObject()[u"latitude"_qs].toDouble();
    auto right = chart.toObject()[u"geoCorners"_qs].toObject()[u"lowerRight"_qs].toObject()[u"longitude"_qs].toDouble();

    QGeoCoordinate const topLeft(top, left);
    if (!topLeft.isValid())
    {
        return false;
    }
    QGeoCoordinate const bottomRight(bottom, right);
    if (!bottomRight.isValid())
    {
        return false;
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
        return false;
    }

    if (ending == u"webp"_qs)
    {
        QFile outFile(newPath);
        if (!outFile.open(QIODeviceBase::WriteOnly))
        {
            return false;
        }
        if (outFile.write(imageData) != imageData.size())
        {
            outFile.close();
            outFile.remove();
            return false;
        }
        outFile.close();
    }
    else
    {
        QImage const img = QImage::fromData(imageData);
        if (!img.save(newPath))
        {
            QFile::remove(newPath);
            return false;
        }
    }
    return true;
}

/***************************************************************************
 *   Copyright (C) 2023 by Heinz Blöchinger                                *
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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <stdexcept>

#include "fileFormats/DataFileAbstract.h"
#include "OpenAir.h"

#include <cmath>

class AirSpace {
public:
    QString ac;
    QString an;
    QString al;
    QString ah;
    QChar variableD {'+'};
    QGeoCoordinate variableX;
    QVector<QGeoCoordinate> polygon;

    void addPoint(const QString& qs)
    {
        QGeoCoordinate const point = toCoord(qs);
        polygon.prepend(point);
    }

    void addCircle(const QString& qs)
    {
        bool ok = false;
        double const radius = qs.toDouble(&ok) * 1852;
        if (!ok)
        {
            throw QObject::tr("Invalid number found: %1", "OpenAir").arg(qs);
        }
        if (variableX.isValid())
        {
            for (int i=0; i <= 360; i += 10)
            {
                polygon.prepend(variableX.atDistanceAndAzimuth(radius, i));
            }
        }
        else
        {
            throw QObject::tr("Variable X is not set but Circle should be drawn", "OpenAir");
        }
    }

    void addArc(const QString& qs)
    {
        bool ok = false;
        QStringList items = qs.split(u',', Qt::SkipEmptyParts);
        double const radius = items[0].toDouble(&ok) * 1852;
        if (!ok)
        {
            throw QObject::tr("Invalid number found: %1", "OpenAir").arg(items[0]);
        }
        double const angleStart = items[1].toDouble(&ok);
        if (!ok)
        {
            throw QObject::tr("Invalid number found: %1", "OpenAir").arg(items[1]);
        }
        double const angleEnd = items[2].toDouble(&ok);
        if (!ok)
        {
            throw QObject::tr("Invalid number found: %1", "OpenAir").arg(items[2]);
        }
        if (variableX.isValid())
        {
            if (variableD == '-')
            {
                if (angleEnd > angleStart)
                {
                    addArcCounterClockwise(radius, angleStart, 0);
                    addArcCounterClockwise(radius, 360, angleEnd);
                }
                else
                {
                    addArcCounterClockwise(radius, angleStart, angleEnd);
                }
            }
            else
            {
                if (angleEnd < angleStart)
                {
                    addArcClockwise(radius,angleStart, 360);
                    addArcClockwise(radius, 0, angleEnd);
                }
                else
                {
                    addArcClockwise(radius,angleStart, angleEnd);
                }
            }
            polygon.prepend(variableX.atDistanceAndAzimuth(radius, angleEnd));
        }
        else
        {
            throw QObject::tr("Variable X is not set but Circle should be drawn", "OpenAir");
        }
    }

    void addArcPoints(const QString& qs)
    {
        QStringList items = qs.split(u',', Qt::SkipEmptyParts);
        QGeoCoordinate const startPoint = toCoord(items[0]);
        if (items[1].startsWith(u" "_qs))
        {
            items[1] = items[1].sliced(1);
        }
        QGeoCoordinate const endPoint = toCoord(items[1]);
        if (!variableX.isValid())
        {
            throw QObject::tr("Variable X is not set but Circle should be drawn", "OpenAir");
        }
        double const radius = variableX.distanceTo(startPoint);
        double const angleStart = variableX.azimuthTo(startPoint);
        double const angleEnd = variableX.azimuthTo(endPoint);
        if (variableD == '-')
        {
            if (angleEnd > angleStart)
            {
                addArcCounterClockwise(radius, angleStart, 0);
                addArcCounterClockwise(radius, 360, angleEnd);
            }
            else
            {
                addArcCounterClockwise(radius, angleStart, angleEnd);
            }
        }
        else
        {
            if (angleEnd < angleStart)
            {
                addArcClockwise(radius,angleStart, 360);
                addArcClockwise(radius, 0, angleEnd);
            }
            else
            {
                addArcClockwise(radius,angleStart, angleEnd);
            }
        }
        polygon.prepend(variableX.atDistanceAndAzimuth(radius, angleEnd));
    }

    void addArcClockwise(double radius, double start, double end)
    {
        if ((radius <= 0) || (start < -360.0) || (start > 360.0) || (end < -360.0) || (end > 360.0) || (start >= end))
        {
            throw QObject::tr("Invalid arc specification", "OpenAir");
        }

        do
        {
            polygon.prepend(variableX.atDistanceAndAzimuth(radius, start));
            start += 10;
        } while (start < end);
    }

    void addArcCounterClockwise(double radius, double start, double end)
    {
        if ((radius <= 0) || (start < -360.0) || (start > 360.0) || (end < -360.0) || (end > 360.0) || (start <= end))
        {
            throw QObject::tr("Invalid arc specification", "OpenAir");
        }

        do
        {
            polygon.prepend(variableX.atDistanceAndAzimuth(radius, start));
            start -= 10;
        } while (start > end);
    }

    /*
     * Final check if all information regarding one AirSpace is read:
     *  - close polygon if it is open
     *  - delete doubled points from the polygon
     *  - reverse polygon if it is clockwise
     *  - check if necessary information is complete (e.g. lower limit and upper limit are defined)
     */
    void finalize(QStringList& errorList)
    {
        if  (polygon.size() > 1)
        {
            //close polygon if it is open
            auto last = polygon.size() - 1;
            if ((polygon.at(0).latitude() != polygon.at(last).latitude()) ||
                (polygon.at(0).longitude() != polygon.at(last).longitude()))
            {
                polygon.prepend(polygon.at(last));
            }
            //delete doubled points
            for (auto i=polygon.size() - 1; i > 0; i--) {
                if ((polygon.at(i).latitude()  == polygon.at(i - 1).latitude() ) &&
                    (polygon.at(i).longitude() == polygon.at(i - 1).longitude()) )
                {
                    polygon.removeAt(i);
                }
            }
            //reverse polygon if it is clockwise
            if (isClockwise())
            {
                reversePolygon();
            }
        }
        if (al.size() < 1)
        {
            errorList.append("Lower Limit not set for AirSpace " + an);
        }
        if (ah.size() < 1)
        {
            errorList.append("Upper Limit not set for AirSpace " + an);
        }
        if ((ac.compare(u"A"_qs)   != 0) &&
            (ac.compare(u"ATZ"_qs) != 0) &&
            (ac.compare(u"B"_qs)   != 0) &&
            (ac.compare(u"CTR"_qs) != 0) &&
            (ac.compare(u"C"_qs)   != 0) &&
            (ac.compare(u"D"_qs)   != 0) &&
            (ac.compare(u"DNG"_qs) != 0) &&
            (ac.compare(u"FIR"_qs) != 0) &&
            (ac.compare(u"FIS"_qs) != 0) &&
            (ac.compare(u"GLD"_qs) != 0) &&
            (ac.compare(u"NRA"_qs) != 0) &&
            (ac.compare(u"P"_qs)   != 0) &&
            (ac.compare(u"PJE"_qs) != 0) &&
            (ac.compare(u"R"_qs)   != 0) &&
            (ac.compare(u"TMZ"_qs) != 0) &&
            (ac.compare(u"SUA"_qs) != 0)   )
        {
            ac = u"SUA"_qs;
        }
    }

    [[nodiscard]] bool isSet() const
    {
        return (ac.length() > 0);
    }

    void setHeight(QString qs, bool higher)
    {
        qs.replace(u"FL"_qs, u"FL "_qs);
        qs.replace(u"ft"_qs, u" ft"_qs);
        qs.replace(u"SFC"_qs, u"GND"_qs);
        qs.replace(u"agl"_qs, u"AGL"_qs);
        QStringList items = qs.split(u' ', Qt::SkipEmptyParts);
        if (items[0].compare(u"0"_qs) == 0)
        {
            items[0] = u"GND"_qs;
        }
        if ((items.size() > 1) && (items[1].compare(u"ft"_qs) == 0))
        {
            items.removeAt(1);
        }
        if ((items.size() > 1) && ((items[0].compare(u"FL"_qs) == 0) || (items[1].compare(u"AGL"_qs) == 0)))
        {
            items[0] = items[0] + " " + items[1];
        }
        if (higher)
        {
            ah = items[0];
        }
        else
        {
            al = items[0];
        }
    }

    void setVar(const QString& qs)
    {
        if (qs.startsWith(u"X="_qs))
        {
            variableX = toCoord(qs.sliced(2));
        }
        else if (qs.startsWith(u"D="_qs))
        {
            variableD = qs.at(2);
            if ((variableD != '-') && (variableD != '+'))
            {
                variableD = '+';
                throw QObject::tr("Invalid content for VariableD (direction): %1", "OpenAir").arg(qs.at(2));
            }
        }
    }

private:
    static double getNumber(const QString& degree)
    {
        bool ok = false;
        double ret = NAN;
        auto i = degree.indexOf(u":"_qs);
        if (i < 0)
        {
            ret = degree.toDouble(&ok);
            if (!ok)
            {
                throw QObject::tr("Invalid number found: %1", "OpenAir").arg(degree);
            }
            return ret;
        }
        ret = degree.first(i).toDouble(&ok) + getNumber(degree.sliced(i + 1)) / 60;
        if (!ok)
        {
            throw QObject::tr("Invalid number found: %1", "OpenAir").arg(degree.first(i));
        }
        return ret;
    }

    [[nodiscard]] bool isClockwise() const
    {
        double area = 0;
        qsizetype j = 0;

        for (auto i=0; i < polygon.size(); i++)
        {
            j = (i + 1) % polygon.size();
            area += polygon.at(i).longitude() * polygon.at(j).latitude() - polygon.at(j).longitude() * polygon.at(i).latitude();
        }
        // If the area is positive, the polygon is defined clockwise, otherwise counterclockwise
        return (area < 0);
    }

    void reversePolygon() {
        auto j = polygon.size() - 1;
        for (int i=0; i < j;i++, j--)
        {
            polygon.swapItemsAt(i, j);
        }
    }

    static QGeoCoordinate toCoord(const QString& qs)
    {
        double latitude = NAN;
        double longitude = NAN;
        QStringList items = qs.split(u' ', Qt::SkipEmptyParts);
        if (items[0].endsWith('N') || items[0].endsWith('S'))
        {
            items.insert(1, items[0].sliced(items[0].length() - 1));
            items[0].chop(1);
        }
        latitude = getNumber(items[0]);
        if (items[1].compare(u"S"_qs) == 0)
        {
            latitude *= -1;
        } else if (items[1].compare(u"N"_qs) != 0)
        {
            throw QObject::tr("Invalid coordinate found: %1", "OpenAir").arg(qs);
        }
        if (items[2].endsWith('W') || items[2].endsWith('E'))
        {
            items.insert(3, items[2].sliced(items[2].length() - 1));
            items[2].chop(1);
        }
        longitude = getNumber(items[2]);
        if (items[3].compare(u"W"_qs) == 0)
        {
            longitude *= -1;
        } else if (items[3].compare(u"E"_qs) != 0)
        {
            throw QObject::tr("Invalid coordinate found: %1", "OpenAir").arg(qs);
        }
        if ((latitude > 180) || (latitude < -180) || (longitude > 90) || (longitude < -90))
        {
            throw QObject::tr("Invalid coordinate found: %1", "OpenAir").arg(qs);
        }
        return {latitude, longitude};
    }
};


class AirSpaceVector {
private:
    QVector<AirSpace> airSpaceVector;

public:
    void addAirSpace(const AirSpace& airSpace)
    {
        airSpaceVector.append(airSpace);
    }

    bool isSameName(const QString& qs)
    {
        return (!airSpaceVector.empty()) && (airSpaceVector.last().an.compare(qs) == 0);
    }

    QGeoCoordinate getLastX()
    {
        return airSpaceVector.last().variableX;
    }

    QJsonDocument getJson(const QString& fileName)
    {
        QJsonObject recObj;
        QJsonObject featureObj;
        QJsonObject propObj;
        QJsonObject geomObj;
        QJsonArray featureArray;
        QJsonArray polygonArray;
        QJsonArray coordArray;
        QJsonArray coord;
        QGeoCoordinate const point;

        recObj.insert(u"type"_qs, QJsonValue::fromVariant("FeatureCollection"));
        recObj.insert(u"info"_qs, QJsonValue::fromVariant(fileName));

        for (int i=0; i < airSpaceVector.size(); i++)
        {
            featureObj.insert(u"type"_qs, QJsonValue::fromVariant("Feature"));
            propObj = QJsonObject();
            propObj.insert(u"NAM"_qs, QJsonValue::fromVariant(airSpaceVector.at(i).an));
            propObj.insert(u"ID"_qs, QJsonValue::fromVariant(airSpaceVector.at(i).an));
            propObj.insert(u"CAT"_qs, QJsonValue::fromVariant(airSpaceVector.at(i).ac));
            propObj.insert(u"TYP"_qs, QJsonValue::fromVariant("AS"));
            if (!airSpaceVector.at(i).al.isEmpty())
            {
                propObj.insert(u"BOT"_qs, QJsonValue::fromVariant(airSpaceVector.at(i).al));
            }
            if (!airSpaceVector.at(i).ah.isEmpty())
            {
                propObj.insert(u"TOP"_qs, QJsonValue::fromVariant(airSpaceVector.at(i).ah));
            }
            featureObj.insert(u"properties"_qs, propObj);

            while (coordArray.count() != 0)
            {
                coordArray.pop_back();
            }
            for (int j=0; j < airSpaceVector.at(i).polygon.size(); j++)
            {
                while (coord.count() != 0)
                {
                    coord.pop_back();
                }
                coord.append(airSpaceVector.at(i).polygon.at(j).longitude());
                coord.append(airSpaceVector.at(i).polygon.at(j).latitude());
                coordArray.append(coord);
            }
            while (polygonArray.count() != 0)
            {
                polygonArray.pop_back();
            }
            polygonArray.append(coordArray);
            if  (airSpaceVector.at(i).polygon.size() > 1)
            {
                geomObj.insert(u"type"_qs, QJsonValue::fromVariant("Polygon"));
                geomObj.insert(u"coordinates"_qs, polygonArray);
                featureObj.insert(u"geometry"_qs, geomObj);
            }


            featureArray.append(featureObj);
        }

        if (featureArray.isEmpty())
        {
            return {};
        }

        recObj.insert(u"features"_qs, featureArray);
        QJsonDocument json(recObj);
        return json;
    }
};


bool GeoMaps::openAir::isValid(const QString& fileName, QString* info)
{
    QStringList errorList;
    QStringList warnings;
    auto json = parse(fileName, errorList, warnings);

    if (info != nullptr)
    {
        *info = {};
        if (!warnings.isEmpty())
        {
            *info += u"<p>"_qs + QObject::tr("Warnings", "OpenAir") + u"</p>"_qs;
            *info += u"<ul style='margin-left:-25px;'>"_qs;
            foreach(auto warning, warnings)
            {
                *info += u"<li>"_qs + warning + u"</li>"_qs;
            }
            *info += u"</ul>"_qs;
        }
    }

    return (!json.isEmpty()) && errorList.isEmpty();
}


QJsonDocument GeoMaps::openAir::parse(const QString& fileName, QStringList& errorList, QStringList& warningList)
{
    QString line;
    AirSpace airSpace;
    AirSpaceVector airSpaceVector;


    auto inputFile = FileFormats::DataFileAbstract::openFileURL(fileName);
    if (!inputFile->open(QIODeviceBase::ReadOnly))
    {
        errorList << QObject::tr("Cannot open file %1", "OpenAir").arg(fileName);
        return {};
    }

    QTextStream inputStream(inputFile.data());
    inputStream.setEncoding(QStringConverter::Latin1);

    bool hadError = false;
    int lineNo = 0;
    while (inputStream.readLineInto(&line))
    {
        lineNo++;

        try {
            if (line.startsWith(u"*"_qs) || line.length() == 0)
            {
                continue;
            }
            if (line.startsWith(u"AC "_qs))
            {
                //if airSpace is already filled, the existing airSpace must be added to the list and a new airSpace must be initialized
                if (airSpace.isSet())
                {
                    if (!hadError)
                    {
                        airSpace.finalize(errorList);
                        airSpaceVector.addAirSpace(airSpace);
                    }
                    airSpace = AirSpace();
                    hadError = false;
                }
                airSpace.ac = line.sliced(3).trimmed();
                continue;
            }
            if (line.startsWith(u"AN "_qs))
            {
                airSpace.an = line.sliced(3);
                if (airSpaceVector.isSameName(airSpace.an))
                {
                    airSpace.variableX = airSpaceVector.getLastX();
                }
                continue;
            }
            if (line.startsWith(u"AL "_qs))
            {
                airSpace.setHeight(line.sliced(3), false);
                continue;
            }
            if (line.startsWith(u"AH "_qs))
            {
                airSpace.setHeight(line.sliced(3), true);
                continue;
            }
            if (line.startsWith(u"V "_qs))
            {
                airSpace.setVar(line.sliced(2));
                continue;
            }
            if (line.startsWith(u"DP "_qs))
            {
                airSpace.addPoint(line.sliced(3));
                continue;
            }
            if (line.startsWith(u"DC "_qs))
            {
                airSpace.addCircle(line.sliced(3));
                continue;
            }
            if (line.startsWith(u"DA "_qs))
            {
                airSpace.addArc(line.sliced(3));
                continue;
            }
            if (line.startsWith(u"DB "_qs))
            {
                airSpace.addArcPoints(line.sliced(3));
                continue;
            }
            if (line.startsWith(u"AT "_qs))
            {
                continue;
            }
            warningList.append(QObject::tr("Unrecognized record type in line %1: %2; Line ignored.", "OpenAir").arg(QString::number(lineNo), line));
        }
        catch (QString& ex)
        {
            hadError = true;
            warningList.append(QObject::tr("Error in line %1: %2; Airspace %3 ignored.", "OpenAir").arg(QString::number(lineNo), ex, airSpace.an));
        }
    }
    if (airSpace.isSet())
    {
        airSpace.finalize(errorList);
        airSpaceVector.addAirSpace(airSpace);
    }

    return airSpaceVector.getJson(fileName);
}

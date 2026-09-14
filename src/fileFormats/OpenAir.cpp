/***************************************************************************
 *   Copyright (C) 2023-2024 by Heinz Blöchinger                           *
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
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>
#include <QtMath>
#include <cmath>

#include "fileFormats/DataFileAbstract.h"
#include "fileFormats/OpenAir.h"

using namespace Qt::Literals::StringLiterals;

class AirSpace {
public:
    QString ac;
    QString ay;
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
        double const radius = toFiniteDouble(qs) * 1852;
        if (radius <= 0)
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
        QStringList items = qs.split(u',', Qt::SkipEmptyParts);
        if (items.size() < 3)
        {
            throw QObject::tr("Invalid arc specification", "OpenAir");
        }
        double const radius = toFiniteDouble(items[0]) * 1852;
        double const angleStart = toFiniteDouble(items[1]);
        double const angleEnd = toFiniteDouble(items[2]);
        if (!variableX.isValid())

        {
            throw QObject::tr("Variable X is not set but Circle should be drawn", "OpenAir");
        }
        addArcBetweenAngles(radius, angleStart, angleEnd);
    }

    void addArcPoints(const QString& qs)
    {
        QStringList const items = qs.split(u',', Qt::SkipEmptyParts);
        if (items.size() < 2)
        {
            throw QObject::tr("Invalid arc specification", "OpenAir");
        }
        // Leading blanks after the comma are handled by the tokenizer in toCoord.
        QGeoCoordinate const startPoint = toCoord(items[0]);
        QGeoCoordinate const endPoint = toCoord(items[1]);
        if (!variableX.isValid())
        {
            throw QObject::tr("Variable X is not set but Circle should be drawn", "OpenAir");
        }
        double const radius = variableX.distanceTo(startPoint);
        double const angleStart = variableX.azimuthTo(startPoint);
        double const angleEnd = variableX.azimuthTo(endPoint);
        addArcBetweenAngles(radius, angleStart, angleEnd);
    }

    /* Draws the arc of the circle around variableX that runs from angleStart to
     * angleEnd in the direction given by variableD, and appends its end point.
     * The DA and DB records differ only in how they spell out the arc, so both
     * end up here.
     */
    void addArcBetweenAngles(double radius, double angleStart, double angleEnd)
    {
        if (variableD == '-')
        {
            if (angleEnd > angleStart)
            {
                /* The arc runs counterclockwise across the discontinuity at
                 * 0/360 degrees and is therefore drawn in two halves. A half is
                 * empty when the arc begins or ends exactly on the
                 * discontinuity, and asking for it would request an arc of zero
                 * length. This happens for perfectly ordinary airspace, such as
                 * a DB arc whose start point lies due north of the center.
                 */
                if (angleStart > 0)
                {
                    addArcCounterClockwise(radius, angleStart, 0);
                }
                if (angleEnd < 360)
                {
                    addArcCounterClockwise(radius, 360, angleEnd);
                }
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
                // As above, for the clockwise direction.
                if (angleStart < 360)
                {
                    addArcClockwise(radius, angleStart, 360);
                }
                if (angleEnd > 0)
                {
                    addArcClockwise(radius, 0, angleEnd);
                }
            }
            else
            {
                addArcClockwise(radius, angleStart, angleEnd);
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
        /* A GeoJSON linear ring needs at least three distinct points plus the
         * repeated closing point. An airspace with fewer cannot be drawn, and
         * emitting a feature without geometry would produce invalid GeoJSON.
         */
        if (polygon.size() < 4)
        {
            errorList.append(QObject::tr("Airspace %1 has no valid outline.", "OpenAir").arg(an.trimmed()));
        }
        if (al.size() < 1)
        {
            errorList.append(QObject::tr("Airspace %1 has no lower limit.", "OpenAir").arg(an.trimmed()));
        }
        if (ah.size() < 1)
        {
            errorList.append(QObject::tr("Airspace %1 has no upper limit.", "OpenAir").arg(an.trimmed()));
        }
        setCategory();
    }

    /* Determines the category used in this app's GeoJSON files from the OpenAir
     * airspace class (AC) and the OpenAir airspace type (AY). The type is the
     * more specific of the two, so it wins whenever it names a category that
     * this app can draw. Everything that cannot be mapped becomes SUA, which is
     * drawn as a generic special use airspace.
     */
    void setCategory()
    {
        static const QHash<QString, QString> typeToCategory {
            {u"ATZ"_s,  u"ATZ"_s},
            {u"CTR"_s,  u"CTR"_s},
            {u"FIR"_s,  u"FIR"_s},
            {u"FIS"_s,  u"FIS"_s},
            {u"GSEC"_s, u"GLD"_s}, // Gliding sector
            {u"P"_s,    u"P"_s},
            {u"Q"_s,    u"DNG"_s}, // OpenAir names danger areas Q
            {u"R"_s,    u"R"_s},
            {u"RMZ"_s,  u"RMZ"_s},
            {u"TIA"_s,  u"TIA"_s},
            {u"TIZ"_s,  u"TIZ"_s},
            {u"TMZ"_s,  u"TMZ"_s}
        };
        auto const fromType = typeToCategory.value(ay.toUpper());
        if (!fromType.isEmpty())
        {
            ac = fromType;
            return;
        }

        /* Airspace classes A-G, plus the airspace types that older OpenAir files
         * put into the AC record, back when AC was used for classes and types
         * alike. The class UNC is deliberately absent: unclassified airspace has
         * no category of its own and becomes SUA below.
         */
        static const QSet<QString> knownCategories {
            u"A"_s,   u"ATZ"_s, u"B"_s,   u"C"_s,   u"CTR"_s, u"D"_s,
            u"DNG"_s, u"E"_s,   u"F"_s,   u"FIR"_s, u"FIS"_s, u"G"_s,
            u"GLD"_s, u"NRA"_s, u"P"_s,   u"PJE"_s, u"R"_s,   u"RMZ"_s,
            u"SUA"_s, u"TIA"_s, u"TIZ"_s, u"TMZ"_s
        };
        auto const fromClass = ac.toUpper();
        ac = knownCategories.contains(fromClass) ? fromClass : u"SUA"_s;
    }

    [[nodiscard]] bool isSet() const
    {
        return (ac.length() > 0);
    }

    void setHeight(const QString& qs, bool higher)
    {
        auto const height = toHeight(qs);
        if (higher)
        {
            ah = height;
        }
        else
        {
            al = height;
        }
    }

    void setVar(const QString& qs)
    {
        if (qs.startsWith(u"X="_s))
        {
            variableX = toCoord(qs.sliced(2));
        }
        else if (qs.startsWith(u"D="_s))
        {
            if (qs.size() < 3)
            {
                throw QObject::tr("Invalid content for VariableD (direction): %1", "OpenAir").arg(qs);
            }
            variableD = qs.at(2);
            if ((variableD != '-') && (variableD != '+'))
            {
                variableD = '+';
                throw QObject::tr("Invalid content for VariableD (direction): %1", "OpenAir").arg(qs.at(2));
            }
        }
    }

private:
    /* Renders an OpenAir altitude specification as a string of the form used in
     * this app's GeoJSON files: "GND", "1500", "1500 AGL" (both in feet) or
     * "FL 65". OpenAir writes altitudes in feet or in meters, with an optional
     * reference; unlike the CUB format, it states them exactly, so the
     * conversion to feet does not round to whole decades. Throws if the
     * specification cannot be understood, rather than guessing at an altitude.
     */
    static QString toHeight(const QString& qs)
    {
        auto const spec = qs.simplified();

        // Ground and unlimited carry no numeric value.
        if ((spec.compare(u"GND"_s, Qt::CaseInsensitive) == 0) ||
            (spec.compare(u"SFC"_s, Qt::CaseInsensitive) == 0) ||
            (spec.compare(u"0"_s) == 0))
        {
            return u"GND"_s;
        }
        if (spec.compare(u"UNL"_s, Qt::CaseInsensitive) == 0)
        {
            /* Unlimited has no equivalent in the GeoJSON vocabulary. Follow the
             * CUB reader, which uses FL 660, above the ceiling of all airspace
             * systems worldwide.
             */
            return u"FL 660"_s;
        }

        // Flight level, as in "FL65" or "FL 65".
        static const QRegularExpression flightLevel(u"^FL\\s*(\\d+(?:\\.\\d+)?)$"_s,
                                                    QRegularExpression::CaseInsensitiveOption);
        auto const flMatch = flightLevel.match(spec);
        if (flMatch.hasMatch())
        {
            return u"FL %1"_s.arg(qRound(flMatch.captured(1).toDouble()));
        }

        /* Numeric altitude with optional unit and optional reference, as in
         * "2500", "2500ft AMSL", "1000 ft AGL" or "3400m AMSL". The references
         * AMSL, MSL and STD all end up as a plain number, which this app reads
         * as an altitude above the QNH.
         */
        static const QRegularExpression altitude(u"^(\\d+(?:\\.\\d+)?)\\s*(FT|M)?\\s*(AGL|AMSL|MSL|STD)?$"_s,
                                                 QRegularExpression::CaseInsensitiveOption);
        auto const altMatch = altitude.match(spec);
        if (!altMatch.hasMatch())
        {
            throw QObject::tr("Invalid altitude specification: %1", "OpenAir").arg(qs);
        }

        auto feet = altMatch.captured(1).toDouble();
        if (altMatch.captured(2).compare(u"M"_s, Qt::CaseInsensitive) == 0)
        {
            feet /= 0.3048;
        }
        if (altMatch.captured(3).compare(u"AGL"_s, Qt::CaseInsensitive) == 0)
        {
            return u"%1 AGL"_s.arg(qRound(feet));
        }
        return QString::number(qRound(feet));
    }

    /* Parses a number and throws unless it is finite. QString::toDouble()
     * happily accepts "nan" and "inf", which would otherwise pass the range
     * checks below (every comparison with NaN is false) and end up as
     * coordinates in the aviation data.
     */
    static double toFiniteDouble(const QString& qs)
    {
        bool ok = false;
        double const result = qs.toDouble(&ok);
        if (!ok || !std::isfinite(result))
        {
            throw QObject::tr("Invalid number found: %1", "OpenAir").arg(qs);
        }
        return result;
    }

    static double getNumber(const QString& degree)
    {
        auto i = degree.indexOf(u":"_s);
        if (i < 0)
        {
            return toFiniteDouble(degree);
        }
        return toFiniteDouble(degree.first(i)) + getNumber(degree.sliced(i + 1)) / 60;
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
        /* The standard spelling is "45:19:40N 006:53:02E", but files found in
         * the wild also separate the hemisphere letters by spaces, or attach
         * them to the wrong number, as in "45:19:40 N006:53:02 E". Isolating the
         * hemisphere letters makes all of these spellings tokenize alike. Since
         * the numbers consist of digits, colons and periods only, the letters
         * are unambiguous.
         */
        static const QRegularExpression hemisphere(u"([NSEW])"_s, QRegularExpression::CaseInsensitiveOption);
        auto normalized = qs;
        normalized.replace(hemisphere, u" \\1 "_s);

        QStringList const items = normalized.split(u' ', Qt::SkipEmptyParts);
        if (items.size() < 4)
        {
            throw QObject::tr("Invalid coordinate found: %1", "OpenAir").arg(qs);
        }

        double latitude = NAN;
        double longitude = NAN;
        try
        {
            latitude = getNumber(items[0]);
            longitude = getNumber(items[2]);
        }
        catch (QString&)
        {
            /* Report the coordinate as a whole. The individual tokens are the
             * result of the normalization above, so naming one of them would
             * point at text that does not appear in the file.
             */
            throw QObject::tr("Invalid coordinate found: %1", "OpenAir").arg(qs);
        }
        if (items[1].compare(u"S"_s, Qt::CaseInsensitive) == 0)
        {
            latitude *= -1;
        }
        else if (items[1].compare(u"N"_s, Qt::CaseInsensitive) != 0)
        {
            throw QObject::tr("Invalid coordinate found: %1", "OpenAir").arg(qs);
        }
        if (items[3].compare(u"W"_s, Qt::CaseInsensitive) == 0)
        {
            longitude *= -1;
        }
        else if (items[3].compare(u"E"_s, Qt::CaseInsensitive) != 0)
        {
            throw QObject::tr("Invalid coordinate found: %1", "OpenAir").arg(qs);
        }
        if ((latitude > 90) || (latitude < -90) || (longitude > 180) || (longitude < -180))
        {
            throw QObject::tr("Invalid coordinate found: %1", "OpenAir").arg(qs);
        }
        QGeoCoordinate const result(latitude, longitude);
        if (!result.isValid())
        {
            throw QObject::tr("Invalid coordinate found: %1", "OpenAir").arg(qs);
        }
        return result;
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
        if (airSpaceVector.isEmpty())
        {
            return {};
        }
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

        recObj.insert(u"type"_s, QJsonValue::fromVariant("FeatureCollection"));
        recObj.insert(u"info"_s, QJsonValue::fromVariant(fileName));

        for (const auto &i : std::as_const(airSpaceVector))
        {
            /* Reset the objects that are reused across iterations. Without this,
             * an airspace whose outline is too small to be written below would
             * silently inherit the geometry of the preceding airspace.
             */
            featureObj = QJsonObject();
            geomObj = QJsonObject();
            featureObj.insert(u"type"_s, QJsonValue::fromVariant("Feature"));
            propObj = QJsonObject();
            propObj.insert(u"NAM"_s, QJsonValue::fromVariant(i.an));
            propObj.insert(u"ID"_s, QJsonValue::fromVariant(i.an));
            propObj.insert(u"CAT"_s, QJsonValue::fromVariant(i.ac));
            propObj.insert(u"TYP"_s, QJsonValue::fromVariant("AS"));
            if (!i.al.isEmpty()) {
                propObj.insert(u"BOT"_s, QJsonValue::fromVariant(i.al));
            }
            if (!i.ah.isEmpty()) {
                propObj.insert(u"TOP"_s, QJsonValue::fromVariant(i.ah));
            }

            // Compute SBO: simplified bottom altitude in feet
            if (!i.al.isEmpty())
            {
                int sbo = 0;
                const QString& al = i.al;
                if (al.compare(u"GND"_s) == 0)
                {
                    sbo = 0;
                }
                else if (al.startsWith(u"FL "_s))
                {
                    // "FL 90" → 9000 ft, "FL 100" → 10000 ft, etc.
                    bool ok = false;
                    const int fl = al.sliced(3).toInt(&ok);
                    sbo = ok ? fl * 100 : 0;
                }
                else
                {
                    // Plain feet AMSL ("3500") or feet AGL ("3500 AGL") — take the leading number
                    bool ok = false;
                    const int ft = al.split(u' ', Qt::SkipEmptyParts).first().toInt(&ok);
                    sbo = ok ? ft : 0;
                }
                propObj.insert(u"SBO"_s, QJsonValue::fromVariant(sbo));
            }

            featureObj.insert(u"properties"_s, propObj);

            while (coordArray.count() != 0)
            {
                coordArray.pop_back();
            }
            for (const auto &j : i.polygon) {
                while (coord.count() != 0) {
                    coord.pop_back();
                }
                coord.append(j.longitude());
                coord.append(j.latitude());
                coordArray.append(coord);
            }
            while (polygonArray.count() != 0)
            {
                polygonArray.pop_back();
            }
            polygonArray.append(coordArray);
            if (i.polygon.size() > 1) {
                geomObj.insert(u"type"_s, QJsonValue::fromVariant("Polygon"));
                geomObj.insert(u"coordinates"_s, polygonArray);
                featureObj.insert(u"geometry"_s, geomObj);
            }

            featureArray.append(featureObj);
        }

        if (featureArray.isEmpty())
        {
            return {};
        }

        recObj.insert(u"features"_s, featureArray);
        QJsonDocument json(recObj);
        return json;
    }
};


bool FileFormats::OpenAir::isValid(const QString& fileName, QString* info)
{
    QStringList errorList;
    QStringList warnings;
    auto json = parse(fileName, errorList, warnings);

    if (info != nullptr)
    {
        *info = {};
        if (!errorList.isEmpty())
        {
            *info += u"<p>"_s + QObject::tr("Errors", "OpenAir") + u"</p>"_s;
            *info += u"<ul style='margin-left:-25px;'>"_s;
            foreach(auto error, errorList)
            {
                *info += u"<li>"_s + error + u"</li>"_s;
            }
            *info += u"</ul>"_s;
        }
        if (!warnings.isEmpty())
        {
            *info += u"<p>"_s + QObject::tr("Warnings", "OpenAir") + u"</p>"_s;
            *info += u"<ul style='margin-left:-25px;'>"_s;
            foreach(auto warning, warnings)
            {
                *info += u"<li>"_s + warning + u"</li>"_s;
            }
            *info += u"</ul>"_s;
        }
    }

    return (!json.isEmpty()) && errorList.isEmpty();
}


QJsonDocument FileFormats::OpenAir::parse(const QString& fileName, QStringList& errorList, QStringList& warningList)
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
    bool hasActivationTimes = false;

    /* Concludes the airspace that has just been read. An airspace whose
     * definition contains errors is not imported. Dropping it silently would
     * leave the user with a map that looks complete but is not, so this is
     * reported as an error and not merely as a warning.
     */
    auto finishAirSpace = [&airSpace, &airSpaceVector, &errorList, &hadError]()
    {
        if (hadError)
        {
            errorList.append(QObject::tr("Airspace %1 was not imported because its definition contains errors.", "OpenAir")
                                 .arg(airSpace.an.trimmed()));
        }
        else
        {
            airSpace.finalize(errorList);
            airSpaceVector.addAirSpace(airSpace);
        }
        airSpace = AirSpace();
        hadError = false;
    };

    int lineNo = 0;
    while (inputStream.readLineInto(&line))
    {
        lineNo++;

        try {
            if (line.startsWith(u"*"_s) || line.length() == 0)
            {
                continue;
            }
            if (line.startsWith(u"AC "_s))
            {
                //if airSpace is already filled, the existing airSpace must be added to the list and a new airSpace must be initialized
                if (airSpace.isSet())
                {
                    finishAirSpace();
                }
                airSpace.ac = line.sliced(3).trimmed();
                continue;
            }
            if (line.startsWith(u"AY "_s))
            {
                airSpace.ay = line.sliced(3).trimmed();
                continue;
            }
            if (line.startsWith(u"AA "_s))
            {
                /* Activation times. This app has no way to represent them, so
                 * the airspace is shown at all times. Warn once per file, so
                 * that the user does not mistake a seasonal airspace for a
                 * permanent one.
                 */
                hasActivationTimes = true;
                continue;
            }
            if (line.startsWith(u"AN "_s))
            {
                airSpace.an = line.sliced(3);
                if (airSpaceVector.isSameName(airSpace.an))
                {
                    airSpace.variableX = airSpaceVector.getLastX();
                }
                continue;
            }
            if (line.startsWith(u"AL "_s))
            {
                airSpace.setHeight(line.sliced(3), false);
                continue;
            }
            if (line.startsWith(u"AH "_s))
            {
                airSpace.setHeight(line.sliced(3), true);
                continue;
            }
            if (line.startsWith(u"V "_s))
            {
                airSpace.setVar(line.sliced(2));
                continue;
            }
            if (line.startsWith(u"DP "_s))
            {
                airSpace.addPoint(line.sliced(3));
                continue;
            }
            if (line.startsWith(u"DC "_s))
            {
                airSpace.addCircle(line.sliced(3));
                continue;
            }
            if (line.startsWith(u"DA "_s))
            {
                airSpace.addArc(line.sliced(3));
                continue;
            }
            if (line.startsWith(u"DB "_s))
            {
                airSpace.addArcPoints(line.sliced(3));
                continue;
            }
            if (line.startsWith(u"AT "_s))
            {
                continue;
            }
            warningList.append(QObject::tr("Unrecognized record type in line %1: %2; Line ignored.", "OpenAir").arg(QString::number(lineNo), line));
        }
        catch (QString& ex)
        {
            hadError = true;
            warningList.append(QObject::tr("Error in line %1: %2", "OpenAir").arg(QString::number(lineNo), ex));
        }
    }
    if (airSpace.isSet())
    {
        finishAirSpace();
    }

    if (hasActivationTimes)
    {
        warningList.append(QObject::tr("This file specifies activation times. Activation times are not evaluated; "
                                       "the airspaces are shown at all times.", "OpenAir"));
    }

    return airSpaceVector.getJson(fileName);
}

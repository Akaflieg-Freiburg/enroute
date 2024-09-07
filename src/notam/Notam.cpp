/***************************************************************************
 *   Copyright (C) 2023-2024 by Stefan Kebekus                             *
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

#include <QJsonArray>
#include <QJsonObject>

#include "GlobalObject.h"
#include "GlobalSettings.h"
#include "notam/Notam.h"
#include "notam/NotamProvider.h"


// Static objects

namespace {


// In cancel notams the text starts as "A0029/23 NOTAMC A0027/23"
Q_GLOBAL_STATIC(QRegularExpression, cancelNotamStart, u"^[A-Z]\\d{4}/\\d{2} NOTAMC [A-Z]\\d{4}/\\d{2}"_qs)

// Necessary because Q_GLOBAL_STATIC does not like templates
using ContractionList = QList<std::pair<QRegularExpression, QString>>;
Q_GLOBAL_STATIC(ContractionList,
                contractions,
                {
                    {QRegularExpression(u"\\bU/S\\b"_qs), u"UNSERVICEABLE"_qs}, // MUST COME BEFORE SOUTH

                    {QRegularExpression(u"\\bACFT\\b"_qs), u"AIRCRAFT"_qs},
                    {QRegularExpression(u"\\bAD\\b"_qs), u"AERODROME"_qs},
                    {QRegularExpression(u"\\bAFIS\\b"_qs), u"AERODROME FLIGHT INFORMATION SERVICE"_qs},
                    {QRegularExpression(u"\\bAFT\\b"_qs), u"AFTER"_qs},
                    {QRegularExpression(u"\\bAMDT\\b"_qs), u"AMENDMENT"_qs},
                    {QRegularExpression(u"\\bAPCH\\b"_qs), u"APPROACH"_qs},
                    {QRegularExpression(u"\\bAPRX\\b"_qs), u"APPROXIMATELY"_qs},
                    {QRegularExpression(u"\\bARP\\b"_qs), u"AERODROME REFERENCE POINT"_qs},
                    {QRegularExpression(u"\\bARR\\b"_qs), u"ARRIVAL"_qs},
                    {QRegularExpression(u"\\bASPH\\b"_qs), u"ASPHALT"_qs},
                    {QRegularExpression(u"\\bAVBL\\b"_qs), u"AVAILABLE"_qs},
                    {QRegularExpression(u"\\bBCST\\b"_qs), u"BROADCAST"_qs},
                    {QRegularExpression(u"\\bBFR\\b"_qs), u"BEFORE"_qs},
                    {QRegularExpression(u"\\bBLW\\b"_qs), u"BELOW"_qs},
                    {QRegularExpression(u"\\bBTN\\b"_qs), u"BETWEEN"_qs},
                    {QRegularExpression(u"\\bCLBR\\b"_qs), u"CALLIBRATION"_qs},
                    {QRegularExpression(u"\\bCLSD\\b"_qs), u"CLOSED"_qs},
                    {QRegularExpression(u"\\bCNL\\b"_qs), u"CANCEL"_qs},
                    {QRegularExpression(u"\\bCTN\\b"_qs), u"CAUTION"_qs},
                    {QRegularExpression(u"\\bDEP\\b"_qs), u"DEPARTURE"_qs},
                    {QRegularExpression(u"\\bDRG\\b"_qs), u"DURING"_qs},
                    {QRegularExpression(u"\\bELEV\\b"_qs), u"ELEVATION"_qs},
                    {QRegularExpression(u"\\bEQPT\\b"_qs), u"EQUIPMENT"_qs},
                    {QRegularExpression(u"\\bEXC\\b"_qs), u"EXCEPTED"_qs},
                    {QRegularExpression(u"\\bEXP\\b"_qs), u"EXPECT"_qs},
                    {QRegularExpression(u"\\bFATO\\b"_qs), u"FINAL APPROACH AND TAKEOFF AREA"_qs},
                    {QRegularExpression(u"\\bFST\\b"_qs), u"FIRST"_qs},
                    {QRegularExpression(u"\\bFLT\\b"_qs), u"FLIGHT"_qs},
                    {QRegularExpression(u"\\bFLW\\b"_qs), u"FOLLOW"_qs},
                    {QRegularExpression(u"\\bGLD\\b"_qs), u"GLIDER"_qs},
                    {QRegularExpression(u"\\bHEL\\b"_qs), u"HELICOPTER"_qs},
                    {QRegularExpression(u"\\bLGT\\b"_qs), u"LIGHT"_qs},
                    {QRegularExpression(u"\\bLGTD\\b"_qs), u"LIGHTED"_qs},
                    {QRegularExpression(u"\\bLTD\\b"_qs), u"LIMITED"_qs},
                    {QRegularExpression(u"\\bMAINT\\b"_qs), u"MAINTENANCE"_qs},
                    {QRegularExpression(u"\\bMIL\\b"_qs), u"MILITARY"_qs},
                    {QRegularExpression(u"\\bN\\b"_qs), u"NORTH"_qs},
                    {QRegularExpression(u"\\bNE\\b"_qs), u"NORTHEAST"_qs},
                    {QRegularExpression(u"\\bNW\\b"_qs), u"NORTHWEST"_qs},
                    {QRegularExpression(u"\\bO/R\\b"_qs), u"AVAILABLE ON REQUEST"_qs},
                    {QRegularExpression(u"\\bOBST\\b"_qs), u"OBSTACLE"_qs},
                    {QRegularExpression(u"\\bPOSS\\b"_qs), u"POSSIBLE"_qs},
                    {QRegularExpression(u"\\bPSN\\b"_qs), u"POSITION"_qs},
                    {QRegularExpression(u"\\bPRKG\\b"_qs), u"PARKING"_qs},
                    {QRegularExpression(u"\\bRTE\\b"_qs), u"ROUTE"_qs},
                    {QRegularExpression(u"\\bRVR\\b"_qs), u"RUNWAY VISUAL RANGE"_qs},
                    {QRegularExpression(u"\\bRWY\\b"_qs), u"RUNWAY"_qs},
                    {QRegularExpression(u"\\bS\\b"_qs), u"SOUTH"_qs},
                    {QRegularExpression(u"\\bSE\\b"_qs), u"SOUTHEAST"_qs},
                    {QRegularExpression(u"\\bSKED\\b"_qs), u"SCHEDULED"_qs},
                    {QRegularExpression(u"\\bSW\\b"_qs), u"SOUTHWEST"_qs},
                    {QRegularExpression(u"\\bTFC\\b"_qs), u"TRAFFIC"_qs},
                    {QRegularExpression(u"\\bTHR\\b"_qs), u"THRESHOLD"_qs},
                    {QRegularExpression(u"\\bTWR\\b"_qs), u"TOWER"_qs},
                    {QRegularExpression(u"\\bTWY\\b"_qs), u"TAXIWAY"_qs},
                    {QRegularExpression(u"\\bW\\b"_qs), u"WEST"_qs},
                    {QRegularExpression(u"\\bWDI\\b"_qs), u"WIND DIRECTION INDICATOR"_qs},
                    {QRegularExpression(u"\\bWI\\b"_qs), u"WITHIN"_qs},
                    {QRegularExpression(u"\\bWIP\\b"_qs), u"WORK IN PROGRESS"_qs},
                })
} // namespace



//
// Constructor/Destructor
//

NOTAM::Notam::Notam(const QJsonObject& jsonObject)
{
    auto notamObject = jsonObject[u"properties"_qs][u"coreNOTAMData"_qs][u"notam"_qs].toObject();

    m_affectedFIR = notamObject[u"affectedFIR"_qs].toString();
    m_coordinate = interpretNOTAMCoordinates(notamObject[u"coordinates"_qs].toString());
    m_effectiveEndString = notamObject[u"effectiveEnd"_qs].toString();
    m_effectiveStartString = notamObject[u"effectiveStart"_qs].toString();
    m_icaoLocation = notamObject[u"icaoLocation"_qs].toString();
    m_maximumFL = notamObject[u"maximumFL"_qs].toString();
    m_minimumFL = notamObject[u"minimumFL"_qs].toString();
    m_number = notamObject[u"number"_qs].toString();
    m_text = notamObject[u"text"_qs].toString();
    m_traffic = notamObject[u"traffic"_qs].toString();
    m_radius = Units::Distance::fromNM(notamObject[u"radius"_qs].toString().toDouble());

    m_effectiveEnd = QDateTime::fromString(m_effectiveEndString, Qt::ISODate);
    m_effectiveStart = QDateTime::fromString(m_effectiveStartString, Qt::ISODate);
    m_region = QGeoCircle(m_coordinate, qMax( Units::Distance::fromNM(1).toM(), m_radius.toM() ));

    if (notamObject.contains(u"schedule"_qs))
    {
        m_schedule = notamObject[u"schedule"_qs].toString();
    }
}



//
// Getter Methods
//

QString NOTAM::Notam::cancels() const
{
    if (!m_text.contains(*cancelNotamStart))
    {
        return {};
    }
    return m_text.mid(16,8);
}


QJsonObject NOTAM::Notam::GeoJSON() const
{
    QMap<QString, QVariant> m_properties;
    m_properties[u"CAT"_qs] = u"NOTAM"_qs;
    m_properties[u"NAM"_qs] = {};

    QJsonArray coords;
    coords.insert(0, m_coordinate.longitude());
    coords.insert(1, m_coordinate.latitude());
    QJsonObject geometry;
    geometry.insert(QStringLiteral("type"), "Point");
    geometry.insert(QStringLiteral("coordinates"), coords);
    QJsonObject feature;
    feature.insert(QStringLiteral("type"), "Feature");
    feature.insert(QStringLiteral("properties"), QJsonObject::fromVariantMap(m_properties));
    feature.insert(QStringLiteral("geometry"), geometry);

    return feature;
}


bool NOTAM::Notam::isValid() const
{
    if (!m_coordinate.isValid())
    {
        return false;
    }
    if ((m_icaoLocation.size() != 3) && (m_icaoLocation.size() != 4))
    {
        return false;
    }
    if (m_text.isEmpty())
    {
        return false;
    }
    return true;
}



//
// Methods
//

QString NOTAM::Notam::richText() const
{
    QStringList result;

    // Generate a string description for the effective start. This might be empty.
    auto effectiveStartString = m_effectiveStartString;
    if (m_effectiveStart.isValid())
    {
        if (m_effectiveStart < QDateTime::currentDateTime())
        {
            effectiveStartString.clear();
        }
        else
        {
            if (m_effectiveStart.date() == QDateTime::currentDateTimeUtc().date())
            {
                effectiveStartString = u"Today %1"_qs.arg(m_effectiveStart.toString(u"hh:mm"_qs));
            }
            else
            {
                effectiveStartString = m_effectiveStart.toString(u"ddMMMyy hh:mm"_qs);
            }
        }
    }

    auto effectiveEndString = m_effectiveEndString;
    if (m_effectiveEnd.isValid())
    {
        if (m_effectiveStart.isValid() && !m_effectiveStartString.isEmpty() && m_effectiveStart.date() == m_effectiveEnd.date() )
        {
            effectiveEndString = m_effectiveEnd.toString(u"hh:mm"_qs);
        }
        else
        {
            effectiveEndString = m_effectiveEnd.toString(u"ddMMMyy hh:mm"_qs);
        }
    }

    if (effectiveStartString.isEmpty())
    {
        if (m_effectiveEnd.isValid())
        {
            result += u"<strong>Until %1</strong>"_qs.arg(effectiveEndString);
        }
        else
        {
            result += u"<strong>%1</strong>"_qs.arg(effectiveEndString);
        }
    }
    else
    {
    result += u"<strong>%1 until %2</strong>"_qs.arg(effectiveStartString, effectiveEndString);
    }

    if ((m_minimumFL.size() == 3) && (m_maximumFL.size() == 3) && !(m_minimumFL == u"000"_qs && m_maximumFL == u"999"_qs))
    {
        result += u"<strong>FL%1-FL%2</strong>"_qs.arg(m_minimumFL, m_maximumFL).replace(u"FL000"_qs, u"GND"_qs);
    }

    if (!m_schedule.isEmpty())
    {
        result += u"<strong>Schedule %1</strong>"_qs.arg(m_schedule);
    }

    if (GlobalObject::globalSettings()->expandNotamAbbreviations())
    {
        QString tmp = m_text;
        foreach(auto contraction, *contractions)
        {
            tmp.replace(contraction.first, contraction.second);
        }

        result += tmp;
    }
    else
    {
        result += m_text;
    }
    return u"<strong>%1: </strong>"_qs.arg(m_icaoLocation) + result.join(u" • "_qs).replace(u"  "_qs, u" "_qs);
}


QString NOTAM::Notam::sectionTitle() const
{
    if (GlobalObject::notamProvider()->isRead(m_number))
    {
        return u"Marked as read"_qs;
    }

    if (m_effectiveStart.isValid())
    {
        if (m_effectiveStart < QDateTime::currentDateTimeUtc())
        {
            return u"Current"_qs;
        }
        if (m_effectiveStart < QDateTime::currentDateTimeUtc().addDays(1))
        {
            return u"Next 24h"_qs;
        }
        if (m_effectiveStart < QDateTime::currentDateTimeUtc().addDays(90))
        {
            return u"Next 90 days"_qs;
        }
        if (m_effectiveStart < QDateTime::currentDateTimeUtc().addDays(90))
        {
            return u"> 90 days"_qs;
        }
    }
    return u"NOTAM"_qs;
}


//
// Non-Member Methods
//

QGeoCoordinate NOTAM::interpretNOTAMCoordinates(const QString& string)
{
    if (string.length() != 11)
    {
        return {};
    }

    bool ok {false};
    auto latD = string.left(2).toDouble(&ok);
    if (!ok)
    {
        return {};
    }
    auto latM = string.mid(2, 2).toDouble(&ok);
    if (!ok)
    {
        return {};
    }

    double lat = latD+latM/60.0;
    if (string[4] == 'S')
    {
        lat *= -1.0;
    }

    auto lonD = string.mid(5, 3).toDouble(&ok);
    if (!ok)
    {
        return {};
    }
    auto lonM = string.mid(8, 2).toDouble(&ok);
    if (!ok)
    {
        return {};
    }
    double lon = lonD+lonM/60.0;
    if (string[10] == 'W')
    {
        lon *= -1.0;
    }

    return {lat, lon};
}


QDataStream& NOTAM::operator<<(QDataStream& stream, const NOTAM::Notam& notam)
{
    stream << notam.m_affectedFIR;
    stream << notam.m_coordinate;
    stream << notam.m_effectiveEnd;
    stream << notam.m_effectiveEndString;
    stream << notam.m_effectiveStart;
    stream << notam.m_effectiveStartString;
    stream << notam.m_icaoLocation;
    stream << notam.m_maximumFL;
    stream << notam.m_minimumFL;
    stream << notam.m_number;
    stream << notam.m_radius;
    stream << notam.m_region;
    stream << notam.m_schedule;
    stream << notam.m_text;
    stream << notam.m_traffic;

    return stream;
}


QDataStream& NOTAM::operator>>(QDataStream& stream, NOTAM::Notam& notam)
{
    stream >> notam.m_affectedFIR;
    stream >> notam.m_coordinate;
    stream >> notam.m_effectiveEnd;
    stream >> notam.m_effectiveEndString;
    stream >> notam.m_effectiveStart;
    stream >> notam.m_effectiveStartString;
    stream >> notam.m_icaoLocation;
    stream >> notam.m_maximumFL;
    stream >> notam.m_minimumFL;
    stream >> notam.m_number;
    stream >> notam.m_radius;
    stream >> notam.m_region;
    stream >> notam.m_schedule;
    stream >> notam.m_text;
    stream >> notam.m_traffic;

    return stream;
}

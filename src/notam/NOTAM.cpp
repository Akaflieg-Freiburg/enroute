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
#include "notam/NOTAM.h"
#include "notam/NOTAMProvider.h"

using namespace Qt::Literals::StringLiterals;


// Static objects

namespace {


// In cancel notams the text starts as "A0029/23 NOTAMC A0027/23"
Q_GLOBAL_STATIC(QRegularExpression, cancelNotamStart, u"^[A-Z]\\d{4}/\\d{2} NOTAMC [A-Z]\\d{4}/\\d{2}"_s)

// Necessary because Q_GLOBAL_STATIC does not like templates
using ContractionList = QList<std::pair<QRegularExpression, QString>>;
Q_GLOBAL_STATIC(ContractionList,
                contractions,
                {
                    {QRegularExpression(u"\\bU/S\\b"_s), u"UNSERVICEABLE"_s}, // MUST COME BEFORE SOUTH

                    {QRegularExpression(u"\\bACFT\\b"_s), u"AIRCRAFT"_s},
                    {QRegularExpression(u"\\bAD\\b"_s), u"AERODROME"_s},
                    {QRegularExpression(u"\\bAFIS\\b"_s), u"AERODROME FLIGHT INFORMATION SERVICE"_s},
                    {QRegularExpression(u"\\bAFT\\b"_s), u"AFTER"_s},
                    {QRegularExpression(u"\\bAMDT\\b"_s), u"AMENDMENT"_s},
                    {QRegularExpression(u"\\bAPCH\\b"_s), u"APPROACH"_s},
                    {QRegularExpression(u"\\bAPRX\\b"_s), u"APPROXIMATELY"_s},
                    {QRegularExpression(u"\\bARP\\b"_s), u"AERODROME REFERENCE POINT"_s},
                    {QRegularExpression(u"\\bARR\\b"_s), u"ARRIVAL"_s},
                    {QRegularExpression(u"\\bASPH\\b"_s), u"ASPHALT"_s},
                    {QRegularExpression(u"\\bAVBL\\b"_s), u"AVAILABLE"_s},
                    {QRegularExpression(u"\\bBCST\\b"_s), u"BROADCAST"_s},
                    {QRegularExpression(u"\\bBFR\\b"_s), u"BEFORE"_s},
                    {QRegularExpression(u"\\bBLW\\b"_s), u"BELOW"_s},
                    {QRegularExpression(u"\\bBTN\\b"_s), u"BETWEEN"_s},
                    {QRegularExpression(u"\\bCLBR\\b"_s), u"CALLIBRATION"_s},
                    {QRegularExpression(u"\\bCLSD\\b"_s), u"CLOSED"_s},
                    {QRegularExpression(u"\\bCNL\\b"_s), u"CANCEL"_s},
                    {QRegularExpression(u"\\bCTN\\b"_s), u"CAUTION"_s},
                    {QRegularExpression(u"\\bDEP\\b"_s), u"DEPARTURE"_s},
                    {QRegularExpression(u"\\bDRG\\b"_s), u"DURING"_s},
                    {QRegularExpression(u"\\bELEV\\b"_s), u"ELEVATION"_s},
                    {QRegularExpression(u"\\bEQPT\\b"_s), u"EQUIPMENT"_s},
                    {QRegularExpression(u"\\bEXC\\b"_s), u"EXCEPTED"_s},
                    {QRegularExpression(u"\\bEXP\\b"_s), u"EXPECT"_s},
                    {QRegularExpression(u"\\bFATO\\b"_s), u"FINAL APPROACH AND TAKEOFF AREA"_s},
                    {QRegularExpression(u"\\bFST\\b"_s), u"FIRST"_s},
                    {QRegularExpression(u"\\bFLT\\b"_s), u"FLIGHT"_s},
                    {QRegularExpression(u"\\bFLW\\b"_s), u"FOLLOW"_s},
                    {QRegularExpression(u"\\bGLD\\b"_s), u"GLIDER"_s},
                    {QRegularExpression(u"\\bHEL\\b"_s), u"HELICOPTER"_s},
                    {QRegularExpression(u"\\bLGT\\b"_s), u"LIGHT"_s},
                    {QRegularExpression(u"\\bLGTD\\b"_s), u"LIGHTED"_s},
                    {QRegularExpression(u"\\bLTD\\b"_s), u"LIMITED"_s},
                    {QRegularExpression(u"\\bMAINT\\b"_s), u"MAINTENANCE"_s},
                    {QRegularExpression(u"\\bMIL\\b"_s), u"MILITARY"_s},
                    {QRegularExpression(u"\\bN\\b"_s), u"NORTH"_s},
                    {QRegularExpression(u"\\bNE\\b"_s), u"NORTHEAST"_s},
                    {QRegularExpression(u"\\bNW\\b"_s), u"NORTHWEST"_s},
                    {QRegularExpression(u"\\bO/R\\b"_s), u"AVAILABLE ON REQUEST"_s},
                    {QRegularExpression(u"\\bOBST\\b"_s), u"OBSTACLE"_s},
                    {QRegularExpression(u"\\bPOSS\\b"_s), u"POSSIBLE"_s},
                    {QRegularExpression(u"\\bPSN\\b"_s), u"POSITION"_s},
                    {QRegularExpression(u"\\bPRKG\\b"_s), u"PARKING"_s},
                    {QRegularExpression(u"\\bRTE\\b"_s), u"ROUTE"_s},
                    {QRegularExpression(u"\\bRVR\\b"_s), u"RUNWAY VISUAL RANGE"_s},
                    {QRegularExpression(u"\\bRWY\\b"_s), u"RUNWAY"_s},
                    {QRegularExpression(u"\\bS\\b"_s), u"SOUTH"_s},
                    {QRegularExpression(u"\\bSE\\b"_s), u"SOUTHEAST"_s},
                    {QRegularExpression(u"\\bSKED\\b"_s), u"SCHEDULED"_s},
                    {QRegularExpression(u"\\bSW\\b"_s), u"SOUTHWEST"_s},
                    {QRegularExpression(u"\\bTFC\\b"_s), u"TRAFFIC"_s},
                    {QRegularExpression(u"\\bTHR\\b"_s), u"THRESHOLD"_s},
                    {QRegularExpression(u"\\bTWR\\b"_s), u"TOWER"_s},
                    {QRegularExpression(u"\\bTWY\\b"_s), u"TAXIWAY"_s},
                    {QRegularExpression(u"\\bW\\b"_s), u"WEST"_s},
                    {QRegularExpression(u"\\bWDI\\b"_s), u"WIND DIRECTION INDICATOR"_s},
                    {QRegularExpression(u"\\bWI\\b"_s), u"WITHIN"_s},
                    {QRegularExpression(u"\\bWIP\\b"_s), u"WORK IN PROGRESS"_s},
                })
} // namespace



//
// Constructor/Destructor
//

NOTAM::NOTAM::NOTAM(const QJsonObject& jsonObject)
{
    auto notamObject = jsonObject[u"properties"_s][u"coreNOTAMData"_s][u"notam"_s].toObject();

    m_affectedFIR = notamObject[u"affectedFIR"_s].toString();
    m_coordinate = interpretNOTAMCoordinates(notamObject[u"coordinates"_s].toString());
    m_effectiveEndString = notamObject[u"effectiveEnd"_s].toString();
    m_effectiveStartString = notamObject[u"effectiveStart"_s].toString();
    m_icaoLocation = notamObject[u"icaoLocation"_s].toString();
    m_maximumFL = notamObject[u"maximumFL"_s].toString();
    m_minimumFL = notamObject[u"minimumFL"_s].toString();
    m_number = notamObject[u"number"_s].toString();
    m_text = notamObject[u"text"_s].toString();
    m_traffic = notamObject[u"traffic"_s].toString();
    m_radius = Units::Distance::fromNM(notamObject[u"radius"_s].toString().toDouble());

    m_effectiveEnd = QDateTime::fromString(m_effectiveEndString, Qt::ISODate);
    m_effectiveStart = QDateTime::fromString(m_effectiveStartString, Qt::ISODate);
    m_region = QGeoCircle(m_coordinate, qMax( Units::Distance::fromNM(1).toM(), m_radius.toM() ));

    if (notamObject.contains(u"schedule"_s))
    {
        m_schedule = notamObject[u"schedule"_s].toString();
    }
}



//
// Getter Methods
//

QString NOTAM::NOTAM::cancels() const
{
    if (!m_text.contains(*cancelNotamStart))
    {
        return {};
    }
    return m_text.mid(16,8);
}


QJsonObject NOTAM::NOTAM::GeoJSON() const
{
    QMap<QString, QVariant> m_properties;
    m_properties[u"CAT"_s] = u"NOTAM"_s;
    m_properties[u"NAM"_s] = {};

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


bool NOTAM::NOTAM::isValid() const
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

QString NOTAM::NOTAM::richText() const
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
                effectiveStartString = u"Today %1"_s.arg(m_effectiveStart.toString(u"hh:mm"_s));
            }
            else
            {
                effectiveStartString = m_effectiveStart.toString(u"ddMMMyy hh:mm"_s);
            }
        }
    }

    auto effectiveEndString = m_effectiveEndString;
    if (m_effectiveEnd.isValid())
    {
        if (m_effectiveStart.isValid() && !m_effectiveStartString.isEmpty() && m_effectiveStart.date() == m_effectiveEnd.date() )
        {
            effectiveEndString = m_effectiveEnd.toString(u"hh:mm"_s);
        }
        else
        {
            effectiveEndString = m_effectiveEnd.toString(u"ddMMMyy hh:mm"_s);
        }
    }

    if (effectiveStartString.isEmpty())
    {
        if (m_effectiveEnd.isValid())
        {
            result += u"<strong>Until %1</strong>"_s.arg(effectiveEndString);
        }
        else
        {
            result += u"<strong>%1</strong>"_s.arg(effectiveEndString);
        }
    }
    else
    {
    result += u"<strong>%1 until %2</strong>"_s.arg(effectiveStartString, effectiveEndString);
    }

    if ((m_minimumFL.size() == 3) && (m_maximumFL.size() == 3) && !(m_minimumFL == u"000"_s && m_maximumFL == u"999"_s))
    {
        result += u"<strong>FL%1-FL%2</strong>"_s.arg(m_minimumFL, m_maximumFL).replace(u"FL000"_s, u"GND"_s);
    }

    if (!m_schedule.isEmpty())
    {
        result += u"<strong>Schedule %1</strong>"_s.arg(m_schedule);
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
    return u"<strong>%1: </strong>"_s.arg(m_icaoLocation) + result.join(u" • "_s).replace(u"  "_s, u" "_s);
}


void NOTAM::NOTAM::updateSectionTitle()
{
    if (GlobalObject::notamProvider()->isRead(m_number))
    {
        m_sectionTitle = u"Marked as read"_s;
        return;
    }

    if (m_effectiveStart.isValid())
    {
        if (m_effectiveStart < QDateTime::currentDateTimeUtc())
        {
            m_sectionTitle = u"Current"_s;
            return;
        }
        if (m_effectiveStart < QDateTime::currentDateTimeUtc().addDays(1))
        {
            m_sectionTitle = u"Next 24h"_s;
            return;
        }
        if (m_effectiveStart < QDateTime::currentDateTimeUtc().addDays(90))
        {
            m_sectionTitle = u"Next 90 days"_s;
            return;
        }
        if (m_effectiveStart < QDateTime::currentDateTimeUtc().addDays(90))
        {
            m_sectionTitle = u"> 90 days"_s;
            return;
        }
    }
    m_sectionTitle = u"NOTAM"_s;
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


QDataStream& NOTAM::operator<<(QDataStream& stream, const NOTAM& notam)
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
    stream << notam.m_sectionTitle;
    stream << notam.m_text;
    stream << notam.m_traffic;

    return stream;
}


QDataStream& NOTAM::operator>>(QDataStream& stream, NOTAM& notam)
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
    stream >> notam.m_sectionTitle;
    stream >> notam.m_text;
    stream >> notam.m_traffic;

    return stream;
}

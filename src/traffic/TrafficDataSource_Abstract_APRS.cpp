/***************************************************************************
 *   Copyright (C) 2021-2025 by Stefan Kebekus                             *
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

#include "GlobalObject.h"
#include "positioning/PositionProvider.h"
#include "traffic/FlarmnetDB.h"
#include "traffic/TrafficDataSource_Abstract.h"
#include "traffic/TrafficFactorAircraftType.h"
#include "TrafficDataSource_OgnParser.h"
#include "TransponderDB.h"

using namespace Qt::Literals::StringLiterals;


void Traffic::TrafficDataSource_Abstract::processAPRS(const QString& data)
{
    static TransponderDB const transponderDB; // Initialize the database

    Traffic::Ogn::OgnMessage m_ognMessage;

    m_ognMessage.sentence = data;
    // notify that we are receiving data
    setReceivingHeartbeat(true);

    // Process APRS-IS sentence
    Ogn::TrafficDataSource_OgnParser::parseAprsisMessage(m_ognMessage);

    if (m_ognMessage.type != Traffic::Ogn::OgnMessageType::TRAFFIC_REPORT)
    {
        return;
    }
    if ((m_ognMessage.aircraftType == Traffic::AircraftType::unknown || m_ognMessage.aircraftType == Traffic::AircraftType::StaticObstacle) && !m_ognMessage.speed.isFinite())
    {
        return;
    }
    if (!m_ognMessage.coordinate.isValid())
    {
        return;
    }

    // Compute horizontal/vertical distance and the alarm Level
    int alarmLevel = 0;
    Units::Distance hDist;
    Units::Distance vDist;
    QGeoCoordinate const ownShipCoordinate = GlobalObject::positionProvider()->positionInfo().coordinate();
    if (ownShipCoordinate.isValid())
    {
        hDist = Units::Distance::fromM(ownShipCoordinate.distanceTo(m_ognMessage.coordinate));
        vDist = Units::Distance::fromM(qFabs(m_ognMessage.coordinate.altitude() - ownShipCoordinate.altitude()));
        if (hDist.toM() < 1000 && vDist.toFeet() < 400)
        {
            alarmLevel = 3; // High alert
        }
        else if (hDist.toM() < 2000 && vDist.toFeet() < 600)
        {
            alarmLevel = 2; // Medium alert
        }
        else if (hDist.toM() < 5000 && vDist.toFeet() < 800)
        {
            alarmLevel = 1; // Low alert
        }
    }

    // Decode callsign
    QString callsign;
    if (m_ognMessage.addressType == Traffic::Ogn::OgnAddressType::FLARM)
    {
        callsign = GlobalObject::flarmnetDB()->getRegistration(QString(m_ognMessage.address));
    }
    else if (static_cast<int>(!m_ognMessage.flightnumber.empty()) != 0)
    {
        callsign = QString(m_ognMessage.flightnumber);
    }
    else if (m_ognMessage.addressType == Traffic::Ogn::OgnAddressType::ICAO)
    {
        callsign = transponderDB.getRegistration(QString(m_ognMessage.address));
    }
#if OGN_SHOW_ADDRESSTYPE
    const QMetaEnum metaEnum = QMetaEnum::fromType<Traffic::Ogn::OgnAddressType>();
    callsign += QString(" (%1)").arg(metaEnum.valueToKey(static_cast<int>(m_ognMessage.addressType)));
#endif


    // PositionInfo
    QGeoPositionInfo pInfo(m_ognMessage.coordinate, QDateTime::currentDateTimeUtc());
    pInfo.setAttribute(QGeoPositionInfo::Direction, m_ognMessage.course.toDEG());
    pInfo.setAttribute(QGeoPositionInfo::GroundSpeed, m_ognMessage.speed.toMPS());
    pInfo.setAttribute(QGeoPositionInfo::VerticalSpeed, m_ognMessage.verticalSpeed);
    if (!pInfo.isValid())
    {
        return;
    }

    // Prepare the m_factor object
    m_factor.setAlarmLevel(alarmLevel);
    m_factor.setCallSign(callsign);
    m_factor.setID(m_ognMessage.sourceId.toString());
    m_factor.setType(m_ognMessage.aircraftType);
    m_factor.setPositionInfo(Positioning::PositionInfo(pInfo, sourceName()));
    m_factor.setHDist(hDist);
    m_factor.setVDist(vDist);
    m_factor.startLiveTime();

    // Emit the factorWithPosition signal
    emit factorWithPosition(m_factor);
}

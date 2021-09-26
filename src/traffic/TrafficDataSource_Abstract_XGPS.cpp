/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#include "Global.h"
#include "positioning/PositionProvider.h"
#include "traffic/TrafficDataSource_Abstract.h"


// Member functions

void Traffic::TrafficDataSource_Abstract::processXGPSString(const QByteArray& data)
{

    //
    // Handle the various message types
    //

    // Ownship report, serves also as heartbeat message
    if (data.startsWith("XGPS")) {

        QString str = QString::fromLatin1(data);
        QStringList list = str.split(QLatin1Char(','));
        if (list.size() != 6) {
            return;
        }

        bool ok = false;
        double lon = list[1].toDouble(&ok);
        if (!ok) {
            return;
        }
        double lat = list[2].toDouble(&ok);
        if (!ok) {
            return;
        }
        double alt = list[3].toDouble(&ok);
        if (!ok) {
            return;
        }
        double tt = list[4].toDouble(&ok);
        if (!ok) {
            return;
        }
        double gs = list[5].toDouble(&ok);
        if (!ok) {
            return;
        }

        QGeoPositionInfo _geoPos(QGeoCoordinate(lat, lon, alt), QDateTime::currentDateTimeUtc());
        _geoPos.setAttribute(QGeoPositionInfo::Direction, tt);
        _geoPos.setAttribute(QGeoPositionInfo::GroundSpeed, gs);

        // Update position information and continue
        if (_geoPos.isValid()) {
            emit positionUpdated( Positioning::PositionInfo(_geoPos) );
            setReceivingHeartbeat(true);
        }

        return;
    }


    // Traffic report
    if (data.startsWith("XTRA")) {

        QString str = QString::fromLatin1(data);
        QStringList list = str.split(QLatin1Char(','));
        if (list.size() != 10) {
            return;
        }

        bool ok = false;
        QString targetID = list[1];
        double lat = list[2].toDouble(&ok);
        if (!ok) {
            return;
        }
        double lon = list[3].toDouble(&ok);
        if (!ok) {
            return;
        }
        auto alt = Units::Distance::fromFT(list[4].toDouble(&ok));
        if (!ok) {
            return;
        }
        auto vSpeed = Units::Speed::fromFPM(list[5].toDouble(&ok));
        if (!ok) {
            return;
        }
        double tt = list[7].toDouble(&ok);
        if (!ok) {
            return;
        }
        auto hSpeed = Units::Speed::fromKN(list[8].toDouble(&ok));
        if (!ok) {
            return;
        }
        auto callsign = list[9].simplified();

        auto trafficCoordinate = QGeoCoordinate(lat, lon, alt.toM());
        if (!trafficCoordinate.isValid()) {
            return;
        }
        QGeoPositionInfo geoPositionInfo(trafficCoordinate, QDateTime::currentDateTimeUtc());
        geoPositionInfo.setAttribute(QGeoPositionInfo::VerticalSpeed, vSpeed.toMPS());
        geoPositionInfo.setAttribute(QGeoPositionInfo::Direction, tt);
        geoPositionInfo.setAttribute(QGeoPositionInfo::GroundSpeed, hSpeed.toMPS());
        if (!geoPositionInfo.isValid()) {
            return;
        }

        // Compute horizontal and vertical distance to traffic if our own position
        // is known.
        Units::Distance hDist {};
        Units::Distance vDist {};
        auto* positionProviderPtr = Global::positionProvider();
        if (positionProviderPtr != nullptr) {
            auto ownShipCoordinate = positionProviderPtr->positionInfo().coordinate();
            if (ownShipCoordinate.isValid()) {
                hDist = Units::Distance::fromM( ownShipCoordinate.distanceTo(trafficCoordinate) );
                vDist = alt - Units::Distance::fromM(ownShipCoordinate.altitude());
            }
        }

        m_factor.setAlarmLevel(0);
        m_factor.setCallSign(callsign);
        m_factor.setHDist(hDist);
        m_factor.setID(targetID);
        m_factor.setPositionInfo(geoPositionInfo);
        m_factor.setType(Traffic::TrafficFactor_Abstract::unknown);
        m_factor.setVDist(vDist);
        m_factor.startLiveTime();
        emit factorWithPosition(m_factor);
        return;
    }

}

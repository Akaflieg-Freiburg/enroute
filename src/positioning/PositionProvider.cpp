/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

#include <QSettings>

#include "positioning/PositionProvider.h"
#include "traffic/TrafficDataProvider.h"


// Static instance of this class. Do not analyze, because of many unwanted warnings.
#ifndef __clang_analyzer__
Q_GLOBAL_STATIC(Positioning::PositionProvider, PositionProviderStatic);
#endif


Positioning::PositionProvider::PositionProvider(QObject *parent) : PositionInfoSource_Abstract(parent)
{
    // Restore the last valid coordiante
    QSettings settings;
    QGeoCoordinate tmp;
    tmp.setLatitude(settings.value(QStringLiteral("PositionProvider/lastValidLatitude"), m_lastValidCoordinate.latitude()).toDouble());
    tmp.setLongitude(settings.value(QStringLiteral("PositionProvider/lastValidLongitude"), m_lastValidCoordinate.longitude()).toDouble());
    tmp.setAltitude(settings.value(QStringLiteral("PositionProvider/lastValidAltitude"), m_lastValidCoordinate.altitude()).toDouble());
    if ((tmp.type() == QGeoCoordinate::Coordinate2D) || (tmp.type() == QGeoCoordinate::Coordinate3D)) {
        m_lastValidCoordinate = tmp;
    }

    // Restore the last valid track
    m_lastValidTT = AviationUnits::Angle::fromDEG( qBound(0, settings.value(QStringLiteral("PositionProvider/lastValidTrack"), 0).toInt(), 359) );

    // Wire up satellite source
    connect(&satelliteSource, &Positioning::PositionInfoSource_Satellite::positionInfoChanged, this, &PositionProvider::onPositionUpdated);
    connect(&satelliteSource, &Positioning::PositionInfoSource_Satellite::pressureAltitudeChanged, this, &PositionProvider::onPressureAltitudeUpdated);

    // Wire up traffic data provider source
    auto* trafficDataProvider = Traffic::TrafficDataProvider::globalInstance();
    if (trafficDataProvider != nullptr) {
        connect(trafficDataProvider, &Traffic::TrafficDataProvider::positionInfoChanged, this, &PositionProvider::onPositionUpdated);
        connect(trafficDataProvider, &Traffic::TrafficDataProvider::pressureAltitudeChanged, this, &PositionProvider::onPressureAltitudeUpdated);
    }
}


Positioning::PositionProvider::~PositionProvider()
{
    // Save the last valid coordinate
    QSettings settings;
    settings.setValue(QStringLiteral("PositionProvider/lastValidLatitude"), m_lastValidCoordinate.latitude());
    settings.setValue(QStringLiteral("PositionProvider/lastValidLongitude"), m_lastValidCoordinate.longitude());
    settings.setValue(QStringLiteral("PositionProvider/lastValidAltitude"), m_lastValidCoordinate.altitude());

    // Save the last valid track
    settings.setValue(QStringLiteral("PositionProvider/lastValidTrack"), m_lastValidTT.toDEG());
}


auto Positioning::PositionProvider::globalInstance() -> PositionProvider *
{
#ifndef __clang_analyzer__
    return PositionProviderStatic;
#else
    return nullptr;
#endif
}


void Positioning::PositionProvider::onPositionUpdated()
{
    // This method is called if one of our providers has a new position info.
    // We go through the list of providers in order of preference, to find the first one
    // that has a valid position info available for us.
    PositionInfo info;

    // Priority #1: Traffic data provider
    auto* trafficDataProvider = Traffic::TrafficDataProvider::globalInstance();
    if (trafficDataProvider != nullptr) {
        info = trafficDataProvider->positionInfo();
    }

    // Priority #2: Built-in sat receiver
    if (!info.isValid()) {
        info = satelliteSource.positionInfo();
    }

    // Set new info
    setPositionInfo(info);
    setLastValidCoordinate(info.coordinate());
    setLastValidTT(info.trueTrack());

    // Change _isInFlight if appropriate.
    auto GS = info.groundSpeed();
    if (GS.isFinite()) {

        if (m_isInFlight) {
            // If we are in flight at present, go back to ground mode only if the ground speed is less than minFlightSpeedInKT-flightSpeedHysteresis
            if ( GS.toKN() < minFlightSpeedInKT-flightSpeedHysteresis ) {
                m_isInFlight = false;
                emit isInFlightChanged();
            }
        } else {
            // If we are on the ground at present, go to flight mode only if the ground sped is more than minFlightSpeedInKT
            if ( GS.toKN() > minFlightSpeedInKT ) {
                m_isInFlight = true;
                emit isInFlightChanged();
            }
        }

    }

}


void Positioning::PositionProvider::onPressureAltitudeUpdated()
{
    // This method is called if one of our providers has a new pressure altitude.
    // We go through the list of providers in order of preference, to find the first one
    // that has valid data for us.
    AviationUnits::Distance pAlt;

    // Priority #1: Traffic data provider
    auto* trafficDataProvider = Traffic::TrafficDataProvider::globalInstance();
    if (trafficDataProvider != nullptr) {
        pAlt = trafficDataProvider->pressureAltitude();
    }

    // Priority #2: Built-in sat receiver
    if (!pAlt.isFinite()) {
        pAlt = satelliteSource.pressureAltitude();
    }

    // Set new info
    setPressureAltitude(pAlt);

}


auto Positioning::PositionProvider::wayTo(const QGeoCoordinate& position) const -> QString
{
    // Paranoid safety checks
    if (!positionInfo().isValid()) {
        return QString();
    }
    if (!position.isValid()) {
        return QString();
    }
    if (!m_lastValidCoordinate.isValid()) {
        return QString();
    }

    auto dist = AviationUnits::Distance::fromM(m_lastValidCoordinate.distanceTo(position));
    auto QUJ = qRound(m_lastValidCoordinate.azimuthTo(position));

    if (GlobalSettings::useMetricUnitsStatic()) {
        return QStringLiteral("DIST %1 km • QUJ %2°").arg(dist.toKM(), 0, 'f', 1).arg(QUJ);
    }
    return QStringLiteral("DIST %1 NM • QUJ %2°").arg(dist.toNM(), 0, 'f', 1).arg(QUJ);
}


void Positioning::PositionProvider::setLastValidCoordinate(const QGeoCoordinate &newCoordinate)
{
    if (!newCoordinate.isValid()) {
        return;
    }
    if (newCoordinate == m_lastValidCoordinate) {
        return;
    }
    m_lastValidCoordinate = newCoordinate;
    emit lastValidCoordinateChanged(m_lastValidCoordinate);
}


void Positioning::PositionProvider::setLastValidTT(AviationUnits::Angle newTT)
{
    if (!newTT.isFinite()) {
        return;
    }
    if (newTT == m_lastValidTT) {
        return;
    }
    m_lastValidTT = newTT;
    emit lastValidTTChanged(m_lastValidTT);
}


auto Positioning::PositionProvider::lastValidCoordinate() -> QGeoCoordinate
{
    auto *positionProvider = globalInstance();
    if (positionProvider == nullptr) {
        return {};
    }
    return positionProvider->m_lastValidCoordinate;
}


auto Positioning::PositionProvider::lastValidTT() -> AviationUnits::Angle
{
    auto *positionProvider = globalInstance();
    if (positionProvider == nullptr) {
        return {};
    }
    return positionProvider->m_lastValidTT;
}


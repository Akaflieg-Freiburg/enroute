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

#include "Global.h"
#include "positioning/PositionProvider.h"
#include "traffic/TrafficDataProvider.h"


// Static instance of this class. Do not analyze, because of many unwanted warnings.
#ifndef __clang_analyzer__
QPointer<Positioning::PositionProvider> positionProviderStatic {};
#endif


Positioning::PositionProvider::PositionProvider(QObject *parent) : PositionInfoSource_Abstract(parent)
{
    // Restore the last valid coordiante and track
    QSettings settings;
    QGeoCoordinate tmp;
    tmp.setLatitude(settings.value(QStringLiteral("PositionProvider/lastValidLatitude"), m_lastValidCoordinate.latitude()).toDouble());
    tmp.setLongitude(settings.value(QStringLiteral("PositionProvider/lastValidLongitude"), m_lastValidCoordinate.longitude()).toDouble());
    tmp.setAltitude(settings.value(QStringLiteral("PositionProvider/lastValidAltitude"), m_lastValidCoordinate.altitude()).toDouble());
    if ((tmp.type() == QGeoCoordinate::Coordinate2D) || (tmp.type() == QGeoCoordinate::Coordinate3D)) {
        m_lastValidCoordinate = tmp;
    }
    m_lastValidTT = Units::Angle::fromDEG( qBound(0, settings.value(QStringLiteral("PositionProvider/lastValidTrack"), 0).toInt(), 359) );

    // Wire up satellite source
    connect(&satelliteSource, &Positioning::PositionInfoSource_Satellite::positionInfoChanged, this, &PositionProvider::onPositionUpdated);
    connect(&satelliteSource, &Positioning::PositionInfoSource_Satellite::pressureAltitudeChanged, this, &PositionProvider::onPressureAltitudeUpdated);

    // Binding for updateStatusString
    connect(this, &Positioning::PositionProvider::receivingPositionInfoChanged, this, &Positioning::PositionProvider::updateStatusString);
    connect(&satelliteSource, &Positioning::PositionInfoSource_Satellite::statusStringChanged, this, &Positioning::PositionProvider::updateStatusString);

    // Wire up traffic data provider source
    QTimer::singleShot(0, this, &Positioning::PositionProvider::deferredInitialization);

    // Update properties
    updateStatusString();
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


void Positioning::PositionProvider::deferredInitialization() const
{

    connect(Global::trafficDataProvider(), &Traffic::TrafficDataProvider::positionInfoChanged, this, &PositionProvider::onPositionUpdated);
    connect(Global::trafficDataProvider(), &Traffic::TrafficDataProvider::pressureAltitudeChanged, this, &PositionProvider::onPressureAltitudeUpdated);

}


auto Positioning::PositionProvider::globalInstance() -> PositionProvider *
{
#ifndef __clang_analyzer__
    if (positionProviderStatic.isNull()) {
        positionProviderStatic = new PositionProvider();
    }
    return positionProviderStatic;
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
    QString source;

    // Priority #1: Traffic data provider
    auto* trafficDataProvider = Global::trafficDataProvider();
    if (trafficDataProvider != nullptr) {
        info = trafficDataProvider->positionInfo();
        source = trafficDataProvider->sourceName();
    }

    // Priority #2: Built-in sat receiver
    if (!info.isValid()) {
        info = satelliteSource.positionInfo();
        source = satelliteSource.sourceName();
    }

    // Set new info
    setPositionInfo(info);
    setLastValidCoordinate(info.coordinate());
    setLastValidTT(info.trueTrack());
    setSourceName(source);
    updateStatusString();
}


void Positioning::PositionProvider::onPressureAltitudeUpdated()
{
    // This method is called if one of our providers has a new pressure altitude.
    // We go through the list of providers in order of preference, to find the first one
    // that has valid data for us.
    Units::Distance pAlt;

    // Priority #1: Traffic data provider
    auto* trafficDataProvider = Global::trafficDataProvider();
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


void Positioning::PositionProvider::setLastValidTT(Units::Angle newTT)
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


auto Positioning::PositionProvider::lastValidTT() -> Units::Angle
{
    auto *positionProvider = globalInstance();
    if (positionProvider == nullptr) {
        return {};
    }
    return positionProvider->m_lastValidTT;
}


void Positioning::PositionProvider::updateStatusString()
{
    if (receivingPositionInfo()) {
        QString result = QString("<p>%1</p><ul style='margin-left:-25px;'>").arg(sourceName());
        result += QString("<li>%1</li>").arg(tr("Receiving position information."));
        if (pressureAltitude().isFinite()) {
            result += QString("<li>%1</li>").arg(tr("Receiving pressure altitude."));
        }
        result += "</ul>";
        setStatusString(result);
        return;
    }

    QString result = QString("<p>%1</p><ul style='margin-left:-25px;'>").arg(tr("Not receiving position information"));
    result += QString("<li>%1: %2</li>").arg( satelliteSource.sourceName(), satelliteSource.statusString());
    result += QString("<li>%1: %2</li>").arg( tr("Traffic receiver"), tr("Not receiving position information"));
    result += "</ul>";
    setStatusString(result);
}

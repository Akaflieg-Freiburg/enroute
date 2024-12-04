/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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

#include <QCoreApplication>
#include <QSettings>

#include "GlobalObject.h"
#include "GlobalSettings.h"
#include "positioning/PositionProvider.h"
#include "traffic/TrafficDataProvider.h"
#include "units/Units.h"

using namespace Qt::Literals::StringLiterals;


Positioning::PositionProvider::PositionProvider(QObject *parent) : PositionInfoSource_Abstract(parent)
{
    // Restore the last valid coordiante and track
    QSettings const settings;
    QGeoCoordinate tmp;
    tmp.setLatitude(settings.value(QStringLiteral("PositionProvider/lastValidLatitude"), m_lastValidCoordinate.value().latitude()).toDouble());
    tmp.setLongitude(settings.value(QStringLiteral("PositionProvider/lastValidLongitude"), m_lastValidCoordinate.value().longitude()).toDouble());
    tmp.setAltitude(settings.value(QStringLiteral("PositionProvider/lastValidAltitude"), m_lastValidCoordinate.value().altitude()).toDouble());
    if ((tmp.type() == QGeoCoordinate::Coordinate2D) || (tmp.type() == QGeoCoordinate::Coordinate3D)) {
        m_lastValidCoordinate = tmp;
    }
    m_lastValidTT = Units::Angle::fromDEG( qBound(0, settings.value(QStringLiteral("PositionProvider/lastValidTrack"), 0).toInt(), 359) );

    // Wire up satellite source
    connect(&satelliteSource, &Positioning::PositionInfoSource_Satellite::positionInfoChanged, this, &PositionProvider::onPositionUpdated);
#warning
    //    connect(&satelliteSource, &Positioning::PositionInfoSource_Satellite::pressureAltitudeChanged, this, &PositionProvider::onPressureAltitudeUpdated);

    // Binding for updateStatusString
#warning This is presently incorrect
//    connect(this, &Positioning::PositionProvider::receivingPositionInfoChanged, this, &Positioning::PositionProvider::updateStatusString);
#warning This is presently incorrect
    //    connect(&satelliteSource, &Positioning::PositionInfoSource_Satellite::statusStringChanged, this, &Positioning::PositionProvider::updateStatusString);

    // Wire up traffic data provider source
    QTimer::singleShot(0, this, &Positioning::PositionProvider::deferredInitialization);

    // Save position at regular intervals
    auto* saveTimer = new QTimer(this);
    saveTimer->setInterval(1min + 57s);
    saveTimer->setSingleShot(false);
    connect(saveTimer, &QTimer::timeout, this, &Positioning::PositionProvider::savePositionAndTrack);
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &Positioning::PositionProvider::savePositionAndTrack);
    saveTimer->start();

    // Update properties
    updateStatusString();
    m_approximateLastValidCoordinate = m_lastValidCoordinate.value();
    connect(this, &Positioning::PositionProvider::lastValidCoordinateChanged, this, [this]() {
        if (m_approximateLastValidCoordinate.value().isValid()
            && (m_approximateLastValidCoordinate.value().distanceTo(m_lastValidCoordinate) < 10000))
        {
            return;
        }
        m_approximateLastValidCoordinate = m_lastValidCoordinate.value();
    });
}



void Positioning::PositionProvider::deferredInitialization() const
{
    connect(GlobalObject::trafficDataProvider(), &Traffic::TrafficDataProvider::positionInfoChanged, this, &PositionProvider::onPositionUpdated);
#warning
//    connect(GlobalObject::trafficDataProvider(), &Traffic::TrafficDataProvider::pressureAltitudeChanged, this, &PositionProvider::onPressureAltitudeUpdated);
}


void Positioning::PositionProvider::onPositionUpdated()
{
    // This method is called if one of our providers has a new position info.
    // We go through the list of providers in order of preference, to find the first one
    // that has a valid position info available for us.
    PositionInfo newInfo;
    QString source;


    if (GlobalObject::globalSettings()->positioningByTrafficDataReceiver())
    {

        // Priority #1: Traffic data provider
        auto* trafficDataProvider = GlobalObject::trafficDataProvider();
        if (trafficDataProvider != nullptr)
        {
            newInfo = trafficDataProvider->positionInfo();
            source = trafficDataProvider->sourceName();
        }

        // Priority #2: Built-in sat receiver
        if (!newInfo.isValid())
        {
            newInfo = satelliteSource.positionInfo();
            source = satelliteSource.sourceName();
        }

    }
    else
    {

        // Priority #1: Built-in sat receiver
        newInfo = satelliteSource.positionInfo();
        source = satelliteSource.sourceName();

        // Priority #2: Traffic data provider
        if (!newInfo.isValid())
        {
            auto* trafficDataProvider = GlobalObject::trafficDataProvider();
            if (trafficDataProvider != nullptr)
            {
                newInfo = trafficDataProvider->positionInfo();
                source = trafficDataProvider->sourceName();
            }

        }

    }

    auto oldInfo = positionInfo();
    auto oldTimeStamp = oldInfo.timestamp();
    auto newTimeStamp = newInfo.timestamp();
    if (newTimeStamp == oldTimeStamp)
    {
        return;
    }

    // If no vertical speed has been provided by the system, we compute our own.
    if (!newInfo.verticalSpeed().isFinite()
            && newInfo.trueAltitudeAMSL().isFinite()
            && oldInfo.trueAltitudeAMSL().isFinite())
    {
        auto deltaV = (newInfo.trueAltitudeAMSL() - oldInfo.trueAltitudeAMSL());
        auto deltaT = Units::Timespan::fromMS( static_cast<double>(oldTimeStamp.msecsTo(newTimeStamp)) );
        auto vSpeed = deltaV/deltaT;
        if (vSpeed.isFinite())
        {
            if (oldInfo.verticalSpeed().isFinite())
            {
                vSpeed = 0.8*vSpeed + 0.2*positionInfo().verticalSpeed();
            }
            QGeoPositionInfo tmp = newInfo;
            tmp.setAttribute(QGeoPositionInfo::VerticalSpeed, vSpeed.toMPS());
            newInfo = PositionInfo(tmp);
        }
    }

    // Set new info
    setPositionInfo(newInfo);
    setLastValidCoordinate(newInfo.coordinate());
    setLastValidTT(newInfo.trueTrack());
    setSourceName(source);
    updateStatusString();
}

#warning
/*
void Positioning::PositionProvider::onPressureAltitudeUpdated()
{
    // This method is called if one of our providers has a new pressure altitude.
    // We go through the list of providers in order of preference, to find the first one
    // that has valid data for us.
    Units::Distance pAlt;

    // Priority #1: Traffic data provider
    auto* trafficDataProvider = GlobalObject::trafficDataProvider();
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
*/

void Positioning::PositionProvider::savePositionAndTrack()
{
    // Save the last valid coordinate
    QSettings settings;
    settings.setValue(QStringLiteral("PositionProvider/lastValidLatitude"), m_lastValidCoordinate.value().latitude());
    settings.setValue(QStringLiteral("PositionProvider/lastValidLongitude"), m_lastValidCoordinate.value().longitude());
    settings.setValue(QStringLiteral("PositionProvider/lastValidAltitude"), m_lastValidCoordinate.value().altitude());

    // Save the last valid track
    settings.setValue(QStringLiteral("PositionProvider/lastValidTrack"), m_lastValidTT.toDEG());
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
    auto *positionProvider = GlobalObject::positionProvider();
    if (positionProvider == nullptr) {
        return {};
    }
    return positionProvider->m_lastValidCoordinate.value();
}


auto Positioning::PositionProvider::lastValidTT() -> Units::Angle
{
    auto *positionProvider = GlobalObject::positionProvider();
    if (positionProvider == nullptr) {
        return {};
    }
    return positionProvider->m_lastValidTT;
}

#warning Turn this into a binding
void Positioning::PositionProvider::updateStatusString()
{
#warning make bindable: receivingPositionInfo, sourceName, statusString
    if (receivingPositionInfo()) {
        QString result = QStringLiteral("<ul style='margin-left:-25px;'>");
        result += QStringLiteral("<li>%1: %2</li>").arg(tr("Source"), sourceName());
        result += QStringLiteral("<li>%1</li>").arg(tr("Receiving position information"));
        result += u"</ul>"_s;
        setStatusString(result);
        return;
    }

    QString result = QStringLiteral("<p>%1</p><ul style='margin-left:-25px;'>").arg(tr("Not receiving position information"));
    result += QStringLiteral("<li>%1: %2</li>").arg( satelliteSource.sourceName(), satelliteSource.statusString());
    result += QStringLiteral("<li>%1: %2</li>").arg( tr("Traffic receiver"), tr("Not receiving position information"));
    result += u"</ul>"_s;
    setStatusString(result);
}

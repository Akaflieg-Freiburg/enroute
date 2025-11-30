/***************************************************************************
 *   Copyright (C) 2019-2025 by Stefan Kebekus                             *
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
#include "Sensors.h"
#include "Units.h"
#include "navigation/Navigator.h"
#include "positioning/PositionProvider.h"
#include "traffic/TrafficDataProvider.h"


using namespace Qt::Literals::StringLiterals;


Positioning::PositionProvider::PositionProvider(QObject* parent)
    : QObject(parent),
    m_receivingPositionInfo(false)
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
    m_approximateLastValidCoordinate = m_lastValidCoordinate.value();

    // Start deferred initialization
    QTimer::singleShot(0, this, &Positioning::PositionProvider::deferredInitialization);

    // Save position at regular intervals
    auto* saveTimer = new QTimer(this);
    saveTimer->setInterval(1min + 57s);
    saveTimer->setSingleShot(false);
    connect(saveTimer, &QTimer::timeout, this, &Positioning::PositionProvider::savePositionAndTrack);
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &Positioning::PositionProvider::savePositionAndTrack);
    saveTimer->start();
}

void Positioning::PositionProvider::deferredInitialization()
{
    // Setup bindings that refer to other global objects
    m_pressureAltitude.setBinding([this]() {return computePressureAltitude();});
    m_statusString.setBinding([this]() {return computeStatusString();});
    m_incomingPositionInfo.setBinding([this]() {return computeIncomingPositionInfo();});
    m_incomingPositionInfoNotifier = m_incomingPositionInfo.addNotifier([this]() {onIncomingPositionInfoUpdated();});
    onIncomingPositionInfoUpdated();
}

void Positioning::PositionProvider::onIncomingPositionInfoUpdated()
{
    auto newInfo = m_incomingPositionInfo.value();
    if (!newInfo.isValid())
    {
        m_receivingPositionInfo = false;
        m_positionInfo = newInfo;
        return;
    }

    // If no vertical speed has been provided by the system, we compute our own.
    auto oldInfo = positionInfo();
    auto oldTimeStamp = oldInfo.timestamp();
    auto newTimeStamp = newInfo.timestamp();
    if (!newInfo.verticalSpeed().isFinite()
        && newInfo.trueAltitudeAMSL().isFinite()
        && oldInfo.trueAltitudeAMSL().isFinite()
        && (newTimeStamp != oldTimeStamp))
    {
        auto deltaV = (newInfo.trueAltitudeAMSL() - oldInfo.trueAltitudeAMSL());
        auto deltaT = Units::Timespan::fromMS( static_cast<double>(oldTimeStamp.msecsTo(newTimeStamp)) );
        auto vSpeed = deltaV/deltaT;
        if (vSpeed.isFinite())
        {
            if (oldInfo.verticalSpeed().isFinite())
            {
                vSpeed = 0.8*vSpeed + 0.2*oldInfo.verticalSpeed();
            }
            QGeoPositionInfo tmp = newInfo;
            QString const src = newInfo.source();
            tmp.setAttribute(QGeoPositionInfo::VerticalSpeed, vSpeed.toMPS());
            newInfo = PositionInfo(tmp, src);
        }
    }

    // Set new info
    m_positionInfo = newInfo;
    m_receivingPositionInfo = true;

    m_lastValidCoordinate = newInfo.coordinate();
    auto TT = newInfo.trueTrack();
    if (TT.isFinite())
    {
        m_lastValidTT = TT;
    }
    if (!m_approximateLastValidCoordinate.value().isValid())
    {
        m_approximateLastValidCoordinate = m_lastValidCoordinate.value();
    }
    if (m_approximateLastValidCoordinate.value().isValid()
        && (m_approximateLastValidCoordinate.value().distanceTo(m_lastValidCoordinate) > 10000))
    {
        m_approximateLastValidCoordinate = m_lastValidCoordinate.value();
    }
}

void Positioning::PositionProvider::savePositionAndTrack()
{
    // Save the last valid coordinate
    QSettings settings;
    settings.setValue(QStringLiteral("PositionProvider/lastValidLatitude"), m_lastValidCoordinate.value().latitude());
    settings.setValue(QStringLiteral("PositionProvider/lastValidLongitude"), m_lastValidCoordinate.value().longitude());
    settings.setValue(QStringLiteral("PositionProvider/lastValidAltitude"), m_lastValidCoordinate.value().altitude());

    // Save the last valid track
    settings.setValue(QStringLiteral("PositionProvider/lastValidTrack"), m_lastValidTT.value().toDEG());
}

QGeoCoordinate Positioning::PositionProvider::lastValidCoordinate()
{
    auto *positionProvider = GlobalObject::positionProvider();
    if (positionProvider == nullptr) {
        return {};
    }
    return positionProvider->m_lastValidCoordinate.value();
}

Units::Angle Positioning::PositionProvider::lastValidTT()
{
    auto *positionProvider = GlobalObject::positionProvider();
    if (positionProvider == nullptr) {
        return {};
    }
    return positionProvider->m_lastValidTT.value();
}


//
// Computing Functions/Bindings
//

QString Positioning::PositionProvider::computeStatusString()
{
    if (m_positionInfo.value().isValid())
    {
        QString result = QStringLiteral("<ul style='margin-left:-25px;'>");
        result += QStringLiteral("<li>%1: %2</li>").arg(tr("Source"), m_positionInfo.value().source());
        result += QStringLiteral("<li>%1</li>").arg(tr("Receiving position information"));
        result += u"</ul>"_s;
        return result;
    }

    QString result = QStringLiteral("<p>%1</p><ul style='margin-left:-25px;'>").arg(tr("Not receiving position information"));
    result += QStringLiteral("<li>%1: %2</li>").arg( satelliteSource.sourceName(), satelliteSource.statusString());
    result += QStringLiteral("<li>%1: %2</li>").arg( tr("Traffic receiver"), tr("Not receiving position information"));
    result += u"</ul>"_s;
    return result;
}

Positioning::PositionInfo Positioning::PositionProvider::computeIncomingPositionInfo()
{
    // This method is called if one of our providers has a new position info.
    // We go through the list of providers in order of preference, to find the first one
    // that has a valid position info available for us.
    PositionInfo newInfo;

    if (GlobalObject::globalSettings()->positioningByTrafficDataReceiver())
    {
        // Priority #1: Traffic data provider
        auto* trafficDataProvider = GlobalObject::trafficDataProvider();
        if (trafficDataProvider != nullptr)
        {
            newInfo = trafficDataProvider->positionInfo();
        }

        // Priority #2: Built-in sat receiver
        if (!newInfo.isValid())
        {
            newInfo = satelliteSource.positionInfo();
        }
    }
    else
    {
        // Priority #1: Built-in sat receiver
        newInfo = satelliteSource.positionInfo();

        // Priority #2: Traffic data provider
        if (!newInfo.isValid())
        {
            auto* trafficDataProvider = GlobalObject::trafficDataProvider();
            if (trafficDataProvider != nullptr)
            {
                newInfo = trafficDataProvider->positionInfo();
            }
        }
    }
    return newInfo;
}

Units::Distance Positioning::PositionProvider::computePressureAltitude()
{
    auto pAlt = GlobalObject::trafficDataProvider()->pressureAltitude();
    if (pAlt.isFinite() || !GlobalObject::navigator()->aircraft().cabinPressureEqualsStaticPressure())
    {
        return pAlt;
    }

    return GlobalObject::sensors()->pressureAltitude();
}

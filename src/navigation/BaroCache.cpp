/***************************************************************************
 *   Copyright (C) 2025 by Markus Marks and Stefan Kebekus                 *
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

#include "BaroCache.h"
#include "positioning/PositionProvider.h"

Navigation::BaroCache::BaroCache(QObject* parent)
    : QObject(parent)
{
    qWarning() << "BaroCache constructor";

    notifiers.push_back(GlobalObject::positionProvider()->bindablePositionInfo().addNotifier([this]() {
        const auto geometricAltitude = GlobalObject::positionProvider()->positionInfo().trueAltitudeAMSL();
        qWarning() << "Geometric Altitude Received" << geometricAltitude.toM();
        if (!geometricAltitude.isFinite() || (geometricAltitude.toM() < -1000) || (geometricAltitude.toM() > 10000))
        {
            return;
        }
        m_incomingGeometricAltitude = geometricAltitude;
        m_incomingGeometricAltitudeTimestamp = QDateTime::currentDateTime();
        addIncomingBaroCacheData();
    }));
    notifiers.push_back(GlobalObject::positionProvider()->bindablePressureAltitude().addNotifier([this]() {
        const auto pressureAltitude = GlobalObject::positionProvider()->pressureAltitude();
        qWarning() << "Pressure Altitude Received" << pressureAltitude.toM();
        if (!pressureAltitude.isFinite() || (pressureAltitude.toM() < -1000) || (pressureAltitude.toM() > 10000))
        {
            return;
        }
        m_incomingPressureAltitude = pressureAltitude;
        m_incomingPressureAltitudeTimestamp = QDateTime::currentDateTime();
        addIncomingBaroCacheData();
    }));

    // Go through the list every 5 minutes and delete entries older than 40 minutes
    auto* timer = new QTimer(this);
    timer->setInterval(5min);
    connect(timer, &QTimer::timeout, this, [this]() {
        QMap<int, altitudeElement> newAltitudeElementsByFlightLevel;
        QMapIterator<int, altitudeElement> i(m_altitudeElementsByFlightLevel);
        while (i.hasNext()) {
            i.next();
            if (i.value().timestamp.secsTo(QDateTime::currentDateTime()) > 2400)
            {
                continue;
            }
            newAltitudeElementsByFlightLevel[i.key()] = i.value();
        }
        m_altitudeElementsByFlightLevel = newAltitudeElementsByFlightLevel;
    });
}


void Navigation::BaroCache::addIncomingBaroCacheData()
{
    qWarning() << "addIncomingBaroCacheData";
    // Update data
    if (!m_incomingPressureAltitude.isFinite() || !m_incomingGeometricAltitudeTimestamp.isValid()
        || !m_incomingGeometricAltitude.isFinite() || !m_incomingGeometricAltitudeTimestamp.isValid()
        || (qAbs(m_incomingGeometricAltitudeTimestamp.secsTo(m_incomingPressureAltitudeTimestamp)) > 3))
    {
        return;
    }

    qWarning() << m_incomingPressureAltitudeTimestamp << m_incomingPressureAltitude.toM() << m_incomingGeometricAltitude.toM();
    auto FL = qRound(m_incomingPressureAltitude.toFeet()/100.0);
    m_altitudeElementsByFlightLevel[FL] = {m_incomingPressureAltitudeTimestamp, m_incomingPressureAltitude, m_incomingGeometricAltitude};
    m_incomingGeometricAltitude = {};
    m_incomingGeometricAltitudeTimestamp = {};
    m_incomingPressureAltitude = {};
    m_incomingPressureAltitudeTimestamp = {};
}

Units::Distance Navigation::BaroCache::estimatedPressureAltitude(Units::Distance geometricAltitude)
{
    altitudeElement closest;
    QMapIterator<int, altitudeElement> i(m_altitudeElementsByFlightLevel);
    while (i.hasNext()) {
        i.next();
        if (!closest.geometricAltitude.isFinite() || (qAbs(i.value().geometricAltitude - geometricAltitude) < qAbs(closest.geometricAltitude - geometricAltitude)))
        {
            closest = i.value();
        }
    }
    if (!closest.geometricAltitude.isFinite())
    {
        return {};
    }
    if (qAbs(closest.geometricAltitude - geometricAltitude).toFeet() > 500)
    {
        return {};
    }
    return closest.pressureAltitude - closest.geometricAltitude + geometricAltitude;
}

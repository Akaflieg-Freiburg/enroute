/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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

#include <QQmlEngine>
#include "navigation/Atmosphere.h"


//
// Constructors and destructors
//

Navigation::Atmosphere::Atmosphere(QObject *parent) : GlobalObject(parent)
{
#if defined(Q_OS_ANDROID) or defined(Q_OS_IOS)
    m_pressureSensor.setActive(true);
    m_temperatureSensor.setActive(true);

    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Navigation::Atmosphere::updateSensorReadings);
    timer->setInterval(1000);
    timer->setSingleShot(false);
    timer->start();
#endif
}


void Navigation::Atmosphere::deferredInitialization()
{
}



//
// Slots
//


void Navigation::Atmosphere::updateSensorReadings()
{
#if defined(Q_OS_ANDROID) or defined(Q_OS_IOS)
    double new_ambientPressure { qQNaN() };
    double new_ambientTemperature { qQNaN() };

    auto* pressureReading = m_pressureSensor.reading();
    if (pressureReading != nullptr)
    {
        new_ambientPressure = pressureReading->pressure();
    }
    delete pressureReading;
    if (new_ambientPressure != m_ambientPressure)
    {
        m_ambientPressure = new_ambientPressure;
        emit ambientPressureChanged();
    }

    auto* temperatureReading = m_temperatureSensor.reading();
    if (temperatureReading != nullptr)
    {
        new_ambientTemperature = temperatureReading->temperature();
    }
    delete temperatureReading;
    if (new_ambientTemperature != m_ambientTemperature)
    {
        m_ambientTemperature = new_ambientTemperature;
        emit ambientTemperatureChanged();
    }
#endif
}

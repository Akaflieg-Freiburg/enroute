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
#include <QTimer>

#include "Sensors.h"


//
// Constructors and destructors
//

Sensors::Sensors(QObject *parent) : GlobalObject(parent)
{
#if defined(Q_OS_ANDROID) or defined(Q_OS_IOS)
    m_pressureSensor.setActive(true);
    m_temperatureSensor.setActive(true);

    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Sensors::updateSensorReadings);
    timer->setInterval(1000);
    timer->setSingleShot(false);
    timer->start();

    connect(&m_pressureSensor, &QPressureSensor::availableSensorsChanged, this, &Sensors::updateStatusString);
#endif
    updateSensorReadings();
    updateStatusString();
}


void Sensors::deferredInitialization()
{
}



//
// Slots
//


void Sensors::updateSensorReadings()
{
#if defined(Q_OS_ANDROID) or defined(Q_OS_IOS)
    Units::Pressure new_ambientPressure;
    Units::Temperature new_ambientTemperature;

    auto* pressureReading = m_pressureSensor.reading();
    if (pressureReading != nullptr)
    {
        new_ambientPressure = Units::Pressure::fromPa(pressureReading->pressure());
    }
    if (new_ambientPressure != m_ambientPressure)
    {
        m_ambientPressure = new_ambientPressure;
        emit ambientPressureChanged();
    }

    auto* temperatureReading = m_temperatureSensor.reading();
    if (temperatureReading != nullptr)
    {
        new_ambientTemperature = Units::Temperature::fromDegreeCelsius(temperatureReading->temperature());
    }
    if (new_ambientTemperature != m_ambientTemperature)
    {
        m_ambientTemperature = new_ambientTemperature;
        emit ambientTemperatureChanged();
    }

    auto new_ambientDensity = Navigation::Atmosphere::density(m_ambientPressure, m_ambientTemperature);
    if (new_ambientDensity != m_ambientDensity)
    {
        m_ambientDensity = new_ambientDensity;
        emit ambientDensityChanged();
    }
#endif

}


void Sensors::updateStatusString()
{
    QString newStatus;

    QStringList sensorNames;
#if defined(Q_OS_ANDROID) or defined(Q_OS_IOS)
    auto types = QPressureSensor::sensorTypes();
    if (types.contains("QPressureSensor"))
    {
        sensorNames += "<li>" + tr("Pressure sensor available") + "</li>";
    }
    if (types.contains("QAmbientTemperatureSensor"))
    {
        sensorNames += "<li>" + tr("Temperature sensor available") + "</li>";
    }
#endif
    if (sensorNames.isEmpty())
    {
        newStatus = tr("No ambient pressure/temperature sensor available");
    }
    else
    {
        newStatus = "<ul style='margin-left:-25px;'>" + sensorNames.join(u""_qs) + "</ul>";
    }

    if (newStatus != m_statusString)
    {
        m_statusString = newStatus;
        emit statusStringChanged();
    }
}

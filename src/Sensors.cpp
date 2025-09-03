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

#include <QQmlEngine>
#include <QTimer>

#include "Sensors.h"
#include "navigation/Atmosphere.h"
#include <chrono>

using namespace std::chrono_literals;

using namespace Qt::Literals::StringLiterals;


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
    timer->setInterval(1s);
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
    auto* pressureReading = m_pressureSensor.reading();
    if (pressureReading != nullptr)
    {
        new_ambientPressure = Units::Pressure::fromPa(pressureReading->pressure());
    }

    m_ambientPressure = new_ambientPressure;
    m_pressureAltitude = Navigation::Atmosphere::height(new_ambientPressure);

    Units::Temperature new_ambientTemperature;
    auto* temperatureReading = m_temperatureSensor.reading();
    if (temperatureReading != nullptr)
    {
        new_ambientTemperature = Units::Temperature::fromDegreeCelsius(temperatureReading->temperature());
    }
    m_ambientTemperature = new_ambientTemperature;
#endif

}


void Sensors::updateStatusString()
{
    QString newStatus;

#if defined(Q_OS_ANDROID) or defined(Q_OS_IOS)
    QStringList sensorNames;
    auto types = QPressureSensor::sensorTypes();
    if (types.contains("QPressureSensor"))
    {
        sensorNames += u"<li>"_s + tr("Pressure sensor available.") + u"</li>"_s;
    }
    if (types.contains("QAmbientTemperatureSensor"))
    {
        sensorNames += u"<li>"_s + tr("Temperature sensor available.") + u"</li>"_s;
    }
#else
    const QStringList sensorNames;
#endif
    if (sensorNames.isEmpty())
    {
        newStatus = tr("No ambient pressure/temperature sensor available.");
    }
    else
    {
        newStatus = u"<ul style='margin-left:-25px;'>"_s + sensorNames.join(u""_s) + u"</ul>"_s;
    }

    m_statusString = newStatus;
}

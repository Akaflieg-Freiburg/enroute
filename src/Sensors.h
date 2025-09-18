/***************************************************************************
 *   Copyright (C) 2023-2025 by Stefan Kebekus                             *
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

#pragma once

#include "GlobalObject.h"
#include "units/Distance.h"
#include "units/Pressure.h"
#include "units/Temperature.h"

#include <QProperty>
#include <QQmlEngine>
#if defined(Q_OS_ANDROID) or defined(Q_OS_IOS)
#include <QAmbientTemperatureSensor>
#include <QPressureSensor>
#endif


/*! \brief Sensor data
 *
 *  This class collects data from ambient pressure/temperature sensors.
 *
 */

class Sensors : public GlobalObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    //
    // Constructors and destructors
    //

    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Sensors(QObject* parent = nullptr);

    // deferred initialization
    void deferredInitialization() override;

    // No default constructor, important for QML singleton
    explicit Sensors() = delete;

    /*! \brief Standard destructor */
    ~Sensors() override = default;

    // factory function for QML singleton
    static Sensors* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::sensors();
    }


    //
    // PROPERTIES
    //
    /*! \brief Ambient pressure
     *
     *  This property holds the ambient pressure recorded by the device sensor (if any).
     */
    Q_PROPERTY(Units::Pressure ambientPressure READ ambientPressure BINDABLE bindableAmbientPressure)

    /*! \brief Ambient temperature
     *
     *  This property holds the ambient temperature recorded by the device sensor (if any).
     */
    Q_PROPERTY(Units::Temperature ambientTemperature READ ambientTemperature BINDABLE bindableAmbientTemperature)

    /*! \brief Pressure altitude
     *
     *  This property holds the pressure altitude for the ambient pressure measured by the device sensor.
     *  In most practical setups, this will be the cabin altitude
     */
    Q_PROPERTY(Units::Distance pressureAltitude READ pressureAltitude BINDABLE bindablePressureAltitude)

    /*! \brief Status string */
    Q_PROPERTY(QString statusString READ statusString BINDABLE bindableStatusString)


    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property ambientPressure
     */
    [[nodiscard]] Units::Pressure ambientPressure() const { return m_ambientPressure.value(); }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property ambientPressure
     */
    [[nodiscard]] QBindable<Units::Pressure> bindableAmbientPressure() { return &m_ambientPressure; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property ambientPressure
     */
    [[nodiscard]] Units::Temperature ambientTemperature() const { return m_ambientTemperature.value(); }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property ambientPressure
     */
    [[nodiscard]] QBindable<Units::Temperature> bindableAmbientTemperature() { return &m_ambientTemperature; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property pressureAltitude
     */
    [[nodiscard]] Units::Distance pressureAltitude() const { return m_pressureAltitude.value(); }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property pressureAltitude
     */
    [[nodiscard]] QBindable<Units::Distance> bindablePressureAltitude() { return &m_pressureAltitude; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property statusString
     */
    [[nodiscard]] QString statusString() const { return m_statusString.value(); }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property statusString
     */
    [[nodiscard]] QBindable<QString> bindableStatusString() { return &m_statusString; }

private slots:
    // Update sensor readings. For performance reasons, we poll sensors.
    void updateSensorReadings();

    // Update the status string
    void updateStatusString();

private:
    Q_DISABLE_COPY_MOVE(Sensors)

#if defined(Q_OS_ANDROID) or defined(Q_OS_IOS)
    // Ambient temperature sensor
    QAmbientTemperatureSensor m_temperatureSensor;

    // Ambient pressure sensor
    QPressureSensor m_pressureSensor;
#endif

    QProperty<Units::Pressure> m_ambientPressure;
    QProperty<Units::Distance> m_pressureAltitude;
    QProperty<Units::Temperature> m_ambientTemperature;
    QProperty<QString> m_statusString;
};

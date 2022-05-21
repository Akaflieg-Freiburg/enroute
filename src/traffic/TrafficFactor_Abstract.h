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

#pragma once

#include <chrono>
#include <QTimer>

#include "units/Distance.h"

using namespace std::chrono_literals;


namespace Traffic {


/*! \brief Abstract base class for traffic factors
 *
 *  This is an abstract base class for traffic factors, as reported by traffic
 *  data receivers (e.g. FLARM devices).
 *
 *  Since the real-world traffic situation changes continuously, instances of this class have a limited lifetime.
 *  The length of the lifetime is specified in the constant "lifeTime". You can (re)start an object's lifetime
 *  startLiveTime(). Once the lift-time of an object is expired, the property "valid" will alway contain
 *  the word "false", regardless of the object's other properties.
 */

class TrafficFactor_Abstract : public QObject {
    Q_OBJECT

public:
    /*! \brief Length of lifetime for objects of this class */
    static constexpr auto lifeTime = 10s;

    /*! \brief Aircraft type
     *
     *  This enum defines a few aircraft type. The list is modeled after the FLARM/NMEA specification.
     */
    enum AircraftType
    {
        unknown, /*!< Unknown aircraft type */
        Aircraft, /*!< Fixed wing aircraft */
        Airship, /*!< Airship, such as a zeppelin or a blimp */
        Balloon, /*!< Balloon */
        Copter, /*!< Helicopter, gyrocopter or rotorcraft */
        Drone, /*!< Drone */
        Glider, /*!< Glider, including powered gliders and touring motor gliders */
        HangGlider, /*!< Hang glider */
        Jet, /*!< Jet aircraft */
        Paraglider, /*!< Paraglider */
        Skydiver, /*!< Skydiver */
        StaticObstacle, /*!< Static obstacle */
        TowPlane /*!< Tow plane */
    };
    Q_ENUM(AircraftType)

    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit TrafficFactor_Abstract(QObject* parent = nullptr);

    // Standard destructor
    ~TrafficFactor_Abstract() override = default;


    //
    // Methods
    //

    /*! \brief Copy data from other object
     *
     *  This method copies all properties from the other object, with two notable exceptions.
     *
     *  - The property "animate" is not copied, the property "animate" of this class is not touched.
     *  - The lifeTime of this object is not changed.
     *
     *  @param other Instance whose properties are copied
     */
    void copyFrom(const TrafficFactor_Abstract& other)
    {
        setAlarmLevel(other.alarmLevel());
        setCallSign(other.callSign());
        setHDist(other.hDist());
        setID(other.ID());
        setType(other.type());
        setVDist(other.vDist());
        updateDescription();
    }

    /*! \brief Estimates if this traffic object has higher priority than other
     *  traffic object
     *
     * The following criteria are applied in order.
     *
     * - Valid traffic objects have higher priority than invalid objects.
     * - Traffic objects with higher alarm level have higher priority.
     * - Traffic objects that are closer have higher priority.
     *
     * @param rhs Right hand side of the comparison
     *
     * @returns Boolean with the result
     */
    [[nodiscard]] auto hasHigherPriorityThan(const TrafficFactor_Abstract& rhs) const -> bool;

    /*! \brief Starts or extends the lifetime of this object
     *
     *  Traffic information is valantile, and is considered valid only
     *  for "lifeTime" seconds.  This method starts or extends the
     *  object's life time.
     */
    void startLiveTime();


    //
    // PROPERTIES
    //

    /*! \brief Alarm Level
     *
     *  This is the alarm level associated with the traffic object. The alarm level is an integer in the
     *  range 0 (no alarm), …, 3 (maximal alarm). The values are not computed by this class, but reported
     *  by the traffic receiver that reports the traffic. The precise meaning depends on the type of
     *  traffic receiver used.
     *
     *  FLARM
     *
     *  - 0 = no alarm (also used for no-alarm traffic information)
     *  - 1 = alarm, 13-18 seconds to impact
     *  - 2 = alarm, 9-12 seconds to impact
     *  - 3 = alarm, 0-8 seconds to impact
     *
     *  Other
     *
     *  - 0 = no alarm
     *  - 1 = alarm
     */
    Q_PROPERTY(int alarmLevel READ alarmLevel WRITE setAlarmLevel NOTIFY alarmLevelChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property alarmLevel
     */
    [[nodiscard]] auto alarmLevel() const -> int
    {
        return m_alarmLevel;
    }

    /*! \brief Setter function for property with the same name
     *
     *  @param newAlarmLevel Property alarmLevel
     */
    void setAlarmLevel(int newAlarmLevel)
    {

        // Safety checks
        if ((newAlarmLevel < 0) || (newAlarmLevel > 3)) {
            return;
        }

        startLiveTime();
        if (m_alarmLevel == newAlarmLevel) {
            return;
        }
        if ((newAlarmLevel < 0) || (newAlarmLevel > 3)) {
            return;
        }
        m_alarmLevel = newAlarmLevel;
        emit alarmLevelChanged();

    }

    /*! \brief Indicates if changes in properties should be animated in the GUI
     *
     *  This boolen properts is used to indicate if changes in properties should be animated in the
     *  GUI.  This property is typically set to "true" before gradual changes are applied, such as
     *  the position change of an aircraft.  It is typically set to "false" before data of a new
     *  aircraft set.
     */
    Q_PROPERTY(bool animate READ animate WRITE setAnimate NOTIFY animateChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property animate
     */
    [[nodiscard]] auto animate() const -> bool
    {
        return m_animate;
    }

    /*! \brief Setter function for property with the same name
     *
     *  @param newAnimate Property animate
     */
    void setAnimate(bool newAnimate) {
        if (m_animate == newAnimate) {
            return;
        }
        m_animate = newAnimate;
        emit animateChanged();
    }

    /*! \brief Call sign
     *
     *  If known, this property holds the call sign of the traffic.  Otherwise, it contains an empty string
     */
    Q_PROPERTY(QString callSign READ callSign WRITE setCallSign NOTIFY callSignChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property callSign
     */
    [[nodiscard]] auto callSign() const -> QString
    {
        return m_callSign;
    }

    /*! \brief Setter function for property with the same name
     *
     *  @param newCallSign Property callSign
     */
    void setCallSign(const QString& newCallSign) {
        if (m_callSign == newCallSign) {
            return;
        }
        m_callSign = newCallSign;
        emit callSignChanged();
    }

    /*! \brief Suggested color for GUI representation of the traffic
     *
     *  This propery suggests a color, depending on the alarmLevel.
     *
     *  - alarmLevel == 0: green
     *  - alarmLevel == 1: yellow
     *  - alarmLevel >= 2: red
     */
    Q_PROPERTY(QString color READ color NOTIFY colorChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property color
     */
    [[nodiscard]] auto color() const -> QString
    {
        if (m_alarmLevel == 0) {
            return QStringLiteral("green");
        }
        if (m_alarmLevel == 1) {
            return QStringLiteral("yellow");
        }
        return QStringLiteral("red");
    }

    /*! \brief Description of the traffic, for use in GUI
     *
     *  This method holds a human-readable, translated description of the
     *  traffic. This is a rich-text string of the form "Glider<br>+15 0m" or
     *  "Airship<br>Position unknown<br>-45 ft".
     */
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property description
     */
    [[nodiscard]] auto description() const -> QString
    {
        return m_description;
    }

    /*! \brief Horizontal distance from own position to the traffic, at the time of report
     *
     *  If known, this property holds the horizontal distance from the own
     *  position to the traffic, at the time of report.  Otherwise, it contains
     *  an invalid distance.
     */
    Q_PROPERTY(Units::Distance hDist READ hDist WRITE setHDist NOTIFY hDistChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property hDist
     */
    [[nodiscard]] auto hDist() const -> Units::Distance
    {
        return m_hDist;
    }

    /*! \brief Setter function for property with the same name
     *
     *  @param newHDist Property hDist
     */
    void setHDist(Units::Distance newHDist)
    {

        if (m_hDist == newHDist) {
            return;
        }
        m_hDist = newHDist;
        emit hDistChanged();
    }

    /*! \brief Identifier string of the traffic
     *
     *  This property holds an identifier string for the traffic, as assigned by
     *  the FLARM device that reported the traffic. This can be the FLARM ID, or
     *  an empty string if no meaningful ID can be assigned.
     */
    Q_PROPERTY(QString ID READ ID WRITE setID NOTIFY IDChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property ID
     */
    [[nodiscard]] auto ID() const -> QString
    {
        return m_ID;
    }

    /*! \brief Setter function for property with the same name
     *
     *  @param newID Property ID
     */
    void setID(const QString& newID) {
        if (m_ID == newID) {
            return;
        }
        m_ID = newID;
        emit IDChanged();
    }

    /*! \brief Type of aircraft, as reported by the traffic receiver */
    Q_PROPERTY(AircraftType type READ type WRITE setType NOTIFY typeChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property type
     */
    [[nodiscard]] auto type() const -> AircraftType
    {
        return m_type;
    }

    /*! \brief Setter function for property with the same name
     *
     *  @param newType Property type
     */
    void setType(AircraftType newType) {
        if (m_type == newType) {
            return;
        }
        m_type = newType;
        emit typeChanged();
    }

    /*! \brief Validity
     *
     *  A traffic object is considered valid if the data is meaningful and if the
     *  lifetime is not expired.  Only valid traffic objects should be shown in the GUI.
     */
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property valid
     */
    [[nodiscard]] auto valid() const -> bool
    {
        return m_valid;
    }

    /*! \brief Vertical distance from own position to the traffic, at the time of report
     *
     *  If known, this property holds the vertical distance from the own
     *  position to the traffic, at the time of report.  Otherwise, it contains
     *  NaN.
     */
    Q_PROPERTY(Units::Distance vDist READ vDist WRITE setVDist NOTIFY vDistChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property vDist
     */
    [[nodiscard]] auto vDist() const -> Units::Distance
    {
        return m_vDist;
    }

    /*! \brief Setter function for property with the same name
     *
     *  @param newVDist Property vDist
     */
    void setVDist(Units::Distance newVDist) {
        if (m_vDist == newVDist) {
            return;
        }
        m_vDist = newVDist;
        emit vDistChanged();
    }

signals:
    /*! \brief Notifier signal */
    void alarmLevelChanged();

    /*! \brief Notifier signal */
    void animateChanged();

    /*! \brief Notifier signal */
    void callSignChanged();

    /*! \brief Notifier signal */
    void colorChanged();

    /*! \brief Notifier signal */
    void descriptionChanged();

    /*! \brief Notifier signal */
    void hDistChanged();

    /*! \brief Notifier signal */
    void IDChanged();

    /*! \brief Notifier signal */
    void typeChanged();

    /*! \brief Notifier signal */
    void vDistChanged();

    /*! \brief Notifier signal */
    void validChanged();


protected:
    // Setter function for the property valid.  This function is virtual and must not be
    // called or accessed from the constructor. For this reason, we have a special function
    // "dispatchUpdateValid", which whose address is already known to the constructor.
    virtual void updateValid();
    void dispatchUpdateValid();
    bool m_valid {false};

    // Setter function for the property "description". This function is virtual and must not be
    // called or accessed from the constructor. For this reason, we have a special function
    // "dispatchUpdateDescription", which whose address is already known to the constructor.
    virtual void updateDescription();
    void dispatchUpdateDescription();
    QString m_description {};

private:
    //
    // Property values
    //
    int m_alarmLevel {0};
    bool m_animate {false};
    QString m_callSign {};
    QString m_color {QStringLiteral("red")};
    Units::Distance m_hDist;
    QString m_ID;
    AircraftType m_type {AircraftType::unknown};
    Units::Distance m_vDist;

    // Timer for timeout. Traffic objects become invalid if their data has not been
    // refreshed for longer than timeout.
    QTimer lifeTimeCounter;
};

}

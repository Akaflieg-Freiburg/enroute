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

/*! \brief Traffic factors
 *
 *  Objects of this class represent traffic factors, as detected by FLARM and
 *  similar devices.  This is an abstract base class.
 */

class TrafficFactor_Abstract : public QObject {
    Q_OBJECT

public:
    /*! \brief Aircraft type */
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
    explicit TrafficFactor_Abstract(QObject *parent = nullptr);

    // Standard destructor
    ~TrafficFactor_Abstract() override = default;


    //
    // Methods
    //

    /*! \brief Copy data from other object */
    virtual void copy(const TrafficFactor_Abstract& other)
    {
        setAlarmLevel(other.alarmLevel());
        setAnimate(other.animate());
        setCallSign(other.callSign());
        setID(other.ID());
        setType(other.type());
        setVDist(other.vDist());
    }

    /*! \brief Updates the timestamp to the current time, extending the life time of the object */
    void updateTimestamp();


    //
    // PROPERTIES
    //

    /*! \brief Alarm Level, as reported by FLARM
     *
     * This is the alarm level associated with the traffic object. Alarm levels
     * are not computed by this class, but by the FLARM device that reports the
     * traffic. This is an integer in the range 0, …, 3 with the following
     * meaning.
     *
     *  - 0 = no alarm (also used for no-alarm traffic information)
     *  - 1 = alarm, 13-18 seconds to impact
     *  - 2 = alarm, 9-12 seconds to impact
     *  - 3 = alarm, 0-8 seconds to impact
     */
    Q_PROPERTY(int alarmLevel READ alarmLevel WRITE setAlarmLevel NOTIFY alarmLevelChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property alarmLevel
     */
    int alarmLevel() const
    {
        return m_alarmLevel;
    }

    /*! \brief Setter function for property with the same name
     *
     *  @param newAlarmLevel Property alarmLevel
     */
    void setAlarmLevel(int newAlarmLevel);

    /*! \brief Indicates if changes in properties should be animated in the GUI
     *
     *  This method indicates if changes in properties should be animated in the
     *  GUI. It is "true" if the class believes that changes in the properties
     *  indicate movement of the traffic opponent. It is "false" if the class
     *  believes that changes in the properties indicate that a new traffic
     *  opponent is being tracked.
     */
    Q_PROPERTY(bool animate READ animate WRITE setAnimate NOTIFY animateChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property animate
     */
    bool animate() const
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
    QString callSign() const
    {
        return m_callSign;
    }

    /*! \brief Setter function for property with the same name
     *
     *  @param newCallSign Property callSign
     */
    void setCallSign(const QString& newCallsign) {
        if (m_callSign == newCallsign) {
            return;
        }
        m_callSign = newCallsign;
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
    QString color() const
    {
        if (m_alarmLevel == 0) {
            return "green";
        }
        if (m_alarmLevel == 1) {
            return "yellow";
        }
        return "red";
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
    virtual QString description() const = 0;

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
    QString ID() const
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

    /*! \brief Type of aircraft, as reported by FLARM */
    Q_PROPERTY(AircraftType type READ type WRITE setType NOTIFY typeChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property vDist
     */
    AircraftType type() const
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
     * A traffic object is considered valid if the data is meaningful and if the
     * report is no older than the time specified in timeoutMS.  Only valid
     * traffic objects should be shown in the GUI.
     */
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property valid
     */
    bool valid() const
    {
        return m_valid;
    }

    /*! \brief Vertical distance from own position to the traffic, at the time
     *  of report
     *
     *  If known, this property holds the vertical distance from the own
     *  position to the traffic, at the time of report.  Otherwise, it contains
     *  NaN.
     */
    Q_PROPERTY(AviationUnits::Distance vDist READ vDist WRITE setVDist NOTIFY vDistChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property vDist
     */
    AviationUnits::Distance vDist() const
    {
        return m_vDist;
    }

    /*! \brief Setter function for property with the same name
     *
     *  @param newVDist Property vDist
     */
    void setVDist(AviationUnits::Distance newVDist) {
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
    void IDChanged();

    /*! \brief Notifier signal */
    void typeChanged();

    /*! \brief Notifier signal */
    void vDistChanged();

    /*! \brief Notifier signal */
    void validChanged();

private slots:
    // Setter function for the property valid. Implementors of this class must bind this to the
    // notifier signals of all the properties that validity depends on.
    virtual void setValid() = 0;

private:
    //
    // Property values
    //
    int m_alarmLevel {0};
    bool m_animate {true};
    QString m_callSign {};
    QString m_color {QStringLiteral("red")};
    QString m_ID;
    AircraftType m_type {AircraftType::unknown};
    AviationUnits::Distance m_vDist;
    bool m_valid {false};

    // Timer for timeout. Traffic objects become invalid if their data has not been
    // refreshed for longer than timeout.
    QTimer timeoutCounter;
    static constexpr auto timeout = 10s;
};

}

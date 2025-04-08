/***************************************************************************
 *   Copyright (C) 2021-2025 by Stefan Kebekus                             *
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

#include <QObjectBindableProperty>
#include <QTimer>

#include "units/Distance.h"
#include "TrafficFactorAircraftType.h"

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
    Q_PROPERTY(int alarmLevel READ alarmLevel WRITE setAlarmLevel NOTIFY alarmLevelChanged BINDABLE bindableAlarmLevel)

    /*! \brief Indicates if changes in properties should be animated in the GUI
     *
     *  This boolen properts is used to indicate if changes in properties should be animated in the
     *  GUI.  This property is typically set to "true" before gradual changes are applied, such as
     *  the position change of an aircraft.  It is typically set to "false" before data of a new
     *  aircraft set.
     */
    Q_PROPERTY(bool animate READ animate WRITE setAnimate NOTIFY animateChanged)

    /*! \brief Call sign
     *
     *  If known, this property holds the call sign of the traffic.  Otherwise, it contains an empty string
     */
    Q_PROPERTY(QString callSign READ callSign WRITE setCallSign NOTIFY callSignChanged)

    /*! \brief Suggested color for GUI representation of the traffic
     *
     *  This propery suggests a color, depending on the alarmLevel.
     *
     *  - alarmLevel == 0: green
     *  - alarmLevel == 1: yellow
     *  - alarmLevel >= 2: red
     */
    Q_PROPERTY(QString color READ color NOTIFY colorChanged)

    /*! \brief Description of the traffic, for use in GUI
     *
     *  This method holds a human-readable, translated description of the
     *  traffic. This is a rich-text string of the form "Glider<br>+15 0m" or
     *  "Airship<br>Position unknown<br>-45 ft".
     */
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)

    /*! \brief Horizontal distance from own position to the traffic, at the time of report
     *
     *  If known, this property holds the horizontal distance from the own
     *  position to the traffic, at the time of report.  Otherwise, it contains
     *  an invalid distance.
     */
    Q_PROPERTY(Units::Distance hDist READ hDist WRITE setHDist NOTIFY hDistChanged BINDABLE bindableHDist)

    /*! \brief Identifier string of the traffic
     *
     *  This property holds an identifier string for the traffic, as assigned by
     *  the FLARM device that reported the traffic. This can be the FLARM ID, or
     *  an empty string if no meaningful ID can be assigned.
     */
    Q_PROPERTY(QString ID READ ID WRITE setID NOTIFY IDChanged)

    /*! \brief Indicates relevant traffic
     *
     *  This property holds 'true' if the traffic is valid, and closer than maxVerticalDistance and maxHorizontalDistance
     *  specified below.
     */
    Q_PROPERTY(bool relevant READ relevant BINDABLE bindableRelevant)

    /*! \brief Translated string containing the 'relevant' property */
    Q_PROPERTY(QString relevantString READ relevantString BINDABLE bindableRelevantString)

    /*! \brief Type of aircraft, as reported by the traffic receiver */
    Q_PROPERTY(AircraftType type READ type WRITE setType NOTIFY typeChanged BINDABLE bindableType)

    /*! \brief Type of aircraft, as reported by the traffic receiver
     *
     *  This property holds a translated, human-readable string.
     */
    Q_PROPERTY(QString typeString READ typeString BINDABLE bindableTypeString)

    /*! \brief Validity
     *
     *  A traffic object is considered valid if the data is meaningful and if the
     *  lifetime is not expired.  Only valid traffic objects should be shown in the GUI.
     */
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged BINDABLE bindableValid)

    /*! \brief Vertical distance from own position to the traffic, at the time of report
     *
     *  If known, this property holds the vertical distance from the own
     *  position to the traffic, at the time of report.  Otherwise, it contains
     *  NaN.
     */
    Q_PROPERTY(Units::Distance vDist READ vDist WRITE setVDist NOTIFY vDistChanged BINDABLE bindableVDist)


    //
    // Getter/Setter Methods
    //

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property alarmLevel
     */
    [[nodiscard]] int alarmLevel() const {return m_alarmLevel.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property alarmLevel
     */
    [[nodiscard]] QBindable<int> bindableAlarmLevel() {return &m_alarmLevel;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property animate
     */
    [[nodiscard]] auto animate() const -> bool
    {
        return m_animate;
    }

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property callSign
     */
    [[nodiscard]] auto callSign() const -> QString
    {
        return m_callSign;
    }

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

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property description
     */
    [[nodiscard]] auto description() const -> QString
    {
        return m_description;
    }

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property hDist
     */
    [[nodiscard]] Units::Distance hDist() const {return m_hDist.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property hDist
     */
    [[nodiscard]] QBindable<Units::Distance> bindableHDist() {return &m_hDist;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property ID
     */
    [[nodiscard]] auto ID() const -> QString
    {
        return m_ID;
    }

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property relevant
     */
    [[nodiscard]] bool relevant() const {return m_relevant.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property relevant
     */
    [[nodiscard]] QBindable<bool> bindableRelevant() {return &m_relevant;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property relevant
     */
    [[nodiscard]] QString relevantString() const {return m_relevantString.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property relevantString
     */
    [[nodiscard]] QBindable<bool> bindableRelevantString() {return &m_relevantString;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property type
     */
    [[nodiscard]] Traffic::AircraftType type() const {return m_type.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property type
     */
    [[nodiscard]] QBindable<Traffic::AircraftType> bindableType() {return &m_type;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property typeString
     */
    [[nodiscard]] QString typeString() const {return m_typeString.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property typeString
     */
    [[nodiscard]] QBindable<QString> bindableTypeString() {return &m_typeString;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property valid
     */
    [[nodiscard]] bool valid() const {return m_valid.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property valid
     */
    [[nodiscard]] QBindable<bool> bindableValid() {return &m_valid;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property vDist
     */
    [[nodiscard]] Units::Distance vDist() const {return m_vDist.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property vDist
     */
    [[nodiscard]] QBindable<Units::Distance> bindableVDist() {return &m_vDist;}


    //
    // Setter Methods
    //

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
        m_alarmLevel = newAlarmLevel;
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

    /*! \brief Setter function for property with the same name
     *
     *  @param newHDist Property hDist
     */
    void setHDist(Units::Distance newHDist) {m_hDist = newHDist;}

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

    /*! \brief Setter function for property with the same name
     *
     *  @param newType Property type
     */
    void setType(Traffic::AircraftType newType) {m_type = newType;}

    /*! \brief Setter function for property with the same name
     *
     *  @param newVDist Property vDist
     */
    void setVDist(Units::Distance newVDist) {m_vDist = newVDist;}


    //
    // Constants
    //

    /*! \brief Maximal vertical distance for relevant traffic
     *
     *  Traffic whose vertical distance to the own aircraft is larger than this
     *  number will be considered irrelevant.
     */
    static constexpr Units::Distance maxVerticalDistance = Units::Distance::fromM(1500.0);

    /*! \brief Maximal horizontal distance for relevant traffic
     *
     *  Traffic whose horizontal distance to the own aircraft is larger than
     *  this number will be considered irrelevant.
     */
    static constexpr Units::Distance maxHorizontalDistance = Units::Distance::fromNM(20.0);

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
    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(Traffic::TrafficFactor_Abstract, bool, m_valid, false, &Traffic::TrafficFactor_Abstract::validChanged);

    // Setter function for the property "description". This function is virtual and must not be
    // called or accessed from the constructor. For this reason, we have a special function
    // "dispatchUpdateDescription", which whose address is already known to the constructor.
    virtual void updateDescription();
    void dispatchUpdateDescription();
    QString m_description;

private:
    Q_DISABLE_COPY_MOVE(TrafficFactor_Abstract)

    //
    // Property values
    //
    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(Traffic::TrafficFactor_Abstract, int, m_alarmLevel, 0, &Traffic::TrafficFactor_Abstract::alarmLevelChanged);
    bool m_animate {false};
    QString m_callSign;
    QString m_color{QStringLiteral("red")};
    Q_OBJECT_BINDABLE_PROPERTY(Traffic::TrafficFactor_Abstract, Units::Distance, m_hDist, &Traffic::TrafficFactor_Abstract::hDistChanged);
    QString m_ID;
    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(Traffic::TrafficFactor_Abstract, Traffic::AircraftType, m_type, unknown, &Traffic::TrafficFactor_Abstract::typeChanged);
    QProperty<bool> m_relevant {false};
    QProperty<QString> m_relevantString;
    QProperty<QString> m_typeString;
    Q_OBJECT_BINDABLE_PROPERTY(Traffic::TrafficFactor_Abstract, Units::Distance, m_vDist, &Traffic::TrafficFactor_Abstract::vDistChanged);

    // Timer for timeout. Traffic objects become invalid if their data has not been
    // refreshed for longer than timeout.
    QTimer lifeTimeCounter;
};

} // namespace Traffic

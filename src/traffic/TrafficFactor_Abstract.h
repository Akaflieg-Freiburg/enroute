/***************************************************************************
 *   Copyright (C) 2021-2026 by Stefan Kebekus                             *
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

using namespace std::chrono_literals;


namespace Traffic {

// Plain-data record fed into updateFrom()/replaceBy(); defined in TrafficFactorData.h
struct TrafficFactorData;

/*! \brief Abstract base class for traffic factors
 *
 *  This is an abstract base class for traffic factors, as reported by traffic
 *  data receivers (e.g. FLARM devices).
 *
 *  Since the real-world traffic situation changes continuously, instances of this class have a limited lifetime.
 *  The length of the lifetime is specified in the constant "lifetime". You can (re)start an object's lifetime
 *  startLifetime(). Once the lifetime of an object is expired, the property "valid" will alway contain
 *  the word "false", regardless of the object's other properties.
 *
 *  Classes that inherit from TrafficFactor_Abstract need to provide a binding for the properties 'description'
 *  and 'valid'.
 */

class TrafficFactor_Abstract : public QObject {
    Q_OBJECT
    QML_ELEMENT

public:
    /*! \brief Aircraft type
     *
     *  This enum defines a few aircraft types. The list is modeled after the FLARM/NMEA specification.
     */
    enum Type
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
    Q_ENUM(Type) // Register the enum with Qt's meta-object system

    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit TrafficFactor_Abstract(QObject* parent = nullptr);

    // Standard destructor
    ~TrafficFactor_Abstract() override;


    //
    // Methods
    //

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
    [[nodiscard]] bool hasHigherPriorityThan(const TrafficFactor_Abstract& rhs) const;

    /*! \brief Relevance criterion shared by the "relevant" property and priority logic
     *
     *  This helper holds the rule that decides whether traffic at the given
     *  distances is relevant, i.e. close enough to be worth showing. It is used
     *  both by the binding of the "relevant" property and by the free function
     *  hasHigherPriorityThan(const TrafficFactorData&, const TrafficFactor_Abstract&),
     *  so that both share a single definition.
     *
     *  @param hDist Horizontal distance to the traffic
     *
     *  @param vDist Vertical distance to the traffic
     *
     *  @returns True if traffic at these distances is considered relevant
     */
    [[nodiscard]] static bool isRelevant(Units::Distance hDist, Units::Distance vDist);

    /*! \brief Starts or extends the lifetime of this object
     *
     *  Traffic information is valantile, and is considered valid only
     *  for "lifetime" seconds.  This method starts or extends the
     *  object's lifetime.
     */
    void startLifetime();


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
    Q_PROPERTY(int alarmLevel READ alarmLevel WRITE setAlarmLevel BINDABLE bindableAlarmLevel)

    /*! \brief Indicates if changes in properties should be animated in the GUI
     *
     *  This boolen properts is used to indicate if changes in properties should be animated in the
     *  GUI.  This property is typically set to "true" before gradual changes are applied, such as
     *  the position change of an aircraft.  It is typically set to "false" before data of a new
     *  aircraft set.
     */
    Q_PROPERTY(bool animate READ animate BINDABLE bindableAnimate)

    /*! \brief Call sign
     *
     *  If known, this property holds the call sign of the traffic.  Otherwise, it contains an empty string
     */
    Q_PROPERTY(QString callSign READ callSign WRITE setCallSign BINDABLE bindableCallSign)

    /*! \brief Suggested color for GUI representation of the traffic
     *
     *  This propery suggests a color, depending on the alarmLevel.
     *
     *  - alarmLevel == 0: green
     *  - alarmLevel == 1: yellow
     *  - alarmLevel >= 2: red
     */
    Q_PROPERTY(QString color READ color BINDABLE bindableColor)

    /*! \brief Description of the traffic, for use in GUI
     *
     *  This method holds a human-readable, translated description of the
     *  traffic. This is a rich-text string of the form "Glider<br>+15 0m" or
     *  "Airship<br>Position unknown<br>-45 ft".
     */
    Q_PROPERTY(QString description READ description BINDABLE bindableDescription)

    /*! \brief Horizontal distance from own position to the traffic, at the time of report
     *
     *  If known, this property holds the horizontal distance from the own
     *  position to the traffic, at the time of report.  Otherwise, it contains
     *  an invalid distance.
     */
    Q_PROPERTY(Units::Distance hDist READ hDist WRITE setHDist BINDABLE bindableHDist)

    /*! \brief Identifier string of the traffic
     *
     *  This property holds an identifier string for the traffic, as assigned by
     *  the FLARM device that reported the traffic. This can be the FLARM ID, or
     *  an empty string if no meaningful ID can be assigned.
     */
    Q_PROPERTY(QString ID READ ID WRITE setID BINDABLE bindableID)

    /*! \brief Indicates relevant traffic
     *
     *  This property holds 'true' if the traffic is valid, and closer than maxVerticalDistance and maxHorizontalDistance
     *  specified below.
     */
    Q_PROPERTY(bool relevant READ relevant BINDABLE bindableRelevant)

    /*! \brief Translated string containing the 'relevant' property
     *
     *  The content of the string is a translated version of "Relevant Traffic" or "Irrelevant Traffic".
     */
    Q_PROPERTY(QString relevantString READ relevantString BINDABLE bindableRelevantString)

    /*! \brief Type of aircraft, as reported by the traffic receiver */
    Q_PROPERTY(Type type READ type WRITE setType BINDABLE bindableType)

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
    Q_PROPERTY(bool valid READ valid BINDABLE bindableValid)

    /*! \brief Vertical distance from own position to the traffic, at the time of report
     *
     *  If known, this property holds the vertical distance from the own
     *  position to the traffic, at the time of report.  Otherwise, it contains
     *  NaN.
     */
    Q_PROPERTY(Units::Distance vDist READ vDist WRITE setVDist BINDABLE bindableVDist)


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
    [[nodiscard]] bool animate() const {return m_animate.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property animate
     */
    [[nodiscard]] QBindable<bool> bindableAnimate() {return &m_animate;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property callSign
     */
    [[nodiscard]] QString callSign() const {return m_callSign.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property callSign
     */
    [[nodiscard]] QBindable<QString> bindableCallSign() const {return &m_callSign;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property color
     */
    [[nodiscard]] QString color() const {return m_color.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property color
     */
    [[nodiscard]] QBindable<QString> bindableColor() const {return &m_color;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property description
     */
    [[nodiscard]] QString description() const {return m_description.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property description
     */
    [[nodiscard]] QBindable<QString> bindableDescription() const {return &m_description;}

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
    [[nodiscard]] QString ID() const {return m_ID.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property ID
     */
    [[nodiscard]] QBindable<QString> bindableID() const {return &m_ID;}

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
    [[nodiscard]] QBindable<QString> bindableRelevantString() {return &m_relevantString;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property type
     */
    [[nodiscard]] TrafficFactor_Abstract::Type type() const {return m_type.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property type
     */
    [[nodiscard]] QBindable<TrafficFactor_Abstract::Type> bindableType() {return &m_type;}

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
        startLifetime();
        m_alarmLevel = newAlarmLevel;
    }

    /*! \brief Setter function for property with the same name
     *
     *  @param newCallSign Property callSign
     */
    void setCallSign(const QString& newCallSign) {m_callSign = newCallSign;}

    /*! \brief Setter function for property with the same name
     *
     *  @param newHDist Property hDist
     */
    void setHDist(Units::Distance newHDist) {m_hDist = newHDist;}

    /*! \brief Setter function for property with the same name
     *
     *  @param newID Property ID
     */
    void setID(const QString& newID) {m_ID = newID;}

    /*! \brief Setter function for property with the same name
     *
     *  @param newType Property type
     */
    void setType(TrafficFactor_Abstract::Type newType) {m_type = newType;}

    /*! \brief Setter function for property with the same name
     *
     *  @param newVDist Property vDist
     */
    void setVDist(Units::Distance newVDist) {m_vDist = newVDist;}


    //
    // Constants
    //

    /*! \brief Length of lifetime for objects of this class */
    static constexpr auto lifetime = 45s;

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

protected:
    QProperty<bool> m_valid {false};

    QProperty<QString> m_description;

    // Indicates that the instance is valid as an abstract traffic factor
    QProperty<bool> m_validAbstractTrafficFactor;

    /*! \brief Update this object with newer data for the same traffic factor
     *
     *  This method is for the case where \a data describes the *same* traffic
     *  factor as *this, observed again with (typically newer) data. The caller is
     *  responsible for establishing that the two refer to the same factor.
     *
     *  Identity data that is already established here is preserved, and the
     *  transition is animated:
     *
     *  - the "animate" property of *this is set to true, so that the GUI animates
     *    the transition from the old to the new data;
     *  - the callsign in \a data is taken over only if the callsign of *this is
     *    still empty (an established callsign is never overwritten);
     *  - the type in \a data is taken over only if the type of *this is still
     *    unknown (an established type is never overwritten).
     *
     *  If there is nothing to continue from — *this is no longer valid, i.e. its
     *  track had expired — there is no established identity to preserve and no
     *  meaningful transition to animate. The call then degrades to replaceBy().
     *
     *  In all cases the lifetime of *this is (re)started, so the caller does not
     *  need to call startLifetime() separately.
     *
     *  @param data Data record whose contents are used to update *this
     */
    void updateFrom(const TrafficFactorData& data);

    /*! \brief Replace this object by a different traffic factor
     *
     *  This method is for the case where *this is repurposed to represent a
     *  *different* traffic factor, namely the one described by \a data. All
     *  properties are overwritten unconditionally and the "animate" property is set
     *  to false, since animating a transition between two unrelated factors would
     *  be meaningless.
     *
     *  The lifetime of *this is (re)started, so the caller does not need to call
     *  startLifetime() separately.
     *
     *  @param data Data record whose contents replace the data of *this
     */
    void replaceBy(const TrafficFactorData& data);

private:
    Q_DISABLE_COPY_MOVE(TrafficFactor_Abstract)

    //
    // Property values
    //
    QProperty<int> m_alarmLevel {0};
    QProperty<bool> m_animate {false};
    QProperty<QString> m_callSign;
    QProperty<QString> m_color {QStringLiteral("red")};
    QProperty<Units::Distance> m_hDist;
    QProperty<QString> m_ID;
    QProperty<Type> m_type {unknown};
    QProperty<bool> m_relevant {false};
    QProperty<QString> m_relevantString;
    QProperty<QString> m_typeString;
    QProperty<Units::Distance> m_vDist;

    // Timer for timeout. Traffic objects become invalid if their data has not been
    // refreshed for longer than timeout.
    QTimer lifetimeCounter;
};

} // namespace Traffic

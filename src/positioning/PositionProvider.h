/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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

#include <QBindable>
#include <QQmlEngine>

#include "GlobalObject.h"
#include "positioning/PositionInfoSource_Satellite.h"


namespace Positioning {

/*! \brief Central Position Provider
 *
 *  This class collects position data from the various sources (satellite,
 *  network, traffic receiver, …)  chooses the best available source and exposes
 *  the data to QML and other parts of the program.
 *
 *  There exists one static instance of this class, which can be accessed via
 *  the method globalInstance().  No other instance of this class should be
 *  used.
 *
 *  Data from the standard operating system data source (typically: satnav or
 *  wifi) is only provided after startUpdates() has been called.
 *
 *  The methods in this class are reentrant, but not thread safe.
 */

class PositionProvider : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON


public:
    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit PositionProvider(QObject* parent = nullptr);

    // No default constructor, important for QML singleton
    explicit PositionProvider() = delete;

    /*! \brief Standard destructor */
    ~PositionProvider() override = default;


    // factory function for QML singleton
    static Positioning::PositionProvider* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::positionProvider();
    }


    //
    // PROPERTIES
    //

    /*! \brief Approximate last valid coordinate
     *
     *  This property equals lastValidCoordinate, except that it is updated only every ten
     *  kilometers.
     */
    Q_PROPERTY(QGeoCoordinate approximateLastValidCoordinate READ approximateLastValidCoordinate BINDABLE bindableApproximateLastValidCoordinate)

    /*! \brief Last valid coordinate reading
     *
     *  This property holds the last valid coordinate known.  At the first
     *  start, this property is set to the location Freiburg Airport, EDTF.  The
     *  value is stored in a QSetting at destruction, and restored in the
     *  construction.
     */
    Q_PROPERTY(QGeoCoordinate lastValidCoordinate READ lastValidCoordinate BINDABLE bindableLastValidCoordinate)

    /*! \brief Last valid true track
     *
     *  This property holds the last valid true track known.  At the first
     *  start, this property is set to 0°.  The value is stored in a QSetting at
     *  destruction, and restored in the construction.
     */
    Q_PROPERTY(Units::Angle lastValidTT READ lastValidTT BINDABLE bindableLastValidTT)

    /*! \brief Position information
     *
     *  This property holds information about the device position. To ensure
     *  that the data is up-to-date, the position information will be set to an
     *  invalid positionInfo when no data has arrived for more than the time
     *  specified in PositionInfo::lifetime.
     */
    Q_PROPERTY(Positioning::PositionInfo positionInfo READ positionInfo BINDABLE bindablePositionInfo NOTIFY positionInfoChanged)

    /*! \brief Indicator that position information is being received
     *
     *  This is a shortcut for positionInfo().isValid. This property exists
     *  because it does not change so often, and can thus be more efficient to
     *  use.
     */
    Q_PROPERTY(bool receivingPositionInfo READ receivingPositionInfo BINDABLE bindableReceivingPositionInfo)

    /*! \brief Source status
     *
     *  This property holds a translated, human-readable string that describes
     *  the status of the positionInfo source. This could typically be a string
     *  of the form "OK" or "Insufficient permission to access position info"
     */
    Q_PROPERTY(QString statusString READ statusString BINDABLE bindableStatusString)


    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property approximateLastValidCoordinate
     */
    [[nodiscard]] QGeoCoordinate approximateLastValidCoordinate() const {return m_approximateLastValidCoordinate.value();}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property approximateLastValidCoordinate
     */
    [[nodiscard]] QBindable<QGeoCoordinate> bindableApproximateLastValidCoordinate() const {return &m_approximateLastValidCoordinate;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property lastValidCoordinate
     */
    [[nodiscard]] static QGeoCoordinate lastValidCoordinate();

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property lastValidCoordinate
     */
    [[nodiscard]] QBindable<QGeoCoordinate> bindableLastValidCoordinate() {return &m_lastValidCoordinate;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property lastValidTrack
     */
    [[nodiscard]] static Units::Angle lastValidTT();

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property lastValidTrack
     */
    [[nodiscard]] QBindable<Units::Angle> bindableLastValidTT() {return &m_lastValidTT;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property positionInfo
     */
    [[nodiscard]] Positioning::PositionInfo positionInfo() const {return m_positionInfo.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property positionInfo
     */
    [[nodiscard]] QBindable<Positioning::PositionInfo> bindablePositionInfo() const {return &m_positionInfo;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property receivingPositionInfo
     */
    [[nodiscard]] bool receivingPositionInfo() const {return m_receivingPositionInfo.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property receivingPositionInfo
     */
    [[nodiscard]] QBindable<bool> bindableReceivingPositionInfo() {return &m_receivingPositionInfo;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property statusString
     */
    [[nodiscard]] QString statusString() const
    {
        return m_statusString.value();
    }

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property statusString
     */
    [[nodiscard]] QBindable<QString> bindableStatusString() const
    {
        return &m_statusString;
    }



    //
    // Methods
    //

    /*! \brief startUpdates
     *
     *  Requests permissions if necessary and starts to provide data
     *  from the standard operating system data source (typically SatNav or
     *  WiFi) if permissions were granted.
     */
    Q_INVOKABLE void startUpdates() { satelliteSource.startUpdates(); }

signals:
    // Notifier signal
    void approximateLastValidCoordinateChanged();

    // Notifier signal
    void positionInfoChanged();

    // Notifier signal
    void receivingPositionInfoChanged(bool);

private slots:   
    // Intializations that are moved out of the constructor, in order to avoid
    // nested uses of constructors in Global.
    void deferredInitialization();

    // Saves last valid position and track
    void savePositionAndTrack();

private:
    Q_DISABLE_COPY_MOVE(PositionProvider)

    // Computation method for property with the same name
    QString computeStatusString();

    // Aircraft is considered flying if speed is at least this high
    static constexpr double minFlightSpeedInKT = 30.0;
    // Hysteresis for flight speed
    static constexpr double flightSpeedHysteresis = 5.0;

    // Coordinates of EDTF airfield
    static constexpr double EDTF_lat = 48.022653;
    static constexpr double EDTF_lon = 7.832583;
    static constexpr double EDTF_ele = 244;

    PositionInfoSource_Satellite satelliteSource;

    // The incoming position info is set by a binding that monitors
    // the satelliteSource and the TrafficDataSource
    QProperty<Positioning::PositionInfo> m_incomingPositionInfo;
    Positioning::PositionInfo computeIncomingPositionInfo();

    // This method updates m_approximateLastValidCoordinate, m_lastValidCoordinate and m_lastValidTT
    // whenever m_incomingPositionInfo changes.
    void onIncomingPositionInfoUpdated();
    QPropertyNotifier m_incomingPositionInfoNotifier;

    Q_OBJECT_BINDABLE_PROPERTY(Positioning::PositionProvider, Positioning::PositionInfo, m_positionInfo, &Positioning::PositionProvider::positionInfoChanged);
    Q_OBJECT_BINDABLE_PROPERTY(Positioning::PositionProvider, QGeoCoordinate, m_approximateLastValidCoordinate, &Positioning::PositionProvider::approximateLastValidCoordinateChanged);
    QProperty<QGeoCoordinate> m_lastValidCoordinate {QGeoCoordinate(EDTF_lat, EDTF_lon, EDTF_ele)};
    QProperty<Units::Angle> m_lastValidTT;

    Q_OBJECT_BINDABLE_PROPERTY(Positioning::PositionProvider, bool, m_receivingPositionInfo, &Positioning::PositionProvider::receivingPositionInfoChanged);
    QProperty<QString> m_statusString;

};

} // namespace Positioning

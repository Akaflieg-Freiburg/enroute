/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#include <QGeoPositionInfoSource>
#include <QLocale>
#include <QTimer>

#include "AviationUnits.h"
#include "GlobalSettings.h"
#include "positioning/Geoid.h"
#include "positioning/PositionInfo.h"

namespace Positioning {

/*! \brief Satellite Navigator

  This class is a thin wrapper around QGeoPositionInfoSource.  The main
  differences to QGeoPositionInfoSource are the following.

  - This class the data in formats suitable for aviation purposes.

  - This class has a well-defined timeout if no data has been received for more
    than two minutes.

  - The signal statusChanged() reliably lets you know if the status changes. The
    QGeoPositionInfoSource, in contrast, only reports errors. It does not report
    explicitly when error conditions are lifted.

  Once constructed, the health status of the satellite navigation subsystem can
  be queried using the status property. If all is well, the class receives
  regular satellite navigation data packets (aka 'fixes'), in the form a
  QGeoPositionInfo object, and the signal 'update' is emitted.  The
  QGeoPositionInfo is considered valid for one minute, and can be accessed via
  the method lastFix(), or via a multitude of properties that present the data
  in formats suitable for GUI applications.  If no new PositionProvider fixes come in for
  one minute, the QGeoPositionInfo will cleared, and the status is set to
  Timeout until new data arrives.

  There exists one static instance of this class, which can be accessed via the
  method globalInstance().  No other instance of this class should be used.

  The methods in this class are reentrant, but not thread safe.
*/

class PositionProvider : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
   *
   * @param parent The standard QObject parent pointer
   */
    explicit PositionProvider(QObject *parent = nullptr);

    /*! \brief Standard deconstructor */
    ~PositionProvider() override;

    /*! \brief Pointer to static instance
     *
     * This method returns a pointer to a static instance of this class. In rare
     * situations, during shutdown of the app, a nullptr might be returned.
     *
     * @returns A pointer to a static instance of this class
     */
    static PositionProvider *globalInstance();

    /*! \brief Estimate whether the device is flying or on the ground
     *  This property holds an estimate, as to whether the device is flying or on
     *  the ground.  The current implementation considers the device is flying if
     *  the groundspeed can be read and is greater then 30 knots.
     */
    Q_PROPERTY(bool isInFlight READ isInFlight NOTIFY isInFlightChanged)

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property isInFlight
     */
    bool isInFlight() const { return _isInFlight; }

    /*! \brief Last valid coordinate reading

    This property holds the last valid coordinate found in a PositionProvider fix.  At
    the first start, this property is set to the location Freiburg Airport,
    EDTF.  The value is stored in a QSetting at destruction, and restored in the
    construction.

    \sa lastValidCoordinate
  */
    Q_PROPERTY(QGeoCoordinate lastValidCoordinate READ lastValidCoordinate NOTIFY update)

    /*! \brief Getter function for the property with the same name

    @returns Property lastValidCoordinate
  */
    QGeoCoordinate lastValidCoordinate() const;


    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property lastValidTrack
     */
    AviationUnits::Angle lastValidTT() const { return _lastValidTT; }


    Q_PROPERTY(AviationUnits::Distance pressureAltitude READ pressureAltitude NOTIFY pressureAltitudeChanged)

#warning documentation
    static AviationUnits::Distance pressureAltitude();

    /*! \brief Status of the PositionProvider class

    This property holds a localized string that describes the status of the
    PositionProvider class.  A typical string is of the form "Connection to PositionProvider
    subsystem lost".
  */
    Q_PROPERTY(QString statusString READ statusString NOTIFY receivingChanged)

    /*! \brief Getter function for the property with the same name

    @returns Property statusAsString
  */
    QString statusString() const;

    /*! \brief Getter function for property of the same name
     *
     * This function differs from lastValidCoordinate() only in that it is static.
     * It uses the globalInstance() to retrieve data.
     *
     * @returns Property lastValidCoordinate
     */
    static QGeoCoordinate lastValidCoordinateStatic();

    Q_PROPERTY(Positioning::PositionInfo positionInfo READ positionInfo NOTIFY update)

    /*! \brief Current position info
     *
     *  If no PositionProvider has been received for more than ten seconds, this method
     *  return an invalid QGeoPositionInfo.
     *
     *  @returns The current position info
     */
    Positioning::PositionInfo positionInfo() const
    {
        return PositionInfo(_positionInfo);
    }

    Q_INVOKABLE bool receiving() const
    {
        return _positionInfo.isValid();
    }

    /*! \brief Description of the way from the current position to the given position
   *
   * This method uses the unit system preferred by the user.
   *
   * @param position Position
   *
   * @returns A string of the form "DIST 65.2 NM • QUJ 276°" or an empty string if the
   * current position is not known.
   */
    Q_INVOKABLE QString wayTo(const QGeoCoordinate& position) const;

signals:
    void pressureAltitudeChanged();

    /*! \brief Emitted whenever the suggested icon changes */
    void isInFlightChanged();

    /*! \brief Emitted whenever the property lastValidTrack changes */
    void lastValidTTChanged(AviationUnits::Angle);

    /*! \brief Emitted whenever a new GS fix has arrived */
    void update();

    void receivingChanged();
private slots:
    // Connected to source, in order to receive new data
    void onPositionUpdated(const QGeoPositionInfo &info);

    // Connected to source, in order to receive error information
    void error(QGeoPositionInfoSource::Error newSourceStatus);

    // Connected to timeoutCounter, in order to receive timeout after one minute
    void timeout();

    void onPositionUpdated_Sat(const QGeoPositionInfo &info);

private:
    Q_DISABLE_COPY_MOVE(PositionProvider)

    // Aircraft is considered flying if speed is at least this high
    static constexpr double minFlightSpeedInKT = 30.0;
    // Hysteresis for flight speed
    static constexpr double flightSpeedHysteresis = 5.0;

    // Coordinates of EDTF airfield
    static constexpr double EDTF_lat = 48.022653;
    static constexpr double EDTF_lon = 7.832583;
    static constexpr double EDTF_ele = 244;

    QLocale myLocale;
    QGeoPositionInfoSource *source;
    QGeoPositionInfo _positionInfo;
    QGeoCoordinate _lastValidCoordinate {EDTF_lat, EDTF_lon, EDTF_ele};
    AviationUnits::Angle _lastValidTT {};
    bool _isInFlight {false};

    Positioning::Geoid* _geoid {nullptr};

    // Constant: timeout occurs after one minute without receiving new data
    const int timeoutThreshold = 10*1000;

    // Set according to the status of *source. We need to replicate the
    // information stored in source because the status of the source can change
    // from "error" to "ok" without notification. This data field is then used to
    // check if something changed, and to emit the signal "statusUpdate" when
    // appropriate.
    QGeoPositionInfoSource::Error sourceStatus {QGeoPositionInfoSource::AccessError};

    // QTimer used to measure time since last data packet was received.  Connected
    // to call timeout() after timeoutThreshold milliseconds of no data.
    QTimer timeoutCounter;
};

}

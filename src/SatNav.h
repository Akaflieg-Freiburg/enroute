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

#include "Geoid.h"

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
  in formats suitable for GUI applications.  If no new SatNav fixes come in for
  one minute, the QGeoPositionInfo will cleared, and the status is set to
  Timeout until new data arrives.

  The methods in this class are reentrant, but not thread safe.
*/

class SatNav : public QObject
{
  Q_OBJECT

public:
  /*! \brief Standard constructor

    @param parent The standard QObject parent pointer
  */
  explicit SatNav(QObject *parent = nullptr);

  /*! \brief Standard deconstructor */
  ~SatNav() override;

  /*! \brief Status codes */
  enum Status
    {
     OK, /*!< The SatNav class works and received data. */
     Timeout, /*!< No error has been reported, but no data has been received for
		more than one minute.  */
     Error /*!< The underlying QGeoPositionInfoSource has reported an error. A
	     human-readable error message can be accessed with the property
	     statusAsString */
    };
  Q_ENUM(Status)

  /*! \brief Raw altitude

    This property holds the raw altitude found in the last satnav fix, in feet
    above sea level.  Negative altitudes are possible.  In case no valid SatNav
    fix exists, or in case where no altitude information is reported, the
    property is set to the value "0".
  */
  Q_PROPERTY(int rawAltitudeInFeet READ rawAltitudeInFeet NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns rawAltitudeInFeet
  */
  int rawAltitudeInFeet() const;

  /*! \brief Raw altitude

    This property holds the raw altitude found in the last SatNav fix, as a
    string of the form "2.943 ft".  In case no valid satnav fix exists, or in
    case where no altitude information is reported, the property is set to the
    value "-".

    \sa rawAltitudeInFeet
  */
  Q_PROPERTY(QString rawAltitudeInFeetAsString READ rawAltitudeInFeetAsString NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns rawAltitudeInFeetAsString
  */
  QString rawAltitudeInFeetAsString() const;

  /*! \brief Altitude

    This property holds the altitude, defined as
    rawAltitude()+altitudeCorrection, in feet above sea level.  Negative
    altitudes are possible.  In case no valid SatNav fix exists, or in case
    where no altitude information is reported, the property is set to the value
    "0".
  */
  Q_PROPERTY(int altitudeInFeet READ altitudeInFeet WRITE setAltitudeInFeet NOTIFY update)

  /*! \brief Set altimeter

    This function sets the altimeter. It computes

    altitudeCorrection := altitudeInFeed - rawAltitude

    and stores altitudeCorrection in QSettings, so it can be used when the
    program is run next. If the rawAltitude cannot be read (e.g. when there is
    not satellite reception), this method does nothing.

    @param altitudeInFeet Property altitudeInFeet
  */
  void setAltitudeInFeet(int altitudeInFeet);

  /*! \brief Getter function for the property with the same name

    @returns Property altitudeInFeet
   */
  int altitudeInFeet() const;

  /*! \brief Altitude reading from the last SatNav fix

    This property holds the altitude, defined as
    rawAltitude()+altitudeCorrection, as a string of the form "2.943 ft".  In
    case no valid SatNav fix exists, or in case where no altitude information is
    reported, the property is set to the value "-".

    \sa altitudeInFeet
  */
  Q_PROPERTY(QString altitudeInFeetAsString READ altitudeInFeetAsString NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns altitudeInFeetAsString
  */
  QString altitudeInFeetAsString() const;

  /*! \brief True if the last SatNav fix contains altitude information

    This property is set to "true" if the last SatNav fix contains altitude
    information.

    \sa altitudeInFeet
  */
  Q_PROPERTY(bool hasAltitude READ hasAltitude NOTIFY update)

  /*! \brief Geoidal separation

    This property holds the geoidal separation between the ellipsoidal height as
    reported by the GPS receiver and the orthometric height (AMSL, commonly used in maps).
    above sea level.
    In case no valid SatNav fix exists, or in case where no altitude information is reported,
    the property is set to the value "0".
  */
  Q_PROPERTY(int geoidalSeparation READ geoidalSeparation NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns geoidalSeparation
  */
  int geoidalSeparation() const;

  /*! \brief geoidal separation as string

    This property holds the geoidal separation, a string of the form "143 ft".
    In case no valid satnav fix exists, or in case where no geoidal separation
    information is reported, the property is set to the value "-".

    \sa geoidalSeparation
  */
  Q_PROPERTY(QString geoidalSeparationAsString READ geoidalSeparationAsString NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns geoidalSeparationAsString
  */
  QString geoidalSeparationAsString() const;

  /*! \brief Getter function for the property with the same name

    @returns Property hasAltitude
  */
  bool hasAltitude() const { return (lastInfo.coordinate().type() == QGeoCoordinate::Coordinate3D); }

  /*! \brief Coordinate reading from the last SatNav fix

    This property holds the coordinate found in the last SatNav fix.  In case no
    valid SatNav fix exists, the property is set an invalid coordinate.
  */
  Q_PROPERTY(QGeoCoordinate coordinate READ coordinate NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns Property coordinate
  */
  QGeoCoordinate coordinate() {return lastInfo.coordinate();}

  /*! \brief Ground speed reading from the last SatNav fix

    This property holds the ground speed found in the last SatNav fix, in knots.
    In case no valid SatNav fix exists, the property is set to the value "-1",
    otherwise the number returned is never negative
  */
  Q_PROPERTY(int groundSpeedInKnots READ groundSpeedInKnots NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns Property groundSpeedInKnots
  */
  int groundSpeedInKnots() const;

  /*! \brief Ground speed reading from the last SatNav fix

    This property holds the ground speed found in the last SatNav fix, in meters
    per second.  In case no valid SatNav fix exists, the property is set to the
    value "-1.0", otherwise the number returned is never negative
  */
  Q_PROPERTY(qreal groundSpeedInMetersPerSecond READ groundSpeedInMetersPerSecond NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns groundSpeedInMetersPerSecond
  */
  qreal groundSpeedInMetersPerSecond() const;

  /*! \brief Ground speed reading from the last SatNav fix in knots

    This property holds the ground speed found in the last SatNav fix, as a string
    of the form "97 kn".  In case no valid SatNav fix exists, the property is set
    to the value "-", otherwise the number returned is never negative
  */
  Q_PROPERTY(QString groundSpeedInKnotsAsString READ groundSpeedInKnotsAsString NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns Property groundSpeedInKnotsAsString
  */
  QString groundSpeedInKnotsAsString() const;

  /*! \brief Ground speed reading from the last SatNav fix in km/h

    This property holds the ground speed found in the last SatNav fix, as a string
    of the form "132 km/h".  In case no valid SatNav fix exists, the property is set
    to the value "-", otherwise the number returned is never negative
  */
  Q_PROPERTY(QString groundSpeedInKMHAsString READ groundSpeedInKMHAsString NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns Property groundSpeedInKMHAsString
  */
  QString groundSpeedInKMHAsString() const;

  /*! \brief Horizontal precision estimate for last SatNav fix

    This property holds an estimate for the precision of the horizontal position
    info, in meters.  If no estimate or no data exists, the property is set to
    "-1". Otherwise, the estimate is always non-negative.
  */
  Q_PROPERTY(int horizontalPrecisionInMeters READ horizontalPrecisionInMeters NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns Property horizontalPrecisionInMeters
  */
  int horizontalPrecisionInMeters() const;

  /*! \brief Horizontal precision estimate for last SatNav fix

    This property holds an estimate for the precision of the horizontal position
    info, as a string of the form "±5 m".  If no estimate or no data exists, the
    property is set to "-".
  */
  Q_PROPERTY(QString horizontalPrecisionInMetersAsString READ horizontalPrecisionInMetersAsString NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns horizontalPrecisionInMetersAsString
  */
  QString horizontalPrecisionInMetersAsString() const;

  /*! \brief Suggested icon

    This property suggests an icon, to be used for the own position.  This is
    always one of "/icons/self-noSatNav.svg", "/icons/self-noDirection.svg" and
    "/icons/self-withDirection.svg"
  */
  Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)

  /*! \brief Getter function for the property with the same name

    @returns Property icon
  */
  QString icon() const;

  /*! \brief Estimate whether the device is flying or on the ground

    This property holds an estimate, as to whether the device is flying or on
    the ground.  The current implementation considers the device is flying if
    the groundspeed can be read and is greater then 30 knots.
  */
  Q_PROPERTY(bool isInFlight READ isInFlight NOTIFY isInFlightChanged)

  /*! \brief Getter function for the property with the same name

    @returns Property isInFlight
  */
  bool isInFlight() const { return _isInFlight; }

  /*! \brief Last valid coordinate reading from the last SatNav fix

    This property holds the last valid coordinate found in a SatNav fix.  At
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

  /*! \brief Last valid track reading from the last SatNav fix

    This property holds the last valid track found in the last SatNav fix.  This
    property differs from track, in that it remains unchanged when no track
    information can be found or when an error occurs. At the first start, this
    property is set to 0.  The property always satisfies 0 <= track < 360. The
    value is stored in a QSetting at destruction, and restored in the
    construction.

    \sa lastValidTrack
  */
  Q_PROPERTY(int lastValidTrack READ lastValidTrack NOTIFY lastValidTrackChanged)

  /*! \brief Getter function for the property with the same name

    @returns Property lastValidTrack
  */
  int lastValidTrack() const { return _lastValidTrack; }

  /*! \brief Latitude in last SatNav fix

    This property holds the latitude of the last SatNav fix, as a string of the
    form "37° 52' 57.36'' N".  If no latitude is available, the property is set
    to "-".
  */
  Q_PROPERTY(QString latitudeAsString READ latitudeAsString NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns Property latitudeAsString
  */
  QString latitudeAsString() const;

  /*! \brief Longitude in last SatNav fix

    This property holds the longitude of the last SatNav fix, as a string of the
    form "122° 16' 1.93'' W".  If no longitude is available, the property is set
    to "-".
  */
  Q_PROPERTY(QString longitudeAsString READ longitudeAsString NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns longitudeAsString
  */
  QString longitudeAsString() const;

  /*! \brief Time and date of last SatNav fix

    This property holds the date and time at which the position was reported
    last, in UTC time. The QDateTime is invalid if no time and date has been
    set.
  */
  Q_PROPERTY(QDateTime timestamp READ timestamp NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns Property timestamp
  */
  QDateTime timestamp() const {return lastInfo.timestamp();}

  /*! \brief Time of last SatNav fix

    This property holds the time at which the position was reported last as a
    string of the form "13:42:32 UTC".  If no time has been set, the property is
    set to "-".
  */
  Q_PROPERTY(QString timestampAsString READ timestampAsString NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns Property timestampAsString
  */
  QString timestampAsString() const;

  /*! \brief Track found in last SatNav fix

    This property holds the track (= true course) found in the last SatNav fix, in
    degrees.  If no track is available, the property is set to "-1", otherwise
    the track is an integer satisfying 0 <= track < 360.
  */
  Q_PROPERTY(int track READ track NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns Property track
  */
  int track() const;

  /*! \brief Track found in last SatNav fix

    This property holds the track (= true course) found in the last SatNav fix, as
    a string of the form "127°".  If no track is available, the property is set
    to "-".
  */
  Q_PROPERTY(QString trackAsString READ trackAsString NOTIFY update)

  /*! \brief Getter function for the property with the same name

    @returns Property trackAsString
  */
  QString trackAsString() const;

  /*! \brief Status of the SatNav class

    This property holds the status of the SatNav class.
  */
  Q_PROPERTY(Status status READ status NOTIFY statusChanged)

  /*! \brief Getter function for the property with the same name

    @returns Property status
  */
  Status status() const;

  /*! \brief Status of the SatNav class

    This property holds a localized string that describes the status of the
    SatNav class.  A typical string is of the form "Connection to SatNav
    subsystem lost".
  */
  Q_PROPERTY(QString statusAsString READ statusAsString NOTIFY statusChanged)

  /*! \brief Getter function for the property with the same name

    @returns Property statusAsString
  */
  QString statusAsString() const;

  /*! \brief Last SatNav fix received

    @returns the last SatNav fix received.  If no SatNav has been received for
    more than one minute, this method return an empty QGeoPositionInfo.
  */
  QGeoPositionInfo lastFix() const { return lastInfo; }

signals:
  /*! \brief Emitted whenever the suggested icon changes */
  void iconChanged();

  /*! \brief Emitted whenever the suggested icon changes */
  void isInFlightChanged();

  /*! \brief Emitted whenever the SatNav status changes */
  void statusChanged();

  /*! \brief Emitted whenever the property lastValidTrack changes */
  void lastValidTrackChanged();

  /*! \brief Emitted whenever a new GS fix has arrived */
  void update();

private slots:
  // Connected to source, in order to receive new data
  void statusUpdate(const QGeoPositionInfo &info);

  // Connected to source, in order to receive error information
  void error(QGeoPositionInfoSource::Error newSourceStatus);

  // Connected to timeoutCounter, in order to receive timeout after one minute
  void timeout();

private:
  Q_DISABLE_COPY_MOVE(SatNav)

  // Aircraft is considered flying is speed is at least this high
  static constexpr double minFlightSpeedInKT = 30.0;
  // Hysteresis for flight speed
  static constexpr double flightSpeedHysteresis = 5.0;

  // Coordinates of EDTF airfield
  static constexpr double EDTF_lat = 48.022653;
  static constexpr double EDTF_lon = 7.832583;
  static constexpr double EDTF_ele = 244;

  QLocale myLocale;
  QGeoPositionInfoSource *source;
  QGeoPositionInfo lastInfo;
  QGeoCoordinate _lastValidCoordinate;
  int _lastValidTrack {0};
  bool _isInFlight {false};

  // altitude = raw altitude + altitudeCorrection
  int altitudeCorrectionInM {0};

  Geoid* _geoid;
  qreal _lastValidGeoidCorrection;

  // Constant: timeout occurs after one minute without receiving new data
  const int timeoutThreshold = 60*1000;

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

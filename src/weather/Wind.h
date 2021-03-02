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

#include <QSettings>
#include "AviationUnits.h"

namespace Weather {

/*! \brief This extremely simple class holds the wind speed and direction */

class Wind : public QObject
{
  Q_OBJECT

public:
  /*! \brief Default constructor

    This constructor reads the values of the properties listed below via
    QSettings. The values are set to NaN if no valid numbers can be found in the
    settings object.

    @param parent The standard QObject parent pointer
  */
  explicit Wind(QObject *parent = nullptr);

  // Standard destructor
  ~Wind() override = default;

  /*! \brief Wind Speed in knots

    This property holds the wind speed. This is a number that lies in the
    interval [minWindSpeed, maxWindSpeed] or NaN if the wind speed has not been
    set.
  */
  Q_PROPERTY(double windSpeedInKT READ windSpeedInKT WRITE setWindSpeedInKT NOTIFY valChanged)

  /*! \brief Getter function for property of the same name

    @returns Property windSpeedInKT
  */
  double windSpeedInKT() const;

  /*! \brief Setter function for property of the same name

    This method saves the new value in a QSetting object. If speedInKT is
    outside of the interval [minWindSpeed, maxWindSpeed], the property
    will be set to NaN.

    @param speedInKT Property windSpeedInKT
  */
  void setWindSpeedInKT(double speedInKT);

  /*! \brief Wind Speed in km/h

    This property holds the wind speed. This is a number that lies in the
    interval [minWindSpeed, maxWindSpeed] or NaN if the wind speed has not been
    set.
  */
  Q_PROPERTY(double windSpeedInKMH READ windSpeedInKMH WRITE setWindSpeedInKMH NOTIFY valChanged)

  /*! \brief Getter function for property of the same name

    @returns Property windSpeedInKMH
  */
  double windSpeedInKMH() const;

  /*! \brief Setter function for property of the same name

    This method saves the new value in a QSetting object. If speedInKMH is
    outside of the interval [minWindSpeed, maxWindSpeed], the property
    will be set to NaN.

    @param speedInKMH Property windSpeedInKMH
  */
  void setWindSpeedInKMH(double speedInKMH);

  /*! \brief Wind Direction

    This property holds the wind direction. This is a number that lies in the
    interval [minWindDirection, maxWindDirection] or NaN if no value has been
    set.
  */
  Q_PROPERTY(double windDirectionInDEG READ windDirectionInDEG WRITE setWindDirectionInDEG NOTIFY valChanged)

  /*! \brief Getter function for property of the same name

    @returns Property windDirectionInDEG
  */
  double windDirectionInDEG() const { return _windDirectionInDEG; }

  /*! \brief Setter function for property of the same name

    This method saves the new value in a QSetting object. If windDirection is
    outside of the interval [minWindDirection, maxWindDirection], the property
    will be set to NaN.

    @param windDirection Property windDirectionInDEG
  */
  void setWindDirectionInDEG(double windDirection);

  /*! \brief Minimal speed of the aircraft that is considered valid */
  Q_PROPERTY(double minWindSpeedInKT MEMBER minWindSpeedInKT CONSTANT)

  /*! \brief Maximal speed of the aircraft that is considered valid */
  Q_PROPERTY(double maxWindSpeedInKT MEMBER maxWindSpeedInKT CONSTANT)

  /*! \brief Minimal speed of the aircraft that is considered valid */
  Q_PROPERTY(double minWindSpeedInKMH MEMBER minWindSpeedInKMH CONSTANT)

  /*! \brief Maximal speed of the aircraft that is considered valid */
  Q_PROPERTY(double maxWindSpeedInKMH MEMBER maxWindSpeedInKMH CONSTANT)

  /*! \brief Minimal wind direction that is considered valid */
  Q_PROPERTY(double minWindDirection MEMBER minWindDirection CONSTANT)

  /*! \brief Maximal wind direction that is considered valid */
  Q_PROPERTY(double maxWindDirection MEMBER maxWindDirection CONSTANT)

signals:
  /*! \brief Notifier signal */
  void valChanged();

private:
  Q_DISABLE_COPY_MOVE(Wind)

  static constexpr double minWindSpeedInKT  = 0.0;
  static constexpr double minWindSpeedInKMH = minWindSpeedInKT * AviationUnits::Speed::KMH_per_KT;
  static constexpr double maxWindSpeedInKT  = 100.0;
  static constexpr double maxWindSpeedInKMH = maxWindSpeedInKT * AviationUnits::Speed::KMH_per_KT;
  static constexpr double minWindDirection  = 0.0;
  static constexpr double maxWindDirection  = 360.0;

  double _windSpeedInKT {qQNaN()};
  double _windDirectionInDEG {qQNaN()};

  QSettings settings;
};

}

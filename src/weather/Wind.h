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

#include "units/Angle.h"
#include "units/Speed.h"

namespace Weather {
  
  /*! \brief This extremely simple class holds the wind speed and direction */
  
  class Wind : public QObject
  {
    Q_OBJECT
    
  public:
    /*! \brief Default constructor
     *
     *  This constructor reads the values of the properties listed below via
     *  QSettings. The values are set to NaN if no valid numbers can be found in
     *  the settings object.
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit Wind(QObject *parent = nullptr);
    
    // Standard destructor
    ~Wind() override = default;
    
    /*! \brief Wind Speed
     *
     *  This property holds the wind speed. This is a number that lies in the
     *  interval [minWindSpeed, maxWindSpeed] or NaN if the wind speed has not
     *  been set.
     */
    Q_PROPERTY(Units::Speed windSpeed READ windSpeed WRITE setWindSpeed NOTIFY valChanged)
    
    /*! \brief Getter function for property of the same name
     *
     *  @returns Property windSpeed
     */
    Units::Speed windSpeed() const
    {
      return _windSpeed;
    }
    
    /*! \brief Setter function for property of the same name
     *
     *  This method saves the new value in a QSetting object. If newWindSpeed is
     *  outside of the interval [minWindSpeed, maxWindSpeed], the property will
     *  be set to NaN.
     *
     *  @param newWindSpeed Property windSpeed
     */
    void setWindSpeed(Units::Speed newWindSpeed);
    
    /*! \brief Wind Direction
     *
     *  This property holds the wind direction. This is NaN if no value has been
     *  set.
     */
    Q_PROPERTY(Units::Angle windDirection READ windDirection WRITE setWindDirection NOTIFY valChanged)

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property windDirection
     */
    Units::Angle windDirection() const
    {
      return _windDirection;
    }

    /*! \brief Setter function for property of the same name
     *
     *  This method saves the new value in a QSetting object.
     *
     *  @param newWindDirection Property windDirection
     */
    void setWindDirection(Units::Angle newWindDirection);
    
    /*! \brief Minimal wind speed that is considered valid */
    Q_PROPERTY(Units::Speed minWindSpeed MEMBER minWindSpeed CONSTANT)
    
    /*! \brief Maximal wind speed that is considered valid */
    Q_PROPERTY(Units::Speed maxWindSpeed MEMBER maxWindSpeed CONSTANT)
    
  signals:
    /*! \brief Notifier signal */
    void valChanged();
    
  private:
    Q_DISABLE_COPY_MOVE(Wind)
    
    static constexpr Units::Speed minWindSpeed = Units::Speed::fromKN(0.0);
    static constexpr Units::Speed maxWindSpeed = Units::Speed::fromKN(100.0);
    
    Units::Speed _windSpeed {};
    Units::Angle _windDirection {};
    
    QSettings settings;
  };
  
}

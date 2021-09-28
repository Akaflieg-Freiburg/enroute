/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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

#include <QPointer>
#include <QtGlobal>

#include "weather/Wind.h"


Weather::Wind::Wind(QObject *parent)
    : QObject(parent)
{
    _windSpeed = Units::Speed::fromKN(settings.value("Wind/windSpeedInKT", -1.0).toDouble());
    if ((_windSpeed < minWindSpeed) || (_windSpeed > maxWindSpeed)) {
        _windSpeed = Units::Speed();
    }

    _windDirection = Units::Angle::fromDEG(settings.value("Wind/windDirectionInDEG", 0.0).toDouble());
}


void Weather::Wind::setWindSpeed(Units::Speed newWindSpeed)
{

    if ((newWindSpeed < minWindSpeed) || (newWindSpeed > maxWindSpeed)) {
        newWindSpeed = Units::Speed();
    }

    if (newWindSpeed == _windSpeed) {
        return;
    }

    _windSpeed = newWindSpeed;
    settings.setValue("Wind/windSpeedInKT", _windSpeed.toKN());
    emit valChanged();
}


void Weather::Wind::setWindDirection(Units::Angle newWindDirection)
{

    if (newWindDirection == _windDirection) {
        return;
    }

    _windDirection = newWindDirection;
    settings.setValue("Wind/windDirectionInDEG", _windDirection.toDEG());
    emit valChanged();
}

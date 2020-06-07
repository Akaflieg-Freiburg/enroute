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

#include <QtGlobal>

#include "Wind.h"


Wind::Wind(QObject *parent)
    : QObject(parent)
{
    _windSpeedInKT = settings.value("Wind/windSpeedInKT", -1.0).toDouble();
    if ((_windSpeedInKT < minWindSpeedInKT) || (_windSpeedInKT > maxWindSpeedInKT))
        _windSpeedInKT = qQNaN();

    _windDirectionInDEG = settings.value("Wind/windDirectionInDEG", -1.0).toDouble();
    if ((_windDirectionInDEG < minWindDirection) || (_windDirectionInDEG > maxWindDirection))
        _windDirectionInDEG = qQNaN();
}


double Wind::windSpeedInKT() const
{
    return _windSpeedInKT;
}


void Wind::setWindSpeedInKT(double speedInKT)
{
    if ((speedInKT < minWindSpeedInKT) || (speedInKT > maxWindSpeedInKT))
        speedInKT = qQNaN();

    if (!qFuzzyCompare(speedInKT, _windSpeedInKT)) {
        _windSpeedInKT = speedInKT;
        settings.setValue("Wind/windSpeedInKT", _windSpeedInKT);
        emit valChanged();
    }
}


double Wind::windSpeedInKMH() const
{
    auto speed = AviationUnits::Speed::fromKT(_windSpeedInKT);
    return speed.toKMH();
}


void Wind::setWindSpeedInKMH(double speedInKMH)
{
    setWindSpeedInKT(
        AviationUnits::Speed::fromKMH(speedInKMH).toKT()
    );
}


void Wind::setWindDirectionInDEG(double windDirection)
{
    if ((windDirection < minWindDirection) || (windDirection > maxWindDirection))
        windDirection = qQNaN();

    if (!qFuzzyCompare(windDirection, _windDirectionInDEG)) {
        _windDirectionInDEG = windDirection;
        settings.setValue("Wind/windDirectionInDEG", _windDirectionInDEG);
        emit valChanged();
    }
}

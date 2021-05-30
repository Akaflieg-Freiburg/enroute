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

#include "Aircraft.h"

#include <QPointer>
#include <QtGlobal>

// Static instance of this class. Do not analyze, because of many unwanted warnings.
#ifndef __clang_analyzer__
QPointer<Aircraft> aircraftStatic {};
#endif


Aircraft::Aircraft(QObject *parent) : QObject(parent) {
    _cruiseSpeedInKT = settings.value("Aircraft/cruiseSpeedInKTS", 0.0).toDouble();
    if ((_cruiseSpeedInKT < minAircraftSpeedInKT) || (_cruiseSpeedInKT > maxAircraftSpeedInKT)) {
        _cruiseSpeedInKT = qQNaN();
    }

    _descentSpeedInKT = settings.value("Aircraft/descentSpeedInKTS", 0.0).toDouble();
    if ((_descentSpeedInKT < minAircraftSpeedInKT) || (_descentSpeedInKT > maxAircraftSpeedInKT)) {
        _descentSpeedInKT = qQNaN();
    }

    _fuelConsumptionInLPH = settings.value("Aircraft/fuelConsumptionInLPH", 0.0).toDouble();
    if ((_fuelConsumptionInLPH < minFuelConsuption) || (_fuelConsumptionInLPH > maxFuelConsuption)) {
        _fuelConsumptionInLPH = qQNaN();
    }
}


auto Aircraft::globalInstance() -> Aircraft*
{
#ifndef __clang_analyzer__
    if (aircraftStatic.isNull()) {
        aircraftStatic = new Aircraft();
    }
    return aircraftStatic;
#else
    return nullptr;
#endif
}


auto Aircraft::cruiseSpeedInKT() const -> double {
    return _cruiseSpeedInKT;
}

void Aircraft::setCruiseSpeedInKT(double speedInKT) {
    if ((speedInKT < minAircraftSpeedInKT) || (speedInKT > maxAircraftSpeedInKT)) {
        speedInKT = qQNaN();
    }

    if (!qFuzzyCompare(speedInKT, _cruiseSpeedInKT)) {
        _cruiseSpeedInKT = speedInKT;
        settings.setValue("Aircraft/cruiseSpeedInKTS", _cruiseSpeedInKT);
        emit valChanged();
    }
}

auto Aircraft::cruiseSpeedInKMH() const -> double {
    auto speed = AviationUnits::Speed::fromKN(_cruiseSpeedInKT);
    return speed.toKMH();
}

void Aircraft::setCruiseSpeedInKMH(double speedInKMH) {
    auto speed = AviationUnits::Speed::fromKMH(speedInKMH);
    setCruiseSpeedInKT(speed.toKN());
}

auto Aircraft::descentSpeedInKT() const -> double {
    return _descentSpeedInKT;
}

void Aircraft::setDescentSpeedInKT(double speedInKT) {
    if ((speedInKT < minAircraftSpeedInKT) || (speedInKT > maxAircraftSpeedInKT)) {
        speedInKT = qQNaN();
    }

    if (!qFuzzyCompare(speedInKT, _descentSpeedInKT)) {
        _descentSpeedInKT = speedInKT;
        settings.setValue("Aircraft/descentSpeedInKTS", _descentSpeedInKT);
        emit valChanged();
    }
}

auto Aircraft::descentSpeedInKMH() const -> double {
    auto speed = AviationUnits::Speed::fromKN(_descentSpeedInKT);
    return speed.toKMH();
}

void Aircraft::setDescentSpeedInKMH(double speedInKMH) {
    auto speed = AviationUnits::Speed::fromKMH(speedInKMH);
    setDescentSpeedInKT(speed.toKN());
}

void Aircraft::setFuelConsumptionInLPH(double fuelConsumptionInLPH) {
    if ((fuelConsumptionInLPH < minFuelConsuption) || (fuelConsumptionInLPH > maxFuelConsuption)) {
        fuelConsumptionInLPH = qQNaN();
    }

    if (!qFuzzyCompare(fuelConsumptionInLPH, _fuelConsumptionInLPH)) {
        _fuelConsumptionInLPH = fuelConsumptionInLPH;
        settings.setValue("Aircraft/fuelConsumptionInLPH", _fuelConsumptionInLPH);
        emit valChanged();
    }
}

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


Navigation::Aircraft::Aircraft(QObject *parent) : QObject(parent) {
    _cruiseSpeed = Units::Speed::fromKN(settings.value("Aircraft/cruiseSpeedInKTS", 0.0).toDouble());
    if ((_cruiseSpeed < minAircraftSpeed) || (_cruiseSpeed > maxAircraftSpeed)) {
        _cruiseSpeed = Units::Speed();
    }

    _descentSpeed = Units::Speed::fromKN(settings.value("Aircraft/descentSpeedInKTS", 0.0).toDouble());
    if ((_descentSpeed < minAircraftSpeed) || (_descentSpeed > maxAircraftSpeed)) {
        _descentSpeed = Units::Speed();
    }

    _fuelConsumptionInLPH = settings.value("Aircraft/fuelConsumptionInLPH", 0.0).toDouble();
    if ((_fuelConsumptionInLPH < minFuelConsuption) || (_fuelConsumptionInLPH > maxFuelConsuption)) {
        _fuelConsumptionInLPH = qQNaN();
    }
}


void Navigation::Aircraft::setCruiseSpeed(Units::Speed newSpeed) {

    if ((newSpeed < minAircraftSpeed) || (newSpeed > maxAircraftSpeed)) {
        newSpeed = Units::Speed();
    }
    if (newSpeed == _cruiseSpeed) {
        return;
    }

    _cruiseSpeed = newSpeed;
    settings.setValue("Aircraft/cruiseSpeedInKTS", _cruiseSpeed.toKN());
    emit valChanged();

}


void Navigation::Aircraft::setDescentSpeed(Units::Speed newSpeed) {

    if ((newSpeed < minAircraftSpeed) || (newSpeed > maxAircraftSpeed)) {
        newSpeed = Units::Speed();
    }
    if (newSpeed == _descentSpeed) {
        return;
    }

    _descentSpeed = newSpeed;
    settings.setValue("Aircraft/descentSpeedInKTS", _descentSpeed.toKN());
    emit valChanged();
}


void Navigation::Aircraft::setMinimumSpeed(Units::Speed newSpeed) {

    if ((newSpeed < minAircraftSpeed) || (newSpeed > maxAircraftSpeed)) {
        newSpeed = Units::Speed();
    }
    if (newSpeed == _minimumSpeed) {
        return;
    }

    _minimumSpeed = newSpeed;
    settings.setValue("Aircraft/minimumSpeedInKTS", _minimumSpeed.toKN());
    emit minimumSpeedChanged();
}


void Navigation::Aircraft::setFuelConsumptionInLPH(double fuelConsumptionInLPH) {
    if ((fuelConsumptionInLPH < minFuelConsuption) || (fuelConsumptionInLPH > maxFuelConsuption)) {
        fuelConsumptionInLPH = qQNaN();
    }

    if (!qFuzzyCompare(fuelConsumptionInLPH, _fuelConsumptionInLPH)) {
        _fuelConsumptionInLPH = fuelConsumptionInLPH;
        settings.setValue("Aircraft/fuelConsumptionInLPH", _fuelConsumptionInLPH);
        emit valChanged();
    }
}

/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include "Aircraft.h"
#include "GlobalObject.h"
#include "Librarian.h"


Navigation::Aircraft::Aircraft(QObject *parent) : QObject(parent) {
}


void Navigation::Aircraft::setCruiseSpeed(Units::Speed newSpeed) {

    if ((newSpeed < minValidSpeed) || (newSpeed > maxValidSpeed)) {
        newSpeed = Units::Speed();
    }
    if (newSpeed == _cruiseSpeed) {
        return;
    }

    _cruiseSpeed = newSpeed;
    emit cruiseSpeedChanged();

}


void Navigation::Aircraft::setDescentSpeed(Units::Speed newSpeed) {

    if ((newSpeed < minValidSpeed) || (newSpeed > maxValidSpeed)) {
        newSpeed = Units::Speed();
    }
    if (newSpeed == _descentSpeed) {
        return;
    }

    _descentSpeed = newSpeed;
    emit descentSpeedChanged();
}


void Navigation::Aircraft::setFuelConsumption(Units::VolumeFlow newFuelConsumption) {
    if ((newFuelConsumption < minValidFuelConsumption) || (newFuelConsumption > maxValidFuelConsumption)) {
        newFuelConsumption = Units::VolumeFlow();
    }

    if (!qFuzzyCompare(newFuelConsumption.toLPH(), _fuelConsumption.toLPH())) {
        _fuelConsumption = newFuelConsumption;
        emit fuelConsumptionChanged();
    }
}


void Navigation::Aircraft::setFuelConsumptionUnit(FuelConsumptionUnit newUnit)
{
    if (newUnit == _fuelConsumptionUnit) {
        return;
    }

    _fuelConsumptionUnit = newUnit;
    emit fuelConsumptionUnitChanged();
}


void Navigation::Aircraft::setHorizontalDistanceUnit(HorizontalDistanceUnit newUnit)
{
    if (newUnit == _horizontalDistanceUnit) {
        return;
    }

    _horizontalDistanceUnit = newUnit;
    emit horizontalDistanceUnitChanged();
}


void Navigation::Aircraft::setMinimumSpeed(Units::Speed newSpeed) {

    if ((newSpeed < minValidSpeed) || (newSpeed > maxValidSpeed)) {
        newSpeed = Units::Speed();
    }
    if (newSpeed == _minimumSpeed) {
        return;
    }

    _minimumSpeed = newSpeed;
    emit minimumSpeedChanged();
}


void Navigation::Aircraft::setName(const QString& newName) {

    if (newName == _name) {
        return;
    }

    _name = newName;
    emit nameChanged();
}


void Navigation::Aircraft::setVerticalDistanceUnit(VerticalDistanceUnit newUnit)
{
    if (newUnit == _verticalDistanceUnit) {
        return;
    }

    _verticalDistanceUnit = newUnit;
    emit verticalDistanceUnitChanged();
}



//
// Methods
//

auto Navigation::Aircraft::loadFromJSON(const QString& fileName) -> QString
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return tr("Unable to open the file '%1' for reading.").arg(fileName);
    }
    return loadFromJSON(file.readAll());
}


auto Navigation::Aircraft::loadFromJSON(const QByteArray &JSON) -> QString
{
    QJsonParseError parseError{};
    auto document = QJsonDocument::fromJson(JSON, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return parseError.errorString();
    }

    auto content = document.object();
    if (content.isEmpty()) {
        return tr("JSON document contains no data.");
    }

    if (content["content"] != "aircraft") {
        return tr("JSON document does not describe an aircraft.");
    }

    setCruiseSpeed( Units::Speed::fromMPS( content["cruiseSpeed_mps"].toDouble(NAN) ));
    setDescentSpeed( Units::Speed::fromMPS( content["descentSpeed_mps"].toDouble(NAN) ));
    setFuelConsumption( Units::VolumeFlow::fromLPH( content["fuelConsumption_lph"].toDouble(NAN) ));
    setFuelConsumptionUnit( static_cast<FuelConsumptionUnit>(content["fuelConsumptionUnit"].toInt(LiterPerHour)) );
    setHorizontalDistanceUnit( static_cast<HorizontalDistanceUnit>(content["horizontalDistanceUnit"].toInt(NauticalMile)) );
    setMinimumSpeed( Units::Speed::fromMPS( content["minimumSpeed_mps"].toDouble(NAN) ));
    setName( content["name"].toString() );
    setVerticalDistanceUnit( static_cast<VerticalDistanceUnit>(content["verticalDistanceUnit"].toInt(Feet)) );

    return {};
}


auto Navigation::Aircraft::save(const QString& fileName) const -> QString
{
    // Make directory, if it does not yet exist.
    QDir dir;
    dir.mkpath(GlobalObject::librarian()->directory(Librarian::Aircraft));

    QFile file(fileName);
    auto success = file.open(QIODevice::WriteOnly);
    if (!success) {
        return tr("Unable to open the file '%1' for writing.").arg(fileName);
    }
    auto numBytesWritten = file.write(toJSON());
    if (numBytesWritten == -1) {
        file.close();
        QFile::remove(fileName);
        return tr("Unable to write to file '%1'.").arg(fileName);
    }
    return {};
}


auto Navigation::Aircraft::toJSON() const -> QByteArray
{
    QJsonObject jsonObj;
    jsonObj.insert(QStringLiteral("content"), "aircraft");

    jsonObj.insert(QStringLiteral("cruiseSpeed_mps"), _cruiseSpeed.toMPS());
    jsonObj.insert(QStringLiteral("descentSpeed_mps"), _descentSpeed.toMPS());
    jsonObj.insert(QStringLiteral("fuelConsumption_lph"), _fuelConsumption.toLPH());
    jsonObj.insert(QStringLiteral("fuelConsumptionUnit"), _fuelConsumptionUnit);
    jsonObj.insert(QStringLiteral("horizontalDistanceUnit"), _horizontalDistanceUnit);
    jsonObj.insert(QStringLiteral("minimumSpeed_mps"), _minimumSpeed.toMPS());
    jsonObj.insert(QStringLiteral("name"), _name);
    jsonObj.insert(QStringLiteral("verticalDistanceUnit"), _verticalDistanceUnit);


    QJsonDocument doc;
    doc.setObject(jsonObj);
    return doc.toJson();
}


auto Navigation::Aircraft::verticalDistanceToString(Units::Distance distance, bool forceSign) const -> QString
{
    if (!distance.isFinite()) {
        return "-";
    }
    QString signString;
    if (forceSign && (distance.toM() >= 0.0)) {
        signString = "+";
    }
    switch(_verticalDistanceUnit) {
    case Navigation::Aircraft::Feet:
        return signString+QString("%L1 ft").arg(qRound(distance.toFeet()));
    case Navigation::Aircraft::Meters:
        return signString+QString("%L1 m").arg(qRound(distance.toM()));
    }
    return {};
}

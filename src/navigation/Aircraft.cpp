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
#include <QJsonObject>
#include <QJsonParseError>

#include "Aircraft.h"
#include "GlobalObject.h"
#include "Librarian.h"
#include "geomaps/Waypoint.h"


//
// Setter Methods
//

void Navigation::Aircraft::setCruiseSpeed(Units::Speed newSpeed)
{
    if ((newSpeed < minValidSpeed) || (newSpeed > maxValidSpeed)) {
        newSpeed = Units::Speed();
    }
    m_cruiseSpeed = newSpeed;
}


void Navigation::Aircraft::setDescentSpeed(Units::Speed newSpeed)
{
    if ((newSpeed < minValidSpeed) || (newSpeed > maxValidSpeed)) {
        newSpeed = Units::Speed();
    }
    m_descentSpeed = newSpeed;
}


void Navigation::Aircraft::setFuelConsumption(Units::VolumeFlow newFuelConsumption) {
    if ((newFuelConsumption < minValidFuelConsumption) || (newFuelConsumption > maxValidFuelConsumption)) {
        newFuelConsumption = Units::VolumeFlow();
    }

    if (!qFuzzyCompare(newFuelConsumption.toLPH(), m_fuelConsumption.toLPH())) {
        m_fuelConsumption = newFuelConsumption;
    }
}


void Navigation::Aircraft::setFuelConsumptionUnit(FuelConsumptionUnit newUnit)
{
    m_fuelConsumptionUnit = newUnit;
}


void Navigation::Aircraft::setHorizontalDistanceUnit(HorizontalDistanceUnit newUnit)
{
    m_horizontalDistanceUnit = newUnit;
}


void Navigation::Aircraft::setMinimumSpeed(Units::Speed newSpeed)
{
    if ((newSpeed < minValidSpeed) || (newSpeed > maxValidSpeed)) {
        newSpeed = Units::Speed();
    }
    m_minimumSpeed = newSpeed;
}


void Navigation::Aircraft::setName(const QString& newName)
{
    m_name = newName;
}


void Navigation::Aircraft::setVerticalDistanceUnit(VerticalDistanceUnit newUnit)
{
    m_verticalDistanceUnit = newUnit;
}


//
// Methods
//

auto Navigation::Aircraft::describeWay(const QGeoCoordinate &from, const QGeoCoordinate &to) const -> QString
{
    // Paranoid safety checks
    if (!from.isValid()) {
        return {};
    }
    if (!to.isValid()) {
        return {};
    }

    QStringList result;

    auto dist = Units::Distance::fromM(from.distanceTo(to));
    result += QStringLiteral("DIST %1").arg(horizontalDistanceToString(dist));

    auto QUJ = qRound(from.azimuthTo(to));
    result += QStringLiteral("QUJ %1°").arg(QUJ);

    if ((from.type() == QGeoCoordinate::Coordinate3D) && (to.type() == QGeoCoordinate::Coordinate3D))
    {
        auto hDistM = from.distanceTo(to);
        auto vDistM = from.altitude() - to.altitude() - 300;

        if (hDistM > 1000.0)
        {
            if (vDistM < 1)
            {
                result += QStringLiteral("E ∞");
            }
            else
            {
                auto E = hDistM/vDistM;
                if (E > 100)
                {
                    result += QStringLiteral("E ∞");
                }
                else
                {
                    result += QStringLiteral("E %1").arg(qRound(E));
                }
            }
        }

    }

    return result.join(" • ");
}


auto Navigation::Aircraft::horizontalDistanceToString(Units::Distance distance) const -> QString
{
    if (!distance.isFinite()) {
        return QStringLiteral("-");
    }

    switch(m_horizontalDistanceUnit) {
    case Navigation::Aircraft::NauticalMile:
        if (distance > Units::Distance::fromNM(10.0)) {
            return QStringLiteral("%L1 nm").arg(distance.toNM(), 0, 'f', 0);
        }
        return QStringLiteral("%L1 nm").arg(distance.toNM(), 0, 'f', 1);
    case Navigation::Aircraft::Kilometer:
        if (distance > Units::Distance::fromKM(10.0)) {
            return QStringLiteral("%L1 km").arg(distance.toKM(), 0, 'f', 0);
        }
        return QStringLiteral("%L1 km").arg(distance.toKM(), 0, 'f', 1);
    case Navigation::Aircraft::StatuteMile:
        if (distance > Units::Distance::fromMIL(10.0)) {
            return QStringLiteral("%L1 mil").arg(distance.toMIL(), 0, 'f', 0);
        }
        return QStringLiteral("%L1 mil").arg(distance.toMIL(), 0, 'f', 1);
    }
    return {};
}


auto Navigation::Aircraft::horizontalSpeedToString(Units::Speed speed) const -> QString
{
    if (!speed.isFinite()) {
        return QStringLiteral("-");
    }

    switch(m_horizontalDistanceUnit) {
    case Navigation::Aircraft::NauticalMile:
        return QStringLiteral("%L1 kn").arg(qRound( speed.toKN() ));
    case Navigation::Aircraft::Kilometer:
        return QStringLiteral("%L1 km/h").arg(qRound( speed.toKMH() ));
    case Navigation::Aircraft::StatuteMile:
        return QStringLiteral("%L1 mph").arg(qRound( speed.toMPH() ));
    }
    return {};
}


auto Navigation::Aircraft::loadFromJSON(const QString& fileName) -> QString
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return QObject::tr("Unable to open the file '%1' for reading.").arg(fileName);
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
        return QObject::tr("JSON document contains no data.");
    }

    if (content[QStringLiteral("content")] != "aircraft") {
        return QObject::tr("JSON document does not describe an aircraft.");
    }

    setCruiseSpeed( Units::Speed::fromMPS( content[QStringLiteral("cruiseSpeed_mps")].toDouble(NAN) ));
    setDescentSpeed( Units::Speed::fromMPS( content[QStringLiteral("descentSpeed_mps")].toDouble(NAN) ));
    setFuelConsumption( Units::VolumeFlow::fromLPH( content[QStringLiteral("fuelConsumption_lph")].toDouble(NAN) ));
    setFuelConsumptionUnit( static_cast<FuelConsumptionUnit>(content[QStringLiteral("fuelConsumptionUnit")].toInt(LiterPerHour)) );
    setHorizontalDistanceUnit( static_cast<HorizontalDistanceUnit>(content[QStringLiteral("horizontalDistanceUnit")].toInt(NauticalMile)) );
    setMinimumSpeed( Units::Speed::fromMPS( content[QStringLiteral("minimumSpeed_mps")].toDouble(NAN) ));
    setName( content[QStringLiteral("name")].toString() );
    setVerticalDistanceUnit( static_cast<VerticalDistanceUnit>(content[QStringLiteral("verticalDistanceUnit")].toInt(Feet)) );

    return {};
}


auto Navigation::Aircraft::operator==(const Navigation::Aircraft& other) const -> bool
{
    return (m_cruiseSpeed == other.m_cruiseSpeed) &&
            (m_descentSpeed == other.m_descentSpeed) &&
            (m_fuelConsumption == other.m_fuelConsumption) &&
            (m_fuelConsumptionUnit == other.m_fuelConsumptionUnit) &&
            (m_horizontalDistanceUnit == other.m_horizontalDistanceUnit) &&
            (m_minimumSpeed == other.m_minimumSpeed) &&
            (m_name == other.m_name) &&
            (m_verticalDistanceUnit == other.m_verticalDistanceUnit);
}


auto Navigation::Aircraft::save(const QString& fileName) const -> QString
{
    // Make directory, if it does not yet exist.
    QDir const dir;
    dir.mkpath(Librarian::directory(Librarian::Aircraft));

    QFile file(fileName);
    auto success = file.open(QIODevice::WriteOnly);
    if (!success) {
        return QObject::tr("Unable to open the file '%1' for writing.").arg(fileName);
    }
    auto numBytesWritten = file.write(toJSON());
    if (numBytesWritten == -1) {
        file.close();
        QFile::remove(fileName);
        return QObject::tr("Unable to write to file '%1'.").arg(fileName);
    }
    return {};
}


auto Navigation::Aircraft::toJSON() const -> QByteArray
{
    QJsonObject jsonObj;
    jsonObj.insert(QStringLiteral("content"), "aircraft");

    jsonObj.insert(QStringLiteral("cruiseSpeed_mps"), m_cruiseSpeed.toMPS());
    jsonObj.insert(QStringLiteral("descentSpeed_mps"), m_descentSpeed.toMPS());
    jsonObj.insert(QStringLiteral("fuelConsumption_lph"), m_fuelConsumption.toLPH());
    jsonObj.insert(QStringLiteral("fuelConsumptionUnit"), m_fuelConsumptionUnit);
    jsonObj.insert(QStringLiteral("horizontalDistanceUnit"), m_horizontalDistanceUnit);
    jsonObj.insert(QStringLiteral("minimumSpeed_mps"), m_minimumSpeed.toMPS());
    jsonObj.insert(QStringLiteral("name"), m_name);
    jsonObj.insert(QStringLiteral("verticalDistanceUnit"), m_verticalDistanceUnit);


    QJsonDocument doc;
    doc.setObject(jsonObj);
    return doc.toJson();
}


auto Navigation::Aircraft::verticalDistanceToString(Units::Distance distance, bool forceSign) const -> QString
{
    if (!distance.isFinite()) {
        return QStringLiteral("-");
    }
    QString signString;
    if (forceSign && (distance.toM() >= 0.0)) {
        signString = QStringLiteral("+");
    }
    switch(m_verticalDistanceUnit) {
    case Navigation::Aircraft::Feet:
        return signString+QStringLiteral("%L1 ft").arg(qRound(distance.toFeet()));
    case Navigation::Aircraft::Meters:
        return signString+QStringLiteral("%L1 m").arg(qRound(distance.toM()));
    }
    return {};
}


auto Navigation::Aircraft::verticalSpeedToString(Units::Speed speed) const -> QString
{
    if (!speed.isFinite()) {
        return QStringLiteral("-");
    }
    switch(m_verticalDistanceUnit) {
    case Navigation::Aircraft::Feet:
        return QStringLiteral("%L1 ft/min").arg(qRound( speed.toFPM() ));
    case Navigation::Aircraft::Meters:
        return QStringLiteral("%L1 m/s").arg(qRound( speed.toMPS() ));
    }
    return {};
}


auto Navigation::Aircraft::volumeToString(Units::Volume volume) const -> QString
{
    if (!volume.isFinite()) {
        return QStringLiteral("-");
    }
    switch(m_fuelConsumptionUnit) {
    case Navigation::Aircraft::LiterPerHour:
        if (volume.toL() < 10.0) {
            return QStringLiteral("%L1 l").arg(volume.toL(), 0, 'f', 1);
        }
        return QStringLiteral("%L1 l").arg(qRound( volume.toL() ));
    case Navigation::Aircraft::GallonPerHour:
        if (volume.toGAL() < 10.0) {
            return QStringLiteral("%L1 gal").arg(volume.toGAL(), 0, 'f', 1);
        }
        return QStringLiteral("%L1 gal").arg(qRound( volume.toGAL() ));
    }
    return {};
}

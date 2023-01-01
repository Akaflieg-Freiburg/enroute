/***************************************************************************
 *   Copyright (C) 2020-2021 by Stefan Kebekus                             *
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

#include <QCoreApplication>

#include "GlobalObject.h"
#include "GlobalSettings.h"
#include "navigation/Navigator.h"
#include "traffic/Warning.h"


Traffic::Warning::Warning(
        const QString& AlarmLevel,
        const QString& RelativeBearing,
        const QString& AlarmType,
        const QString& RelativeVertical,
        const QString& RelativeDistance)
{  

    // Alarm level
    if (AlarmLevel == QLatin1String("0")) {
        m_alarmLevel = 0;
    }
    if (AlarmLevel == QLatin1String("1")) {
        m_alarmLevel = 1;
    }
    if (AlarmLevel == QLatin1String("2")) {
        m_alarmLevel = 2;
    }
    if (AlarmLevel == QLatin1String("3")) {
        m_alarmLevel = 3;
    }

    // Alarm Type
    if (AlarmType == QLatin1String("2")) {
        m_alarmType = 2;
    }
    if (AlarmType == QLatin1String("3")) {
        m_alarmType = 3;
    }
    if (AlarmType == QLatin1String("4")) {
        m_alarmType = 4;
    }

    // hDist
    bool ok = false;
    auto hDistM = RelativeDistance.toDouble(&ok);
    if (ok) {
        m_hDist = Units::Distance::fromM(hDistM);
    } else {
        m_hDist = Units::Distance::fromM(qQNaN());
    }

    // Relative Bearing
    auto rb = RelativeBearing.toDouble(&ok);
    if (ok) {
        m_relativeBearing = Units::Angle::fromDEG(rb);
    } else {
        m_relativeBearing = Units::Angle::fromRAD(qQNaN());
    }

    // vDist
    auto vDistM = RelativeVertical.toDouble(&ok);
    if (ok) {
        m_vDist = Units::Distance::fromM(vDistM);
    } else {
        m_vDist = Units::Distance::fromM(qQNaN());
    }

}


auto Traffic::Warning::description() const -> QString
{
    QStringList result;

    // Alarm type
    if (m_alarmType == 2) {
        result << QCoreApplication::translate("Traffic::Warning", "Traffic");
    }
    if (m_alarmType == 3) {
        result << QCoreApplication::translate("Traffic::Warning", "Obstacle");
    }
    if (m_alarmType == 4) {
        result << QCoreApplication::translate("Traffic::Warning", "Traffic advisory");
    }


    // Relative bearing
    if (m_relativeBearing.isFinite()) {
        result << QCoreApplication::translate("Traffic::Warning", "%1 position").arg(m_relativeBearing.toClock());
    }


    // Horizontal distance
    if (m_hDist.isFinite() && !m_hDist.isNegative()) {

        switch(GlobalObject::navigator()->aircraft().horizontalDistanceUnit()) {
        case Navigation::Aircraft::Kilometer:
            result << QCoreApplication::translate("Traffic::Warning", "Distance %1 km").arg(qRound(m_hDist.toKM()*10.0)/10.0);
            break;
        case Navigation::Aircraft::NauticalMile:
            result << QCoreApplication::translate("Traffic::Warning", "Distance %1 nm").arg(qRound(m_hDist.toNM()*10.0)/10.0);
            break;
        case Navigation::Aircraft::StatuteMile:
            result << QCoreApplication::translate("Traffic::Warning", "Distance %1 mil").arg(qRound(m_hDist.toMIL()*10.0)/10.0);
            break;
        }

    }

    // Vertical distance
    if (m_vDist.isFinite()) {
        QString vDistString = GlobalObject::navigator()->aircraft().verticalDistanceToString(m_vDist, true);

        if (qAbs(m_vDist.toFeet()) < 100) {
            result << QCoreApplication::translate("Traffic::Warning", "Same altitude");
        } else {
            if (m_vDist.isNegative()) {
                result << QCoreApplication::translate("Traffic::Warning", "%1 below").arg(vDistString);
            } else {
                result << QCoreApplication::translate("Traffic::Warning", "%1 above").arg(vDistString);
            }
        }
    }

    return result.join(QStringLiteral(" • "));
}


auto Traffic::Warning::operator==(const Traffic::Warning &rhs) -> bool
{
    if (m_alarmLevel != rhs.m_alarmLevel) {
        return false;
    }
    if (m_alarmType!= rhs.m_alarmType) {
        return false;
    }
    if (m_hDist != rhs.m_hDist) {
        return false;
    }
    if (m_relativeBearing != rhs.m_relativeBearing) {
        return false;
    }
    if (m_vDist != rhs.m_vDist) {
        return false;
    }
    return true;
}


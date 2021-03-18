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


#include "GlobalSettings.h"
#include "traffic/FLARMWarning.h"


Traffic::FLARMWarning::FLARMWarning(
        const QString&  /*RX*/,
        const QString&  /*TX*/,
        const QString&  /*GPS*/,
        const QString&  /*Power*/,
        const QString& AlarmLevel,
        const QString& RelativeBearing,
        const QString& AlarmType,
        const QString& RelativeVertical,
        const QString& RelativeDistance,
        QObject *parent) : QObject(parent)
{  

    // Alarm level
    if (AlarmLevel == "0") {
        _alarmLevel = 0;
    }
    if (AlarmLevel == "1") {
        _alarmLevel = 1;
    }
    if (AlarmLevel == "2") {
        _alarmLevel = 2;
    }
    if (AlarmLevel == "3") {
        _alarmLevel = 3;
    }

    // Relative Bearing
    bool ok = false;
    _relativeBearing = RelativeBearing.toDouble(&ok);
    if (!ok || (_relativeBearing < -180.0)  || (_relativeBearing > 180.0)) {
        _relativeBearing = qQNaN();
    }

    // Alarm Type
    if (AlarmType == "2") {
        _alarmType = 2;
    }
    if (AlarmType == "3") {
        _alarmType = 3;
    }
    if (AlarmType == "4") {
        _alarmType = 4;
    }

    // vDist
    auto vDistM = RelativeVertical.toDouble(&ok);
    if (ok) {
        _vDist = AviationUnits::Distance::fromM(vDistM);
    } else {
        _vDist = AviationUnits::Distance::fromM(qQNaN());
    }

    // hDist
    auto hDistM = RelativeDistance.toDouble(&ok);
    if (ok) {
        _hDist = AviationUnits::Distance::fromM(hDistM);
    } else {
        _hDist = AviationUnits::Distance::fromM(qQNaN());
    }

}


auto Traffic::FLARMWarning::relativeBearingClock() const -> QString
{
    if (!qIsFinite(_relativeBearing) || (_relativeBearing < -180.0) || (_relativeBearing > 180.0)) {
        return {};
    }

    auto clockTime = qRound(_relativeBearing/30.0);
    if (clockTime < 0) {
        clockTime += 12;
    }

    return tr("%1 o'clock").arg(clockTime);
}


auto Traffic::FLARMWarning::vDistString() const -> QString
{
    if (!_vDist.isFinite()) {
        return {};
    }

#warning not implemented
    return QString("%1 m").arg(_vDist.toM());
}


auto Traffic::FLARMWarning::hDistString() const -> QString
{
    if (!_hDist.isFinite()) {
        return {};
    }

#warning not implemented
    return QString("%1 m").arg(_hDist.toM());
}

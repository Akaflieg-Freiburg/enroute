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

#pragma once

#include <QGeoCoordinate>
#include <QGeoPositionInfo>
#include <QTimer>
#include <QObject>

#include "AviationUnits.h"
#include "GlobalSettings.h"


namespace Navigation {

class TrafficNoPos : public QObject {
    Q_OBJECT

public:
    /*! \brief TrafficNoPos type */
    enum Type
    {
        unknown,
        Aircraft,
        Airship,
        Balloon,
        Copter, // helicopter/gyrocopter/rotorcraft
        Drone,
        Glider,
        HangGlider,
        Jet,
        Paraglider,
        Skydiver,
        StaticObstacle,
        TowPlane
    };
    Q_ENUM(Type)

    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit TrafficNoPos(QObject *parent = nullptr);

    // Standard destructor
    ~TrafficNoPos() override = default;

    void setData(int __alarmLevel, QString __ID,  AviationUnits::Distance __hdist, AviationUnits::Distance __vdist, Type __type);


    void copyFrom(const TrafficNoPos & other);

    Q_PROPERTY(int alarmLevel READ alarmLevel NOTIFY alarmLevelChanged)

    int alarmLevel() const
    {
        return _alarmLevel;
    }

    Q_PROPERTY(QString color READ color NOTIFY alarmLevelChanged)

    QString color() const;

    Q_PROPERTY(QString ID READ ID NOTIFY IDChanged)

    QString ID() const
    {
        return _ID;
    }

    void setID(QString i);


    Q_PROPERTY(AviationUnits::Distance vDist READ vDist NOTIFY vDistChanged)

    AviationUnits::Distance vDist() const
    {
        return _vDist;
    }

    Q_PROPERTY(AviationUnits::Distance hDist READ hDist NOTIFY hDistChanged)

    AviationUnits::Distance hDist() const
    {
        return _hDist;
    }

    Q_PROPERTY(double hDistM READ hDistM NOTIFY hDistChanged)

    double hDistM() const
    {
        return _hDist.toM();
    }

    Q_PROPERTY(QString vDistText READ vDistText NOTIFY vDistChanged)

    QString vDistText() const
    {
        return _vDist.toString(GlobalSettings::useMetricUnitsStatic(), true, true);
    }


    Q_PROPERTY(Type type READ type NOTIFY typeChanged)

    Type type() const
    {
        return _type;
    }


    Q_PROPERTY(bool animate READ animate NOTIFY animateChanged)

    bool animate() const
    {
        return _animate;
    }

    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)

    bool valid() const;

    bool operator>(const TrafficNoPos &other);

signals:
    void alarmLevelChanged();
    void animateChanged();
    void IDChanged();
    void typeChanged();
    void hDistChanged();
    void vDistChanged();
    void validChanged();

private slots:
    void timeOut();

private:
    void setAnimate(bool a);

    int _alarmLevel {0};
    bool _animate {true};
    QString _icon;
    QString _ID;
    Type _type {Type::unknown};
    AviationUnits::Distance _vDist;
    AviationUnits::Distance _hDist;

    QTimer timeOutCounter;
};

}

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

#include <QGeoPositionInfo>
#include <QTimer>

#include "AviationUnits.h"


namespace Navigation {

/*! \brief Traffic
*/

class Traffic : public QObject {
    Q_OBJECT

public:
    /*! \brief Traffic type */
    enum AircraftType
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
    Q_ENUM(AircraftType)

    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Traffic(QObject *parent = nullptr);

    // Standard destructor
    ~Traffic() override = default;

    void setData(int newAlarmLevel, const QString& newID, AviationUnits::Distance newHDist, AviationUnits::Distance newVDist, AircraftType newType, const QGeoPositionInfo& newPositionInfo);

    void copyFrom(const Traffic & other);

    Q_PROPERTY(int alarmLevel READ alarmLevel NOTIFY alarmLevelChanged)

    int alarmLevel() const
    {
        return _alarmLevel;
    }

    void setAlarmLevel(int level);

    Q_PROPERTY(QString color READ color NOTIFY alarmLevelChanged)

    QString color() const;



    Q_PROPERTY(QString ID READ ID NOTIFY IDChanged)

    QString ID() const
    {
        return _ID;
    }

    void setID(QString i);


    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate NOTIFY positionInfoChanged)

    QGeoCoordinate coordinate() const
    {
        return _positionInfo.coordinate();
    }


    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)

    QString icon() const
    {
        return _icon;
    }

    Q_PROPERTY(AviationUnits::Distance hDist READ hDist NOTIFY hDistChanged)

    AviationUnits::Distance hDist() const
    {
        return _hDist;
    }

    Q_PROPERTY(double hDistM READ hDistM NOTIFY hDistChanged)

    double hDistM() const
    {
        qWarning() << "hDist" << _hDist.toM();
        return _hDist.toM();
    }

    Q_PROPERTY(AviationUnits::Distance vDist READ vDist NOTIFY vDistChanged)

    AviationUnits::Distance vDist() const
    {
        return _vDist;
    }

    Q_PROPERTY(QString vDistText READ vDistText NOTIFY vDistChanged)

    QString vDistText() const
    {
        return _vDist.toString(GlobalSettings::useMetricUnitsStatic(), true, true);
    }


    Q_PROPERTY(double TT READ TT NOTIFY positionInfoChanged)

    double TT() const
    {
        if (_positionInfo.hasAttribute(QGeoPositionInfo::Direction))
            return _positionInfo.attribute(QGeoPositionInfo::Direction);
        return qQNaN();
    }


    Q_PROPERTY(AircraftType type READ type NOTIFY typeChanged)

    AircraftType type() const
    {
        return _type;
    }

    Q_PROPERTY(QString typeString READ typeString NOTIFY typeChanged)

    QString typeString() const;


    Q_PROPERTY(bool animate READ animate NOTIFY animateChanged)

    bool animate() const
    {
        return _animate;
    }

    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)

    bool valid() const;

    bool operator>(const Traffic &rhs);

signals:
    void alarmLevelChanged();
    void animateChanged();
    void iconChanged();
    void IDChanged();
    void positionInfoChanged();
    void typeChanged();
    void hDistChanged();
    void vDistChanged();
    void validChanged();

private slots:
    void setIcon();
    void timeOut();

private:
    void setAnimate(bool a);

    int _alarmLevel {0};
    bool _animate {true};
    QGeoPositionInfo _positionInfo;
    QString _icon;
    QString _ID;
    AircraftType _type {AircraftType::unknown};
    AviationUnits::Distance _vDist;
    AviationUnits::Distance _hDist;

    QTimer timeOutCounter;
};

}

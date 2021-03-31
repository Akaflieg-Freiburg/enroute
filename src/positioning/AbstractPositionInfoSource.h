/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#include "units/Distance.h"


namespace Positioning {

class AbstractPositionInfoSource : public QObject {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit AbstractPositionInfoSource(QObject *parent = nullptr);

    //
    // Properties
    //

    /*! \brief Receiving data from one data source*/
    Q_PROPERTY(QGeoPositionInfo positionInfo READ positionInfo WRITE setPositionInfo RESET resetPositionInfo NOTIFY positionInfoChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property positionInfo
     */
    QGeoPositionInfo positionInfo() const
    {
        return m_positionInfo;
    }

    /*! \brief Receiving data from one data source*/
    Q_PROPERTY(AviationUnits::Distance pressureAltitude READ pressureAltitude WRITE setPressureAltitude RESET resetPressureAltitude NOTIFY pressureAltitudeChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property positionInfo
     */
    AviationUnits::Distance pressureAltitude() const
    {
        return m_pressureAltitude;
    }

    /*! \brief Receiving data from one data source*/
//    Q_PROPERTY(bool receiving READ receiving NOTIFY receivingChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property receiving
     */
//    virtual bool receiving() const = 0;

    /*! \brief Receiving data from one data source*/
    Q_PROPERTY(QString statusString READ statusString NOTIFY statusStringChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property statusString
     */
    virtual QString statusString() const = 0;


    void setPositionInfo(const QGeoPositionInfo &info);
    void resetPositionInfo();

    void setPressureAltitude(AviationUnits::Distance newPressureAltitude);
    void resetPressureAltitude();

signals:
    /*! \brief Notifier signal */
    void pressureAltitudeChanged();

    /*! \brief Notifier signal */
    void receivingChanged();

    /*! \brief Notifier signal */
    void statusStringChanged();

    /*! \brief Notifier signal */
    void positionInfoChanged(const QGeoPositionInfo &info);

private:
    AviationUnits::Distance m_pressureAltitude {};
    QTimer m_pressureAltitudeTimer;

    QGeoPositionInfo m_positionInfo;
    QTimer m_positionInfoTimer;
};

}

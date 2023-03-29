/***************************************************************************
 *   Copyright (C) 2021-2023 by Stefan Kebekus                             *
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

#include "positioning/PositionInfo.h"
#include "units/Distance.h"


namespace Positioning {

/*! \brief Abstract base class for all classes that provide geographic position information
 *
 *  This is the base class for all classes that provide geographic position
 *  information.  The information is exposed via two properties, positionInfo
 *  and pressureAltitude.
 *
 *  The property statusString gives more information about the status of the
 *  source
 */


class PositionInfoSource_Abstract : public QObject {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit PositionInfoSource_Abstract(QObject *parent = nullptr);

    //
    // Properties
    //

    /*! \brief Position information
     *
     *  This property holds information about the device position. To ensure
     *  that the data is up-to-date, the position information will be set to an
     *  invalid positionInfo when no data has arrived for more than the time
     *  specified in PositionInfo::lifetime.
     *
     *  Consumers of the class can use positionInfo().isValid() property to
     *  check if position data is continually arriving.
     */
    Q_PROPERTY(Positioning::PositionInfo positionInfo READ positionInfo NOTIFY positionInfoChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property positionInfo
     */
    [[nodiscard]] auto positionInfo() const -> Positioning::PositionInfo
    {
        return m_positionInfo;
    }

    /*! \brief Pressure altitude
     *
     *  This property holds information about the pressure altitude, that is,
     *  the altitude that you would read off your altimeter if the altimeter is
     *  set to 1013.2 hPa. To ensure that the data is up-to-date, the position
     *  information will be set to "invalid" when no data has arrived for more
     *  than the time specified in PositionInfo::lifetime.
     */
    Q_PROPERTY(Units::Distance pressureAltitude READ pressureAltitude NOTIFY pressureAltitudeChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property pressureAltitude
     */
    [[nodiscard]] auto pressureAltitude() const -> Units::Distance
    {
        return m_pressureAltitude;
    }

    /*! \brief Indicator that position information is being received
     *
     *  Use this property to tell if position information is being received.
     */
    Q_PROPERTY(bool receivingPositionInfo READ receivingPositionInfo NOTIFY receivingPositionInfoChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property receivingPositionInfo
     */
    [[nodiscard]] auto receivingPositionInfo() const -> bool
    {
        return _receivingPositionInfo;
    }

    /*! \brief Source name
     *
     *  This property holds a translated, human-readable string that describes
     *  the source. This could typically be a string of the form "Traffic
     *  Receiver" or "Built-in satellite receiver".
     */
    Q_PROPERTY(QString sourceName READ sourceName NOTIFY sourceNameChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property sourceName
     */
    [[nodiscard]] auto sourceName() const -> QString
    {
        return m_sourceName;
    }

    /*! \brief Source status
     *
     *  This property holds a translated, human-readable string that describes
     *  the status of the positionInfo source. This could typically be a string
     *  of the form "OK" or "Insufficient permission to access position info"
     */
    Q_PROPERTY(QString statusString READ statusString NOTIFY statusStringChanged)

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property statusString
     */
    [[nodiscard]] auto statusString() const -> QString
    {
        return m_statusString;
    }

signals:
    /*! \brief Notifier signal */
    void positionInfoChanged();

    /*! \brief Notifier signal */
    void pressureAltitudeChanged();

    /*! \brief Notifier signal */
    void receivingPositionInfoChanged();

    /*! \brief Notifier signal */
    void sourceNameChanged(const QString &name);

    /*! \brief Notifier signal */
    void statusStringChanged(const QString &status);

protected:
    // This method must be used by child classes to update the position info.
    // The class uses a timer internally to reset the position info to "invalid"
    // after the time specified in PositionInfo::lifetime seconds. It also
    // updates the property receivingPositionInfo.
    void setPositionInfo(const Positioning::PositionInfo &info);

    // This method must be used by child classes to update the pressure altitude
    // The class uses a timer internally to reset the position info to "invalid"
    // after the time specified in PositionInfo::lifetime seconds.
    void setPressureAltitude(Units::Distance newPressureAltitude);

    // This method must be used by child classes to update the source name
    void setSourceName(const QString &name);

    // This method must be used by child classes to update the status string
    void setStatusString(const QString &status);

private:
    // Resets the position info to "invalid"
    void resetPositionInfo();

    // Resets the pressure altitude to "invalid"
    void resetPressureAltitude();

    Units::Distance m_pressureAltitude {};
    QTimer m_pressureAltitudeTimer;

    Positioning::PositionInfo m_positionInfo;
    QTimer m_positionInfoTimer;

    QString m_sourceName {};
    QString m_statusString {};

    bool _receivingPositionInfo {false};
};

} // namespace Positioning

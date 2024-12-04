/***************************************************************************
 *   Copyright (C) 2021-2024 by Stefan Kebekus                             *
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
#include <QProperty>
#include <QTimer>

#include "positioning/PositionInfo.h"


namespace Positioning {

/*! \brief Abstract base class for all classes that provide geographic position
 *  information
 *
 *  This is the base class for all classes that provide geographic position
 *  information.
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
     */
    Q_PROPERTY(Positioning::PositionInfo positionInfo READ positionInfo BINDABLE bindablePositionInfo)

    /*! \brief Indicator that position information is being received
     *
     *  This is a shortcut for positionInfo().isValid. This property exists
     *  because it does not change so often, and can thus be more efficient to
     *  use.
     */
    Q_PROPERTY(bool receivingPositionInfo READ receivingPositionInfo BINDABLE bindableReceivingPositionInfo)

    /*! \brief Source name
     *
     *  This property holds a translated, human-readable string that describes
     *  the source. This could typically be a string of the form "Traffic
     *  Receiver" or "Built-in satellite receiver".
     */
    Q_PROPERTY(QString sourceName READ sourceName BINDABLE bindableSourceName)

    /*! \brief Source status
     *
     *  This property holds a translated, human-readable string that describes
     *  the status of the positionInfo source. This could typically be a string
     *  of the form "OK" or "Insufficient permission to access position info"
     */
    Q_PROPERTY(QString statusString READ statusString BINDABLE bindableStatusString)


    //
    // Getter Methods
    //

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property positionInfo
     */
    [[nodiscard]] Positioning::PositionInfo positionInfo() const
    {
        return m_positionInfo.value();
    }

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property positionInfo
     */
    [[nodiscard]] QBindable<Positioning::PositionInfo> bindablePositionInfo() const
    {
        return &m_positionInfo;
    }

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property receivingPositionInfo
     */
    [[nodiscard]] bool receivingPositionInfo() const {return m_receivingPositionInfo.value();}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property receivingPositionInfo
     */
    [[nodiscard]] QBindable<bool> bindableReceivingPositionInfo() {return &m_receivingPositionInfo;}

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property sourceName
     */
    [[nodiscard]] QString sourceName() const
    {
        return m_sourceName.value();
    }

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property sourceName
     */
    [[nodiscard]] QBindable<QString> bindableSourceName() const
    {
        return &m_sourceName;
    }

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property statusString
     */
    [[nodiscard]] QString statusString() const
    {
        return m_statusString.value();
    }

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property statusString
     */
    [[nodiscard]] QBindable<QString> bindableStatusString() const
    {
        return &m_statusString;
    }

protected:
    // This method must be used by child classes to update the position info.
    // The class uses a timer internally to reset the position info to "invalid"
    // after the time specified in PositionInfo::lifetime seconds. It also
    // updates the property receivingPositionInfo.
    void setPositionInfo(const Positioning::PositionInfo& info);

    // This method must be used by child classes to update the source name
    void setSourceName(const QString& name) {m_sourceName = name;}

    // This method must be used by child classes to update the status string
    void setStatusString(const QString& status)  {m_statusString = status;}

private:
    QProperty<Positioning::PositionInfo> m_positionInfo;
    QTimer m_positionInfoTimer;

    QProperty<bool> m_receivingPositionInfo {false};
    QProperty<QString> m_sourceName;
    QProperty<QString> m_statusString;
};

} // namespace Positioning

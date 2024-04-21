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

#include <QBluetoothDeviceInfo>


namespace Traffic {

/*! \brief Connection to a traffic data receiver
 *
 *  This class describes a connection to a traffic data receiver.
 *  It exposes connection properties in a format that is suitable
 *  for QML.
 */
class ConnectionInfo {
    Q_GADGET
    QML_VALUE_TYPE(connectionInfo)

public:
    /*! \brief Connection Type */
    enum Type {
        Invalid,            /*!< Invalid connection */
        BluetoothClassic,   /*!< Bluetooth Classic */
        BluetoothLowEnergy, /*!< Bluetooth LE */
        TCP,                /*!< TCP Connection */
        UDP,                /*!< UDP Connection */
        Serial,             /*!< Serial Port Connection */
        FLARMFile           /*!< FLARM Simulator File */
    };
    Q_ENUM(Type)

    ConnectionInfo() = default;

    explicit ConnectionInfo(const QBluetoothDeviceInfo& info);

    Q_PROPERTY(QString name READ name CONSTANT)
    [[nodiscard]] QString name() const;

    Q_PROPERTY(QString description READ description CONSTANT)
    [[nodiscard]] QString description() const;

    Q_PROPERTY(QString icon READ icon CONSTANT)
    [[nodiscard]] QString icon() const;

    Q_PROPERTY(bool canAddConnection READ canAddConnection CONSTANT)
    [[nodiscard]] bool canAddConnection() const;

    Q_PROPERTY(Traffic::ConnectionInfo::Type type READ type CONSTANT)
    [[nodiscard]] Traffic::ConnectionInfo::Type type() const { return Traffic::ConnectionInfo::Type::Invalid; }

    bool operator== (const ConnectionInfo& other) const = default;

    bool operator< (const ConnectionInfo& other) const;

    [[nodiscard]] QBluetoothDeviceInfo deviceInfo() const { return m_deviceInfo; }

private:
    QBluetoothDeviceInfo m_deviceInfo;
};

} // namespace Traffic

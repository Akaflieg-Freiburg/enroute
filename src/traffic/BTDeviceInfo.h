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

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QNetworkDatagram>
#include <QPointer>
#include <QQmlEngine>
#include <QUdpSocket>



namespace Traffic {


class BTDeviceInfo {
    Q_GADGET
    QML_VALUE_TYPE(btDeviceInfo)

public:
    BTDeviceInfo() = default;
    explicit BTDeviceInfo(const QBluetoothDeviceInfo& info);

    Q_PROPERTY(QString name READ name CONSTANT)
    [[nodiscard]] QString name() const;

    Q_PROPERTY(QString description READ description CONSTANT)
    [[nodiscard]] QString description() const;

    Q_PROPERTY(QString icon READ icon CONSTANT)
    [[nodiscard]] QString icon() const;

    Q_PROPERTY(bool canAddConnection READ canAddConnection CONSTANT)
    [[nodiscard]] bool canAddConnection();

    bool operator == (const BTDeviceInfo& other) const = default;

    [[nodiscard]] QBluetoothDeviceInfo deviceInfo() const { return m_deviceInfo; }

private:
    QBluetoothDeviceInfo m_deviceInfo;
};

} // namespace Traffic

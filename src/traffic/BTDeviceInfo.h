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
    QML_ELEMENT

public:
    BTDeviceInfo() = default;
    explicit BTDeviceInfo(const QBluetoothDeviceInfo& info);

    Q_PROPERTY(QString name READ name CONSTANT)
    [[nodiscard]] QString name() const { return m_deviceInfo.name(); }

    Q_PROPERTY(QString icon READ icon CONSTANT)
    [[nodiscard]] QString icon() const { return "/icons/material/ic_add.svg"; }

    Q_PROPERTY(QString category READ category CONSTANT)
    [[nodiscard]] QString category() const { return "Bluetooth Device"; }

    bool operator == (const BTDeviceInfo& other) const = default;

private:
    QBluetoothDeviceInfo m_deviceInfo;
};

} // namespace Traffic

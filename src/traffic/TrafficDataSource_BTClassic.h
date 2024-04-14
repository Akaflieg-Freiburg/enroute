/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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

#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceInfo>
#include <QBluetoothServiceInfo>
#include <QBluetoothSocket>

#include <QObject>

#include "traffic/TrafficDataSource_AbstractSocket.h"


namespace Traffic {

class TrafficDataSource_BTClassic : public TrafficDataSource_AbstractSocket {
    Q_OBJECT

public:
    TrafficDataSource_BTClassic(const QBluetoothDeviceInfo& info, QObject* parent=nullptr);

    ~TrafficDataSource_BTClassic() override = default;

    [[nodiscard]] QString sourceName() const override;
    [[nodiscard]] QBluetoothDeviceInfo sourceInfo() const
    {
        return m_info;
    }

public slots:
    void connectToTrafficReceiver() override;
    void disconnectFromTrafficReceiver() override;

    void onErrorOccurred(QBluetoothSocket::SocketError error);
    void onStateChanged(QBluetoothSocket::SocketState state);
    void onReadyRead();

private:
    Q_DISABLE_COPY_MOVE(TrafficDataSource_BTClassic)

    QBluetoothDeviceInfo m_info;
    QBluetoothSocket socket {QBluetoothServiceInfo::RfcommProtocol};
    QTextStream m_textStream {&socket};
};

} // namespace Traffic

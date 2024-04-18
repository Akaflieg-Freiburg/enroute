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
#include <QBluetoothPermission>
#include <QNetworkDatagram>
#include <QPointer>
#include <QQmlEngine>
#include <QUdpSocket>



namespace Traffic {


class BTScanner : public QObject {
    Q_OBJECT
    QML_ELEMENT

public:
    explicit BTScanner(QObject *parent = nullptr);

    // No default constructor, important for QML singleton
    explicit BTScanner() = delete;

    Q_PROPERTY(QString devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(bool isScanning READ isScanning NOTIFY isScanningChanged)

    QString devices() { return {}; }
    [[nodiscard]] QString error() const { return m_error; }
    [[nodiscard]] bool isScanning() const { return m_isScanning; }

public slots:
    void start();
    void stop() {;}

signals:
    void devicesChanged();
    void errorChanged();
    void isScanningChanged();

private slots:
    void setError(const QString& errorString);
    void setIsScanning(bool _isScanning);

    void onCanceled();
    void onDeviceDiscovered(const QBluetoothDeviceInfo&info);
    void onErrorOccurred(QBluetoothDeviceDiscoveryAgent::Error error);
    void onFinished();

private:
    // Bluetooth related members
    QBluetoothPermission m_bluetoothPermission;
    QBluetoothDeviceDiscoveryAgent m_discoveryAgent;

    QString m_error {};
    bool m_isScanning {false};
};

} // namespace Traffic

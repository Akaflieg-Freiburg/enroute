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

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothPermission>
#include <QQmlEngine>

#include "traffic/ConnectionScanner_Abstract.h"


namespace Traffic {

/*! \brief Connection Scanner: Bluetooth Devices
 *
 *  This class is a wrapper for QBluetoothDeviceDiscoveryAgent, providing data
 *  in a format that can by used by C++ and QML.
 */
class ConnectionScanner_Bluetooth : public ConnectionScanner_Abstract {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Properties need to be repeated, or else the Qt CMake macros cannot find them.
    Q_PROPERTY(QList<ConnectionInfo> connectionInfos READ connectionInfos NOTIFY connectionInfosChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)

public:
    /*! \brief Constructor
     *
     *  This constructor does not start the scanning process.
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit ConnectionScanner_Bluetooth(QObject* parent = nullptr);

    // No default constructor, important for QML singleton
    explicit ConnectionScanner_Bluetooth() = delete;

    // factory function for QML singleton
    static Traffic::ConnectionScanner_Bluetooth* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return new Traffic::ConnectionScanner_Bluetooth(nullptr);
    }



    //
    // Methods
    //

    /*!
     * \brief Bluetooth system status
     *
     * \return In case where Bluetooth is INOP, this method returns a string
     * with a human-readable, translated error message. Otherwise, an empty
     * string is returned.
     */
    [[nodiscard]] static QString bluetoothStatus();

public slots:
    // Re-implemented from ConnectionScanner_Abstract
    void start() override;

    // Re-implemented from ConnectionScanner_Abstract
    void stop() override;


private slots:
    // Connected to m_discoveryAgent
    void onCanceled();

    // Connected to m_discoveryAgent
    void onDeviceDiscovered(const QBluetoothDeviceInfo& info);

    // Connected to m_discoveryAgent
    void onDeviceUpdated(const QBluetoothDeviceInfo& info, QBluetoothDeviceInfo::Fields updatedFields);

    // Connected to m_discoveryAgent
    void onErrorOccurred(QBluetoothDeviceDiscoveryAgent::Error error);

    // Connected to m_discoveryAgent
    void onFinished();

private:
    // Update connectionInfos
    void updateConnectionInfos();

    // Bluetooth related members
    QBluetoothPermission m_bluetoothPermission;
    QBluetoothDeviceDiscoveryAgent m_discoveryAgent;

};

} // namespace Traffic

/***************************************************************************
 *   Copyright (C) 2025 by Stefan Kebekus                                  *
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
#include <QBluetoothLocalDevice>
#include <QBluetoothServiceInfo>
#include <QBluetoothPermission>
#include <QBluetoothSocket>
#include <QLowEnergyController>

#include "traffic/TrafficDataSource_AbstractSocket.h"

using namespace Qt::Literals::StringLiterals;


namespace Traffic {

/*! \brief Traffic receiver: Bluetooth LE connection to a FLARM/NMEA source via
 * the "Nordic UART Service"
 *
 * This class connects to a traffic receiver via Bluetooth LE via the "Nordic
 * UART Service" (NUS). This is a custom service developed by Nordic
 * Semiconductor for Bluetooth Low Energy (BLE) devices. It acts as a bridge
 * between BLE and UART (Universal Asynchronous Receiver/Transmitter) interfaces
 * and allows for bidirectional communication between devices using a simple
 * serial protocol over BLE.
 *
 * Details are described here:
 * https://docs.ruuvi.com/communication/bluetooth-connection/nordic-uart-service-nus
 *
 * At the time of writing (early Mar 25), the implementation is not well-tested.
 * The connection procedure is rather complicated.
 *
 * - Someone calls connectToTrafficReceiver(). The implementation calls
 *   m_control->connectToDevice().
 *
 * - Once a connection is established, signal/slots ensure that
 *   m_control->discoverServices() is called.
 *
 * - Once all services are found, the slot onDiscoveryFinished() is called. The
 *   implementation checks if the device offers the "Nordic UART Service" (NUS).
 *   If so, we need to find the service details. To start the search, the
 *   implementation calls m_NUSService->discoverDetails().
 *
 * - Once the service details are found, the slot onServiceStateChanged() is
 *   called with argument QLowEnergyService::RemoteServiceDiscovered. The
 *   implementation checks if the service offers the "TX" characteristic. If so,
 *   it switched notifications on.
 *
 * - Data will now flow in via the slot onCharacteristcChanged()
 */

class TrafficDataSource_BluetoothLowEnergy : public TrafficDataSource_AbstractSocket {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * @param isCanonical Intializer for property canonical
     *
     * @param info Description of a Bluetooth LE device
     *
     * @param parent The standard QObject parent pointer
     */
    TrafficDataSource_BluetoothLowEnergy(bool isCanonical, const QBluetoothDeviceInfo& info, QObject* parent);

    // Standard destructor
    ~TrafficDataSource_BluetoothLowEnergy() override = default;



    //
    // Properties
    //

    /*! \brief Source info
     *
     *  Device info, as set in the constructor of this class.
     */
    Q_PROPERTY(QBluetoothDeviceInfo sourceInfo READ sourceInfo CONSTANT)



    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property connectionInfo
     */
    [[nodiscard]] Traffic::ConnectionInfo connectionInfo() const override
    {
        return Traffic::ConnectionInfo(m_info, canonical());
    }

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property dataFormat
     */
    [[nodiscard]] QString dataFormat() const override { return u"FLARM/NMEA"_s; }

    /*! \brief Getter function for the property with the same name
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     *
     *  @returns Property icon
     */
    [[nodiscard]] QString icon() const override { return u"/icons/material/ic_bluetooth.svg"_s; }

    /*! \brief Getter function for the property with the same name
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     *
     *  @returns Property sourceName
     */
    [[nodiscard]] QString sourceName() const override;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property sourceInfo
     */
    [[nodiscard]] QBluetoothDeviceInfo sourceInfo() const
    {
        return m_info;
    }


public slots:
    /*! \brief Start attempt to connect to traffic receiver
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     */
    void connectToTrafficReceiver() override;

    /*! \brief Disconnect from traffic receiver
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     */
    void disconnectFromTrafficReceiver() override;

private slots:
    // Read and process NMEA sentences
    void onCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);

    // Handle errors
    void onErrorOccurred(QLowEnergyController::Error error);

    void onServiceDiscoveryFinished();

    void onServiceStateChanged(QLowEnergyService::ServiceState newState);

    void onStateChanged(QLowEnergyController::ControllerState state);

private:
    Q_DISABLE_COPY_MOVE(TrafficDataSource_BluetoothLowEnergy)

    // Permissions
    QBluetoothPermission m_bluetoothPermission;

    // Copied from the constructor
    QBluetoothDeviceInfo m_info;

    QLowEnergyController* m_control {nullptr};

    QLowEnergyService* m_nordicUARTService {nullptr};
    QBluetoothUuid const nordicUARTServiceUuid {"6e400001-b5a3-f393-e0a9-e50e24dcca9e"};
    QBluetoothUuid const nordicUARTTxCharacteristicID {"6e400003-b5a3-f393-e0a9-e50e24dcca9e"};

    QLowEnergyService* m_simpleUARTService {nullptr};
    QBluetoothUuid const simpleUARTServiceUuid {"0000ffe0-0000-1000-8000-00805f9b34fb"};
    QBluetoothUuid const simpleUARTCharacteristicID {"0000ffe1-0000-1000-8000-00805f9b34fb"};
};

} // namespace Traffic

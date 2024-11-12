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

#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>
#include <QBluetoothServiceInfo>
#include <QBluetoothSocket>
#include <QLowEnergyController>

#include "traffic/TrafficDataSource_AbstractSocket.h"

using namespace Qt::Literals::StringLiterals;


namespace Traffic {

/*! \brief Traffic receiver: Bluetooth LE connection to FLARM/NMEA source
 *
 *  This class connects to a traffic receiver via a Bluetooth LE serial port
 *  service.
 */

class TrafficDataSource_BluetoothLowEnergy : public TrafficDataSource_AbstractSocket {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * @param isCanonical Intializer for property canonical
     *
     *  @param info Description of a Bluetooth LE device offering
     *  serial port service.
     *
     *  @param parent The standard QObject parent pointer
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
    // Handle BT socket errors
    void onErrorOccurred(QLowEnergyController::Error error);

    // Handle BT socket state changes
    void onStateChanged(QLowEnergyController::ControllerState state);

    // Read and process received NMEA sentences
    void onConnected();


    // Read and process received NMEA sentences
    void onReadyRead();

    void onServiceDiscovered(const QBluetoothUuid &newService);


private:
    Q_DISABLE_COPY_MOVE(TrafficDataSource_BluetoothLowEnergy)

    // Copied from the constructor
    QBluetoothDeviceInfo m_info;

    // BT socket used for reading data
    QBluetoothSocket socket {QBluetoothServiceInfo::RfcommProtocol};

    // Text stream used for reading NMEA sentences
    QTextStream m_textStream {&socket};

    QLowEnergyController* m_control {nullptr};

};

} // namespace Traffic

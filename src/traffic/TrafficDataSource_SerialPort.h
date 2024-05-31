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

#include <QSerialPort>
#include <QSerialPortInfo>

#include "traffic/TrafficDataSource_AbstractSocket.h"


namespace Traffic {

/*! \brief Traffic receiver: Bluetooth Classic connection to FLARM/NMEA source
 *
 *  This class connects to a traffic receiver via a Bluetooth Classic serial port
 *  service.
 */

class TrafficDataSource_SerialPort : public TrafficDataSource_AbstractSocket {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * @param isCanonical Intializer for property canonical
     *
     *  @param info Description of a Bluetooth Classic device offering
     *  serial port service.
     *
     *  @param parent The standard QObject parent pointer
     */
    TrafficDataSource_SerialPort(bool isCanonical, const QString& portName, QObject* parent);

    // Standard destructor
    ~TrafficDataSource_SerialPort() override = default;



    //
    // Properties
    //

    /*! \brief Source info
     *
     *  Device info, as set in the constructor of this class.
     */
    Q_PROPERTY(QSerialPortInfo sourceInfo READ sourceInfo CONSTANT)



    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property connectionInfo
     */
    [[nodiscard]] Traffic::ConnectionInfo connectionInfo() const override
    {
        return Traffic::ConnectionInfo(sourceInfo(), canonical());
    }

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property dataFormat
     */
    [[nodiscard]] QString dataFormat() const override { return u"FLARM/NMEA"_qs; }

    /*! \brief Getter function for the property with the same name
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     *
     *  @returns Property icon
     */
    [[nodiscard]] QString icon() const override { return u"/icons/material/ic_bluetooth.svg"_qs; }

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
    [[nodiscard]] QSerialPortInfo sourceInfo() const
    {
        return QSerialPortInfo(m_port.portName());
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
    // Handle serial port errors
    void onErrorOccurred(QSerialPort::SerialPortError error);

    // Read and process received NMEA sentences
    void onReadyRead();

private:
    Q_DISABLE_COPY_MOVE(TrafficDataSource_SerialPort)

    // Copied from the constructor
    QSerialPort m_port;

    // Text stream used for reading NMEA sentences
    QTextStream m_textStream {&m_port};
};

} // namespace Traffic

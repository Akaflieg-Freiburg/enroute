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

#include <QPropertyNotifier>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "traffic/TrafficDataSource_AbstractSocket.h"

using namespace Qt::Literals::StringLiterals;


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
     *  This class provides a FLARM/NMEA data connection via a serial port. The
     *  constructor takes a name, which is either a proper port name (such as
     *  "ttyS0") or a description string (such as "ublox 7 - GPS GNSS Receiver")
     *  provided by QSerialPortInfo. This class will then connect to the first
     *  serial port whose port name or description matches the given string.
     *
     *  @param isCanonical Intializer for property canonical
     *
     *  @param portNameOrDescription Name or description of the port, as
     *  provided by QSerialPortInfo.portName() or QSerialPortInfo.description()
     *
     *  @param parent The standard QObject parent pointer
     */
    TrafficDataSource_SerialPort(bool isCanonical, const QString& portNameOrDescription, QObject* parent);

    // Standard destructor
    ~TrafficDataSource_SerialPort() override;


    /*! \brief Port Name or Description
     *
     *  This property contains the port name or description, as set in the constructor.
     */
    Q_PROPERTY(QString portNameOrDescription READ portNameOrDescription CONSTANT)

    /*! \brief Baud Rate of the Serial Port Connection */
    Q_PROPERTY(QSerialPort::BaudRate baudRate READ baudRate BINDABLE bindableBaudRate WRITE setBaudRate)

    /*! \brief Stop Bits of the Serial Port Connection */
    Q_PROPERTY(QSerialPort::StopBits stopBits READ stopBits BINDABLE bindableStopBits WRITE setStopBits)

    /*! \brief Flow Control */
    Q_PROPERTY(QSerialPort::FlowControl flowControl READ flowControl BINDABLE bindableFlowControl WRITE setFlowControl)


    //
    // Getter Methods
    //

#warning implement
    [[nodiscard]] QSerialPort::BaudRate baudRate() { return m_baudRate.value();};
    [[nodiscard]] QBindable<QSerialPort::BaudRate> bindableBaudRate() { return &m_baudRate;};
    void setBaudRate(QSerialPort::BaudRate rate);

    [[nodiscard]] QSerialPort::StopBits stopBits() { return m_stopBits.value();};
    [[nodiscard]] QBindable<QSerialPort::StopBits> bindableStopBits() { return &m_stopBits;};
    void setStopBits(QSerialPort::StopBits sb);

    [[nodiscard]] QSerialPort::FlowControl flowControl() { return m_flowControl.value();};
    [[nodiscard]] QBindable<QSerialPort::FlowControl> bindableFlowControl() { return &m_flowControl;};
    void setFlowControl(QSerialPort::FlowControl fc);

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
    [[nodiscard]] QString icon() const override { return u"/icons/material/ic_settings_ethernet.svg"_s; }

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
     *  This method implements the pure virtual method declared by its
     *  superclass.
     *
     *  @returns Property sourceName
     */
    [[nodiscard]] QString portNameOrDescription() const {return m_portNameOrDescription;};


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
    QSerialPort* m_port {nullptr};
    QString m_portNameOrDescription;

    // Text stream used for reading NMEA sentences
    QTextStream* m_textStream {nullptr};

    QProperty<QSerialPort::BaudRate> m_baudRate {QSerialPort::BaudRate::Baud9600};
    QProperty<QSerialPort::StopBits> m_stopBits {QSerialPort::StopBits::OneStop};
    QProperty<QSerialPort::FlowControl> m_flowControl {QSerialPort::FlowControl::NoFlowControl};
};

} // namespace Traffic

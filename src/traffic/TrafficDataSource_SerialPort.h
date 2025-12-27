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

#if __has_include(<QSerialPortInfo>)
#include <QSerialPort>
#include <QSerialPortInfo>
#endif

#include <QPropertyNotifier>

#include "traffic/TrafficDataSource_Abstract.h"

using namespace Qt::Literals::StringLiterals;


namespace Traffic {

/*! \brief Traffic receiver: Serial Port Connection to a FLARM/NMEA Source
 *
 *  This class connects to a traffic receiver via a serial port.
 */

class TrafficDataSource_SerialPort : public TrafficDataSource_Abstract {
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
     *  @param baudRate Baud Rate
     *
     *  @param stopBits Stop Bits
     *
     *  @param flowControl Flow Control
     *
     *  @param parent The standard QObject parent pointer
     */
    TrafficDataSource_SerialPort(bool isCanonical, const QString& portNameOrDescription,
                                 ConnectionInfo::BaudRate baudRate,
                                 ConnectionInfo::StopBits stopBits,
                                 ConnectionInfo::FlowControl flowControl,
                                 QObject* parent);

    // Standard destructor
    ~TrafficDataSource_SerialPort() override;


    /*! \brief Port Name or Description
     *
     *  This property contains the port name or description, as set in the constructor.
     */
    Q_PROPERTY(QString portNameOrDescription READ portNameOrDescription CONSTANT)
    [[nodiscard]] QString portNameOrDescription() const {return m_portNameOrDescription;};

    /*! \brief Baud Rate of the Serial Port Connection */
    Q_PROPERTY(ConnectionInfo::BaudRate baudRate READ baudRate BINDABLE bindableBaudRate WRITE setBaudRate)
    [[nodiscard]] ConnectionInfo::BaudRate baudRate() { return m_baudRate.value();};
    [[nodiscard]] QBindable<ConnectionInfo::BaudRate> bindableBaudRate() { return &m_baudRate;};
    void setBaudRate(ConnectionInfo::BaudRate rate);

    /*! \brief Stop Bits of the Serial Port Connection */
    Q_PROPERTY(ConnectionInfo::StopBits stopBits READ stopBits BINDABLE bindableStopBits WRITE setStopBits)
    [[nodiscard]] ConnectionInfo::StopBits stopBits() { return m_stopBits.value();};
    [[nodiscard]] QBindable<ConnectionInfo::StopBits> bindableStopBits() { return &m_stopBits;};
    void setStopBits(ConnectionInfo::StopBits sb);

    /*! \brief Flow Control */
    Q_PROPERTY(ConnectionInfo::FlowControl flowControl READ flowControl BINDABLE bindableFlowControl WRITE setFlowControl)
    [[nodiscard]] ConnectionInfo::FlowControl flowControl() { return m_flowControl.value();};
    [[nodiscard]] QBindable<ConnectionInfo::FlowControl> bindableFlowControl() { return &m_flowControl;};
    void setFlowControl(ConnectionInfo::FlowControl fc);


    //
    // Getter Methods
    //

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
#if __has_include(<QSerialPortInfo>)
    // Handle serial port errors
    void onErrorOccurred(QSerialPort::SerialPortError error);
#endif

    // Read and process received NMEA sentences
    void onReadyRead();

private:
    Q_DISABLE_COPY_MOVE(TrafficDataSource_SerialPort)

#if defined(Q_OS_ANDROID)
    QTimer pollTimer;
    void setParameters();
#endif
#if __has_include(<QSerialPortInfo>)
    QSerialPort* m_port {nullptr};
    QTextStream* m_textStream {nullptr};
#endif
    QString m_portNameOrDescription;
    QProperty<ConnectionInfo::BaudRate> m_baudRate {ConnectionInfo::BaudRate::Baud9600};
    QProperty<ConnectionInfo::StopBits> m_stopBits {ConnectionInfo::StopBits::OneStop};
    QProperty<ConnectionInfo::FlowControl> m_flowControl {ConnectionInfo::FlowControl::NoFlowControl};
};

} // namespace Traffic

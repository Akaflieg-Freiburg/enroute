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

#include <QCoreApplication>

#include "traffic/TrafficDataSource_SerialPort.h"


Traffic::TrafficDataSource_SerialPort::TrafficDataSource_SerialPort(bool isCanonical, const QString& portName, QObject* parent) :
    TrafficDataSource_AbstractSocket(isCanonical, parent),
    m_portName(portName)
{
}


Traffic::TrafficDataSource_SerialPort::~TrafficDataSource_SerialPort()
{
    if (m_textStream != nullptr)
    {
        delete m_textStream;
        m_textStream = nullptr;
    }

    if (m_port != nullptr)
    {
        delete m_port;
        m_port = nullptr;
    }
}


void Traffic::TrafficDataSource_SerialPort::connectToTrafficReceiver()
{

    // Do not do anything if the traffic receiver is connected and is receiving.
    if (receivingHeartbeat())
    {
        return;
    }

#if defined(Q_OS_IOS)
    setErrorString( tr("Due to platform limitations, serial ports are not supported on iOS."));
    return;
#endif

    // Close old connection
    disconnectFromTrafficReceiver();

    // Create and connect QSerialPort and QTextStream
    auto deviceInfos = QSerialPortInfo::availablePorts();
    foreach (auto deviceInfo, deviceInfos)
    {
        if ((deviceInfo.portName() == m_portName) || (deviceInfo.description() == m_portName))
        {
            m_port = new QSerialPort(deviceInfo);
            break;
        }
    }
    if (m_port == nullptr)
    {
        setErrorString(tr("Device not found."));
        return;
    }

    connect(m_port, &QSerialPort::readyRead, this, &Traffic::TrafficDataSource_SerialPort::onReadyRead);
    connect(m_port, &QSerialPort::errorOccurred, this, &Traffic::TrafficDataSource_SerialPort::onErrorOccurred);
    m_textStream = new QTextStream(m_port);

    // Start new connection
    m_port->open(QIODeviceBase::ReadWrite);
    if (m_port->isOpen())
    {
        setConnectivityStatus(tr("Connected."));
    }

}


void Traffic::TrafficDataSource_SerialPort::disconnectFromTrafficReceiver()
{
    if (m_textStream != nullptr)
    {
        delete m_textStream;
        m_textStream = nullptr;
    }

    if (m_port != nullptr)
    {
        if (m_port->isOpen())
        {
            m_port->clear();
            m_port->close();
        }
        m_port->clearError();
        onErrorOccurred(m_port->error());
        delete m_port;
        m_port = nullptr;
    }
    setConnectivityStatus(tr("Not connected."));
}


void Traffic::TrafficDataSource_SerialPort::onErrorOccurred(QSerialPort::SerialPortError error)
{
    switch (error)
    {
    case QSerialPort::NoError:
        setErrorString(u""_s);
        break;
    case QSerialPort::DeviceNotFoundError:
        setErrorString( tr("Non-existing device") );
        break;
    case QSerialPort::PermissionError:
        setErrorString( tr("Attempting to open an already opened device by another process or a user not having enough permission and credentials to open.") );
        break;
    case QSerialPort::OpenError:
        setErrorString( tr("An error occurred while attempting to open an already opened device in this object.") );
        break;
    case QSerialPort::NotOpenError:
        setErrorString( tr("Attempted to execute an operation that can only be successfully performed if the device is open.") );
        break;
    case QSerialPort::WriteError:
        setErrorString( tr("I/O error while writing data.") );
        break;
    case QSerialPort::ReadError:
        setErrorString( tr("I/O error while reading data.") );
        break;
    case QSerialPort::ResourceError:
        setErrorString( tr("I/O error occurred when a resource becomes unavailable, e.g. when the device is unexpectedly removed from the system.") );
        break;
    case QSerialPort::UnsupportedOperationError:
        setErrorString( tr("Device operation unsupported or prohibited by the operating system.") );
        break;
    case  QSerialPort::TimeoutError:
        setErrorString( tr("A timeout error occurred.") );
        break;
    case QSerialPort::UnknownError:
        setErrorString( tr("An unidentified error occurred.") );
        break;
    }
}


void Traffic::TrafficDataSource_SerialPort::onReadyRead()
{
    if (m_textStream == nullptr)
    {
        return;
    }

    QString sentence;
    while(m_textStream->readLineInto(&sentence))
    {
        processFLARMData(sentence);
    }
}


QString Traffic::TrafficDataSource_SerialPort::sourceName() const
{
    auto name = m_portName;
    if (name.isEmpty())
    {
        name = tr("Unnamed Device");
    }
    return tr("Serial port connection to %1").arg(name);
}

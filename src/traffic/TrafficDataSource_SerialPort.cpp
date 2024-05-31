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

#include <QCoreApplication>

#include "traffic/TrafficDataSource_SerialPort.h"


Traffic::TrafficDataSource_SerialPort::TrafficDataSource_SerialPort(bool isCanonical, const QString& portName, QObject* parent) :
    TrafficDataSource_AbstractSocket(isCanonical, parent),
    m_port(portName)
{
    // Connect socket
    //connect(&m_socket, &QBluetoothSocket::errorOccurred, this, &Traffic::TrafficDataSource_SerialPort::onErrorOccurred, Qt::QueuedConnection);
    //connect(&m_socket, &QBluetoothSocket::stateChanged, this, &Traffic::TrafficDataSource_SerialPort::onStateChanged);
    connect(&m_port, &QSerialPort::readyRead, this, &Traffic::TrafficDataSource_SerialPort::onReadyRead);

    auto errorChangeHandler = m_port.bindableError().subscribe([&]{onErrorOccurred(m_port.error());});
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
    m_port.clear();
    m_port.close();
    m_port.clearError();
    setErrorString();

    // Start new connection
    m_port.open(QIODeviceBase::ReadWrite);
}

void Traffic::TrafficDataSource_SerialPort::disconnectFromTrafficReceiver()
{
    m_port.close();
}


void Traffic::TrafficDataSource_SerialPort::onErrorOccurred(QSerialPort::SerialPortError error)
{
#warning This is never called!
    switch (error)
    {
    case QSerialPort::NoError:
        setErrorString( tr("No error.") );
    case QSerialPort::DeviceNotFoundError:
        setErrorString( tr("An error occurred while attempting to open an non-existing device.") );
        break;
    case QSerialPort::PermissionError:
        setErrorString( tr("An error occurred while attempting to open an already opened device by another process or a user not having enough permission and credentials to open.") );
        break;
    case QSerialPort::OpenError:
        setErrorString( tr("An error occurred while attempting to open an already opened device in this object.") );
        break;
    case QSerialPort::NotOpenError:
        setErrorString( tr("This error occurs when an operation is executed that can only be successfully performed if the device is open.") );
        break;
    case QSerialPort::WriteError:
        setErrorString( tr("An I/O error occurred while writing the data.") );
        break;
    case QSerialPort::ReadError:
        setErrorString( tr("An I/O error occurred while reading the data.") );
        break;
    case QSerialPort::ResourceError:
        setErrorString( tr("An I/O error occurred when a resource becomes unavailable, e.g. when the device is unexpectedly removed from the system.") );
        break;
    case QSerialPort::UnsupportedOperationError:
        setErrorString( tr("The requested device operation is not supported or prohibited by the running operating system.") );
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
    QString sentence;
    while(m_textStream.readLineInto(&sentence) )
    {
        processFLARMData(sentence);
    }
}

QString Traffic::TrafficDataSource_SerialPort::sourceName() const
{
    auto name = m_port.portName();
    if (name.isEmpty())
    {
        name = tr("Unnamed Device");
    }
    return tr("Serial port connection to %1").arg(name);
}

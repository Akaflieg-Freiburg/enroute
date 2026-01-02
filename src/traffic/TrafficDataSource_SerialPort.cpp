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

#if defined(Q_OS_ANDROID)
#include <QtCore/QJniObject>
#include <QtCore/qjnitypes.h>
Q_DECLARE_JNI_CLASS(UsbSerialHelper, "de/akaflieg_freiburg/enroute/UsbSerialHelper");
using namespace QtJniTypes;
#endif

#include "GlobalObject.h"
#include "platform/PlatformAdaptor.h"
#include "traffic/TrafficDataSource_SerialPort.h"


Traffic::TrafficDataSource_SerialPort::TrafficDataSource_SerialPort(bool isCanonical, const QString& portNameOrDescription,
                                                                    ConnectionInfo::BaudRate baudRate,
                                                                    ConnectionInfo::StopBits stopBits,
                                                                    ConnectionInfo::FlowControl flowControl,
                                                                    QObject* parent) :
    TrafficDataSource_Abstract(isCanonical, parent),
    m_portNameOrDescription(portNameOrDescription),
    m_baudRate(baudRate),
    m_stopBits(stopBits),
    m_flowControl(flowControl)
{
    m_connectionInfo.setBinding([this]() {
        return Traffic::ConnectionInfo(m_portNameOrDescription,
                                       m_baudRate.value(),
                                       m_stopBits.value(),
                                       m_flowControl.value(), false);
    });

#if defined(Q_OS_ANDROID)
    pollTimer.setInterval(200ms);
    pollTimer.setSingleShot(false);
    connect(&pollTimer, &QTimer::timeout, this, &Traffic::TrafficDataSource_SerialPort::read);
#endif
    connect(GlobalObject::platformAdaptor(), &Platform::PlatformAdaptor::serialPortsChanged, this, &Traffic::TrafficDataSource_SerialPort::connectToTrafficReceiver);
}

Traffic::TrafficDataSource_SerialPort::~TrafficDataSource_SerialPort()
{
#if defined(Q_OS_ANDROID)
    pollTimer.stop();
    UsbSerialHelper::callStaticMethod<jboolean>("close", m_portNameOrDescription);
#endif
#if __has_include(<QSerialPortInfo>)
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
#endif
}

void Traffic::TrafficDataSource_SerialPort::connectToTrafficReceiver()
{
    // Check if the connection is already open. If so, do not do anything
    // and return immediately
#if defined(Q_OS_ANDROID)
    const bool isOpen = (bool)UsbSerialHelper::callStaticMethod<jboolean>("isOpen", m_portNameOrDescription);
    if (isOpen && errorString().isEmpty())
    {
        return;
    }
#endif
#if __has_include(<QSerialPortInfo>)
    // Do not do anything if the traffic receiver is connected.
    if (m_port != nullptr)
    {
        if (m_port->isOpen() && (m_port->error() == QSerialPort::NoError))
        {
            return;
        }
    }
#endif

    // Close old connection
    disconnectFromTrafficReceiver();
    setErrorString();

    // Open new connection
#if defined(Q_OS_ANDROID)
    if ((bool)UsbSerialHelper::callStaticMethod<jboolean>("exists", m_portNameOrDescription))
    {
        // Open
        auto errorString = UsbSerialHelper::callStaticMethod<jstring>("openDevice", m_portNameOrDescription).toString();
        if (!errorString.isEmpty())
        {
            setErrorString(errorString);
            return;
        }

        // Set parameters
        errorString = UsbSerialHelper::callStaticMethod<jstring>("setParameters", m_portNameOrDescription,
                                                                 (int)m_baudRate.value(), (int)m_stopBits.value(), (int)m_flowControl.value()).toString();
        if (!errorString.isEmpty())
        {
            setErrorString(errorString);
            return;
        }

        // Start polling
        setConnectivityStatus(tr("Connected."));
        pollTimer.start();
        return;
    }
#endif
#if __has_include(<QSerialPortInfo>)
    // Create and connect QSerialPort and QTextStream
    auto deviceInfos = QSerialPortInfo::availablePorts();
    foreach (auto deviceInfo, deviceInfos)
    {
        if ((deviceInfo.portName() == m_portNameOrDescription) || (deviceInfo.description() == m_portNameOrDescription))
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
    m_port->setBaudRate(m_baudRate.value());
    m_port->setStopBits((QSerialPort::StopBits)m_stopBits.value());
    m_port->setFlowControl((QSerialPort::FlowControl)m_flowControl.value());

    connect(m_port, &QSerialPort::readyRead, this, &Traffic::TrafficDataSource_SerialPort::onReadyRead);
    connect(m_port, &QSerialPort::errorOccurred, this, &Traffic::TrafficDataSource_SerialPort::onErrorOccurred);
    m_textStream = new QTextStream(m_port);

    // Start new connection
    m_port->open(QIODeviceBase::ReadWrite);
    if (m_port->isOpen())
    {
        setConnectivityStatus(tr("Connected."));
    }
    return;
#endif
    setErrorString( tr("Serial ports are not supported on this platform."));
}

void Traffic::TrafficDataSource_SerialPort::disconnectFromTrafficReceiver()
{
#if defined(Q_OS_ANDROID)
    pollTimer.stop();
    UsbSerialHelper::callStaticMethod<jboolean>("close", m_portNameOrDescription);
#endif
#if __has_include(<QSerialPortInfo>)
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
#endif
    setConnectivityStatus(tr("Not connected."));
}

#if __has_include(<QSerialPortInfo>)
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
#endif

#if defined(Q_OS_ANDROID)
void Traffic::TrafficDataSource_SerialPort::setParameters()
{
    auto errorString = UsbSerialHelper::callStaticMethod<jstring>("setParameters", m_portNameOrDescription,
                                                                  (int)m_baudRate.value(), (int)m_stopBits.value(), (int)m_flowControl.value()).toString();
    if (!errorString.isEmpty())
    {
        setErrorString(errorString);
        disconnectFromTrafficReceiver();
    }
}
#endif

void Traffic::TrafficDataSource_SerialPort::setBaudRate(ConnectionInfo::BaudRate rate)
{
    m_baudRate = rate;
#if defined(Q_OS_ANDROID)
    setParameters();
#endif
#if __has_include(<QSerialPortInfo>)
    if (m_port != nullptr)
    {
        m_port->setBaudRate(rate);
    }
#endif
}

void Traffic::TrafficDataSource_SerialPort::setStopBits(ConnectionInfo::StopBits sb)
{
    m_stopBits = sb;
#if defined(Q_OS_ANDROID)
    setParameters();
#endif
#if __has_include(<QSerialPortInfo>)
    if (m_port != nullptr)
    {
        m_port->setStopBits((QSerialPort::StopBits)sb);
    }
#endif
}

void Traffic::TrafficDataSource_SerialPort::setFlowControl(ConnectionInfo::FlowControl fc)
{
    m_flowControl = fc;
#if defined(Q_OS_ANDROID)
    setParameters();
#endif
#if __has_include(<QSerialPortInfo>)
    if (m_port != nullptr)
    {
        m_port->setFlowControl((QSerialPort::FlowControl)fc);
    }
#endif
}

#if defined(Q_OS_ANDROID)
void Traffic::TrafficDataSource_SerialPort::read()
{
    const QJniObject jDevicePath = QJniObject::fromString(m_portNameOrDescription);
    auto jResult = QJniObject::callStaticObjectMethod(
        "de/akaflieg_freiburg/enroute/UsbSerialHelper",
        "read",
        "(Ljava/lang/String;II)[B",
        jDevicePath.object<jstring>(),
        1024, 1 //maxBytes, timeoutMs
        );

    if (!jResult.isValid())
    {
        setErrorString(u"Exception while reading"_s);
        disconnectFromTrafficReceiver();
        return;
    }

    const QJniEnvironment env;
    auto* jArray = jResult.object<jbyteArray>();
    const jsize length = env->GetArrayLength(jArray);
    if (length > 0)
    {
        QByteArray result;
        result.resize(length);
        env->GetByteArrayRegion(jArray, 0, length, reinterpret_cast<jbyte*>(result.data()));
        const QString sentence = QString::fromLatin1(result);
        for(auto& st : sentence.split('\n'))
        {
            st = st.trimmed();
            if (!st.isEmpty())
            {
                emit dataReceived(st.trimmed());
            }
        }
        processFLARMData(sentence);
    }
}
#endif

#if __has_include(<QSerialPortInfo>)
void Traffic::TrafficDataSource_SerialPort::onReadyRead()
{
    if (m_textStream == nullptr)
    {
        return;
    }

    QString sentence;
    while(m_textStream->readLineInto(&sentence))
    {
        emit dataReceived(sentence);
        processFLARMData(sentence);
    }
}
#endif

QString Traffic::TrafficDataSource_SerialPort::sourceName() const
{
    auto name = m_portNameOrDescription;
    if (name.isEmpty())
    {
        name = tr("Unnamed Device");
    }
    return tr("Serial port connection to %1").arg(name);
}

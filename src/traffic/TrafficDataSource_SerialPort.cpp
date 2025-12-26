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

#include "GlobalObject.h"
#include "platform/PlatformAdaptor.h"
#include "traffic/TrafficDataSource_SerialPort.h"

#warning TODO
#warning Request permissions and respond to request result
#warning Meaningful error messages
#warning automatic opening of connections
#warning set port parameters

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
    pollTimer.setInterval(200);
    pollTimer.setSingleShot(false);
    connect(&pollTimer, &QTimer::timeout, this, &Traffic::TrafficDataSource_SerialPort::onReadyRead);
#endif
    connect(GlobalObject::platformAdaptor(), &Platform::PlatformAdaptor::serialPortsChanged, this, &Traffic::TrafficDataSource_SerialPort::connectToTrafficReceiver);
}

Traffic::TrafficDataSource_SerialPort::~TrafficDataSource_SerialPort()
{
#if defined(Q_OS_ANDROID)
#warning
    QJniObject jDevicePath = QJniObject::fromString(m_portNameOrDescription);
    bool success = QJniObject::callStaticMethod<jboolean>(
        "de/akaflieg_freiburg/enroute/UsbSerialHelper",
        "close",
        "(Ljava/lang/String;)Z",  // Signature: takes String, returns boolean
        jDevicePath.object<jstring>());
    qWarning() << "Port closed" << success;
#elif __has_include(<QSerialPortInfo>)
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
#if defined(Q_OS_ANDROID)
#warning
    try
    {
        QJniObject jDevicePath = QJniObject::fromString(m_portNameOrDescription);
        bool isOpen = QJniObject::callStaticMethod<jboolean>(
            "de/akaflieg_freiburg/enroute/UsbSerialHelper",
            "isOpen",
            "(Ljava/lang/String;)Z",
            jDevicePath.object<jstring>()
            );
        qWarning() << "isOpen" << isOpen;
        if (isOpen && errorString().isEmpty())
        {
            return;
        }
    }
    catch (...)
    {
        return;
    }

    // Close old connection
    disconnectFromTrafficReceiver();

    try
    {
        QJniObject jDevicePath = QJniObject::fromString(m_portNameOrDescription);
        bool success = QJniObject::callStaticMethod<jboolean>(
            "de/akaflieg_freiburg/enroute/UsbSerialHelper",
            "openDevice",
            "(Ljava/lang/String;)Z",
            jDevicePath.object<jstring>()
            );
        qWarning() << "OPENDING" << success;
        if (success)
        {
            qDebug() << "Opened device:" << m_portNameOrDescription;
            setConnectivityStatus(tr("Connected."));
            pollTimer.start();

            QJniObject::callStaticMethod<jboolean>(
                "de/akaflieg_freiburg/enroute/UsbSerialHelper",
                "setParameters",
                "(Ljava/lang/String;III)Z",
                jDevicePath.object<jstring>(),
                m_baudRate.value(), m_stopBits.value(), m_flowControl.value()
                );

        }
        else
        {
            qWarning() << "Failed to open device:" << m_portNameOrDescription;
            setErrorString(tr("Failed to open device."));
        }
        return;

    }
    catch (...)
    {
        qWarning() << "Exception occurred while opening device:" << m_portNameOrDescription;
        setErrorString(tr("Exception occurred while opening device."));
    }

#elif __has_include(<QSerialPortInfo>)
    // Do not do anything if the traffic receiver is connected.
    if (m_port != nullptr)
    {
        if (m_port->isOpen() && (m_port->error() == QSerialPort::NoError))
        {
            return;
        }
    }

    // Close old connection
    disconnectFromTrafficReceiver();

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
#else
    setErrorString( tr("Serial ports are not supported on this platform."));
#endif
}

void Traffic::TrafficDataSource_SerialPort::disconnectFromTrafficReceiver()
{
#if defined(Q_OS_ANDROID)
#warning
    pollTimer.stop();
    QJniObject jDevicePath = QJniObject::fromString(m_portNameOrDescription);
    bool success = QJniObject::callStaticMethod<jboolean>(
        "de/akaflieg_freiburg/enroute/UsbSerialHelper",
        "close",
        "(Ljava/lang/String;)Z",  // Signature: takes String, returns boolean
        jDevicePath.object<jstring>());

    qWarning() << "Port closed" << success;
#elif __has_include(<QSerialPortInfo>)
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

#if !defined(Q_OS_ANDROID) && __has_include(<QSerialPortInfo>)
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

void Traffic::TrafficDataSource_SerialPort::setBaudRate(ConnectionInfo::BaudRate rate)
{
#if defined(Q_OS_ANDROID)
    QJniObject jDevicePath = QJniObject::fromString(m_portNameOrDescription);
    QJniObject::callStaticMethod<jboolean>(
        "de/akaflieg_freiburg/enroute/UsbSerialHelper",
        "setParameters",
        "(Ljava/lang/String;III)Z",
        jDevicePath.object<jstring>(),
        m_baudRate.value(), m_stopBits.value(), m_flowControl.value()
        );
#elif __has_include(<QSerialPortInfo>)
    if (m_port != nullptr)
    {
        m_port->setBaudRate(rate);
    }
#endif
    m_baudRate = rate;
}

void Traffic::TrafficDataSource_SerialPort::setStopBits(ConnectionInfo::StopBits sb)
{
#if defined(Q_OS_ANDROID)
    QJniObject jDevicePath = QJniObject::fromString(m_portNameOrDescription);
    QJniObject::callStaticMethod<jboolean>(
        "de/akaflieg_freiburg/enroute/UsbSerialHelper",
        "setParameters",
        "(Ljava/lang/String;III)Z",
        jDevicePath.object<jstring>(),
        m_baudRate.value(), m_stopBits.value(), m_flowControl.value()
        );
#elif __has_include(<QSerialPortInfo>)
    if (m_port != nullptr)
    {
        m_port->setStopBits((QSerialPort::StopBits)sb);
    }
#endif
    m_stopBits = sb;
}

void Traffic::TrafficDataSource_SerialPort::setFlowControl(ConnectionInfo::FlowControl fc)
{
#if defined(Q_OS_ANDROID)
    QJniObject jDevicePath = QJniObject::fromString(m_portNameOrDescription);
    QJniObject::callStaticMethod<jboolean>(
        "de/akaflieg_freiburg/enroute/UsbSerialHelper",
        "setParameters",
        "(Ljava/lang/String;III)Z",
        jDevicePath.object<jstring>(),
        m_baudRate.value(), m_stopBits.value(), m_flowControl.value()
        );
#elif __has_include(<QSerialPortInfo>)
    if (m_port != nullptr)
    {
        m_port->setFlowControl((QSerialPort::FlowControl)fc);
    }
#endif
    m_flowControl = fc;
}

void Traffic::TrafficDataSource_SerialPort::onReadyRead()
{
#if defined(Q_OS_ANDROID)
    QByteArray result;
    try
    {
        QJniObject jDevicePath = QJniObject::fromString(m_portNameOrDescription);
        auto jResult = QJniObject::callStaticObjectMethod(
            "de/akaflieg_freiburg/enroute/UsbSerialHelper",
            "read",
            "(Ljava/lang/String;II)[B",
            jDevicePath.object<jstring>(),
            1024, 100 //maxBytes, timeoutMs
            );

        if (jResult.isValid())
        {
            QJniEnvironment env;
            jbyteArray jArray = jResult.object<jbyteArray>();
            jsize length = env->GetArrayLength(jArray);
            if (length > 0)
            {
                result.resize(length);
                env->GetByteArrayRegion(jArray, 0, length, reinterpret_cast<jbyte*>(result.data()));
#warning Want to split sentence, I guess
                QString sentence = QString::fromLatin1(result);
                emit dataReceived(sentence);
                processFLARMData(sentence);
            }
        }
    }
    catch (...)
    {
        qWarning() << "Exception occurred while reading";
    }
#elif __has_include(<QSerialPortInfo>)
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
#endif
}

QString Traffic::TrafficDataSource_SerialPort::sourceName() const
{
    auto name = m_portNameOrDescription;
    if (name.isEmpty())
    {
        name = tr("Unnamed Device");
    }
    return tr("Serial port connection to %1").arg(name);
}

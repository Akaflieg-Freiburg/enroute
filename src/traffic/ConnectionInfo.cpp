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

#include <QBluetoothAddress>
#include <QCoreApplication>
#include <QQmlEngine>

#include "traffic/ConnectionInfo.h"



//
// Constructors
//

Traffic::ConnectionInfo::ConnectionInfo(const QBluetoothDeviceInfo& info, bool canonical)
    : m_bluetoothDeviceInfo(info),
    m_canonical(canonical)
{
    // Set Name
    {
        if (m_bluetoothDeviceInfo.isValid())
        {
            m_name = m_bluetoothDeviceInfo.name();
            if (m_name.isEmpty())
            {
                m_name = QObject::tr("Unnamed Device", "Traffic::ConnectionInfo");
            }
        }
    }

    // Set Description (must come after name)
    {
        QStringList descriptionItems;
        if (m_bluetoothDeviceInfo.coreConfigurations() == QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
        {
            descriptionItems += QObject::tr("Bluetooth Low Energy Device (unsupported)", "Traffic::ConnectionInfo");
        }
        else
        {
            switch(m_bluetoothDeviceInfo.majorDeviceClass())
            {
            case QBluetoothDeviceInfo::MiscellaneousDevice:
                descriptionItems += QObject::tr("Miscellaneous Device", "Traffic::ConnectionInfo");
                break;
            case QBluetoothDeviceInfo::ComputerDevice:
                descriptionItems += QObject::tr("Computer or PDA Device", "Traffic::ConnectionInfo");
                break;
            case QBluetoothDeviceInfo::PhoneDevice:
                descriptionItems += QObject::tr("Telephone Device", "Traffic::ConnectionInfo");
                break;
            case QBluetoothDeviceInfo::NetworkDevice:
                descriptionItems += QObject::tr("Network Device", "Traffic::ConnectionInfo");
                break;
            case QBluetoothDeviceInfo::AudioVideoDevice:
                descriptionItems += QObject::tr("Audio/Video Device", "Traffic::ConnectionInfo");
                break;
            case QBluetoothDeviceInfo::PeripheralDevice:
                descriptionItems += QObject::tr("Peripheral Device", "Traffic::ConnectionInfo");
                break;
            case QBluetoothDeviceInfo::ImagingDevice:
                descriptionItems += QObject::tr("Imaging Device", "Traffic::ConnectionInfo");
                break;
            case QBluetoothDeviceInfo::WearableDevice:
                descriptionItems += QObject::tr("Wearable Device", "Traffic::ConnectionInfo");
                break;
            case QBluetoothDeviceInfo::ToyDevice:
                descriptionItems += QObject::tr("Toy Device", "Traffic::ConnectionInfo");
                break;
            case QBluetoothDeviceInfo::HealthDevice:
                descriptionItems += QObject::tr("Health Device", "Traffic::ConnectionInfo");
                break;
            case QBluetoothDeviceInfo::UncategorizedDevice:
                descriptionItems += QObject::tr("Uncategorized Device", "Traffic::ConnectionInfo");
                break;
            }
            if (m_bluetoothDeviceInfo.serviceUuids().contains(QBluetoothUuid::ServiceClassUuid::SerialPort))
            {
                descriptionItems += QObject::tr("Serial Port Service", "Traffic::ConnectionInfo");
            }
        }
        m_description = u"%1<br><font size='2'>%2</font>"_qs.arg(m_name, descriptionItems.join(u" • "_qs));
    }

    // Set Icon
    {
        if (m_bluetoothDeviceInfo.isValid())
        {
            if (m_bluetoothDeviceInfo.coreConfigurations() == QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
            {
                m_icon = u"/icons/material/ic_bluetooth_disabled.svg"_qs;
            }
            else
            {
                m_icon = u"/icons/material/ic_bluetooth.svg"_qs;
            }
        }
    }

    // Set canConnect
    {
        if (m_bluetoothDeviceInfo.isValid() && (m_bluetoothDeviceInfo.coreConfigurations() != QBluetoothDeviceInfo::LowEnergyCoreConfiguration))
        {
            m_canConnect = true;
        }
    }

    // Set type
    {
        if (m_bluetoothDeviceInfo.coreConfigurations() == QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
        {
            m_type = Traffic::ConnectionInfo::BluetoothLowEnergy;
        }
        else
        {
            m_type = Traffic::ConnectionInfo::BluetoothClassic;
        }
    }
}


Traffic::ConnectionInfo::ConnectionInfo(quint16 port, bool canonical)
    : m_port(port), m_canonical(canonical)
{
    m_name = QObject::tr("UDP connection to port %1", "Traffic::ConnectionInfo").arg(m_port);
    m_icon = u"/icons/material/ic_wifi.svg"_qs;
    m_canConnect = true;
    m_type = Traffic::ConnectionInfo::UDP;
}



//
// Methods
//

bool Traffic::ConnectionInfo::sameConnectionAs(const ConnectionInfo& other) const
{
    if (m_type != other.m_type)
    {
        return false;
    }

    if (m_type == Traffic::ConnectionInfo::BluetoothClassic)
    {
        if (m_bluetoothDeviceInfo.isValid() != other.m_bluetoothDeviceInfo.isValid())
        {
            return false;
        }
        if (!m_bluetoothDeviceInfo.isValid() && !other.m_bluetoothDeviceInfo.isValid())
        {
            return true;
        }

#if defined(Q_OS_IOS)
        // iOS hides the device address and works with anonymized device UUIDs
        return m_bluetoothDeviceInfo.deviceUuid() == other.m_bluetoothDeviceInfo.deviceUuid();
#else
        return m_bluetoothDeviceInfo.address() == other.m_bluetoothDeviceInfo.address();
#endif
    }

    if (m_type == Traffic::ConnectionInfo::BluetoothLowEnergy)
    {
        if (m_bluetoothDeviceInfo.isValid() != other.m_bluetoothDeviceInfo.isValid())
        {
            return false;
        }
        if (!m_bluetoothDeviceInfo.isValid() && !other.m_bluetoothDeviceInfo.isValid())
        {
            return true;
        }
#if defined(Q_OS_IOS)
        // iOS hides the device address and works with anonymized device UUIDs
        return m_bluetoothDeviceInfo.deviceUuid() == other.m_bluetoothDeviceInfo.deviceUuid();
#else
        return m_bluetoothDeviceInfo.address() == other.m_bluetoothDeviceInfo.address();
#endif
    }

    return true;
}

bool Traffic::ConnectionInfo::operator< (const ConnectionInfo& other) const
{
    const bool canConnectFst = canConnect();
    const bool canConnectSnd = other.canConnect();
    if (canConnectFst != canConnectSnd)
    {
        return static_cast<int>(canConnectFst) < static_cast<int>(canConnectSnd);
    }

    if (m_type != other.m_type)
    {
        return m_type < other.m_type;
    }

    return name() < other.name();
}



//
// Friends
//

QDataStream& Traffic::operator<<(QDataStream& stream, const Traffic::ConnectionInfo &connectionInfo)
{
    stream << connectionInfo.m_canConnect;
    stream << connectionInfo.m_canonical;
    stream << connectionInfo.m_description;
    stream << connectionInfo.m_icon;
    stream << connectionInfo.m_name;
    stream << connectionInfo.m_type;

    switch(connectionInfo.m_type)
    {
    case Traffic::ConnectionInfo::Invalid:
        break;
    case Traffic::ConnectionInfo::BluetoothClassic:
    case Traffic::ConnectionInfo::BluetoothLowEnergy:
    {
        stream << connectionInfo.m_bluetoothDeviceInfo.address().toUInt64();
        stream << connectionInfo.m_bluetoothDeviceInfo.deviceUuid();
        stream << connectionInfo.m_bluetoothDeviceInfo.name();

        quint32 classOfDevice = 0;
        classOfDevice |= connectionInfo.m_bluetoothDeviceInfo.minorDeviceClass() << 2;
        classOfDevice |= connectionInfo.m_bluetoothDeviceInfo.majorDeviceClass() << 8;
        classOfDevice |= connectionInfo.m_bluetoothDeviceInfo.serviceClasses() << 13;
        stream << classOfDevice;
        break;
    }
    case Traffic::ConnectionInfo::TCP:
    case Traffic::ConnectionInfo::UDP:
        stream << connectionInfo.m_port;
        break;
    case Traffic::ConnectionInfo::Serial:
    case Traffic::ConnectionInfo::FLARMFile:
        break;
    }

    return stream;
}


QDataStream& Traffic::operator>>(QDataStream& stream, Traffic::ConnectionInfo& connectionInfo)
{
    stream >> connectionInfo.m_canConnect;
    stream >> connectionInfo.m_canonical;
    stream >> connectionInfo.m_description;
    stream >> connectionInfo.m_icon;
    stream >> connectionInfo.m_name;
    stream >> connectionInfo.m_type;

    switch(connectionInfo.m_type)
    {
    case Traffic::ConnectionInfo::Invalid:
        break;
    case Traffic::ConnectionInfo::BluetoothClassic:
    case Traffic::ConnectionInfo::BluetoothLowEnergy:
    {
        quint64 address = 0;
        QBluetoothUuid uuid;
        quint32 classOfDevice = 0;
        QString name;

        stream >> address;
        stream >> uuid;
        stream >> name;
        stream >> classOfDevice;

#if defined(Q_OS_IOS)
        // iOS hides the device address and works with anonymized device UUIDs
        connectionInfo.m_bluetoothDeviceInfo = QBluetoothDeviceInfo(uuid, name, classOfDevice);
#else
        connectionInfo.m_bluetoothDeviceInfo = QBluetoothDeviceInfo(QBluetoothAddress(address), name, classOfDevice);
#endif
        break;
    }
    case Traffic::ConnectionInfo::TCP:
    case Traffic::ConnectionInfo::UDP:
        stream >> connectionInfo.m_port;
        break;
    case Traffic::ConnectionInfo::Serial:
    case Traffic::ConnectionInfo::FLARMFile:
        break;
    }

    return stream;
}

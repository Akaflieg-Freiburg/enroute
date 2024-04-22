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
#include <QQmlEngine>

#include "GlobalObject.h"
#include "traffic/ConnectionInfo.h"
#include "traffic/TrafficDataProvider.h"


// Member functions

Traffic::ConnectionInfo::ConnectionInfo(const QBluetoothDeviceInfo& info)
    : m_bluetoothDeviceInfo(info)
{

    // Set Name
    {
        if (m_bluetoothDeviceInfo.isValid())
        {
            m_name = m_bluetoothDeviceInfo.name();
            if (m_name.isEmpty())
            {
                m_name = QObject::tr("Unnamed Device", "BTDeviceInfo");
            }
        }
    }

    // Set Description (must come after name)
    {
        QStringList descriptionItems;
        if (m_bluetoothDeviceInfo.coreConfigurations() == QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
        {
            descriptionItems += QObject::tr("Bluetooth Low Energy Device (mot supported)", "BTDeviceInfo");
        }
        else
        {
            switch(m_bluetoothDeviceInfo.majorDeviceClass())
            {
            case QBluetoothDeviceInfo::MiscellaneousDevice:
                descriptionItems += QObject::tr("Miscellaneous Device", "BTDeviceInfo");
                break;
            case QBluetoothDeviceInfo::ComputerDevice:
                descriptionItems += QObject::tr("Computer or PDA Device", "BTDeviceInfo");
                break;
            case QBluetoothDeviceInfo::PhoneDevice:
                descriptionItems += QObject::tr("Telephone Device", "BTDeviceInfo");
                break;
            case QBluetoothDeviceInfo::NetworkDevice:
                descriptionItems += QObject::tr("Network Device", "BTDeviceInfo");
                break;
            case QBluetoothDeviceInfo::AudioVideoDevice:
                descriptionItems += QObject::tr("Audio/Video Device", "BTDeviceInfo");
                break;
            case QBluetoothDeviceInfo::PeripheralDevice:
                descriptionItems += QObject::tr("Peripheral Device", "BTDeviceInfo");
                break;
            case QBluetoothDeviceInfo::ImagingDevice:
                descriptionItems += QObject::tr("Imaging Device", "BTDeviceInfo");
                break;
            case QBluetoothDeviceInfo::WearableDevice:
                descriptionItems += QObject::tr("Wearable Device", "BTDeviceInfo");
                break;
            case QBluetoothDeviceInfo::ToyDevice:
                descriptionItems += QObject::tr("Toy Device", "BTDeviceInfo");
                break;
            case QBluetoothDeviceInfo::HealthDevice:
                descriptionItems += QObject::tr("Health Device", "BTDeviceInfo");
                break;
            case QBluetoothDeviceInfo::UncategorizedDevice:
                descriptionItems += QObject::tr("Uncategorized Device", "BTDeviceInfo");
                break;
            }
            if (m_bluetoothDeviceInfo.serviceUuids().contains(QBluetoothUuid::ServiceClassUuid::SerialPort))
            {
                descriptionItems += QObject::tr("Serial Port Service", "BTDeviceInfo");
            }
        }
        m_description = u"%1<br><font size='2'>%2</font>"_qs.arg(m_name, descriptionItems.join(" • "));
    }

    // Set Icon
    {
        if (m_bluetoothDeviceInfo.isValid())
        {
            if (m_bluetoothDeviceInfo.coreConfigurations() == QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
            {
                m_icon = "/icons/material/ic_bluetooth_disabled.svg";
            }
            else
            {
                m_icon = "/icons/material/ic_bluetooth.svg";
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

bool Traffic::ConnectionInfo::operator== (const ConnectionInfo& other) const
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
        if (!m_bluetoothDeviceInfo.isValid() & !other.m_bluetoothDeviceInfo.isValid())
        {
            return true;
        }
        return m_bluetoothDeviceInfo.address() == other.m_bluetoothDeviceInfo.address();
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
        return m_bluetoothDeviceInfo.address() == other.m_bluetoothDeviceInfo.address();
    }

    return true;
}

bool Traffic::ConnectionInfo::operator< (const ConnectionInfo& other) const
{
    const bool canConnectFst = canConnect();
    const bool canConnectSnd = other.canConnect();
    if (canConnectFst && !canConnectSnd)
    {
        return true;
    }
    if (!canConnectFst && canConnectSnd)
    {
        return false;
    }

    return name() < other.name();
}



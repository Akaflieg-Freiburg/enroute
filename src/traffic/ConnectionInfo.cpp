/***************************************************************************
 *   Copyright (C) 2021-2024 by Stefan Kebekus                             *
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
    : m_deviceInfo(info)
{
}

QString Traffic::ConnectionInfo::name() const
{
    if (!m_deviceInfo.isValid())
    {
        return QObject::tr("Invalid Device", "BTDeviceInfo");
    }

    QString name = m_deviceInfo.name();
    if (name.isEmpty())
    {
        return QObject::tr("Unnamed Device", "BTDeviceInfo");
    }

    return name;
}

QString Traffic::ConnectionInfo::description() const
{

    QStringList descriptionItems;
    if (m_deviceInfo.coreConfigurations() == QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
    {
        descriptionItems += QObject::tr("Bluetooth Low Energy Device (mot supported)", "BTDeviceInfo");
    }
    else
    {
        if (GlobalObject::trafficDataProvider()->hasSource(m_deviceInfo))
        {
            descriptionItems += QObject::tr("Device already added", "BTDeviceInfo");
        }
        else
        {
            switch(m_deviceInfo.majorDeviceClass())
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
            if (m_deviceInfo.serviceUuids().contains(QBluetoothUuid::ServiceClassUuid::SerialPort))
            {
                descriptionItems += QObject::tr("Serial Port Service", "BTDeviceInfo");
            }
        }
    }

    return u"%1<br><font size='2'>%2</font>"_qs.arg(name(), descriptionItems.join(" • "));
}

bool Traffic::ConnectionInfo::canAddConnection() const
{
    if (!m_deviceInfo.isValid())
    {
        return false;
    }
    if (m_deviceInfo.coreConfigurations() == QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
    {
        return false;
    }
    if (GlobalObject::trafficDataProvider()->hasSource(m_deviceInfo))
    {
        return false;
    }

    return true;
}

QString Traffic::ConnectionInfo::icon() const
{
    if (!m_deviceInfo.isValid())
    {
        return "/icons/material/ic_delete.svg";
    }
    if (m_deviceInfo.coreConfigurations() == QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
    {
        return "/icons/material/ic_bluetooth_disabled.svg";
    }
    return "/icons/material/ic_bluetooth.svg";
}

bool Traffic::ConnectionInfo::operator< (const ConnectionInfo& other) const
{
    const bool canAddConnectionFst = canAddConnection();
    const bool canAddConnectionSnd = other.canAddConnection();
    if (canAddConnectionFst && !canAddConnectionSnd)
    {
        return true;
    }
    if (!canAddConnectionFst && canAddConnectionSnd)
    {
        return false;
    }

    return name() < other.name();
}

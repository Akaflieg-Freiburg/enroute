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

#include "traffic/BTDeviceInfo.h"


// Member functions

Traffic::BTDeviceInfo::BTDeviceInfo(const QBluetoothDeviceInfo& info)
    : m_deviceInfo(info)
{
}

QString Traffic::BTDeviceInfo::name() const
{
    //<strong>%1</strong><br><font size='2'>%2</font>

    if (!m_deviceInfo.isValid())
    {
        return "<strong>Invalid Device</strong>";;
    }

    QString result;
    QString name = m_deviceInfo.name();
    if (name.isEmpty())
    {
        result += "<strong>Unnamed Device</strong>";
    }
    else
    {
        result += "<strong>"+name+"</strong>";
    }

    if (m_deviceInfo.coreConfigurations() == QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
    {
        result += "<br><font size='2'>Bluetooth Low Energy Device (mot supported)</font>";
    }
    return result;
}

QString Traffic::BTDeviceInfo::icon() const
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

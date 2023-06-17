/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#include "platform/PlatformAdaptor_Linux.h"


Platform::PlatformAdaptor::PlatformAdaptor(QObject *parent)
    : PlatformAdaptor_Abstract(parent)
{
    connect(&networkManagerInterface, SIGNAL(StateChanged(uint)), this, SLOT(onNetworkStateChanged(uint)));
}


//
// Methods
//

auto Platform::PlatformAdaptor::checkPermissions() -> QString
{
    return {};
}


auto Platform::PlatformAdaptor::currentSSID() -> QString
{
    // get primary connection devices
    auto primaryConnection = networkManagerInterface.property("PrimaryConnection").value<QDBusObjectPath>();
    QDBusInterface primaryConnectionInterface(QStringLiteral("org.freedesktop.NetworkManager"), primaryConnection.path(),
            QStringLiteral("org.freedesktop.NetworkManager.Connection.Active"), QDBusConnection::systemBus());
    if(primaryConnectionInterface.isValid())
    {
        return primaryConnectionInterface.property("Id").toString();
    }
    return tr("unknown network name");
}


void Platform::PlatformAdaptor::disableScreenSaver()
{
}


void Platform::PlatformAdaptor::lockWifi(bool lock)
{
    Q_UNUSED(lock)
}


void Platform::PlatformAdaptor::onGUISetupCompleted()
{
}


void Platform::PlatformAdaptor::requestPermissionsSync()
{
}


void Platform::PlatformAdaptor::vibrateBrief()
{
}


void Platform::PlatformAdaptor::vibrateLong()
{
}


//
// Private slots
//

void Platform::PlatformAdaptor::onNetworkStateChanged(uint x)
{
    // x >= 50 means that a new connection has come up.
    if (x >= 50)
    {
        emit wifiConnected();
    }
}

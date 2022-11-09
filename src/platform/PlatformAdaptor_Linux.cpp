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

#include <QDBusArgument>
#include <QDebug>
#include <QThread>

Platform::PlatformAdaptor::PlatformAdaptor(QObject *parent)
    : PlatformAdaptor_Abstract(parent)
{
    connect(&networkManagerInterface, SIGNAL(StateChanged(uint)), this, SLOT(onNetworkStateChanged(uint)));

    // get the interface to nm
    QDBusInterface nm("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
            "org.freedesktop.NetworkManager", QDBusConnection::systemBus());
    if(!nm.isValid())
    {
        qFatal("Failed to connect to the system bus");
    }

    // get all devices
    qWarning() << "GetDevices reply: " << nm.call("PrimaryConnection") << Qt::endl;

    QDBusMessage msg = nm.call("GetDevices");
    qWarning() << "GetDevices reply: " << msg << Qt::endl;
    QDBusArgument arg = msg.arguments().at(0).value<QDBusArgument>();

    if(arg.currentType() != QDBusArgument::ArrayType)
    {
        qFatal("Something went wrong with getting the device list");
    }
    QList<QDBusObjectPath> pathsLst = qdbus_cast<QList<QDBusObjectPath> >(arg);
    foreach(QDBusObjectPath p, pathsLst)
    {
        qWarning() << "DEV PATH: " << p.path();
        // creating an interface used to gather this devices properties
        QDBusInterface device("org.freedesktop.NetworkManager", p.path(),
        "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus());
        // 2 is WiFi dev, see https://people.freedesktop.org/~lkundrak/nm-docs/nm-dbus-types.html#NMDeviceType
        if (device.property("DeviceType").toInt() != 2)
        {
            continue;
        }
        // we got a wifi device, let's get an according dbus interface
        QDBusInterface wifi_device("org.freedesktop.NetworkManager", p.path(),
        "org.freedesktop.NetworkManager.Device.Wireless", QDBusConnection::systemBus());
        // we need to call scan on the inteface prior to request the list of interfaces
        QMap<QString, QVariant> argList;
        QDBusMessage msg = wifi_device.call("RequestScan", argList);
        QThread::sleep(2); // not the best solution, but here we just wait for the scan

        // doing the actual call
        msg = wifi_device.call("GetAllAccessPoints");
        qWarning()<< "Answer for GetAllAccessPoints:  " << msg << Qt::endl << Qt::endl;
        // dig out the paths of the Access Point objects:
        QDBusArgument ap_list_arg = msg.arguments().at(0).value<QDBusArgument>();
        QList<QDBusObjectPath> ap_path_list = qdbus_cast<QList<QDBusObjectPath> >(ap_list_arg);
        // and iterate through the list
        foreach(QDBusObjectPath p ,ap_path_list)
        {
            // for each Access Point we create an interface
            QDBusInterface ap_interface("org.freedesktop.NetworkManager", p.path(),
            "org.freedesktop.NetworkManager.AccessPoint", QDBusConnection::systemBus());
            // and getting the name of the SSID
            qWarning() << "SSID: " << ap_interface.property("Ssid").toString();
        }
    }

}


//
// Methods
//

auto Platform::PlatformAdaptor::currentSSID() -> QString
{
#warning Want to implement
    return QStringLiteral("<unknown ssid>");
}


void Platform::PlatformAdaptor::disableScreenSaver()
{
}


auto Platform::PlatformAdaptor::hasRequiredPermissions() -> bool
{
    return true;
}


void Platform::PlatformAdaptor::lockWifi(bool lock)
{
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


//
// Private slots
//

void Platform::PlatformAdaptor::onNetworkStateChanged(uint x)
{
    if (x >= 50)
    {
        emit wifiConnected();
    }
}

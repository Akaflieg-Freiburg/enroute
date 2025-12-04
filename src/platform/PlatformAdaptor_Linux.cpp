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

#include <QSocketNotifier>
#include <libudev.h>

#include "platform/PlatformAdaptor_Linux.h"


Platform::PlatformAdaptor::PlatformAdaptor(QObject *parent)
    : PlatformAdaptor_Abstract(parent)
{
    connect(&networkManagerInterface, SIGNAL(StateChanged(uint)), this, SLOT(onNetworkStateChanged(uint)));

    // Watch changes of serial ports
    udev_ = udev_new();
    if (udev_)
    {
        mon_ = udev_monitor_new_from_netlink(udev_, "udev");
        if (mon_)
        {
            // Watch TTY devices (covers /dev/ttyUSB*, /dev/ttyACM*, etc.)
            udev_monitor_filter_add_match_subsystem_devtype(mon_, "tty", nullptr);
            udev_monitor_enable_receiving(mon_);

            int fd = udev_monitor_get_fd(mon_);
            notifier_ = new QSocketNotifier(fd, QSocketNotifier::Read, this);
            connect(notifier_, &QSocketNotifier::activated, this, [this]() {
                udev_device *dev = udev_monitor_receive_device(mon_);
                if (!dev)
                {
                    return;
                }
//                const char *action  = udev_device_get_action(dev);   // "add" / "remove"
//                const char *devNode = udev_device_get_devnode(dev);  // e.g. "/dev/ttyUSB0"
//                qWarning() << "Serial port activity" << action << devNode;
                qWarning() << "Serial port activity";
                emit serialPortsChanged();


                udev_device_unref(dev);

            });
        }
    }
}


Platform::PlatformAdaptor::~PlatformAdaptor()
{
    // Delete serial port monitors and watchers
    if (notifier_)
    {
        delete notifier_;
    }
    if (mon_)
    {
        udev_monitor_unref(mon_);
    }
    if (udev_)
    {
        udev_unref(udev_);
    }
}


//
// Methods
//

auto Platform::PlatformAdaptor::currentSSID() -> QString
{
    // get primary connection devices
    auto primaryConnection = networkManagerInterface.property("PrimaryConnection").value<QDBusObjectPath>();
    QDBusInterface const
        primaryConnectionInterface(QStringLiteral("org.freedesktop.NetworkManager"),
                                   primaryConnection.path(),
                                   QStringLiteral(
                                       "org.freedesktop.NetworkManager.Connection.Active"),
                                   QDBusConnection::systemBus());
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

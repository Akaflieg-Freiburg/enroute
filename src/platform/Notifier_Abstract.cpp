/***************************************************************************
 *   Copyright (C) 2021-2022 by Stefan Kebekus                             *
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

#include <QGuiApplication>

#include "platform/Notifier_Abstract.h"


Platform::Notifier_Abstract::Notifier_Abstract(QObject *parent)
    : GlobalObject(parent)
{

    connect(qGuiApp, &QGuiApplication::applicationStateChanged, this,
            [this](Qt::ApplicationState state)
    {
        if (state == Qt::ApplicationSuspended)
        {
            hideAll();
        }
    });
}


void Platform::Notifier_Abstract::hideAll()
{
    hideNotification(DownloadInfo);
    hideNotification(TrafficReceiverSelfTestError);
    hideNotification(TrafficReceiverRuntimeError);
    hideNotification(GeoMapUpdatePending);
}


auto Platform::Notifier_Abstract::title(Platform::Notifier_Abstract::NotificationTypes notification) -> QString
{
    switch (notification)
    {
    case DownloadInfo:
        return tr("Downloading map and data…");
    case TrafficReceiverRuntimeError:
        return tr("Traffic data receiver problem");
    case TrafficReceiverSelfTestError:
        return tr("Traffic data receiver self test error");
    case GeoMapUpdatePending:
        return tr("Map and data updates available");
    }

    return {};
}


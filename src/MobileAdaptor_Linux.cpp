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

#include <QCoreApplication>
#include <QDir>
#include <QPointer>
#include <QStandardPaths>
#include <QTimer>

#include "Global.h"
#include "MobileAdaptor.h"
#include "geomaps/GeoMapProvider.h"
#include "traffic/TrafficDataProvider.h"


void MobileAdaptor::hideSplashScreen()
{
}


void MobileAdaptor::lockWifi(bool lock)
{
    Q_UNUSED(lock)
}


Q_INVOKABLE auto MobileAdaptor::missingPermissionsExist() -> bool
{
    return false;
}


void MobileAdaptor::vibrateBrief()
{
}


auto MobileAdaptor::getSSID() -> QString
{
    return "<unknown ssid>";
}


void MobileAdaptor::showDownloadNotification(bool show)
{

    if (show) {
        if (downloadNotification.isNull()) {
            downloadNotification = new KNotification(QStringLiteral("downloading"), KNotification::Persistent, this);
            downloadNotification->setPixmap( {":/icons/appIcon.png"} );
            downloadNotification->setText(tr("Downloading map data…"));
        }
        downloadNotification->sendEvent();
    } else {
        if (!downloadNotification.isNull()) {
            downloadNotification->close();
        }
    }

}


void MobileAdaptor::showTrafficReceiverErrorNotification(QString message)
{

    if (message.isEmpty()) {
        if (!trafficReceiverErrorNotification.isNull()) {
            trafficReceiverErrorNotification->close();
        }
        delete trafficReceiverErrorNotification;
        return;
    }

    if (trafficReceiverErrorNotification.isNull()) {
        trafficReceiverErrorNotification = new KNotification(QStringLiteral("trafficReceiverProblem"), KNotification::Persistent, this);
        trafficReceiverErrorNotification->setPixmap( {":/icons/appIcon.png"} );
        trafficReceiverErrorNotification->setTitle(tr("Traffic Receiver Problem"));
    }

    trafficReceiverErrorNotification->setText(message);
    trafficReceiverErrorNotification->sendEvent();

}

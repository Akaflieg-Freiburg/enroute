/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#include <QQmlEngine>

#include "traffic/TrafficDataSource_Abstract.h"


// Member functions

Traffic::TrafficDataSource_Abstract::TrafficDataSource_Abstract(QObject *parent) : QObject(parent) {

    QQmlEngine::setObjectOwnership(&m_factor, QQmlEngine::CppOwnership);

    // Setup heartbeat timer
    m_heartbeatTimer.setSingleShot(true);
    m_heartbeatTimer.setInterval(5s);
    connect(&m_heartbeatTimer, &QTimer::timeout, this, &Traffic::TrafficDataSource_Abstract::resetReceivingHeartbeat);

    // Setup other times
    m_pressureAltitudeTimer.setInterval(5s);
    m_pressureAltitudeTimer.setSingleShot(true);
    m_trueAltitudeTimer.setInterval(5s);
    m_trueAltitudeTimer.setSingleShot(true);

}


void Traffic::TrafficDataSource_Abstract::setConnectivityStatus(const QString& newConnectivityStatus)
{
    if (m_connectivityStatus == newConnectivityStatus) {
        return;
    }

    m_connectivityStatus = newConnectivityStatus;
    emit connectivityStatusChanged(m_connectivityStatus);
}


void Traffic::TrafficDataSource_Abstract::setErrorString(const QString& newErrorString)
{
    if (m_errorString == newErrorString) {
        return;
    }

    m_errorString = newErrorString;
    emit errorStringChanged(m_errorString);
}


void Traffic::TrafficDataSource_Abstract::setReceivingHeartbeat(bool newReceivingHeartbeat)
{
    if (newReceivingHeartbeat) {
        m_heartbeatTimer.start();
    } else {
        m_heartbeatTimer.stop();
    }

    if (m_hasHeartbeat == newReceivingHeartbeat) {
        return;
    }
    m_hasHeartbeat = newReceivingHeartbeat;
    emit receivingHeartbeatChanged(m_hasHeartbeat);
}


void Traffic::TrafficDataSource_Abstract::resetReceivingHeartbeat()
{
    setReceivingHeartbeat(false);
}



#include <notification/Notification.h>
#include <notification/NotificationManager.h>

void Traffic::TrafficDataSource_Abstract::setTrafficReceiverRuntimeError(const QString &newErrorString)
{
    if (m_trafficReceiverRuntimeError == newErrorString) {
        return;
    }

    m_trafficReceiverRuntimeError = newErrorString;
    emit trafficReceiverRuntimeErrorChanged(newErrorString);

#warning Does this belong here?
    if (m_trafficReceiverRuntimeError.isEmpty())
    {
        return;
    }
    auto* notification = new Notifications::Notification(this);
    notification->setTitle(tr("Traffic data receiver problem"));
    notification->setText(m_trafficReceiverRuntimeError);
    notification->setButton1Text(tr("Dismiss"));
    notification->setButton2Text({});
    notification->setImportance(Notifications::Notification::Warning);
    connect(this, &TrafficDataSource_Abstract::trafficReceiverRuntimeErrorChanged, notification, &QObject::deleteLater);
    GlobalObject::notificationManager()->add(notification);
}


void Traffic::TrafficDataSource_Abstract::setTrafficReceiverSelfTestError(const QString &newErrorString)
{
    if (m_trafficReceiverSelfTestError == newErrorString) {
        return;
    }

    m_trafficReceiverSelfTestError = newErrorString;
    emit trafficReceiverSelfTestErrorChanged(newErrorString);

#warning Does this belong here?
    if (m_trafficReceiverSelfTestError.isEmpty())
    {
        return;
    }
    auto* notification = new Notifications::Notification(this);
    notification->setTitle(tr("Traffic data receiver self test error"));
    notification->setText(m_trafficReceiverSelfTestError);
    notification->setButton1Text(tr("Dismiss"));
    notification->setButton2Text({});
    notification->setImportance(Notifications::Notification::Warning);
    connect(this, &TrafficDataSource_Abstract::trafficReceiverSelfTestErrorChanged, notification, &QObject::deleteLater);
    GlobalObject::notificationManager()->add(notification);
}

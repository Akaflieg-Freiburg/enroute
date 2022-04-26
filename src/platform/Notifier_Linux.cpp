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

#include <QApplication>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QImage>

#include "dataManagement/DataManager.h"
#include "GlobalObject.h"
#include "platform/Notifier_Linux.h"


// The freedesktop specification for icons require that icons be sent via DBus in the following structure.
// In order to be used in QDBus, the structure needs to be registred via qDBusRegisterMetaType, which in turn requires that
//
// * it is declared via Q_DECLARE_METATYPE
//
// * operators << and >> are defined that read/write from QDBusArguments
//
// see https://specifications.freedesktop.org/notification-spec/latest/ar01s08.html

struct FreeDesktopImageStructure {
    int width;
    int height;
    int rowStride;
    bool hasAlpha;
    int bitsPerSample;
    int channels;
    QByteArray data;
};
Q_DECLARE_METATYPE(FreeDesktopImageStructure)

QDBusArgument& operator<<(QDBusArgument& argument, const FreeDesktopImageStructure& fdImage)
{
    argument.beginStructure();
    argument << fdImage.width
             << fdImage.height
             << fdImage.rowStride
             << fdImage.hasAlpha
             << fdImage.bitsPerSample
             << fdImage.channels
             << fdImage.data;
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument &argument, FreeDesktopImageStructure& fdImage)
{
    argument.beginStructure();
    argument >> fdImage.width
            >> fdImage.height
            >> fdImage.rowStride
            >> fdImage.hasAlpha
            >> fdImage.bitsPerSample
            >> fdImage.channels
            >> fdImage.data;
    argument.endStructure();
    return argument;
}


Platform::Notifier_Linux::Notifier_Linux(QObject *parent)
    : Platform::Notifier_Linux::Notifier(parent)
{
    connect(&notificationInterface, SIGNAL(ActionInvoked(uint,QString)), this, SLOT(onActionInvoked(uint,QString)));
    connect(&notificationInterface, SIGNAL(NotificationClosed(uint,uint)), this, SLOT(onNotificationClosed(uint,uint)));

    // Construct iconVariant, which contains the app icon in a format that can be transferred via QDBus
    QImage icon(":/icons/appIcon.png");
    QImage image = icon.convertToFormat(QImage::Format_RGBA8888);
    QByteArray data((const char *)image.constBits(), image.sizeInBytes());
    qDBusRegisterMetaType<FreeDesktopImageStructure>();
    FreeDesktopImageStructure specImage;
    specImage.width = image.width();
    specImage.height = image.height();
    specImage.rowStride = image.bytesPerLine();
    specImage.hasAlpha = true;
    specImage.bitsPerSample = 8;
    specImage.channels = 4;
    specImage.data = data;
    iconVariant = QVariant::fromValue(specImage);
}


Platform::Notifier_Linux::~Notifier_Linux()
{
    foreach(auto notificationID, notificationIDs) {
        if (notificationID == 0) {
            continue;
        }
        notificationInterface.call("CloseNotification", notificationID);
    }
}


void Platform::Notifier_Linux::hideNotification(Platform::Notifier::NotificationTypes notificationType)
{

    auto notificationID = notificationIDs.value(notificationType, 0);
    if (notificationID == 0) {
        return;
    }

    notificationInterface.call("CloseNotification", notificationID);
    notificationIDs.remove(notificationType);

}


void Platform::Notifier_Linux::onActionInvoked(uint, QString key)
{

    if (key == "GeoMap_Dismiss") {
        // The method onNotificationClosed will later be called. The following line ensures
        // that the signal notificationClicked() is then no longer emitted.
        notificationIDs.remove(GeoMapUpdatePending);
    }
    if (key == "GeoMap_Update") {
        emit action(GeoMapUpdatePending_UpdateRequested);

        // The method onNotificationClosed will later be called. The following line ensures
        // that the signal notificationClicked() is then no longer emitted.
        notificationIDs.remove(GeoMapUpdatePending);
    }

}


void Platform::Notifier_Linux::onNotificationClosed(uint id, uint reason)
{
    // reason == 2 means: notification is closed because user clicked on it
    if (reason != 2) {
        return;
    }

    // Find notification type and emit signal
    NotificationTypes type;
    bool haveType = false;
    QMap<NotificationTypes, uint>::const_iterator i = notificationIDs.constBegin();
    while (i != notificationIDs.constEnd()) {
        if (i.value() == id) {
            // We do not emit here because that will change the QMap while
            // we iterate over it -> crash
            haveType = true;
            type = i.key();
        }
        i++;
    }
    if (haveType) {
        switch (type) {
        case DownloadInfo:
            emit action(DownloadInfo_Clicked);
            break;
        case TrafficReceiverSelfTestError:
            emit action(TrafficReceiverSelfTestError_Clicked);
            break;
        case TrafficReceiverRuntimeError:
            emit action(TrafficReceiverRuntimeError_Clicked);
            break;
        case GeoMapUpdatePending:
            emit action(GeoMapUpdatePending_Clicked);
            break;
        }
    }
}


void Platform::Notifier_Linux::showNotification(NotificationTypes notificationType, const QString& text, const QString& longText)
{
    Q_UNUSED(text)
    Q_UNUSED(longText)

    // Close pending notification, if any
    hideNotification(notificationType);

    QStringList actions;
    if (notificationType == GeoMapUpdatePending) {
        actions << "GeoMap_Update" << tr("Update");
        actions << "GeoMap_Dismiss" << tr("Dismiss");
    }
    QMap<QString,QVariant> hints;
    hints["image-data"] = iconVariant;
    QDBusReply<uint> reply = notificationInterface.call("Notify",
                                                        "Enroute Flight Navigation",
                                                        notificationIDs.value(notificationType, 0), // Notification to replace. 0 means: do not replace
                                                        "", // Icon
                                                        "Enroute Flight Navigation", // Title
                                                        title(notificationType), // Summary
                                                        actions, // actions_list
                                                        hints, // QMap<QString,QVariant>(), // hint
                                                        0); // time: 0 means: never expire.
    if (reply.isValid()) {
        notificationIDs.insert(notificationType, reply.value());
    }

}

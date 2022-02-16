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

#include <QAndroidJniEnvironment>
#include <QCoreApplication>
#include <QtAndroidExtras/QAndroidJniObject>

#include "GlobalObject.h"
#include "platform/Notifier_Android.h"


Platform::Notifier_Android::Notifier_Android(QObject *parent)
    : Platform::Notifier(parent)
{
    ;
}


Platform::Notifier_Android::~Notifier_Android()
{
    ;
}


void Platform::Notifier_Android::hideNotification(Platform::Notifier::NotificationTypes notificationType)
{
    jint jni_ID                   = notificationType;
    QAndroidJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/Notifier", "hideNotification", "(I)V", jni_ID);
}


void Platform::Notifier_Android::onNotificationClicked(Platform::Notifier::NotificationTypes notificationType, int actionID)
{
    hideNotification(notificationType);
    switch (notificationType) {
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
        if (actionID == 0) {
            emit action(GeoMapUpdatePending_Clicked);
        } else {
            emit action(GeoMapUpdatePending_UpdateRequested);
        }
        break;
    }
}


void Platform::Notifier_Android::showNotification(Platform::Notifier::NotificationTypes notificationType, const QString& text, const QString& longText)
{
    jint jni_ID                    = notificationType;
    QAndroidJniObject jni_title    = QAndroidJniObject::fromString(title(notificationType));
    QAndroidJniObject jni_text     = QAndroidJniObject::fromString(text);
    QAndroidJniObject jni_longText = QAndroidJniObject::fromString(longText);
    QAndroidJniObject jni_actionText;
    if (notificationType == GeoMapUpdatePending) {
        jni_actionText = QAndroidJniObject::fromString(tr("Update"));
    }

    QAndroidJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/Notifier",
                                              "showNotification",
                                              "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                                              jni_ID,
                                              jni_title.object<jstring>(),
                                              jni_text.object<jstring>(),
                                              jni_longText.object<jstring>(),
                                              jni_actionText.object<jstring>()
                                              );

}


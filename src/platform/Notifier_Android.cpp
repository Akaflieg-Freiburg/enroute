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
#include <QJniEnvironment>
#include <QJniObject>

#include "GlobalObject.h"
#include "platform/Notifier_Android.h"


Platform::Notifier::Notifier(QObject *parent)
    : Platform::Notifier_Abstract(parent)
{
    ;
}


Platform::Notifier::~Notifier()
{
    ;
}


void Platform::Notifier::hideNotification(Platform::Notifier_Abstract::NotificationTypes notificationType)
{
    jint jni_ID                   = notificationType;
    QJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/Notifier", "hideNotification", "(I)V", jni_ID);
}


void Platform::Notifier::onNotificationClicked(Platform::Notifier_Abstract::NotificationTypes notificationType, int actionID)
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


void Platform::Notifier::showNotification(Platform::Notifier_Abstract::NotificationTypes notificationType, const QString& text, const QString& longText)
{

    jint jni_ID                    = notificationType;
    QJniObject jni_title    = QJniObject::fromString(title(notificationType));
    QJniObject jni_text     = QJniObject::fromString(text);
    QJniObject jni_longText = QJniObject::fromString(longText);
    QJniObject jni_actionText;
    if (notificationType == GeoMapUpdatePending) {
        jni_actionText = QJniObject::fromString(tr("Update"));
    }

    QJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/Notifier",
                                              "showNotification",
                                              "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                                              jni_ID,
                                              jni_title.object<jstring>(),
                                              jni_text.object<jstring>(),
                                              jni_longText.object<jstring>(),
                                              jni_actionText.object<jstring>()
                                              );

}



//
// C Methods
//

extern "C" {

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_MobileAdaptor_onNotificationClicked(JNIEnv* /*unused*/, jobject /*unused*/, jint notifyID, jint actionID)
{
    // This method is called from Java to indicate that the user has clicked into the Android
    // notification for reporting traffic data receiver errors

    // This method gets called from Java before main() has executed
    // and thus before a QApplication instance has been constructed.
    // In these cases, the methods of the Global class must not be called
    // and we simply return.
    if (!GlobalObject::canConstruct())
    {
        return;
    }
    auto* ptr = qobject_cast<Platform::Notifier*>(GlobalObject::notifier());

    if (ptr == nullptr)
    {
        return;
    }
    ptr->onNotificationClicked((Platform::Notifier_Abstract::NotificationTypes)notifyID, actionID);
}

}

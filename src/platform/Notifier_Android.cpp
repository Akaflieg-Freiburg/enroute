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
#include "platform/Notifier.h"


Platform::Notifier::Notifier(QObject *parent)
    : QObject(parent)
{
    ;
}


Platform::Notifier::~Notifier()
{
    ;
}


void Platform::Notifier::hideNotification(Platform::Notifier::NotificationTypes notificationType)
{
    jint jni_ID                   = notificationType;
//    QAndroidJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "hideNotification", "(I)V", jni_ID);
    QAndroidJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/Notifier", "hideNotification", "(I)V", jni_ID);
}


void Platform::Notifier::showNotification(Platform::Notifier::NotificationTypes notificationType, const QString& text, const QString& longText)
{
    jint jni_ID                    = notificationType;
    QAndroidJniObject jni_title    = QAndroidJniObject::fromString(title(notificationType));
    QAndroidJniObject jni_text     = QAndroidJniObject::fromString(text);
    QAndroidJniObject jni_longText = QAndroidJniObject::fromString(longText);

    QAndroidJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/Notifier",
                                              "showNotification",
                                              "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
                                              jni_ID,
                                              jni_title.object<jstring>(),
                                              jni_text.object<jstring>(),
                                              jni_longText.object<jstring>()
                                              );

}


/***************************************************************************
 *   Copyright (C) 2020 by Johannes Zellner                                *
 *   johannes@zellner.org                                                  *
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


#include "MobileAdaptor.h"

#include <QDebug>
#include <QDateTime>
#include <QFile>

#if defined(Q_OS_ANDROID)
#include <QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>
#include <QAndroidJniEnvironment>
#endif


bool MobileAdaptor::sendContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate)
{
    Q_UNUSED(content)
    Q_UNUSED(mimeType)
    Q_UNUSED(fileNameTemplate)

#if defined(Q_OS_ANDROID)
  QString tmpPath = contentToTempFile(content, fileNameTemplate);
  return outgoingIntent("sendFile", tmpPath, mimeType);
#endif
  return false;
#warning not defined for desktop
}


bool MobileAdaptor::viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate)
{
    Q_UNUSED(content)
    Q_UNUSED(mimeType)
    Q_UNUSED(fileNameTemplate)

#if defined(Q_OS_ANDROID)
  QString tmpPath = contentToTempFile(content, fileNameTemplate);
  return outgoingIntent("viewFile", tmpPath, mimeType);
#endif
#warning not defined for desktop
  return false;
}


#if defined(Q_OS_ANDROID)
QString MobileAdaptor::contentToTempFile(const QByteArray& content, const QString& fileNameTemplate)
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    QString fname = fileNameTemplate.arg(now.toString("yyyy-MM-dd_hh.mm.ss"));

    // in Qt, resources are not stored absolute file paths, so in order to
    // share the content we save it to disk. We save these temporary files
    // when creating new Share objects.
    //
    auto filePath = androidExchangeDirectoryName + fname;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).debug() << "Share::share " << filePath;
        return nullptr;
    }

    file.write(content);
    file.close();

    return filePath;
}


bool MobileAdaptor::outgoingIntent(const QString& methodName, const QString& filePath, const QString& mimeType)
{
    if (filePath == nullptr)
        return false;

    QAndroidJniObject jsPath = QAndroidJniObject::fromString(filePath);
    QAndroidJniObject jsMimeType = QAndroidJniObject::fromString(mimeType);
    auto ok = QAndroidJniObject::callStaticMethod<jboolean>(
        "de/akaflieg_freiburg/enroute/IntentLauncher",
        methodName.toStdString().c_str(),
        "(Ljava/lang/String;Ljava/lang/String;)Z",
        jsPath.object<jstring>(),
        jsMimeType.object<jstring>());
    return ok;
}
#endif

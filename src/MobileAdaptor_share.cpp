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


void MobileAdaptor::sendContent(const QString& content, const QString& mimeType, const QString& suffix)
{
  qWarning() << "MobileAdaptor::sendContent(" << content << ", " << mimeType << ", " << suffix << ")";

  QString tmpPath = contentToTempFile(content, suffix);
  outgoingIntent("sendFile", tmpPath, mimeType);
}

// helper method which saves content to temporary file in the app's private cache dir
//
QString MobileAdaptor::contentToTempFile(const QString& content, const QString& suffix)
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    QString fname = now.toString("yyyy-MM-dd_hh.mm.ss") + "." + suffix;

    // in Qt, resources are not stored absolute file paths, so in order to
    // share the content we save it to disk. We save these temporary files
    // when creating new Share objects.
    //
    auto filePath = androidExchangeDirectoryName + fname;
    qWarning() << "filePath" << filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).debug() << "Share::share " << filePath;
        return nullptr;
    }

    QTextStream out(&file);
    out << content;
    out.flush();
    file.close();

    return filePath;
}



void MobileAdaptor::outgoingIntent(const QString& methodName, const QString& filePath, const QString& mimeType)
{
    if (filePath == nullptr)
        return;

    QAndroidJniObject jsPath = QAndroidJniObject::fromString(filePath);
    QAndroidJniObject jsMimeType = QAndroidJniObject::fromString(mimeType);
    jboolean ok = QAndroidJniObject::callStaticMethod<jboolean>(
        "de/akaflieg_freiburg/enroute/IntentLauncher",
        methodName.toStdString().c_str(),
        "(Ljava/lang/String;Ljava/lang/String;)Z",
        jsPath.object<jstring>(),
        jsMimeType.object<jstring>());
    if (!ok) {
        qWarning() << "Unable to resolve activity from Java";
#warning XXX
//        emit shareNoAppAvailable();
    }
}

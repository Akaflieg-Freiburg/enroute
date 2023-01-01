/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "GlobalSettings.h"
#include "dataManagement/SSLErrorHandler.h"


DataManagement::SSLErrorHandler::SSLErrorHandler(QObject *parent) :
    GlobalObject(parent)
{
    ;
}


void DataManagement::SSLErrorHandler::deferredInitialization()
{
    QObject::connect(GlobalObject::networkAccessManager(), &QNetworkAccessManager::sslErrors,
                     this, &DataManagement::SSLErrorHandler::onSSLError);
}


void DataManagement::SSLErrorHandler::onSSLError(QNetworkReply *reply, const QList<QSslError> &errors)
{

    if (GlobalObject::globalSettings()->ignoreSSLProblems()) {
        if (reply != nullptr) {
            reply->ignoreSslErrors();
        }
        return;
    }


    QString result;
    result += "<p>" +
              tr("Enroute is unable to establish a secure internet connection to one or several servers.") +
              "</p>";
    result += QLatin1String("<ul style='margin-left:-25px;'>");
    foreach(auto error, errors) {
        result += "<li>" + error.errorString() + "</li>";
    }
    result += QLatin1String("</ul>");
    result += "<p>" +
              tr("You can choose to ignore this warning in the future and to connect anyway. "
                 "This will however leave the data transfer open to tampering and manipulation.") +
              "</p>";
    result += "<p>" +
              tr("On older Android devices, the problem is most likely "
                 "caused by outdated security certificates in your system.  Certificates "
                 "can only be installed by the hardware manufacturer via system updates. "
                 "If your device has not received any system security updates in a while, then secure "
                 "internet connections are no longer possible. "
                 "<a href='https://akaflieg-freiburg.github.io/enrouteText/manual/03-reference/platform.html#network-security-problems'>See "
                 "the platform notes in the manual</a> for more details.") +
              "</p>";
    result += "<p>" +
              tr("On recent devices, this problem is a strong indication that your "
                 "internet connection is being manipulated.") +
              "</p>";

    emit sslError(result);
}

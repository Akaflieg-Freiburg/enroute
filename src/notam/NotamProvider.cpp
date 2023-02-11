/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "notam/NotamProvider.h"


NOTAM::NotamProvider::NotamProvider(QObject* parent) : GlobalObject(parent)
{

}


void NOTAM::NotamProvider::deferredInitialization()
{

}


NOTAM::NotamList NOTAM::NotamProvider::notams(const QString& icaoLocation)
{
    NotamList notamList;

    QFile jsonFile("/home/kebekus/Austausch/notams-response.json");
    jsonFile.open(QIODeviceBase::ReadOnly);

    auto doc = QJsonDocument::fromJson(jsonFile.readAll());
    auto items = doc["items"].toArray();

    foreach(auto item, items)
    {
        Notam notam;
        notam.read(item.toObject());

        if (notam.m_icaoLocation != icaoLocation)
        {
            continue;
        }
        notamList.m_notams.append(notam);
    }

    notamList.m_retrieved = QDateTime::currentDateTimeUtc();

    return notamList;
}

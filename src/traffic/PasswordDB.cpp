/***************************************************************************
 *   Copyright (C) 2020-2021 by Stefan Kebekus                             *
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

#include "traffic/PasswordDB.h"

#include <QDataStream>
#include <QFile>
#include <QStandardPaths>


Traffic::PasswordDB::PasswordDB(QObject* parent) : QObject(parent)
{
    passwordDBFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "trafficDataReceiverPasswordDB.dat";

    auto passwordFile = QFile(passwordDBFileName);
    if (!passwordFile.open(QIODevice::ReadOnly)) {
        return;
    }
    auto inputStream = QDataStream(&passwordFile);

    QHash<QString, QString> tmp;
    inputStream >> tmp;
    if (inputStream.status() != QDataStream::Ok) {
        return;
    }
    if (passwordFile.error() != QFileDevice::NoError) {
        return;
    }

    passwordFile.close();
    m_passwordDB = tmp;
    updateEmpty();
}


void Traffic::PasswordDB::clear()
{
    m_passwordDB.clear();
    save();
    updateEmpty();
}


void Traffic::PasswordDB::removePassword(const QString& key)
{
    if (!m_passwordDB.contains(key)) {
        return;
    }
    m_passwordDB.remove(key);
    save();
    updateEmpty();
}


void Traffic::PasswordDB::save()
{
    auto passwordDBFile = QFile(passwordDBFileName);
    if (!passwordDBFile.open(QIODevice::WriteOnly)) {
        return;
    }
    if (passwordDBFile.error() != QFileDevice::NoError) {
        return;
    }

    auto outputStream = QDataStream(&passwordDBFile);
    outputStream << m_passwordDB;
    passwordDBFile.close();

}


void Traffic::PasswordDB::setPassword(const QString& key, const QString& password)
{

    if (m_passwordDB.contains(key) && (m_passwordDB.value(key) == password)) {
        return;
    }

    m_passwordDB[key] = password;
    save();
    updateEmpty();

}


void Traffic::PasswordDB::updateEmpty()
{

    if (m_passwordDB.isEmpty() == m_empty) {
        return;
    }

    m_empty = m_passwordDB.isEmpty();
    emit emptyChanged();

}

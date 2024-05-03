/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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

#include "traffic/ConnectionScanner_Abstract.h"


Traffic::ConnectionScanner_Abstract::ConnectionScanner_Abstract(QObject* parent)
    : QObject(parent)
{
}


//
// Getter functions
//

void Traffic::ConnectionScanner_Abstract::setDevices(const QList<Traffic::ConnectionInfo>& newConnectionInfos)
{
    if (newConnectionInfos == m_connectionInfos)
    {
        return;
    }
    m_connectionInfos = newConnectionInfos;
    emit connectionInfosChanged();
}

void Traffic::ConnectionScanner_Abstract::setError(const QString& newError)
{
    if (newError == m_error)
    {
        return;
    }
    m_error = newError;
    emit errorChanged();
}

void Traffic::ConnectionScanner_Abstract::setIsScanning(bool newScanning)
{
    if (newScanning == m_isScanning)
    {
        return;
    }
    m_isScanning = newScanning;
    emit scanningChanged();
}

/***************************************************************************
 *   Copyright (C) 2021-2025 by Stefan Kebekus                             *
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

#if __has_include(<QSerialPortInfo>)
#include <QSerialPortInfo>
#endif
#include "GlobalObject.h"
#include "platform/PlatformAdaptor.h"
#include "traffic/ConnectionScanner_SerialPort.h"


// Member functions

Traffic::ConnectionScanner_SerialPort::ConnectionScanner_SerialPort(QObject* parent)
    : ConnectionScanner_Abstract(parent)
{
    connect(GlobalObject::platformAdaptor(), &Platform::PlatformAdaptor::serialPortsChanged, this, &Traffic::ConnectionScanner_SerialPort::start);
}

void Traffic::ConnectionScanner_SerialPort::start()
{
    setDevices(GlobalObject::platformAdaptor()->serialPortConnectionInfos());
}

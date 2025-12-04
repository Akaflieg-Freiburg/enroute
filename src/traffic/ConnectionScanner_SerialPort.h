/***************************************************************************
 *   Copyright (C) 2025 by Stefan Kebekus                                  *
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

#pragma once

#include <QQmlEngine>

#include "traffic/ConnectionScanner_Abstract.h"


namespace Traffic {

/*! \brief Connection Scanner: SerialPort Devices
 *
 *  This class discovers all serial ports available. If a serial port has a
 *  description (such as "ublox 7 - GPS GNSS Receiver") then the port will be
 *  reported twice, once by port name and once by description.
 */
class ConnectionScanner_SerialPort : public ConnectionScanner_Abstract {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Properties need to be repeated, or else the Qt CMake macros cannot find them.
    Q_PROPERTY(QList<Traffic::ConnectionInfo> connectionInfos READ connectionInfos NOTIFY connectionInfosChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)

public:
    /*! \brief Constructor
     *
     *  This constructor does not start the scanning process.
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit ConnectionScanner_SerialPort(QObject* parent = nullptr);

    // No default constructor, important for QML singleton
    explicit ConnectionScanner_SerialPort() = delete;

    // factory function for QML singleton
    static Traffic::ConnectionScanner_SerialPort* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return new Traffic::ConnectionScanner_SerialPort(nullptr);
    }

public slots:
    // Re-implemented from ConnectionScanner_Abstract
    void start() override;

    // Re-implemented from ConnectionScanner_Abstract
    void stop() override {};
};

} // namespace Traffic

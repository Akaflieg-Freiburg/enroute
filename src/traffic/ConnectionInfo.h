/***************************************************************************
 *   Copyright (C) 2021-2023 by Stefan Kebekus                             *
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

#include <QBluetoothDeviceInfo>
#include <QQmlEngine>


namespace Traffic {

/*! \brief Connection to a traffic data receiver
 *
 *  This class describes a connection to a traffic data receiver.
 *  It exposes connection properties in a format that is suitable
 *  for QML.
 */
class ConnectionInfo {
    Q_GADGET
    QML_VALUE_TYPE(connectionInfo)

    friend QDataStream& operator<<(QDataStream& stream, const Traffic::ConnectionInfo &connectionInfo);
    friend QDataStream& operator>>(QDataStream& stream, Traffic::ConnectionInfo& connectionInfo);

public:
    /*! \brief Connection Type */
    enum Type {
        Invalid,            /*!< Invalid connection */
        BluetoothClassic,   /*!< Bluetooth Classic */
        BluetoothLowEnergy, /*!< Bluetooth LE */
        TCP,                /*!< TCP Connection */
        UDP,                /*!< UDP Connection */
        Serial,             /*!< Serial Port Connection */
        FLARMFile           /*!< FLARM Simulator File */
    };
    Q_ENUM(Type)

    ConnectionInfo() = default;

    explicit ConnectionInfo(const QBluetoothDeviceInfo& info, bool canonical=false);



    //
    // Properties
    //

    Q_PROPERTY(bool canConnect READ canConnect CONSTANT)
    Q_PROPERTY(bool canonical READ canonical CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(Traffic::ConnectionInfo::Type type READ type CONSTANT)



    //
    // Getter Methods
    //

    /*!
     * \brief Getter function for the property with the same name
     *
     * \returns Property canConnect
     */
    [[nodiscard]] bool canConnect() const { return m_canConnect; }

    /*!
     * \brief Getter function for the property with the same name
     *
     * \returns Property canonical
     */
    [[nodiscard]] bool canonical() const { return m_canonical; }

    /*!
     * \brief Getter function for the property with the same name
     *
     * \returns Property description
     */
    [[nodiscard]] QString description() const { return m_description; }

    /*!
     * \brief Getter function for the property with the same name
     *
     * \returns Property name
     */
    [[nodiscard]] QString name() const { return m_name; }

    /*!
     * \brief Getter function for the property with the same name
     *
     * \returns Property icon
     */
    [[nodiscard]] QString icon() const { return m_icon; }

    /*!
     * \brief Getter function for the property with the same name
     *
     * \returns Property type
     */
    [[nodiscard]] Traffic::ConnectionInfo::Type type() const { return m_type; }



    //
    // Methods
    //

    [[nodiscard]] QBluetoothDeviceInfo bluetoothDeviceInfo() const { return m_bluetoothDeviceInfo; }

    bool operator== (const ConnectionInfo& other) const;

    bool operator< (const ConnectionInfo& other) const;

private:
    //
    // Properties
    //
    bool                          m_canConnect { false };
    bool                          m_canonical { false };
    QString                       m_description {};
    QString                       m_icon { u"/icons/material/ic_delete.svg"_qs };
    QString                       m_name { QObject::tr("Invalid Device", "BTDeviceInfo") };
    Traffic::ConnectionInfo::Type m_type { Traffic::ConnectionInfo::Invalid };

    //
    // Private members, depending on m_type
    //
    QBluetoothDeviceInfo          m_bluetoothDeviceInfo {};
};

/*! \brief Serialization
 *
 *  There is no checks for errors of any kind.
 */
QDataStream& operator<<(QDataStream& stream, const Traffic::ConnectionInfo &connectionInfo);

/*! \brief Deserialization
 *
 *  There is no checks for errors of any kind.
 */
QDataStream& operator>>(QDataStream& stream, Traffic::ConnectionInfo& connectionInfo);

} // namespace Traffic

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

#pragma once

#include <QBluetoothDeviceInfo>
#include <QQmlEngine>


namespace Traffic {

/*!
 * \brief Connection to a traffic data receiver
 *
 * This class describes a connection to a traffic data receiver. It exposes
 * connection properties in a format that is suitable for QML.
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

    /*!
     * \brief Default constructor
     *
     * This method constructs an invalid ConnectionInfo
     */
    ConnectionInfo() = default;

    /*!
     * \brief Constructor for Bluetooth Device Connections
     *
     * This method constructs a ConnectionInfo for a connection to a Bluetooth
     * device. The type will either be BluetoothClassic or BluetoothLowEnergy.
     *
     * \param info QBluetoothDeviceInfo that describes the Bluetooth device
     *
     * \param canonical Property 'canonical', as described below.
     */
    explicit ConnectionInfo(const QBluetoothDeviceInfo& info, bool canonical=false);

    /*!
     * \brief Constructor for UDP Connections
     *
     * This method constructs a ConnectionInfo for a UDP connection.
     *
     * \param port Port number
     *
     * \param canonical Property 'canonical', as described below.
     */
    explicit ConnectionInfo(quint16 port, bool canonical=false);

    /*!
     * \brief Constructor for TCP Connections
     *
     * This method constructs a ConnectionInfo for a TCP connection.
     *
     * \param host Host name or IP Address
     *
     * \param port Port number
     *
     * \param canonical Property 'canonical', as described below.
     */
    explicit ConnectionInfo(const QString& host, quint16 port, bool canonical=false);



    //
    // Properties
    //

    /*!
     * \brief Connectability
     *
     * This property holds true if a connection is possible in principle. It
     * does not guarantee that a connection attempt will be successful. Example:
     * This property is 'false' for connections to Bluetooth Low Energy devices,
     * which are currently unsupported.
     */
    Q_PROPERTY(bool canConnect READ canConnect CONSTANT)

    /*!
     * \brief Canonicity
     *
     * This property holds true if the connection is canonical. Canonical
     * connections are those that will always be in the connection library and
     * cannot be edited or deleted in the GUI.
     */
    Q_PROPERTY(bool canonical READ canonical CONSTANT)

    /*!
     * \brief Description
     *
     * This property holds a human-readable, translated, two-line description of
     * the connection, in HTML format.
     */
    Q_PROPERTY(QString description READ description CONSTANT)

    /*!
     * \brief Description
     *
     * This property holds the name of an icon file that can be used to
     * represent the connection in the GUI.
     */
    Q_PROPERTY(QString icon READ icon CONSTANT)

    /*!
     * \brief Description
     *
     * This property holds a human-readable, translated name of the connection.
     */
    Q_PROPERTY(QString name READ name CONSTANT)

    /*!
     * \brief Type
     *
     * This property holds the type of the connection.
     */
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

    /*!
     * \brief Bluetooth Device Info
     *
     * \return If the connection is of type BluetoothClassic or
     * BluetoothLowEnergy, this method returns a QBluetoothDeviceInfo describing
     * the Bluetooth device. For all other connections, an invalid
     * QBluetoothDeviceInfo is returned.
     */
    [[nodiscard]] QBluetoothDeviceInfo bluetoothDeviceInfo() const { return m_bluetoothDeviceInfo; }

    /*!
     * \brief Equality of ConnectionInfos
     *
     * \param other Other ConnectionInfo to compare with.
     *
     * \return True if the ConnectionInfo describe the same connection.
     */
    [[nodiscard]] bool operator== (const Traffic::ConnectionInfo& other) const = default;

    /*!
     * \brief Port
     *
     * \return If the connection is of type UDP, this method returns
     * the port used in the UDP connection.
     */
    [[nodiscard]] quint16 port() const { return m_port; }

    /*!
     * \brief Equality of connection
     *
     * This test for equality is not strict. It returns 'true' if the two
     * connection infos describe the same connection. Still, they can differ in
     * aspects such as the connection name or description. This is possible,
     * e.g., for Bluetooth devices whose name was not know when one of the
     * ConnectionInfos was constructed.
     *
     * \param other Other ConnectionInfo to compare with.
     *
     * \return True if the ConnectionInfo describe the same connection.
     */
    [[nodiscard]] bool sameConnectionAs(const Traffic::ConnectionInfo& other) const;

    /*!
     * \brief Comparison
     *
     * This operator can be used to sort ConnectionInfos in the GUI. Sorting is
     * by connectability, type and name.
     *
     * \param other Other ConnectionInfo to compare with.
     *
     * \return True if the ConnectionInfo describe the same connection.
     */
    bool operator< (const Traffic::ConnectionInfo& other) const;

private:
    //
    // Properties
    //
    bool                          m_canConnect { false };
    bool                          m_canonical { false };
    QString                       m_description {};
    QString                       m_icon { u"/icons/material/ic_delete.svg"_qs };
    QString                       m_name { QObject::tr("Invalid Device", "Traffic::ConnectionInfo") };
    Traffic::ConnectionInfo::Type m_type { Traffic::ConnectionInfo::Invalid };

    //
    // Private members, depending on m_type
    //
    QBluetoothDeviceInfo          m_bluetoothDeviceInfo {};
    quint16                       m_port {0};
    QString                       m_host;
};

/*!
 * \brief Serialization
 *
 * There are no checks for errors of any kind.
 */
QDataStream& operator<<(QDataStream& stream, const Traffic::ConnectionInfo &connectionInfo);

/*!
 * \brief Deserialization
 *
 * There are no checks for errors of any kind.
 */
QDataStream& operator>>(QDataStream& stream, Traffic::ConnectionInfo& connectionInfo);

} // namespace Traffic

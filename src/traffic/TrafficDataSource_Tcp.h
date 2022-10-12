/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#include <QPointer>
#include <QTcpSocket>

#include "traffic/TrafficDataSource_AbstractSocket.h"


namespace Traffic {

/*! \brief Traffic receiver: TCP connection to FLARM/NMEA source
 *
 *  This class connects to a traffic receiver via a TCP connection. It expects
 *  to find a receiver at the specifed IP-Address and port that emits FLARM/NMEA
 *  sentences.
 *
 *  In most use cases, the connection will be established via the device's WiFi
 *  interface.  The class will therefore try to lock the WiFi once a heartbeat
 *  has been detected, and release the WiFi at the appropriate time.
 */

class TrafficDataSource_Tcp : public TrafficDataSource_AbstractSocket {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     *  @param hostName Name of the host where the traffic receiver is expected
     *
     *  @param port Port at the host where the traffic receiver is expected
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit TrafficDataSource_Tcp(QString hostName, quint16 port, QObject *parent = nullptr);

    // Standard destructor
    ~TrafficDataSource_Tcp() override;

    /*! \brief Getter function for the property with the same name
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     *
     *  @returns Property sourceName
     */
    [[nodiscard]] auto sourceName() const -> QString override
    {
        return tr("TCP connection to %1 port %2").arg(m_hostName).arg(m_port);
    }

public slots:
    /*! \brief Start attempt to connect to traffic receiver
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     */
    void connectToTrafficReceiver() override;

    /*! \brief Disconnect from traffic receiver
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     */
    void disconnectFromTrafficReceiver() override;

    /*! \brief Set password
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     */
    void setPassword(const QString& SSID, const QString& password) override;

private slots:
    // Read lines from the socket's text stream and passes the string on to
    // processFLARMMessage.
    void onReadyRead();

    // This method does the actual job of sending the password to the traffic
    // data receiver
    //
    // It checks if the instance is actually waiting for a password and returns
    // if it is not.
    //
    // It connects the slots updatePasswordStatusOnHeartbeatChange that will
    // respond to heartbeat (=password accepted) and
    // updatePasswordStatusOnDisconnected (=password rejected). It will send the
    // password and set passwordRequest_Status to "waitingForDevice"
    void sendPassword_internal();

    // This method set passwordRequest_Status to "idle", resets
    // passwordRequest_SSID and passwordRequest_password, and disconnects the
    // slots updatePasswordStatusOnHeartbeatChange and
    // updatePasswordStatusOnDisconnected);
    void resetPasswordLifecycle();

    // This slot is called when the password has been rejected by the traffic
    // data receiver. It clears the password from the database, schedules a
    // reconnect and calls resetPasswordLifecycle().
    void updatePasswordStatusOnDisconnected();

    // This slot is called when the password has been accepted by the traffic
    // data receiver. It emits a password storage request if appropriate and
    // calls resetPasswordLifecycle().
    void updatePasswordStatusOnHeartbeatChange(bool newHeartbeat);

private:
    QTcpSocket m_socket;
    QTextStream m_textStream;
    QString m_hostName;
    quint16 m_port;


    /* Password lifecycle
     *
     * - The method onReadyRead detects that the device requests password. It
     *   will store the current SSID in passwordRequest_SSID and set
     *   passwordRequest_Status to waitingForPassword.
     *
     * - If a password for the SSID is found in the database, the method
     *   sendPassword is called with that password.  Otherwise, the signal
     *   passwordRequest is emitted, which will hopefully lead to lead to a
     *   user-provided password through sendPassword()
     *
     * - The method send password will store the password in
     *   passwordRequest_password, send the password to the device and set
     *   passwordRequest_Status to waitingForDevice.
     *
     * - When the connection is closed while passwordRequest_Status ==
     *   waitingForDevice, this means that the traffic data receiver has
     *   rejected the password. The password stored in passwordRequest_password
     *   for passwordRequest_SSID is removed from the password database,
     *   passwordRequest_Status is set to idle, the members passwordRequest_SSID
     *   and passwordRequest_password are cleared and an immediate reconnect is
     *   scheduled.
     *
     * - When the heartbeat is received while passwordRequest_Status ==
     *   waitingForDevice, this means that the traffic data receiver has
     *   accepted the password. The instance will then emit the
     *   passwordStorageRequest. The member passwordRequest_Status is set to
     *   idle, and the members passwordRequest_SSID and passwordRequest_password
     *   are cleared.
     */
    enum {
        /*  No password-related activity is pending */
        idle,

        /*  Waiting for password
         *
         *  A password has been requested by the traffic data receiver.
         *
         *  At this stage, the member passwordRequest_SSID contains the network
         *  name of the WiFi network that the device was connected to at the
         *  time of the request.
         */
        waitingForPassword,

        /*  Waiting for device
         *
         *  A password has been sent to the traffic data receiver, and this
         *  class is now waiting for the device to respond.
         *
         *  At this stage, the member passwordRequest_SSID contains the network
         *  name of the WiFi network that the device was connected to at the
         *  time of the request. The member passwordRequest_password contains
         *  the password that has been sent to the traffic data receiver
         */
        waitingForDevice
    } passwordRequest_Status {idle};

    // See passwordRequest_Status documentation
    QString passwordRequest_SSID;

    // See passwordRequest_Status documentation
    QString passwordRequest_password;
};

} // namespace Traffic

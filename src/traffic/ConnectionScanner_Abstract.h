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

#include <QObject>

#include "traffic/ConnectionInfo.h"


namespace Traffic {

/*! \brief Connection Scanner
 *
 *  This is an abstract base class for classes that scan for potential
 *  connections to traffic data receivers and other devices.
 */
class ConnectionScanner_Abstract : public QObject {
    Q_OBJECT

public:
    /*! \brief Constructor
     *
     *  This constructor does not start the scanning process.
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit ConnectionScanner_Abstract(QObject* parent = nullptr);



    //
    // Properties
    //

    /*! \brief List of device connections found in the scanning process
     *
     *  This property is typically updated multiple times during a scan process
     */
    Q_PROPERTY(QList<Traffic::ConnectionInfo> connectionInfos READ connectionInfos NOTIFY connectionInfosChanged)

    /*! \brief Errors encountered during the scanning process
     *
     *  This property holds an empty string in case of no error.
     */
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

    /*! \brief Indicator if scan is in progress */
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)



    //
    // Properties
    //

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property connectionInfos
     */
    [[nodiscard]] QList<Traffic::ConnectionInfo> connectionInfos() const { return m_connectionInfos; }

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property error
     */
    [[nodiscard]] QString error() const { return m_error; }

    /*! \brief Getter method for property with the same name
     *
     *  @returns Property scanning
     */
    [[nodiscard]] bool scanning() const { return m_isScanning; }


public slots:
    /*! \brief Start scan process
     *
     *  This class will not start the scanning process automatically. Use this
     *  method to start.  It is safe to call this method anytime.
     */
    virtual void start() = 0;

    /*! \brief Stop scan process
     *
     *  This class will abort the running scanning process automatically. It is
     *  safe to call this method anytime.
     */
    virtual void stop() = 0;

signals:
    /*! \brief Notifier signal */
    void connectionInfosChanged();

    /*! \brief This signal to indicate whenever a device has been discovered
     *
     *  This signal is emitted during the scan process. Consumers should expect
     *  that identical connections are emitted multiple times. This signal is
     *  emitted only if the connections looks like a connections to a traffic
     *  data receiver. The final list of connections might contain elements that
     *  have not been advertised via this signal.
     */
    void connectionDiscovered(const Traffic::ConnectionInfo& info);

    /*! \brief Notifier signal */
    void errorChanged();

    /*! \brief Scan finished
     *
     *  This signal is emitted once the scan is finished and the property
     *  devices() holds the final list .
     */
    void scanFinished();

    /*! \brief Notifier signal */
    void scanningChanged();

protected:
    // Setter method for property with same name
    void setDevices(const QList<Traffic::ConnectionInfo>& newConnectionInfos);

    // Setter method for property with same name
    void setError(const QString& newError);

    // Setter method for property with same name
    void setIsScanning(bool newScanning);

private:
    // Property connectionInfos
    QList<Traffic::ConnectionInfo> m_connectionInfos;

    // Property error
    QString m_error;

    // Property isScanning
    bool m_isScanning {false};
};

} // namespace Traffic

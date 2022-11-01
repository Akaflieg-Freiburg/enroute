/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

#include "GlobalObject.h"

namespace Platform {

/*! \brief Interface to platform-specific functionality
 *
 * This pure virtual class is an interface to capabilities of mobile devices
 * (e.g. vibration) that need platform-specific code to operate.  The files
 * PlatformAdaptor_XXX.(h|cpp) implement a child class PlatformAdaptor that
 * contains the actual implementation.
 *
 * Child classes need to implement all pure virtual functions, and need to
 * provide the following additional functionality.
 *
 * - Child classes need to monitor the Wi-Fi network and emit the signal
 *   wifiConnected() whenever a new Wi-Fi connection becomes available. The app
 *   will then check if a traffic data receiver is active in the network.
 *
 * - If supported by the platform, child classes need to react to requests by
 *   the platform to open a file (e.g. a GeoJSON file containing a flight
 *   route). Once a request is received, the method processFileRequest() should
 *   be called.
 */

class PlatformAdaptor_Abstract : public GlobalObject
{
    Q_OBJECT

public:
    /*! \brief Functions and types of a file that this app handles */
    enum FileFunction
      {
        UnknownFunction, /*< Unknown file */
        FlightRouteOrWaypointLibrary, /*< File contains a flight route or a waypoint library. */
        FlightRoute, /*< File contains a flight route. */
        VectorMap, /*< File contains a vector map. */
        RasterMap, /*< File contains a raster map. */
        WaypointLibrary /*< Waypoint library in CUP or GeoJSON format */
      };
    Q_ENUM(FileFunction)

    /*! \brief Notification types */
    enum NotificationType
    {
        DownloadInfo = 0,                 /*< Info that  download is in progress */
        TrafficReceiverSelfTestError = 1, /*< Traffic receiver reports problem on self-test */
        TrafficReceiverProblem = 2        /*< Traffic receiver reports problem while running */
    };
    Q_ENUM(NotificationType)


    /*! \brief Standard constructor
     *
     * @param parent Standard QObject parent pointer
    */
    explicit PlatformAdaptor_Abstract(QObject *parent = nullptr);

    ~PlatformAdaptor_Abstract() override = default;


    //
    // Methods
    //

    /*! \brief SSID of current Wi-Fi network
     *
     * @returns The SSID of the current Wi-Fi networks, an empty string if the
     * device is not connected to a Wi-Fi or a generic string if the SSID cannot
     * be determined.
     */
    Q_INVOKABLE virtual QString currentSSID() = 0;

    /*! \brief Disable the screen saver
     *
     * On platforms that support this, this method shall disable to screen
     * saver, so that the display does not switch off automatically.  This is
     * meant to ensure that the display remains on while the app is in use (e.g.
     * while the pilot is following a non-standard traffic pattern).
     */
    virtual void disableScreenSaver() = 0;

    /*! \brief Checks if all required permissions have been granted
     *
     * Depending on the platform, the app needs to ask for permissions to
     * operate properly. This method can be used to check if all permissions
     * have been granted.
     *
     * @returns 'False' if all required permissions have been granted.
    */
    Q_INVOKABLE virtual bool hasMissingPermissions() = 0;

    /*! \brief Import content from file
     *
     * On desktop systems, this method is supposed to open a file dialog to
     * import a file. On mobile systems, this method is supposed to do nothing.
     *
     */
    Q_INVOKABLE virtual void importContent() = 0;

    /*! \brief Lock connection to Wi-Fi network
     *
     * If supported by the platform, this method is supposed to lock the current
     * Wi-Fi connection, that is, to prevent the device from dropping the
     * connection or shutting down the Wi-Fi interface.
     *
     * The app calls that method after connecting to a traffic data receiver, in
     * order to ensure that traffic data is continuously received.
     *
     * @param lock If true, then lock the network. If false, then release the
     * lock.
     */
    virtual void lockWifi(bool lock) = 0;

    /*! \brief Request permissions
     *
     * On some platforms, the app needs to ask for permissions to perform
     * certain functions (e.g. receive location, query Wi-Fi status, …). This
     * method is called before the GUI is set up and must request ALL
     * permissions that the app will ever use. The method is meant to run
     * synchroneously and shall return only once all permissions have been
     * granted (or not). 
     */
    virtual void requestPermissionsSync() = 0;

    /*! \brief Share content
     *
     * On desktop systems, this method is supposed to show a file dialog to save
     * the file. On mobile devices, this method is supposed to open a dialog
     * that allows to chose the method to send this file (e-mail, dropbox,
     * signal chat, …)
     *
     * @param content File content
     *
     * @param mimeType the mimeType of the content
     *
     * @param fileNameTemplate A string of the form "EDTF - EDTG", without
     * suffix of path. This can be used, e.g. as the name of the attachment when
     * sending files by e-mail.
     *
     * @returns Empty string on success, the string "abort" on abort, and a
     * translated error message otherwise
     */
    Q_INVOKABLE virtual QString shareContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) = 0;

    /*! \brief Make the device briefly vibrate
     *
     * On platforms that support this, make the device briefly vibrate if haptic
     * feedback is enabled in the system settings.
     */
    Q_INVOKABLE virtual void vibrateBrief() = 0;

    /*! \brief View content
     *
     * This method is supposed open the content in an appropriate app.  Example:
     * if the content is GeoJSON, the content might be opened in Google Earth,
     * or in a mobile mapping application.
     *
     * @param content content text
     *
     * @param mimeType the mimeType of the content
     *
     * @param fileNameTemplate A string of the form "FlightRoute-%1.geojson".
     *
     * @returns Empty string on success, a translated error message otherwise
     */
    Q_INVOKABLE virtual QString viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) = 0;

public slots:
    /*! \brief Signal handler: GUI setup completed
     *
     *  This method is called as soon as the GUI setup is completed. On Android,
     *  this method is used to hide the splash screen and to show the app.
     *
     *  The implementation should guarentee that nothing bad happens if the
     *  method is called more than once.
     */
    virtual void onGUISetupCompleted() = 0;

    /*! \brief Determine file function and emit openFileRequest()
     *
     * This helper function is called by platform-dependent code whenever the
     * app is asked to open a file.  It will look at the file, determine the
     * file function and emit the signal openFileRequest() as appropriate.
     *
     * @param path File name
     */
    virtual void processFileOpenRequest(const QString& path);

    /*! \brief Determine file function and emit openFileRequest()
     *
     * Overloaded function for convenience
     *
     * @param QByteArray containing an UTF8-Encoded strong
     */
    void processFileOpenRequest(const QByteArray& path);

signals:
    /*! \brief Emitted when platform asks this app to open a file
     *
     * This signal is emitted whenever the platform-dependent code receives
     * information that enroute is requested to open a file.
     *
     * @param fileName Path of the file on the local file system
     *
     * @param fileFunction Function and file type.
     *
     * On Android, other apps can request that enroute 'views' a file, via
     * Android's INTENT system.
     */
    void openFileRequest(QString fileName, Platform::PlatformAdaptor_Abstract::FileFunction fileFunction);

    /*! \brief Emitted when a new WiFi connections becomes available
     *
     *  This signal is emitted when a new WiFi connection becomes available.
     */
    void wifiConnected();

private:
    Q_DISABLE_COPY_MOVE(PlatformAdaptor_Abstract)
};

} // namespace Platform

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

#include <QObject>

namespace Platform {

/*! \brief Interface to platform-specific functionality
 *
 * This pure virtual class is an interface to capabilities of mobile devices (e.g. vibration)
 * that need platform-specific code to operate.  The files PlatformAdaptor_XXX.(h|cpp) implement
 * a child class PlatformAdaptor that contains the actual implementation.
 *
 * Child classes need to implement all pure virtual functions, and need to provide the following additional functionality.
 *
 * - Child classes need to monitor the Wi-Fi network and emit the signal wifiConnected() whenever a new Wi-Fi connection becomes available. The
 *   app will then check if a traffic data receiver is active in the network.
 *
 * - If supported by the platform, child classes need to react to requests by the platform to open a file (e.g. a GeoJSON file containing a flight route).
 *   Once a request is received, the signal openFileRequest() must be emitted.
 */

class PlatformAdaptor_Abstract : public QObject
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
     * @returns The SSID of the current Wi-Fi networks, an empty string
     * if the device is not connected to a Wi-Fi or a generic string if the SSID cannot be determined.
     */
    Q_INVOKABLE virtual QString currentSSID() = 0;

    /*! \brief Checks if all required permissions have been granted
     *
     * Depending on the platform, the app needs to ask for permissions to operate properly.
     * This method can
     * be used to check if all permissions have been granted.
     *
     * @returns 'False' if all required permissions have been granted.
    */
    Q_INVOKABLE virtual bool hasMissingPermissions() = 0;

    /*! \brief Import content from file
     *
     * On desktop systems, this method is supposed to open a file dialog to import a file.
     * On mobile systems, this method is supposed to do nothing.
     *
     */
    Q_INVOKABLE virtual void importContent() = 0;

    /*! \brief Lock connection to Wi-Fi network
     *
     * If supported by the platform, this method is supposed to lock the current Wi-Fi
     * connection, that is, to prevent the device from dropping the connection or shutting down the Wi-Fi interface.
     *
     * The app calls that method after connecting to a traffic data receiver, in order to ensure that traffic data is continuously received.
     *
     * @param lock If true, then lock the network. If false, then release the lock.
     */
    Q_INVOKABLE virtual void lockWifi(bool lock) = 0;

    /*! \brief Export content to file or to file sending app
     *
     * On desktop systems, this method is supposed to show a file dialog to save the file.
     * On mobile devices, this method is supposed to open a dialog that allows to chose the method to send this file (e-mail, dropbox, signal chat, …)
     *
     * @param content File content
     *
     * @param mimeType the mimeType of the content
     *
     * @param fileNameTemplate A string of the form "EDTF - EDTG", without suffix of path. This
     * can be used, e.g. as the name of
     * the attachment when sending files by e-mail.
     *
     * @returns Empty string on success, the string "abort" on abort, and a translated error message otherwise
     */
    Q_INVOKABLE virtual QString shareContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) = 0;

    /*! \brief Make the device briefly vibrate
     *
     * On platforms that support this, make the device briefly vibrate if haptic feedback is enabled in the system settings.
     */
    Q_INVOKABLE virtual void vibrateBrief() = 0;

    /*! \brief View content
     *
     * This method is supposed open the content in an appropriate app.  Example: if the content is GeoJSON, the content might be opened in Google Earth, or in a mobile
     * mapping application.
     *
     * @param content content text
     *
     * @param mimeType the mimeType of the content
     *
     * @param fileNameTemplate A string of the form "EDTF - EDTG", without suffix of path.
     *
     * @returns Empty string on success, a translated error message otherwise
     */
    Q_INVOKABLE virtual QString viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) = 0;

    // ------------------

    /*! \brief Hides the android splash screen.
     *
     * On Android, hides the android splash screen.
     *
     * On other platforms, this does nothing. The implementation ensures that
     * QtAndroid::hideSplashScreen is called (only once, regardless of how often
     * this slot is used).
    */
#warning do I need this here?
    Q_INVOKABLE virtual void hideSplashScreen() = 0;

    /*! \brief Device manufacturer
     *
     * @returns On Android, returns device manufacturer. On other systems, always returns an empty string.
    */
#warning do I need this here?
    Q_INVOKABLE virtual QString manufacturer() = 0;

    /*! \brief Helper function, not for public consumption
     *
     * This helper function is called by platform-dependent code whenever the
     * app is asked to open a file.  It will look at the file, determine the
     * file function and emit the signal openFileRequest() as appropriate.
     *
     * On Android, the slot is called from JAVA.
     *
     * @param path File name
     *
     */
#warning do I need this here?
    Q_INVOKABLE virtual void processFileOpenRequest(const QString &path) = 0;

    /*! \brief Helper function, not for public consumption
     *
     * Overloaded function for convenience
     *
     * @param path UTF8-Encoded strong
     */
#warning do I need this here?
    Q_INVOKABLE virtual void processFileOpenRequest(const QByteArray &path) = 0;

    /*! \brief Start receiving "open file" requests from platform
     *
     * This method should be called to indicate that the GUI is set up and ready
     * to receive platform-specific requests to open files.  The
     * openFileRequest() might be emitted immediately
     *
     * On Android, this method checks if there are pending intents which should
     * be processed in the java activity
     * de.akaflieg_freiburg.enroute.ShareActivity.  This is usually necessary if
     * the app has been launched by an incoming intent and the java
     * ShareActivity postponed processing of the intent until enroute has been
     * fully initialized.
     */
#warning do I need this here?
     Q_INVOKABLE virtual void startReceiveOpenFileRequests() = 0;

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
    void openFileRequest(QString fileName, PlatformAdaptor_Abstract::FileFunction fileFunction);

    /*! \brief Emitted when a new WiFi connections becomes available
     *
     *  This signal is emitted when a new WiFi connection becomes available.
     */
    void wifiConnected();

private:
    Q_DISABLE_COPY_MOVE(PlatformAdaptor_Abstract)

    // Intializations that are moved out of the constructor, in order to avoid
    // nested uses of Global.
    void deferredInitialization();

    // Helper function. Saves content to a file in a directory from where
    // sharing to other android apps is possible
    auto contentToTempFile(const QByteArray& content, const QString& fileNameTemplate) -> QString;

    // Name of a subdirectory within the AppDataLocation for sending and
    // receiving files.
    QString fileExchangeDirectoryName;

    bool receiveOpenFileRequestsStarted {false};
    QString pendingReceiveOpenFileRequest {};
    bool splashScreenHidden {false};
};

} // namespace Platform

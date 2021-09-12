/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#include <QtGlobal>
#include <QTimer>

#include <QObject>

/*! \brief Interface to platform-specific capabilities of mobile devices
 *
 * This class is an interface to capabilities of mobile devices (e.g. vibration)
 * that need platform-specific code to operate.
 *
 * This class also receives platform-specific requests to open files and exposes
 * these requests to C++ and QML via the signal openFileRequest().  The signal
 * openFileRequest() is however only emitted after the method
 * startReceiveOpenFileRequests() has been called, requests that come in earlier
 * will be put on hold.  This allows apps to set up their GUI before processing
 * file open requests (that might need user interaction).
 */

class MobileAdaptor : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * On Android, this constructor disables the screen lock and asks for the
     * permissions that are needed to run the app.
     *
     * @param parent Standard QObject parent pointer
    */
    explicit MobileAdaptor(QObject *parent = nullptr);

    ~MobileAdaptor();

    /*! \brief Function and type of a file that we have been requested to
     * open */
    enum FileFunction
      {
        UnknownFunction,
        FlightRoute_GPX, /*!< File contains a flight route, stored as GeoJSON. */
        FlightRoute_GeoJSON /*!< File contains a flight route, stored as GPX. */
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

    /*! \brief Checks if all requred permissions have been granted
     *
     * On Android, the app requirs certain permissions to run. This method can
     * be used to check if all permissions have been granted.
     *
     * @returns On Android, returns 'true' if not all required permissions have
     * been granted. On other systems, always returns 'false'
    */
    Q_INVOKABLE bool missingPermissionsExist();

    /*! \brief Export content to file or to file sending app
     *
     * On Android systems, this method will save content to temporary file in
     * the app's private cache and call the corresponding java static method
     * where a SEND intent is created and startActivity is called
     *
     * On other desktop systems, this method uses a file dialog to save the file.
     *
     * @param content content text
     *
     * @param mimeType the mimeType of the content
     *
     * @param fileNameTemplate A string of the form "EDTF - EDTG", without suffix of path. This
     * file name is visible to the user. It appears for instance as the name of
     * the attachment when sending files by e-mail.
     *
     * @returns Empty string on success, the string "abort" on abort, and a translated error message otherwise
     */
    Q_INVOKABLE QString exportContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate);

    /*! \brief Lock connection to Wi-Fi network
     *
     * Under Android, this method can lock the Wi-Fi connection by acquiring a
     * WifiManager.WifiLock. On other platforms, this method does nothing.
     *
     * @param lock If true, then lock the network. If false, then release the lock.
     */
    Q_INVOKABLE static void lockWifi(bool lock);

    /*! \brief Get SSID of current Wi-Fi network
     *
     * @returns The SSID of the current Wi-Fi networks, or an empty of generic string
     * if the device is not connected to a Wi-Fi or if the SSID could not be determined.
     */
    Q_INVOKABLE static QString getSSID();

    /*! \brief Import content from file
     *
     * On Android systems, this method does nothing.
     *
     * On other desktop systems, this method uses a file dialog to import a file.
     */
    Q_INVOKABLE void importContent();

    /*! \brief View content in other app
     *
     * On Android systems, this method will save content to temporary file in
     * the app's private cache and call the corresponding java static method
     * where a SEND intent is created and startActivity is called.
     *
     * On other systems, this method opens the file using QDesktopServices.
     *
     * @param content content text
     *
     * @param mimeType the mimeType of the content
     *
     * @param fileNameTemplate A string of the form "EDTF - EDTG", without suffix of path. This
     * file name is visible to the user. It appears for instance as the name of
     * the attachment when sending files by e-mail.
     *
     * @returns Empty string on success, a translated error message otherwise
     */
    Q_INVOKABLE QString viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate);

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
     Q_INVOKABLE void startReceiveOpenFileRequests();

#if defined (Q_OS_ANDROID)
    // Emits the signal "WifiConnected".
    void emitWifiConnected() {
        emit wifiConnected();
    }
#endif

public slots:
    /*! \brief Hides the android splash screen.
     *
     * On Android, hides the android splash screen.
     *
     * On other platforms, this does nothing. The implementation ensures that
     * QtAndroid::hideSplashScreen is called (only once, regardless of how often
     * this slot is used).
    */
    void hideSplashScreen();

    /*! \brief Make the device briefly vibrate
     *
     * On Android, make the device briefly vibrate.
     *
     * On other platforms, this does nothing.
    */
    static void vibrateBrief();

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
    void processFileOpenRequest(const QString &path);

    /*! \brief Helper function, not for public consumption
     *
     * Overloaded function for convenience
     *
     * @param path UTF8-Encoded strong
     */
    void processFileOpenRequest(const QByteArray &path);

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
    void openFileRequest(QString fileName, MobileAdaptor::FileFunction fileFunction);

    /*! \brief Emitted when a new WiFi connections becomes available
     *
     *  This signal is emitted when a new WiFi connection becomes available.
     */
    void wifiConnected();

private slots:
    // Intializations that are moved out of the constructor, in order to avoid
    // nested uses of globalInstance().
    void deferredInitialization();

private:
    Q_DISABLE_COPY_MOVE(MobileAdaptor)
  
    // Helper function. Saves content to a file in a directory from where
    // sharing to other android apps is possible
    QString contentToTempFile(const QByteArray& content, const QString& fileNameTemplate);

    // Name of a subdirectory within the AppDataLocation for sending and
    // receiving files.
    QString fileExchangeDirectoryName;

#if defined (Q_OS_ANDROID)
    // @returns True if an app could be started, false if no app was found
    static bool outgoingIntent(const QString& methodName, const QString& filePath, const QString& mimeType);

    QStringList permissions;
#endif

    bool receiveOpenFileRequestsStarted {false};
    QString pendingReceiveOpenFileRequest {};
    bool splashScreenHidden {false};
};

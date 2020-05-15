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

#include <QObject>

/*! \brief Interface to platform-specific capabilities of mobile devices
  
  This class is an interface to capabilities of mobile devices (e.g. vibration)
  that need platform-specific code to operate. On desktop platforms, the methods
  of this class generally do nothing.
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

    /*! \brief Checks if all requred permissions have been granted
     *
     * On Android, the app requirs certain permissions to run. This method can
     * be used to check if all permissions have been granted.
     *
     * @returns On Android, returns 'true' if not all required permissions have
     * been granted. On other systems, always returns 'false'
    */
    Q_INVOKABLE bool missingPermissionsExist();

    /*! \brief Send content with other app
     *
     * On Android systems, this method will do the following.
     *
     * - save content to temporary file in the app's private cache
     * - call the corresponding java static method where a SEND intent is created and startActivity is called
     *
     * On other systems, this method does nothing yet.
     *
     * @param content content text
     * @param mimeType the mimeType of the content
     * @param fileNameTemplate A string of the form "FlightRoute-%1.geojson" the substring "%1" will later be replaced by the current time and date. This file name is visible to the user. It appears for instance as the name of the attachment when sending files by e-mail
     *
     * @returns True on success, false if no suitable app could be found
     */
    Q_INVOKABLE bool sendContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate);


    /*! \brief View content in other app
     *
     * On Android systems, this method will do the following.
     *
     * - save content to temporary file in the app's private cache
     * - call the corresponding java static method where a SEND intent is created and startActivity is called
     *
     * On other systems, this method does nothing yet.
     *
     * @param content content text
     * @param mimeType the mimeType of the content
     * @param fileNameTemplate A string of the form "FlightRoute-%1.geojson" the substring "%1" will later be replaced by the current time and date. This file name is visible to the user. It appears for instance as the name of the attachment when sending files by e-mail
     *
     * @returns True on success, false if no suitable app could be found
     */
    Q_INVOKABLE bool viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate);

#if defined (Q_OS_ANDROID)
    // Get single instance of the Share. This is used from the JNI "callback" setFileReceived(). It returns the single instance of the Share class.
    static MobileAdaptor* getInstance();

    // Helper function, called from platform-dependent code when enroute is asked
    // to open a file
    void receiveFile(const QString &path);

    /*! \brief fired if the main window becomes active, triggered from main.qml.
     *
     * This method checks if there are pending intents which should be processed
     * in the java activity de.akaflieg_freiburg.enroute.ShareActivity.
     *
     * This is usually necessary if the app has been launched by an incoming intent
     * and the java ShareActivity postponed processing of the intent until enroute
     * has been fully initialized.
     */
    Q_INVOKABLE void checkPendingIntents();
#endif

public slots:
    /*! \brief Hides the android splash screen.

    On Android, hides the android splash screen. On other platforms, this does
    nothing. The implementation ensures that QtAndroid::hideSplashScreen is
    called (only once, regardless of how often this slot is used).
    */
    void hideSplashScreen();

    /*! \brief Make the device briefly vibrate

    On Android, make the device briefly vibrate. On other platforms, this does
    nothing.
    */
    void vibrateBrief();

    /*! \brief Shows a notifaction, indicating that a download is in progress

    @param show If set to 'true', a notification will be shown. If set to
    'false', any existing notification will be withdrawn
    */
    void showDownloadNotification(bool show);

private:
    Q_DISABLE_COPY_MOVE(MobileAdaptor)
  
    // Helper function. Saves content to a file in a directory from where sharing to other android apps is possible
    QString contentToTempFile(const QByteArray& content, const QString& fileNameTemplate);

    // Name of a subdirectory within the AppDataLocation for
    // sending and receiving files.
    QString fileExchangeDirectoryName;

#if defined (Q_OS_ANDROID)
    // @returns True if an app could be started, false if no app was found
    bool outgoingIntent(const QString& methodName, const QString& filePath, const QString& mimeType);

    // Pointer to instance of this class, required for JNI calls
    static MobileAdaptor* mInstance;
#endif

    bool splashScreenHidden {false};
};

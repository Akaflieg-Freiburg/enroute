/*!
 * Copyright (C) 2020 by Johannes Zellner, johannes@zellner.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef SHARE_H
#define SHARE_H

#include <QObject>

/*!
 * \brief The Share class provides synchronous and asynchronous communication
 * between the android native share and the Qt application.
 *
 * Sending is done via IntentLauncher.java.
 * Receiving is via setFileReceived() from ShareActivity.java.
 *
 * Send content
 * ------------
 *
 * sendContent - android "share" with other apps (email, messenger, ...)
 * viewContent - open content in other android app
 * saveContent - save to file
 *
 * all three methods call static methods in IntentLauncher.java to create
 * correspoding intents and call startActivity(). This works asynchronously
 * for send and view which means that enroute doesn't block after the
 * receiving app has received the content. It works synchronously for save
 * thus save returns only if the file has been saved or the operation has
 * been cancelled.
 *
 * Receive content
 * ---------------
 *
 * importFile - to import file from file system.
 * This calls a static method in IntentLauncher.java to create correspoding
 * intents and call the "blocking" startActivityForResult().
 * This works synchronously, therefore enroute will block until either a new
 * file URL will be sent back from ShareActivity or the activity has been
 * cancelled.
 *
 * furthermore, SEND and VIEW intents from other apps will trigger the
 * setFileReceived() methods too.
 *
 * Technically, all content is received in the class
 * de.akaflieg_freiburg.enroute.ShareActivity from share intents
 * by the two methods
 *
 * - onActivityResult()
 * - onNewIntent()
 *
 * The file URL which is passed in these intents is then sent back
 * from java to Qt in the following order:
 *
 * java (de.akaflieg_freiburg.enroute.ShareActivity from share intents)
 * ->Java_de_akaflieg_1freiburg_enroute_ShareActivity_setFileReceived()
 * ->setFileReceived()
 * ->emit fileReceived()
 * ->to connected slots like FlightRoute::fromGpx()
 */
class Share : public QObject
{
    Q_OBJECT

public:
    explicit Share(QObject *parent = nullptr);

    /*! \brief share text content with other app.
     *
     * This method will
     * - save content to temporary file in the app's private cache
     * - call the corresponding java static method where a SEND intent is created and startActivity is called
     *
     * @param content content text
     * @param mimeType the mimeType of the content
     * @param suffix the suffix for a temporary file
     */
    Q_INVOKABLE
    void sendContent(const QString& content, const QString& mimeType, const QString& suffix);

    /*! \brief open content in other app
     *
     * This method will
     * - save content to temporary file in the app's private cache
     * - call the corresponding java static method where a VIEW intent is created and startActivity is called
     *
     * @param content content text
     * @param mimeType the mimeType of the content
     * @param suffix the suffix for a temporary file
     */
    Q_INVOKABLE
    void viewContent(const QString& content, const QString& mimeType, const QString& suffix);

    /*! \brief save content to file system
     *
     * This method will
     * - save content to temporary file in the app's private cache
     * - call the corresponding java static method where a CREATE_DOCUMENT intent
     *   is created and startActivity is called
     *
     * @param content content text
     * @param mimeType the mimeType of the content
     * @param suffix the suffix for the file
     */
    Q_INVOKABLE
    void saveContent(const QString& content, const QString& mimeType, const QString& suffix);

    /*! \brief import file
     *
     * This method will call java static method where an OPEN intent
     * is created and startActivityForResult() is called.
     *
     * @param mimeType the mimeType of the content
     */
    Q_INVOKABLE void importFile(const QString& mimeType = nullptr);

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

    /*! \brief get single instance of the Share.
     * used from the JNI "callback" setFileReceived()
     *
     * @returns the single instance of the Share class.
     */
    static Share* getInstance();

    /*! \brief called from the JNI "callback" setFileReceived() to receive
     * the received file name and to emit fileReceived() to connected slots.
     *
     * @param fname absolute file path in cache directory
     */
    void setFileReceived(const QString& fname);

signals:
    /*! \brief emits when a share fails because an app could not be found
     */
    void shareNoAppAvailable();

    /*! \brief emits when a file is received
     *
     * @param fname absolute file path in cache directory
     */
    void fileReceived(const QString& fname);

private:
    static Share* mInstance;
    QString mSavePath;

    QString contentToTempFile(const QString& content, const QString& suffix);
    void clearTempDir();
    QString tempDir();
    void outgoingIntent(const QString& methodName, const QString& filePath, const QString& mimeType);
};

#endif // SHARE_H

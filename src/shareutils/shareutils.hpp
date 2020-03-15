/*!
 * MIT License
 *
 * Copyright (c) 2019 Tim Seemann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// (c) 2017 Ekkehard Gentz (ekke)
// this project is based on ideas from
// http://blog.lasconic.com/share-on-ios-and-android-using-qml/
// see github project https://github.com/lasconic/ShareUtils-QML
// also inspired by:
// https://www.androidcode.ninja/android-share-intent-example/
// https://www.calligra.org/blogs/sharing-with-qt-on-android/
// https://stackoverflow.com/questions/7156932/open-file-in-another-app
// http://www.qtcentre.org/threads/58668-How-to-use-QAndroidJniObject-for-intent-setData
// see also /COPYRIGHT and /LICENSE

// (c) 2017 Ekkehard Gentz (ekke) @ekkescorner
// my blog about Qt for mobile: http://j.mp/qt-x
// see also /COPYRIGHT and /LICENSE

#ifndef SHARE_UTILS_H
#define SHARE_UTILS_H

#include <QDebug>
#include <QObject>

#if defined(Q_OS_ANDROID)
#include <QAndroidActivityResultReceiver>
#include <QtAndroid>
#endif // Q_OS_ANDROID

/*!
 * \brief The PlatformShareUtils class is an abstract class that implements the
 * functionality for sending and viewing files through the native mobile
 * interfaces. Each mobile interface has its own derived class that handles
 * communicating with native mobile code.
 */
class PlatformShareUtils : public QObject {
    Q_OBJECT
public:
    PlatformShareUtils(QObject* parent) : QObject(parent) {}

    /*!
     * \brief checkMimeTypeView check whether the mimeType provided is compatible
     * with the application that will be sending/viewing the file. \param mimeType
     * mimeType to check \return true if the mimeType is supported, false
     * otherwise
     */
    virtual bool checkMimeTypeView(const QString& mimeType) = 0;

    /*!
     * \brief sendFile send a file through the native mobile share interface
     * \param filePath the absolute path to the file
     * \param title the title of the share
     * \param mimeType the mimeType of the file
     * \param requestId the ID to give the task of sending a file. Set this to a
     * value other than 0 if you want to track the completion of the task.
     */
    virtual void sendFile(const QString& filePath,
                          const QString& title,
                          const QString& mimeType,
                          int requestId) = 0;

    /*!
     * \brief vuewFile view a file through the native mobile share interface
     * \param filePath the absolute path to the file
     * \param title the title of the share
     * \param mimeType the mimeType of the file
     * \param requestId the ID to give the task of sending a file. Set this to a
     * value other than 0 if you want to track the completion of the task.
     */
    virtual void viewFile(const QString& filePath,
                          const QString& title,
                          const QString& mimeType,
                          int requestId) = 0;

    /*!
     * \brief checkPendingIntents used by android only, this checks for pending
     * intents when an application becomes active again.
     */
    virtual void checkPendingIntents() {}

    /*!
     * \brief clearTempDir android only, this provides an option to clear the temp
     * save directory.
     */
    virtual void clearTempDir() {}

    /*!
     * \brief tempDir android only, return temporary directory
     */
    virtual QString tempDir() { return QString(""); }

signals:
    /// emits when a share finishes successfully.
    void shareFinished(int requestID);

    /// emits when a share fails because an app could not be found
    void shareNoAppAvailable(int requestID);

    /// emits when there is an error with sharing, will give message and requestID
    /// if provided
    void shareError(int requestID, QString message);

    /// emits when a URL is received
    void fileUrlReceived(QString url);
};

/*!
 * \brief The DummyShareUtils class dervies from the PlatformShareUtils class
 * and exists only to make it so that you don't need to wrap `ShareUtils` in
 * preprocessor macros when compiling for desktop. When working with viewing and
 * sending files on desktop, you should instead use QFileDialog.
 */
class DummyShareUtils : public PlatformShareUtils {
    Q_OBJECT
public:
    DummyShareUtils(QObject* parent) : PlatformShareUtils(parent) {}

    /// @copydoc PlatformShareUtils::checkMimeTypeView(const QString&)
    virtual bool checkMimeTypeView(const QString& mimeType) override {
        qDebug() << __func__ << "mimType:" << mimeType;
        return false;
    }

    /// @copydoc PlatformShareUtils::sendFile(const QString&, const QString&,
    /// const QString&, int)
    virtual void sendFile(const QString& filePath,
                          const QString& title,
                          const QString& mimeType,
                          int requestId) override {
        qDebug() << __func__ << "filePath:" << filePath << " title: " << title
                 << " mimType: " << mimeType << " requestID: " << requestId;
    }

    /// @copydoc PlatformShareUtils::viewFile(const QString&, const QString&,
    /// const QString&, int)
    virtual void viewFile(const QString& filePath,
                          const QString& title,
                          const QString& mimeType,
                          int requestId) override {
        qDebug() << __func__ << "filePath:" << filePath << " title: " << title
                 << " mimType: " << mimeType << " requestID: " << requestId;
    }
};

#if defined(Q_OS_IOS)

/*!
 * \brief The IosShareUtils class dervies from the PlatformShareUtils class and
 * implements communication between the iOS native share and the Qt application.
 */
class IosShareUtils : public PlatformShareUtils {
    Q_OBJECT

public:
    explicit IosShareUtils(QObject* parent);

    /// @copydoc PlatformShareUtils::checkMimeTypeView(const QString&)
    bool checkMimeTypeView(const QString& mimeType) override;

    /// @copydoc PlatformShareUtils::sendFile(const QString&, const QString&,
    /// const QString&, int)
    void sendFile(const QString& filePath,
                  const QString& title,
                  const QString& mimeType,
                  int requestId) override;

    /// @copydoc PlatformShareUtils::viewFile(const QString&, const QString&,
    /// const QString&, int)
    void viewFile(const QString& filePath,
                  const QString& title,
                  const QString& mimeType,
                  int requestId) override;

    void handleDocumentPreviewDone(const int& requestId);

public slots:

    void handleFileUrlReceived(const QUrl& url);
};

#endif

#if defined(Q_OS_ANDROID)

/*!
 * \brief The AndroidShareUtils class dervies from the PlatformShareUtils class
 * and implements communication between the android native share and the Qt
 * application.
 */
class AndroidShareUtils : public PlatformShareUtils, public QAndroidActivityResultReceiver {
    Q_OBJECT
public:
    AndroidShareUtils(QObject* parent = nullptr);

    /// @copydoc PlatformShareUtils::checkMimeTypeView(const QString&)
    bool checkMimeTypeView(const QString& mimeType) override;

    /// @copydoc PlatformShareUtils::sendFile(const QString&, const QString&,
    /// const QString&, int)
    void sendFile(const QString& filePath,
                  const QString& title,
                  const QString& mimeType,
                  int requestId) override;

    /// @copydoc PlatformShareUtils::viewFile(const QString&, const QString&,
    /// const QString&, int)
    void viewFile(const QString& filePath,
                  const QString& title,
                  const QString& mimeType,
                  int requestId) override;

    /// @copydoc PlatformShareUtils::checkPendingIntents()
    void checkPendingIntents() override;

    /// @copydoc PlatformShareUtils::clearTempDir()
    void clearTempDir() override;

    /// @copydoc PlatformShareUtils::tempDir()
    QString tempDir() override;

    void handleActivityResult(int receiverRequestCode,
                              int resultCode,
                              const QAndroidJniObject& data) override;

    void onActivityResult(int requestCode, int resultCode);

    /// gets single instance of the AndroidShareUtils
    static AndroidShareUtils* getInstance();

public slots:
    void setFileUrlReceived(const QString& url);

    bool checkFileExits(const QString& url);

private:
    static AndroidShareUtils* mInstance;

    QString mSavePath;

    void processActivityResult(int requestCode, int resultCode);
};

#endif

/*!
 * \brief The ShareUtils class interfaces the various ShareUtils for each
 * platform with a standard Qt interface. This allows you to include a
 * `ShareUtils` object in your application as a standard Qt object. The
 * `ShareUtils` class abstracts away the native code and emits signals instead.
 * For android and iOS this provides the ability to view and send files. For
 * desktop builds, this only provides a dummy. In desktop environments this
 * design pattern doesn't make sense and QFileDialog is preferred.
 */
class ShareUtils : public QObject {
    Q_OBJECT

public:
    /// constructor, the `PlatformShareUtils` will be different in different OSes.
    explicit ShareUtils(QObject* parent) : QObject(parent) {
#if defined(Q_OS_IOS)
        mPlatformShareUtils = new IosShareUtils(this);
#elif defined(Q_OS_ANDROID)
        mPlatformShareUtils = new AndroidShareUtils(this);
#else
        // NOTE: The dummy is here so that the object compiles well for desktop
        // builds, however, since this project is meant for iOS and Android native
        // share design patterns, it doesn't really make sense to implement share
        // methods for desktop as well. If you're interested in accessing files on
        // desktop, Qt provides `QFileDialog` for this explicit purpose, and this
        // class better matches the desktop design pattern.
        mPlatformShareUtils = new DummyShareUtils(this);
#endif

        connect(mPlatformShareUtils, SIGNAL(shareFinished(int)), this, SLOT(onShareFinished(int)));

        connect(mPlatformShareUtils,
                SIGNAL(shareNoAppAvailable(int)),
                this,
                SLOT(onShareNoAppAvailable(int)));

        connect(mPlatformShareUtils,
                SIGNAL(shareError(int, QString)),
                this,
                SLOT(onShareError(int, QString)));

        connect(mPlatformShareUtils,
                SIGNAL(fileUrlReceived(QString)),
                this,
                SLOT(onFileUrlReceived(QString)));
    }

    /// @copydoc PlatformShareUtils::checkMimeTypeView(const QString&)
    bool checkMimeTypeView(const QString& mimeType) {
        return mPlatformShareUtils->checkMimeTypeView(mimeType);
    }

    /// @copydoc PlatformShareUtils::sendFile(const QString&, const QString&,
    /// const QString&, int)
    void sendFile(const QString& filePath,
                  const QString& title,
                  const QString& mimeType,
                  const int& requestId) {
        mPlatformShareUtils->sendFile(filePath, title, mimeType, requestId);
    }

    /// @copydoc PlatformShareUtils::viewFile(const QString&, const QString&,
    /// const QString&, int)
    void viewFile(const QString& filePath,
                  const QString& title,
                  const QString& mimeType,
                  const int& requestId) {
        mPlatformShareUtils->viewFile(filePath, title, mimeType, requestId);
    }

    /// @copydoc PlatformShareUtils::checkPendingIntents()
    void checkPendingIntents() { mPlatformShareUtils->checkPendingIntents(); }

    /// @copydoc PlatformShareUtils::clearTempDir();
    void clearTempDir() { mPlatformShareUtils->clearTempDir(); }

    /// @copydoc PlatformShareUtils::tempDir();
    QString tempDir() { return mPlatformShareUtils->tempDir(); }

signals:
    /// emits when a share finishes successfully
    void shareFinished(int requestCode);

    /// emits when a share fails because an app could not be found
    void shareNoAppAvailable(int requestCode);

    /// emits when there is an error with sharing, will give message and requestID
    /// if provided
    void shareError(int requestCode, QString message);

    /// emits when a URL is received
    void fileUrlReceived(QString url);

    /// emits when a URL is received and a file is saved.
    void fileReceivedAndSaved(QString url);

private slots:
    /// emits a signal when a share finishes successfully
    void onShareFinished(int requestCode) { emit shareFinished(requestCode); }

    /// emits a signal when a share fails because an app could not be found
    void onShareNoAppAvailable(int requestCode) { emit shareNoAppAvailable(requestCode); }

    /// emits a signal when there is an error with sharing, will give message and
    /// requestID if provided
    void onShareError(int requestCode, QString message) { emit shareError(requestCode, message); }

    /// emits a signal when a URL is received
    void onFileUrlReceived(QString url) { emit fileUrlReceived(url); }

private:
    /*! the object that is used to handle the native share interface. This object
     * will be a different class depending on what OS the Qt code is compiled for.
     */
    PlatformShareUtils* mPlatformShareUtils;
};

#endif // SHARE_UTILS_H

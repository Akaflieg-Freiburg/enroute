#include <stdio.h>
#include <QtCore/QString>

#include <QObject>
#include <QQmlEngine>

class ObjCAdapter {
    public:
        static void vibrateBrief();
        static void vibrateError();
        static void vibrateLong();
        static void disableScreenSaver();
        static QString shareContent(const QByteArray&, const QString&, const QString&, const QString&);
        static QString preferredLanguage();
        static void saveToGallery(QString&);
        static void requestNotificationPermission();
        static void postNotification(const QString& title, const QString& body);

        /*! \brief Request background location updates
         *
         *  Starts the background-location keep-alive if "Always" location
         *  access has already been granted. If authorization has not been
         *  decided yet, requests it (the system dialog is shown at most
         *  once; iOS silently ignores repeat requests once the user has
         *  answered). If access was denied, restricted, or only granted for
         *  "While Using the App", no dialog can be triggered again — the
         *  user must fix this in Settings.
         *
         *  @returns true if background location updates are active, false
         *           if authorization is missing or insufficient.
         */
        static bool enableBackgroundLocation();
        static void disableBackgroundLocation();
};

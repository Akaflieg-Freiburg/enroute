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
};

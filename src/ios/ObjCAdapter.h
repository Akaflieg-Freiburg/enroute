#include <stdio.h>
#include <QtCore/QString>

#include <QObject>
#include <QQmlEngine>

class ObjCAdapter {
    public:
        static void vibrateBrief();
        static void vibrateError();
        static double safeAreaTopInset();
        static double safeAreaLeftInset();
        static double safeAreaBottomInset();
        static double safeAreaRightInset();
        static void disableScreenSaver();
        static void requestNotificationPermission();
        static bool hasLocationPermission();
        static void sendNotification(QString, QString);
        static bool hasNotificationPermission();
};

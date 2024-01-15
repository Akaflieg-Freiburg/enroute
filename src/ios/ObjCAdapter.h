#include <stdio.h>
#include <QtCore/QString>

#include <QObject>
#include <QQmlEngine>

class ObjCAdapter {
    public:
        static void vibrateBrief();
        static void vibrateError();
        static void vibrateLong();
        static double safeAreaTopInset();
        static double safeAreaLeftInset();
        static double safeAreaBottomInset();
        static double safeAreaRightInset();
        static void disableScreenSaver();
        static QString shareContent(const QByteArray&, const QString&, const QString&, const QString&);
        static QString preferredLanguage();
        static void saveToGallery(QString&);
};

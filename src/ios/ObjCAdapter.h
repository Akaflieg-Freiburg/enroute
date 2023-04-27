#include <stdio.h>
#include <QtCore/QString>

#include <QObject>
#include <QQmlEngine>

class ObjCAdapter {
    public:
        static QString objectiveC_Call(); //We define a static method to call the function directly using the class_name
        static void vibrateBrief();
        static void vibrateError();
        static double safeAreaTopInset();
        static double safeAreaLeftInset();
        static double safeAreaBottomInset();
        static double safeAreaRightInset();
};

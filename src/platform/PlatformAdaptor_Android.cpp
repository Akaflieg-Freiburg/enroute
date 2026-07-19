/***************************************************************************
 *   Copyright (C) 2019-2025 by Stefan Kebekus                             *
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

#include <QGuiApplication>
#include <QHash>
#include <QInputMethod>
#include <QJniEnvironment>
#include <QJniObject>
#include <QPermissions>
#include <QProcess>
#include <QScreen>
#include <QTimer>
#include <chrono>

#include "platform/PlatformAdaptor_Android.h"
#include "traffic/TrafficDataProvider.h"

using namespace Qt::Literals::StringLiterals;
using namespace std::chrono_literals;


Platform::PlatformAdaptor::PlatformAdaptor(QObject *parent)
    : Platform::PlatformAdaptor_Abstract(parent)
{
    // Keep the property imeBottomInset up-to-date. The window insets settle
    // only after the keyboard animation has finished, so in addition to the
    // immediate update, re-check after one second. This follows the pattern
    // of the former SafeInsets implementation.
    auto* inputMethod = QGuiApplication::inputMethod();
    connect(inputMethod, &QInputMethod::visibleChanged, this, &PlatformAdaptor::updateImeBottomInset);
    connect(inputMethod, &QInputMethod::keyboardRectangleChanged, this, &PlatformAdaptor::updateImeBottomInset);

    auto* timer = new QTimer(this);
    timer->setInterval(1s);
    timer->setSingleShot(true);
    connect(inputMethod, &QInputMethod::visibleChanged, timer, qOverload<>(&QTimer::start));
    connect(inputMethod, &QInputMethod::keyboardRectangleChanged, timer, qOverload<>(&QTimer::start));
    connect(timer, &QTimer::timeout, this, &PlatformAdaptor::updateImeBottomInset);
}


void Platform::PlatformAdaptor::updateImeBottomInset()
{
    double newInset = 0.0;

    // Qt reports the keyboard visibility reliably, but not its geometry, so
    // the total bottom inset (keyboard/system bars/cutout union) is read from
    // the Android window via JNI. Depending on the device and Android
    // version, Qt's own safe-area margins may or may not already contain the
    // keyboard; subtracting them here yields exactly the part that is
    // missing, so the combined margin is correct either way. Note that
    // QWindow::safeAreaMargins() is the pure platform value: it never
    // includes the additional margins that the QML code sets from this
    // property, so there is no feedback loop.
    if (QGuiApplication::inputMethod()->isVisible())
    {
        auto devicePixelRatio = QGuiApplication::primaryScreen()->devicePixelRatio();
        if (qIsFinite(devicePixelRatio) && (devicePixelRatio > 0.0))
        {
            auto inset = static_cast<double>(QJniObject::callStaticMethod<jdouble>("de/akaflieg_freiburg/enroute/MobileAdaptor", "bottomInset"));
            if (qIsFinite(inset) && (inset > 0.0))
            {
                QWindow* window = QGuiApplication::focusWindow();
                if ((window == nullptr) && !QGuiApplication::topLevelWindows().isEmpty())
                {
                    window = QGuiApplication::topLevelWindows().constFirst();
                }

                double qtBottomMargin = 0.0;
                if (window != nullptr)
                {
                    qtBottomMargin = window->safeAreaMargins().bottom();

                    // Re-check whenever Qt's own margins change, so a margin
                    // update arriving after this poll corrects the value.
                    if (m_watchedWindow != window)
                    {
                        if (!m_watchedWindow.isNull())
                        {
                            disconnect(m_watchedWindow, &QWindow::safeAreaMarginsChanged, this, &PlatformAdaptor::updateImeBottomInset);
                        }
                        connect(window, &QWindow::safeAreaMarginsChanged, this, &PlatformAdaptor::updateImeBottomInset);
                        m_watchedWindow = window;
                    }
                }

                newInset = qMax(0.0, inset/devicePixelRatio - qtBottomMargin);
            }
        }
    }

    if (newInset != m_imeBottomInset)
    {
        m_imeBottomInset = newInset;
        emit imeBottomInsetChanged();
    }
}


void Platform::PlatformAdaptor::deferredInitialization()
{
    QJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "startWiFiMonitor");
}


//
// Methods
//

auto Platform::PlatformAdaptor::currentSSID() -> QString
{
    QJniObject const stringObject
        = QJniObject::callStaticObjectMethod("de/akaflieg_freiburg/enroute/MobileAdaptor",
                                             "getSSID",
                                             "()Ljava/lang/String;");
    return stringObject.toString();
}

void Platform::PlatformAdaptor::disableScreenSaver()
{
    bool const on = true;
    // Implementation follows a suggestion found in https://stackoverflow.com/questions/27758499/how-to-keep-the-screen-on-in-qt-for-android
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([on] {
        QJniObject const activity = QNativeInterface::QAndroidApplication::context();
        if (activity.isValid())
        {
            QJniObject const window = activity.callObjectMethod("getWindow",
                                                                "()Landroid/view/Window;");

            if (window.isValid())
            {
                const int FLAG_KEEP_SCREEN_ON = 128;
                if (on)
                {
                    window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                }
                else
                {
                    window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                }
            }
        }
        QJniEnvironment const env;
        if (env->ExceptionCheck() != 0U) {
            env->ExceptionClear();
        }
    });
}

void Platform::PlatformAdaptor::lockWifi(bool lock)
{
    QJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "lockWiFi", "(Z)V", lock);
}

void Platform::PlatformAdaptor::onGUISetupCompleted()
{
    if (splashScreenHidden)
    {
        return;
    }

    splashScreenHidden = true;

    // Hide Splash Screen
    QNativeInterface::QAndroidApplication::hideSplashScreen(200);
}

QVector<Traffic::ConnectionInfo> Platform::PlatformAdaptor::serialPortConnectionInfos()
{
    QVector<Traffic::ConnectionInfo> result;

    auto deviceListJNI = QJniObject::callStaticObjectMethod(
        "de/akaflieg_freiburg/enroute/UsbSerialHelper",
        "listSerialDevices",
        "()[Ljava/lang/String;"
        );
    if (deviceListJNI.isValid())
    {
        const QJniEnvironment env;
        auto length = env->GetArrayLength(deviceListJNI.object<jobjectArray>());
        for (jsize i = 0; i < length; ++i)
        {
            const QJniObject jString = env->GetObjectArrayElement(deviceListJNI.object<jobjectArray>(), i);
            if (jString.isValid())
            {
                result += Traffic::ConnectionInfo(jString.toString());
            }
        }
    }
    return result + PlatformAdaptor_Abstract::serialPortConnectionInfos();
}

QString Platform::PlatformAdaptor::systemInfo()
{
    auto result = Platform::PlatformAdaptor_Abstract::systemInfo();

    // Device Name
    QJniObject const stringObject
        = QJniObject::callStaticObjectMethod<jstring>("de/akaflieg_freiburg/enroute/MobileAdaptor",
                                                      "deviceName");
    result += u"<h3>Device</h3>\n"_s;
    result += stringObject.toString();

    // System Log
    QProcess proc;
    proc.startCommand(u"logcat -t 300"_s);
    proc.waitForFinished();
    result += u"<h3>System Log</h3>\n"_s;
    result += u"<pre>\n"_s + proc.readAllStandardOutput() + u"\n</pre>\n"_s;

    return result;
}

void Platform::PlatformAdaptor::vibrateBrief()
{
    QJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "vibrateBrief");
}

void Platform::PlatformAdaptor::vibrateLong()
{
    QJniObject::callStaticMethod<void>("de/akaflieg_freiburg/enroute/MobileAdaptor", "vibrateLong");
}


//
// C Methods
//

extern "C" {

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_MobileAdaptor_onLanguageChanged(JNIEnv* /*unused*/, jobject /*unused*/)
{
    exit(0);
}

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_MobileAdaptor_onWifiConnected(JNIEnv* /*unused*/, jobject /*unused*/)
{
    // This method gets called from Java before main() has executed
    // and thus before a QApplication instance has been constructed.
    // In these cases, the methods of the Global class must not be called
    // and we simply return.
    if (GlobalObject::canConstruct())
    {
        emit GlobalObject::platformAdaptor()->wifiConnected();
    }
}

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_ShareActivity_onOpenUSBRequestReceived(JNIEnv* env, jobject /*unused*/, jstring deviceName)
{

    // This method gets called from Java before main() has executed
    // and thus before a QApplication instance has been constructed.
    // In these cases, the methods of the Global class must not be called
    // and we simply return.
    if (GlobalObject::canConstruct())
    {
        // A little complicated because GlobalObject::fileExchange() lives in a different thread
        const char* fname = env->GetStringUTFChars(deviceName, nullptr);
        QMetaObject::invokeMethod(GlobalObject::trafficDataProvider(), "addDataSource", Qt::QueuedConnection,
                                  Q_ARG( Traffic::ConnectionInfo, Traffic::ConnectionInfo(QString::fromUtf8(fname))) );
        env->ReleaseStringUTFChars(deviceName, fname);

        emit GlobalObject::platformAdaptor()->serialPortsChanged();
    }

}

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_UsbConnectionReceiver_onSerialPortConnectionsChanged(JNIEnv* /*unused*/, jobject /*unused*/)
{
    // This method gets called from Java before main() has executed
    // and thus before a QApplication instance has been constructed.
    // In these cases, the methods of the Global class must not be called
    // and we simply return.
    if (GlobalObject::canConstruct())
    {
        emit GlobalObject::platformAdaptor()->serialPortsChanged();
    }
}

JNIEXPORT void JNICALL
Java_de_akaflieg_1freiburg_enroute_UsbSerialHelper_onPermissionResult(JNIEnv* /*unused*/, jclass /*unused*/, jstring /*unused*/, jboolean /*unused*/)
{
    // This method gets called from Java before main() has executed
    // and thus before a QApplication instance has been constructed.
    // In these cases, the methods of the Global class must not be called
    // and we simply return.
    if (GlobalObject::canConstruct())
    {
        emit GlobalObject::platformAdaptor()->serialPortsChanged();
    }
}

}

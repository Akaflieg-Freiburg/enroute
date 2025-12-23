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
#include <QJniEnvironment>
#include <QJniObject>
#include <QPermissions>
#include <QProcess>
#include <QScreen>

#include "platform/PlatformAdaptor_Android.h"

using namespace Qt::Literals::StringLiterals;


Platform::PlatformAdaptor::PlatformAdaptor(QObject *parent)
    : Platform::PlatformAdaptor_Abstract(parent)
{
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

void Platform::PlatformAdaptor::openSatView(const QGeoCoordinate& coordinate)
{
    auto st = u"geo:%1,%2?z=10"_s.arg(coordinate.latitude()).arg(coordinate.longitude());
    QJniObject const c = QJniObject::fromString(st);
    auto ok = QJniObject::callStaticMethod<jboolean>("de/akaflieg_freiburg/enroute/MobileAdaptor",
                                                     "openInGoogleEarth",
                                                     "(Ljava/lang/String;)Z",
                                                     c.object<jstring>());
    if (!ok)
    {
        PlatformAdaptor_Abstract::openSatView(coordinate);
    }
}

QVector<Traffic::ConnectionInfo> Platform::PlatformAdaptor::serialPortConnectionInfos()
{
    QVector<Traffic::ConnectionInfo> result;

    // Get Android context
    auto *nativeInterface = QCoreApplication::instance()
                                ->nativeInterface<QNativeInterface::QAndroidApplication>();

    if (!nativeInterface) {
        qWarning() << "Failed to get native interface";
        return result;
    }

    QJniObject context = nativeInterface->context();
    if (!context.isValid()) {
        qWarning() << "Invalid context";
        return result;
    }

    // Get UsbManager system service
    QJniObject usbServiceString = QJniObject::fromString("usb");
    QJniObject usbManager = context.callObjectMethod(
        "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;",
        usbServiceString.object()
        );

    if (!usbManager.isValid()) {
        qWarning() << "Failed to get UsbManager";
        return result;
    }

    // Get default UsbSerialProber
    QJniObject prober = QJniObject::callStaticObjectMethod(
        "com/hoho/android/usbserial/driver/UsbSerialProber",
        "getDefaultProber",
        "()Lcom/hoho/android/usbserial/driver/UsbSerialProber;"
        );

    if (!prober.isValid()) {
        qWarning() << "Failed to get UsbSerialProber";
        return result;
    }

    // Get list of available drivers
    QJniObject driverList = prober.callObjectMethod(
        "findAllDrivers",
        "(Landroid/hardware/usb/UsbManager;)Ljava/util/List;",
        usbManager.object()
        );

    if (!driverList.isValid()) {
        qWarning() << "Failed to get driver list";
        return result;
    }

    // Get the size of the list
    int size = driverList.callMethod<jint>("size", "()I");
    qWarning() << "Found" << size << "USB serial device(s)";

    // Iterate through all drivers
    for (int i = 0; i < size; i++) {
        // Get driver at index i
        QJniObject driver = driverList.callObjectMethod(
            "get",
            "(I)Ljava/lang/Object;",
            i
            );

        if (!driver.isValid()) {
            continue;
        }


        // Get UsbDevice
        QJniObject usbDevice = driver.callObjectMethod(
            "getDevice",
            "()Landroid/hardware/usb/UsbDevice;"
            );

        if (usbDevice.isValid()) {
            // Get device name
            QJniObject deviceNameObj = usbDevice.callObjectMethod("getProductName", "()Ljava/lang/String;");
            QString serialPortNameOrDescription = deviceNameObj.toString();
            if (serialPortNameOrDescription.isEmpty())
            {
                QJniObject deviceNameObj = usbDevice.callObjectMethod("getDeviceName", "()Ljava/lang/String;");
                serialPortNameOrDescription = deviceNameObj.toString();
            }
            if (!serialPortNameOrDescription.isEmpty())
            {
                result += Traffic::ConnectionInfo(deviceNameObj.toString());
            }

            // device.deviceName = deviceNameObj.toString();

            // Get vendor ID
            // device.vendorId = usbDevice.callMethod<jint>("getVendorId", "()I");

            // Get product ID
            // device.productId = usbDevice.callMethod<jint>("getProductId", "()I");
        }
    }
    return result;
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
    const char* fname = env->GetStringUTFChars(deviceName, nullptr);
    qWarning() << "Received request to open USB Serial Port Device" << QString::fromUtf8(fname);
    /*
    // A little complicated because GlobalObject::fileExchange() lives in a different thread
    QMetaObject::invokeMethod( GlobalObject::fileExchange(),
                              "processText",
                              Qt::QueuedConnection,
                              Q_ARG( QString, QString::fromUtf8(fname)) );
    */
    env->ReleaseStringUTFChars(deviceName, fname);
}

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_UsbConnectionReceiver_onSerialPortConnectionsChanged(JNIEnv* /*unused*/, jobject /*unused*/)
{
    qDebug() << "onSerialPortConnectionsChanged";
}

}

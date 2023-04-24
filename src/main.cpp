/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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

#include <QCommandLineParser>
#include <QFile>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSettings>
#include <QTranslator>

#if defined(Q_OS_ANDROID) or defined(Q_OS_IOS)
#include <QtWebView/QtWebView>
#else
#include <QApplication>
#include <kdsingleapplication.h>
#endif

#include "DemoRunner.h"
#include "GlobalObject.h"
#include "dataManagement/DataManager.h"
#include "dataManagement/SSLErrorHandler.h"
#include "geomaps/Airspace.h"
#include "geomaps/GeoMapProvider.h"
#include "geomaps/WaypointLibrary.h"
#include "platform/FileExchange_Abstract.h"
#include "platform/Notifier_Abstract.h"
#include "platform/PlatformAdaptor_Abstract.h"
#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficFactor_WithPosition.h"
#include "weather/Station.h"
#include <chrono>

using namespace std::chrono_literals;

auto main(int argc, char *argv[]) -> int
{
    // It seems that MapBoxGL does not work well with threaded rendering, so we disallow that.
    qputenv("QSG_RENDER_LOOP", "basic");

    // Register types
    qRegisterMetaType<GeoMaps::Airspace>();
    qRegisterMetaType<Platform::Notifier_Abstract::NotificationActions>();
    qRegisterMetaType<Platform::Notifier_Abstract::NotificationTypes>("Platform::Notifier::Notifications");
    qRegisterMetaType<Platform::FileExchange_Abstract::FileFunction>();
    qRegisterMetaType<Traffic::Warning>();

    qmlRegisterUncreatableType<DemoRunner>("enroute", 1, 0, "DemoRunner", QStringLiteral("DemoRunner objects cannot be created in QML"));
    qmlRegisterUncreatableType<DataManagement::SSLErrorHandler>("enroute", 1, 0, "SSLErrorHandler", QStringLiteral("SSLErrorHandler objects cannot be created in QML"));
    qmlRegisterUncreatableType<GeoMaps::GeoMapProvider>("enroute", 1, 0, "GeoMapProvider", QStringLiteral("GeoMapProvider objects cannot be created in QML"));
    qmlRegisterUncreatableType<GeoMaps::WaypointLibrary>("enroute", 1, 0, "WaypointLibrary", QStringLiteral("WaypointLibrary objects cannot be created in QML"));
    qmlRegisterUncreatableType<DataManagement::DataManager>("enroute", 1, 0, "DataManager", QStringLiteral("DataManager objects cannot be created in QML"));
    qmlRegisterUncreatableType<Traffic::TrafficDataProvider>("enroute", 1, 0, "TrafficDataProvider", QStringLiteral("TrafficDataProvider objects cannot be created in QML"));
    qmlRegisterUncreatableType<Platform::Notifier_Abstract>("enroute", 1, 0, "Notifier", QStringLiteral("Notifier objects cannot be created in QML"));
    qmlRegisterUncreatableType<Traffic::TrafficFactor_WithPosition>("enroute", 1, 0, "TrafficFactor_WithPosition", QStringLiteral("TrafficFactor_WithPosition objects cannot be created in QML"));
    qmlRegisterType<Weather::Station>("enroute", 1, 0, "WeatherStation");


    // Required by the maplibre plugin to QtLocation
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    // Set up application

#if defined(Q_OS_ANDROID) or defined(Q_OS_IOS)
    QtWebView::initialize();
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
    QGuiApplication::setDesktopFileName(QStringLiteral("de.akaflieg_freiburg.enroute"));
#endif
    QCoreApplication::setOrganizationName(QStringLiteral("Akaflieg Freiburg"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("akaflieg_freiburg.de"));
    QCoreApplication::setApplicationName(QStringLiteral("enroute flight navigation"));
    QCoreApplication::setApplicationVersion(QStringLiteral(PROJECT_VERSION));
    QGuiApplication::setWindowIcon(QIcon(u":/icons/appIcon.png"_qs));

    // Install translators
    auto* enrouteTranslator = new QTranslator(&app);
    if (enrouteTranslator->load(QStringLiteral(":i18n/enroute_%1.qm").arg(QLocale::system().name().left(2))))
    {
        QCoreApplication::installTranslator(enrouteTranslator);
    }
    else
    {
        delete enrouteTranslator;
    }
    enrouteTranslator = new QTranslator(&app);
    if (enrouteTranslator->load(QStringLiteral(":i18n/qtbase_%1.qm").arg(QLocale::system().name().left(2))))
    {
        QCoreApplication::installTranslator(enrouteTranslator);
    }
    else
    {
        delete enrouteTranslator;
    }
    enrouteTranslator = new QTranslator(&app);
    if (enrouteTranslator->load(QStringLiteral(":i18n/qtdeclarative_%1.qm").arg(QLocale::system().name().left(2))))
    {
        QCoreApplication::installTranslator(enrouteTranslator);
    }
    else
    {
        delete enrouteTranslator;
    }


    // Workaround for crappy Hauwei and Samsung devices.
    //
    // On Huawei devices, set the environment variable "QT_ANDROID_NO_EXIT_CALL", which
    // prevents an exit() call, and thereby prevents a crash on these devices. Same problem on Samsung Galaxy S21 devices with Android 12.
    qputenv("QT_ANDROID_NO_EXIT_CALL", "1");


    // Command line parsing
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Enroute Flight Navigation is a free nagivation app for VFR pilots,\ndeveloped as a project of Akaflieg Freiburg."));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption googlePlayScreenshotOption(QStringLiteral("sg"), QCoreApplication::translate("main", "Run simulator and generate screenshots for GooglePlay"));
    parser.addOption(googlePlayScreenshotOption);
    QCommandLineOption manualScreenshotOption(QStringLiteral("sm"), QCoreApplication::translate("main", "Run simulator and generate screenshots for the manual"));
    parser.addOption(manualScreenshotOption);
    parser.addPositionalArgument(QStringLiteral("[fileName]"), QCoreApplication::translate("main", "File to import."));
    parser.process(app);
    auto positionalArguments = parser.positionalArguments();
    if (positionalArguments.length() > 1)
    {
        parser.showHelp();
    }

#if !defined(Q_OS_ANDROID) and !defined(Q_OS_IOS)
    // Single application on desktops
    KDSingleApplication kdsingleapp;
    if (!kdsingleapp.isPrimaryInstance())
    {
        if (positionalArguments.length() > 0)
        {
            kdsingleapp.sendMessage(positionalArguments[0].toUtf8());
        }
        else
        {
            kdsingleapp.sendMessage(QByteArray());
        }
        return 0;
    }
#endif

    // Create mobile platform adaptor and ask to disable to screen saver.
    GlobalObject::platformAdaptor()->disableScreenSaver();
    if (positionalArguments.length() == 1)
    {
        GlobalObject::fileExchange()->processFileOpenRequest(positionalArguments[0]);
    }
#if !defined(Q_OS_ANDROID) and !defined(Q_OS_IOS)
    QObject::connect(&kdsingleapp,
                     &KDSingleApplication::messageReceived,
                     GlobalObject::fileExchange(),
                     qOverload<const QByteArray&>(&Platform::FileExchange_Abstract::processFileOpenRequest));
#endif

    /*
     * Set up ApplicationEngine for QML
     */

    auto* engine = new QQmlApplicationEngine();
    engine->rootContext()->setContextProperty(QStringLiteral("manual_location"), MANUAL_LOCATION );
    engine->rootContext()->setContextProperty(QStringLiteral("global"), new GlobalObject(engine) );
    engine->load(u"qrc:/qml/main.qml"_qs);

    if (parser.isSet(googlePlayScreenshotOption))
    {
        GlobalObject::demoRunner()->setEngine(engine);
        QTimer::singleShot(1s, GlobalObject::demoRunner(), &DemoRunner::generateGooglePlayScreenshots);
    }
    if (parser.isSet(manualScreenshotOption))
    {
        GlobalObject::demoRunner()->setEngine(engine);
        QTimer::singleShot(1s, GlobalObject::demoRunner(), &DemoRunner::generateManualScreenshots);
    }

    // Load GUI and enter event loop
    auto result = QGuiApplication::exec();

    // Ensure that the engine does not hold objects that will interfere when we close down.
    delete engine;
    GlobalObject::clear();

    return result;
}

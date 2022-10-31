/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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
#include <QLibraryInfo>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>
#include <QSettings>
#include <QTranslator>

#if defined(Q_OS_ANDROID)
#include <QtWebView/QtWebView>
#else
#include <QApplication>
#include <kdsingleapplication.h>
#endif

#include "DemoRunner.h"
#include "GlobalObject.h"
#include "Librarian.h"
#include "Settings.h"
#include "dataManagement/DataManager.h"
#include "dataManagement/SSLErrorHandler.h"
#include "geomaps/Airspace.h"
#include "geomaps/GeoMapProvider.h"
#include "geomaps/WaypointLibrary.h"
#include "navigation/Aircraft.h"
#include "navigation/Clock.h"
#include "navigation/Navigator.h"
#include "platform/Notifier.h"
#include "platform/PlatformAdaptor_Android.h"
#include "platform/PlatformAdaptor_Linux.h"
#include "positioning/PositionProvider.h"
#include "traffic/PasswordDB.h"
#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficFactor_WithPosition.h"
#include "ui/ScaleQuickItem.h"
#include "units/Angle.h"
#include "units/Distance.h"
#include "units/Speed.h"
#include "units/Time.h"
#include "units/Volume.h"
#include "units/VolumeFlow.h"
#include "weather/WeatherDataProvider.h"
#include <chrono>

using namespace std::chrono_literals;

auto main(int argc, char *argv[]) -> int
{
    // It seems that MapBoxGL does not work well with threaded rendering, so we disallow that.
    qputenv("QSG_RENDER_LOOP", "basic");

    // Register types
    qRegisterMetaType<Units::Angle>();
    qRegisterMetaType<Units::Distance>();
    qRegisterMetaType<Units::Speed>();
    qRegisterMetaType<Units::Time>();
    qRegisterMetaType<Units::Volume>();
    qRegisterMetaType<Units::VolumeFlow>();
    qRegisterMetaType<GeoMaps::Airspace>();
    qRegisterMetaType<GeoMaps::Waypoint>();
    qRegisterMetaType<Positioning::PositionInfo>();
    qRegisterMetaType<Traffic::Warning>();
    qRegisterMetaType<Platform::Notifier::NotificationActions>();

    qRegisterMetaType<Platform::PlatformAdaptor::FileFunction>("Platform::PlatformAdaptor::FileFunction");
    qRegisterMetaType<Platform::Notifier::NotificationTypes>("Platform::Notifier::Notifications");
    qmlRegisterUncreatableType<DemoRunner>("enroute", 1, 0, "DemoRunner", QStringLiteral("DemoRunner objects cannot be created in QML"));
    qmlRegisterUncreatableType<Navigation::Aircraft>("enroute", 1, 0, "Aircraft", QStringLiteral("Aircraft objects cannot be created in QML"));
    qmlRegisterUncreatableType<Navigation::RemainingRouteInfo>("enroute", 1, 0, "RemainingRouteInfo", QStringLiteral("RemainingRouteInfo objects cannot be created in QML"));
    qmlRegisterType<Navigation::Clock>("enroute", 1, 0, "Clock");
    qmlRegisterUncreatableType<DataManagement::SSLErrorHandler>("enroute", 1, 0, "SSLErrorHandler", QStringLiteral("SSLErrorHandler objects cannot be created in QML"));
    qmlRegisterUncreatableType<DataManagement::Downloadable_Abstract>("enroute", 1, 0, "Downloadable_Abstract", QStringLiteral("Downloadable_Abstract objects cannot be created in QML"));
    qmlRegisterUncreatableType<DataManagement::Downloadable_SingleFile>("enroute", 1, 0, "Downloadable_SingleFile", QStringLiteral("Downloadable_SingleFile objects cannot be created in QML"));
    qmlRegisterUncreatableType<Librarian>("enroute", 1, 0, "Librarian", QStringLiteral("Librarian objects cannot be created in QML"));
    qmlRegisterUncreatableType<GeoMaps::GeoMapProvider>("enroute", 1, 0, "GeoMapProvider", QStringLiteral("GeoMapProvider objects cannot be created in QML"));
    qmlRegisterUncreatableType<GeoMaps::WaypointLibrary>("enroute", 1, 0, "WaypointLibrary", QStringLiteral("WaypointLibrary objects cannot be created in QML"));
    qmlRegisterUncreatableType<DataManagement::DataManager>("enroute", 1, 0, "DataManager", QStringLiteral("DataManager objects cannot be created in QML"));
    qmlRegisterType<Settings>("enroute", 1, 0, "GlobalSettings");
    qmlRegisterUncreatableType<Platform::PlatformAdaptor_Abstract>("enroute", 1, 0, "PlatformAdaptor_Abstract", QStringLiteral("PlatformAdaptor_Abstract objects cannot be created in QML"));
    qmlRegisterUncreatableType<Navigation::Navigator>("enroute", 1, 0, "Navigator", QStringLiteral("Navigator objects cannot be created in QML"));
    qmlRegisterUncreatableType<Traffic::PasswordDB>("enroute", 1, 0, "PasswordDB", QStringLiteral("PasswordDB objects cannot be created in QML"));
    qmlRegisterUncreatableType<Traffic::TrafficDataProvider>("enroute", 1, 0, "TrafficDataProvider", QStringLiteral("TrafficDataProvider objects cannot be created in QML"));
    qmlRegisterUncreatableType<Platform::Notifier>("enroute", 1, 0, "Notifier", QStringLiteral("Notifier objects cannot be created in QML"));
    qmlRegisterUncreatableType<Positioning::PositionProvider>("enroute", 1, 0, "PositionProvider", QStringLiteral("PositionProvider objects cannot be created in QML"));
    qmlRegisterUncreatableType<Traffic::TrafficFactor_WithPosition>("enroute", 1, 0, "TrafficFactor_WithPosition", QStringLiteral("TrafficFactor_WithPosition objects cannot be created in QML"));
    qmlRegisterType<Ui::ScaleQuickItem>("enroute", 1, 0, "Scale");
    qmlRegisterUncreatableType<Weather::WeatherDataProvider>("enroute", 1, 0, "WeatherProvider", QStringLiteral("Weather::WeatherProvider objects cannot be created in QML"));
    qmlRegisterType<Weather::Station>("enroute", 1, 0, "WeatherStation");

    // Initialize web view on platforms where we use it
#if defined(Q_OS_ANDROID)
    QtWebView::initialize();
#endif

    // Set up application
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#if defined(Q_OS_ANDROID)
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif
    QCoreApplication::setOrganizationName(QStringLiteral("Akaflieg Freiburg"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("akaflieg_freiburg.de"));
    QCoreApplication::setApplicationName(QStringLiteral("enroute flight navigation"));
    QCoreApplication::setApplicationVersion(QStringLiteral(PROJECT_VERSION));
    QGuiApplication::setWindowIcon(QIcon(":/icons/appIcon.png"));
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    QGuiApplication::setDesktopFileName(QStringLiteral("de.akaflieg_freiburg.enroute"));
#endif

    // Install translator
    auto* enrouteTranslator = new QTranslator(&app);
    if (enrouteTranslator->load(QStringLiteral(":enroute_%1.qm").arg(QLocale::system().name().left(2))))
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
    QCommandLineOption screenshotOption(QStringLiteral("s"), QCoreApplication::translate("main", "Run simulator and generate screenshots for manual"));
    parser.addOption(screenshotOption);
    parser.addPositionalArgument(QStringLiteral("[fileName]"), QCoreApplication::translate("main", "File to import."));
    parser.process(app);
    auto positionalArguments = parser.positionalArguments();
    if (positionalArguments.length() > 1)
    {
        parser.showHelp();
    }

#if !defined(Q_OS_ANDROID)
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

    // Create mobile platform adaptor. We do this before creating the application engine because this also asks for permissions
    GlobalObject::platformAdaptor()->requestPermissionsSync();
    if (positionalArguments.length() == 1)
    {
        GlobalObject::platformAdaptor()->processFileOpenRequest(positionalArguments[0]);
    }
#if !defined(Q_OS_ANDROID)
    QObject::connect(&kdsingleapp, SIGNAL(messageReceived(QByteArray)), GlobalObject::platformAdaptor(), SLOT(processFileOpenRequest(QByteArray)));
#endif

    /*
     * Set up ApplicationEngine for QML
     */
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("angle"), QVariant::fromValue(Units::Angle()) );
    engine.rootContext()->setContextProperty(QStringLiteral("distance"), QVariant::fromValue(Units::Distance()) );
    engine.rootContext()->setContextProperty(QStringLiteral("manual_location"), MANUAL_LOCATION );
    engine.rootContext()->setContextProperty(QStringLiteral("global"), new GlobalObject(&engine) );
    engine.rootContext()->setContextProperty(QStringLiteral("leg"), QVariant::fromValue(Navigation::Leg()) );
    engine.rootContext()->setContextProperty(QStringLiteral("speed"), QVariant::fromValue(Units::Speed()) );
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (parser.isSet(screenshotOption))
    {
        GlobalObject::demoRunner()->setEngine(&engine);
        QTimer::singleShot(1s, GlobalObject::demoRunner(), &DemoRunner::run);
    }

    // Load GUI and enter event loop
    return QGuiApplication::exec();
}

/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

#if !defined(Q_OS_ANDROID)
#include <QApplication>
#include <kdsingleapplication.h>
#endif

#include "Aircraft.h"
#include "Clock.h"
#include "FlightRoute.h"
#include "GlobalSettings.h"
#include "Librarian.h"
#include "MobileAdaptor.h"
#include "geomaps/Airspace.h"
#include "geomaps/GeoMapProvider.h"
#include "geomaps/MapManager.h"
#include "Global.h"
#include "navigation/Navigator.h"
#include "positioning/PositionProvider.h"
#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficFactor.h"
#include "ui/ScaleQuickItem.h"
#include "units/Angle.h"
#include "units/Distance.h"
#include "units/Speed.h"
#include "units/Time.h"
#include "weather/DownloadManager.h"
#include "weather/Wind.h"
#include <chrono>

using namespace std::chrono_literals;

auto main(int argc, char *argv[]) -> int
{
    // It seems that MapBoxGL does not work well with threaded rendering, so we disallow that.
    qputenv("QSG_RENDER_LOOP", "basic");

    // Register types
    qRegisterMetaType<AviationUnits::Angle>();
    qRegisterMetaType<AviationUnits::Distance>();
    qRegisterMetaType<AviationUnits::Speed>();
    qRegisterMetaType<AviationUnits::Time>();
    qRegisterMetaType<GeoMaps::Airspace>();
    qRegisterMetaType<Positioning::PositionInfo>();
    qRegisterMetaType<Traffic::Warning>();

    qRegisterMetaType<MobileAdaptor::FileFunction>("MobileAdaptor::FileFunction");
    qmlRegisterType<Clock>("enroute", 1, 0, "Clock");
    qmlRegisterType<GeoMaps::DownloadableGroup>("enroute", 1, 0, "DownloadableGroup");
    qmlRegisterType<GeoMaps::DownloadableGroupWatcher>("enroute", 1, 0, "DownloadableGroupWatcher");
    qmlRegisterUncreatableType<GeoMaps::GeoMapProvider>("enroute", 1, 0, "GeoMapProvider", "GeoMapProvider objects cannot be created in QML");
    qmlRegisterUncreatableType<GeoMaps::MapManager>("enroute", 1, 0, "MapManager", "MapManager objects cannot be created in QML");
    qmlRegisterType<GlobalSettings>("enroute", 1, 0, "GlobalSettings");
    qmlRegisterUncreatableType<MobileAdaptor>("enroute", 1, 0, "MobileAdaptor", "MobileAdaptor objects cannot be created in QML");
    qmlRegisterUncreatableType<Traffic::TrafficDataProvider>("enroute", 1, 0, "FLARMAdaptor", "FLARMAdaptor objects cannot be created in QML");
    qmlRegisterUncreatableType<Positioning::PositionProvider>("enroute", 1, 0, "SatNav", "SatNav objects cannot be created in QML");
    qmlRegisterUncreatableType<Traffic::TrafficFactor>("enroute", 1, 0, "TrafficFactor", "TrafficFactor objects cannot be created in QML");
    qmlRegisterType<Ui::ScaleQuickItem>("enroute", 1, 0, "Scale");
    qmlRegisterUncreatableType<Weather::DownloadManager>("enroute", 1, 0, "WeatherDownloadManager", "Weather::DownloadManager objects cannot be created in QML");
    qmlRegisterType<Weather::Station>("enroute", 1, 0, "WeatherStation");
    qmlRegisterType<GeoMaps::Waypoint>("enroute", 1, 0, "Waypoint");

    // Set up application
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#if defined(Q_OS_ANDROID)
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif
    QCoreApplication::setOrganizationName("Akaflieg Freiburg");
    QCoreApplication::setOrganizationDomain("akaflieg_freiburg.de");
    QCoreApplication::setApplicationName("enroute flight navigation");
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);
    QGuiApplication::setWindowIcon(QIcon(":/icons/appIcon.png"));
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    QGuiApplication::setDesktopFileName("de.akaflieg_freiburg.enroute");
#endif

    // Command line parsing
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Enroute Flight Navigation is a free nagivation app for VFR pilots, developed as a project of Akaflieg Freiburg."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("[fileName]", QCoreApplication::translate("main", "File to import."));
    parser.process(app);
    auto positionalArguments = parser.positionalArguments();
    if (positionalArguments.length() > 1) {
        parser.showHelp();
    }

#if !defined(Q_OS_ANDROID)
    // Single application on desktops
    KDSingleApplication kdsingleapp;
    if (!kdsingleapp.isPrimaryInstance()) {
        if (positionalArguments.length() > 0) {
            kdsingleapp.sendMessage(positionalArguments[0].toUtf8());
        } else {
            kdsingleapp.sendMessage(QByteArray());
        }
        return 0;
    }
#endif

    // Create mobile platform adaptor. We do this before creating the application engine because this also asks for permissions
    if (positionalArguments.length() == 1) {
        MobileAdaptor::globalInstance()->processFileOpenRequest(positionalArguments[0]);
    }
#if !defined(Q_OS_ANDROID)
    QObject::connect(&kdsingleapp, SIGNAL(messageReceived(QByteArray)), MobileAdaptor::globalInstance(), SLOT(processFileOpenRequest(QByteArray)));
#endif

    /*
     * Set up ApplicationEngine for QML
     */
    auto *engine = new QQmlApplicationEngine();

    // Make global objects available to QML engine
    Global globalObjectStorage;
    engine->rootContext()->setContextProperty("global", &globalObjectStorage);

    // Make GPS available to QML engine
    engine->rootContext()->setContextProperty("positionProvider", Positioning::PositionProvider::globalInstance());

    // Make FLARM available to QML engine
    engine->rootContext()->setContextProperty("flarmAdaptor", Traffic::TrafficDataProvider::globalInstance());

    // Attach global settings object
    engine->rootContext()->setContextProperty("globalSettings", GlobalSettings::globalInstance());

    // Make MobileAdaptor available to QML engine
    QTimer::singleShot(4s, MobileAdaptor::globalInstance(), &MobileAdaptor::hideSplashScreen);
    engine->rootContext()->setContextProperty("mobileAdaptor", MobileAdaptor::globalInstance());

    // Attach library info
    engine->rootContext()->setContextProperty("librarian", Librarian::globalInstance());

    // Attach aircraft info
    auto *aircraft = new Aircraft(engine);
    engine->rootContext()->setContextProperty("aircraft", aircraft);

    // Attach wind info
    auto *wind = new Weather::Wind(engine);
    engine->rootContext()->setContextProperty("wind", wind);

    // Attach clock
    engine->rootContext()->setContextProperty("clock", Clock::globalInstance());

    // Attach flight route
    engine->rootContext()->setContextProperty("flightRoute", FlightRoute::globalInstance());

    // Attach map manager
    engine->rootContext()->setContextProperty("mapManager", Global::mapManager());
    QObject::connect(Global::mapManager()->geoMaps(), &GeoMaps::DownloadableGroup::downloadingChanged, MobileAdaptor::globalInstance(), &MobileAdaptor::showDownloadNotification);

    // Attach geo map provider
    engine->rootContext()->setContextProperty("geoMapProvider", GeoMaps::GeoMapProvider::globalInstance());

    // Attach Weather::DownloadManager
    engine->rootContext()->setContextProperty("weatherDownloadManager", Weather::DownloadManager::globalInstance());
    GeoMaps::GeoMapProvider::globalInstance()->setDownloadManager(Weather::DownloadManager::globalInstance());

    // Attach Navigation::Navigator
    engine->rootContext()->setContextProperty("navigator", Navigation::Navigator::globalInstance());

    // Restore saved settings and make them available to QML
    QSettings settings;
    engine->rootContext()->setContextProperty("savedCenter", settings.value("Map/center", QVariant::fromValue(QGeoCoordinate(48.022653, 7.832583))));
    engine->rootContext()->setContextProperty("savedBearing", settings.value("Map/bearing", 0.0));
    engine->rootContext()->setContextProperty("savedZoomLevel", settings.value("Map/zoomLevel", 9));

    // Load GUI
    engine->load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine->rootObjects().isEmpty()) {
        return -1;
    }

    // Enter event loop
    QGuiApplication::exec();

    // Save settings
    // Obtain a pointer to the flightMap
    QQuickItem *flightMap = nullptr;
    foreach (auto rootItem, engine->rootObjects()) {
        flightMap = rootItem->findChild<QQuickItem*>("flightMap");
        if (flightMap != nullptr) {
            break;
        }
    }
    if (flightMap != nullptr) {
        settings.setValue("Map/center", QQmlProperty::read(flightMap, "center"));
        settings.setValue("Map/bearing", QQmlProperty::read(flightMap, "bearing"));
        settings.setValue("Map/zoomLevel", QQmlProperty::read(flightMap, "zoomLevel"));
    }

    // Ensure that things get deleted in the right order
    delete engine;

    return 0;
}

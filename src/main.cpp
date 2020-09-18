/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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
#include <QQuickItem>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlProperty>
#include <QSettings>
#include <QTranslator>

#if !defined(Q_OS_ANDROID)
#include <QApplication>
#include <kdsingleapplication.h>
#endif

#include "Aircraft.h"
#include "Clock.h"
#include "FlightRoute.h"
#include "GeoMapProvider.h"
#include "GlobalSettings.h"
#include "Librarian.h"
#include "MapManager.h"
#include "Meteorologist.h"
#include "MobileAdaptor.h"
#include "SatNav.h"
#include "ScaleQuickItem.h"
#include "WeatherReport.h"
#include "Wind.h"

int main(int argc, char *argv[])
{
    // Register types
    qRegisterMetaType<MobileAdaptor::FileFunction>("MobileAdaptor::FileFunction");
    qmlRegisterType<Airspace>("enroute", 1, 0, "Airspace");
    qmlRegisterType<Clock>("enroute", 1, 0, "Clock");
    qmlRegisterType<DownloadableGroup>("enroute", 1, 0, "DownloadableGroup");
    qmlRegisterType<DownloadableGroup>("enroute", 1, 0, "DownloadableGroupWatcher");
    qmlRegisterUncreatableType<MobileAdaptor>("enroute", 1, 0, "MobileAdaptor", "MobileAdaptor objects cannot be created in QML");
    qmlRegisterUncreatableType<SatNav>("enroute", 1, 0, "SatNav", "SatNav objects cannot be created in QML");
    qmlRegisterUncreatableType<Meteorologist>("enroute", 1, 0, "Meteorologist", "Meteorologist objects cannot be created in QML");
    qmlRegisterType<ScaleQuickItem>("enroute", 1, 0, "Scale");
    qmlRegisterType<WeatherReport>("enroute", 1, 0, "WeatherReport");
    qmlRegisterType<Waypoint>("enroute", 1, 0, "Waypoint");

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
    if (positionalArguments.length() > 1)
        parser.showHelp();

#if !defined(Q_OS_ANDROID)
    // Single application on desktops
    KDSingleApplication kdsingleapp;
    if (!kdsingleapp.isPrimaryInstance()) {
        if (positionalArguments.length() > 0)
            kdsingleapp.sendMessage(positionalArguments[0].toUtf8());
        else
            kdsingleapp.sendMessage(QByteArray());
        return 0;
    }
#endif

    // Create global settings object. We do this before creating the application engine because this also installs translators.
    auto globalSettings = new GlobalSettings();

    // Create mobile platform adaptor. We do this before creating the application engine because this also asks for permissions
    auto *adaptor = new MobileAdaptor();
    if (positionalArguments.length() == 1)
        adaptor->processFileOpenRequest(positionalArguments[0]);
#if !defined(Q_OS_ANDROID)
    QObject::connect(&kdsingleapp, SIGNAL(messageReceived(const QByteArray &)), adaptor, SLOT(processFileOpenRequest(const QByteArray &)));
#endif

    /*
     * Set up ApplicationEngine for QML
     */
    auto engine = new QQmlApplicationEngine();
    QObject::connect(globalSettings, &GlobalSettings::preferEnglishChanged, engine, &QQmlApplicationEngine::retranslate);

    // Make GPS available to QML engine
    auto navEngine = new SatNav(engine);
    engine->rootContext()->setContextProperty("satNav", navEngine);

    // Attach global settings object
    engine->rootContext()->setContextProperty("globalSettings", globalSettings);

    // Make MobileAdaptor available to QML engine
    QTimer::singleShot(4000, adaptor, SLOT(hideSplashScreen()));
    engine->rootContext()->setContextProperty("mobileAdaptor", adaptor);

    // Attach library info
    auto librarian = new Librarian(engine);
    engine->rootContext()->setContextProperty("librarian", librarian);

    // Attach aircraft info
    auto aircraft = new Aircraft(engine);
    engine->rootContext()->setContextProperty("aircraft", aircraft);

    // Attach wind info
    auto wind = new Wind(engine);
    engine->rootContext()->setContextProperty("wind", wind);

    // Attach clock
    auto clock = new Clock(engine);
    engine->rootContext()->setContextProperty("clock", clock);

    // Attach map manager
    auto networkAccessManager = new QNetworkAccessManager();
    networkAccessManager->setTransferTimeout();
    auto mapManager = new MapManager(networkAccessManager);
    engine->rootContext()->setContextProperty("mapManager", mapManager);
    QObject::connect(mapManager->geoMaps(), &DownloadableGroup::downloadingChanged, adaptor, &MobileAdaptor::showDownloadNotification);

    // Attach geo map provider
    auto geoMapProvider = new GeoMapProvider(mapManager, globalSettings, librarian);
    engine->rootContext()->setContextProperty("geoMapProvider", geoMapProvider);

    // Attach flight route
    auto flightroute = new FlightRoute(aircraft, wind, engine);
    engine->rootContext()->setContextProperty("flightRoute", flightroute);

    // Attach meteorologist
    auto meteorologist = new Meteorologist(clock, navEngine, flightroute, globalSettings, networkAccessManager, engine);
    engine->rootContext()->setContextProperty("meteorologist", meteorologist);

    // Restore saved settings and make them available to QML
    QSettings settings;
    engine->rootContext()->setContextProperty("savedBearing", settings.value("Map/bearing", 0.0));
    engine->rootContext()->setContextProperty("savedZoomLevel", settings.value("Map/zoomLevel", 9));

    // Load GUI
    engine->load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine->rootObjects().isEmpty())
        return -1;

    // Enter event loop
    QGuiApplication::exec();

    // Save settings
    // Obtain a pointer to the flightMap
    QQuickItem *flightMap = nullptr;
    foreach (auto rootItem, engine->rootObjects()) {
        flightMap = rootItem->findChild<QQuickItem*>("flightMap");
        if (flightMap != nullptr)
            break;
    }
    if (flightMap) {
        settings.setValue("Map/bearing", QQmlProperty::read(flightMap, "bearing"));
        settings.setValue("Map/zoomLevel", QQmlProperty::read(flightMap, "zoomLevel"));
    }

    // Ensure that things get deleted in the right order
    delete engine;
    delete mapManager; // This will also delete geoMapProvider
    delete networkAccessManager;
    delete adaptor;

    return 0;
}

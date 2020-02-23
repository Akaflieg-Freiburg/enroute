/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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

#include <QFile>
#include <QGuiApplication>
#include <QIcon>
#include <QQuickItem>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlProperty>
#include <QSettings>

#include "Aircraft.h"
#include "FlightRoute.h"
#include "GeoMapProvider.h"
#include "GlobalSettings.h"
#include "MapManager.h"
#include "MobileAdaptor.h"
#include "SatNav.h"
#include "ScaleQuickItem.h"
#include "Wind.h"

int main(int argc, char *argv[])
{
    // Register QML types
    qmlRegisterType<Airspace>("enroute", 1, 0, "Airspace");
    qmlRegisterUncreatableType<SatNav>("enroute", 1, 0, "SatNav", "SatNav objects cannot be created in QML");
    qmlRegisterType<ScaleQuickItem>("enroute", 1, 0, "Scale");
    qmlRegisterType<Waypoint>("enroute", 1, 0, "Waypoint");

    // Set up application
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Akaflieg Freiburg");
    QCoreApplication::setOrganizationDomain("akaflieg_freiburg.de");
    QCoreApplication::setApplicationName("Enroute");
    QGuiApplication::setWindowIcon(QIcon(":/icons/appIcon.png"));
#if defined(Q_OS_LINUX)
    QGuiApplication::setDesktopFileName("de.akaflieg_freiburg.enroute");
#endif
    QSettings settings;

    /*
     * Set up ApplicationEngine for QML
     */
    QQmlApplicationEngine engine;

    // Make GPS available to QML engine
    auto navEngine = new SatNav(&engine);
    engine.rootContext()->setContextProperty("satNav", navEngine);

    // Make MobileAdaptor available to QML engine
    auto *adaptor = new MobileAdaptor(&engine);
#warning temporary hack
    adaptor->disableScreenLock(true);
    QTimer::singleShot(4000, adaptor, SLOT(hideSplashScreen()));
    engine.rootContext()->setContextProperty("MobileAdaptor", adaptor);

    // Attach global settings object
    auto globalSettings = new GlobalSettings(&engine);
    engine.rootContext()->setContextProperty("globalSettings", globalSettings);

    // Attach aircraft info
    auto aircraft = new Aircraft(&engine);
    engine.rootContext()->setContextProperty("aircraft", aircraft);

    // Attach wind info
    auto wind = new Wind(&engine);
    engine.rootContext()->setContextProperty("wind", wind);

    // Attach map manager
    auto networkAccessManager = new QNetworkAccessManager(&engine);
    auto mapManager = new MapManager(networkAccessManager, &engine);
    engine.rootContext()->setContextProperty("mapManager", mapManager);

    // Attach geo map provider
    auto geoMapProvider = new GeoMapProvider(mapManager, globalSettings, mapManager);
    engine.rootContext()->setContextProperty("geoMapProvider", geoMapProvider);

    // Attach flight route
    auto flightroute = new FlightRoute(aircraft, wind, &engine);
    engine.rootContext()->setContextProperty("flightRoute", flightroute);

    /*
     * Load large strings from files, in order to make them available to QML
     */
    {
        QFile file(":text/firstStart.html");
        file.open(QIODevice::ReadOnly);
        engine.rootContext()->setContextProperty("firstStartText", file.readAll());
    }
    {
        QFile file(":text/info_author.html");
        file.open(QIODevice::ReadOnly);
        engine.rootContext()->setContextProperty("infoText_author", file.readAll());
    }
    {
        QFile file(":text/info_enroute.html");
        file.open(QIODevice::ReadOnly);
        engine.rootContext()->setContextProperty("infoText_enroute", file.readAll());
    }
    {
        QFile file(":text/info_license.html");
        file.open(QIODevice::ReadOnly);
        engine.rootContext()->setContextProperty("infoText_license", file.readAll());
    }
    {
        QFile file(":text/participate.html");
        file.open(QIODevice::ReadOnly);
        engine.rootContext()->setContextProperty("participateText", file.readAll());
    }
    {
        QFile file(":text/whatsnew.html");
        file.open(QIODevice::ReadOnly);
        engine.rootContext()->setContextProperty("whatsnew", file.readAll());
    }

    // Restore saved settings and make them available to QML
    engine.rootContext()->setContextProperty("savedBearing", settings.value("Map/bearing", 0.0));
    engine.rootContext()->setContextProperty("savedZoomLevel", settings.value("Map/zoomLevel", 9));

    // Load GUI
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    // Enter event loop
    QGuiApplication::exec();

    // Save settings
    // Obtain a pointer to the flightMap
    QQuickItem *flightMap = nullptr;
    foreach (auto rootItem, engine.rootObjects()) {
        flightMap = rootItem->findChild<QQuickItem*>("flightMap");
        if (flightMap != nullptr)
            break;
    }
    if (flightMap) {
        settings.setValue("Map/bearing", QQmlProperty::read(flightMap, "bearing"));
        settings.setValue("Map/zoomLevel", QQmlProperty::read(flightMap, "zoomLevel"));
    }

    return 0;
}

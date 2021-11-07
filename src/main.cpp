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
#include <QtWebView/QtWebView>

#if !defined(Q_OS_ANDROID)
#include <QApplication>
#warning
//#include <kdsingleapplication.h>
#endif

#include "DemoRunner.h"
#include "GlobalObject.h"
#include "Librarian.h"
#include "MobileAdaptor.h"
#include "Settings.h"
#include "dataManagement/DataManager.h"
#include "dataManagement/SSLErrorHandler.h"
#include "geomaps/Airspace.h"
#include "geomaps/GeoMapProvider.h"
#include "navigation/Aircraft.h"
#include "navigation/Clock.h"
#include "navigation/Navigator.h"
#include "platform/Notifier.h"
#include "positioning/PositionProvider.h"
#include "traffic/PasswordDB.h"
#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficFactor_WithPosition.h"
#include "ui/ScaleQuickItem.h"
#include "units/Angle.h"
#include "units/Distance.h"
#include "units/Speed.h"
#include "units/Time.h"
#include "weather/WeatherDataProvider.h"
#include "weather/Wind.h"
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
    qRegisterMetaType<GeoMaps::Airspace>();
    qRegisterMetaType<GeoMaps::Waypoint>();
    qRegisterMetaType<Positioning::PositionInfo>();
    qRegisterMetaType<Traffic::Warning>();

    qRegisterMetaType<MobileAdaptor::FileFunction>("MobileAdaptor::FileFunction");
    qRegisterMetaType<Platform::Notifier::Notifications>("Platform::Notifier::Notifications");
    qmlRegisterUncreatableType<DemoRunner>("enroute", 1, 0, "DemoRunner", "DemoRunner objects cannot be created in QML");
    qmlRegisterType<Navigation::Aircraft>("enroute", 1, 0, "Aircraft");
    qmlRegisterType<Navigation::Clock>("enroute", 1, 0, "Clock");
    qmlRegisterUncreatableType<DataManagement::SSLErrorHandler>("enroute", 1, 0, "SSLErrorHandler", "SSLErrorHandler objects cannot be created in QML");
    qmlRegisterType<DataManagement::DownloadableGroup>("enroute", 1, 0, "DownloadableGroup");
    qmlRegisterType<DataManagement::DownloadableGroupWatcher>("enroute", 1, 0, "DownloadableGroupWatcher");
    qmlRegisterUncreatableType<Librarian>("enroute", 1, 0, "Librarian", "Librarian objects cannot be created in QML");
    qmlRegisterUncreatableType<GeoMaps::GeoMapProvider>("enroute", 1, 0, "GeoMapProvider", "GeoMapProvider objects cannot be created in QML");
    qmlRegisterUncreatableType<DataManagement::DataManager>("enroute", 1, 0, "DataManager", "DataManager objects cannot be created in QML");
    qmlRegisterType<Settings>("enroute", 1, 0, "GlobalSettings");
    qmlRegisterUncreatableType<MobileAdaptor>("enroute", 1, 0, "MobileAdaptor", "MobileAdaptor objects cannot be created in QML");
    qmlRegisterUncreatableType<Navigation::Navigator>("enroute", 1, 0, "Navigator", "Navigator objects cannot be created in QML");
    qmlRegisterUncreatableType<Traffic::PasswordDB>("enroute", 1, 0, "PasswordDB", "PasswordDB objects cannot be created in QML");
    qmlRegisterUncreatableType<Traffic::TrafficDataProvider>("enroute", 1, 0, "TrafficDataProvider", "TrafficDataProvider objects cannot be created in QML");
    qmlRegisterUncreatableType<Platform::Notifier>("enroute", 1, 0, "Notifier", "Notifier objects cannot be created in QML");
    qmlRegisterUncreatableType<Positioning::PositionProvider>("enroute", 1, 0, "PositionProvider", "PositionProvider objects cannot be created in QML");
    qmlRegisterUncreatableType<Traffic::TrafficFactor_WithPosition>("enroute", 1, 0, "TrafficFactor_WithPosition", "TrafficFactor_WithPosition objects cannot be created in QML");
    qmlRegisterType<Ui::ScaleQuickItem>("enroute", 1, 0, "Scale");
    qmlRegisterUncreatableType<Weather::WeatherDataProvider>("enroute", 1, 0, "WeatherProvider", "Weather::WeatherProvider objects cannot be created in QML");
    qmlRegisterType<Weather::Station>("enroute", 1, 0, "WeatherStation");

    // Initialize web view on platforms where we use it
    QtWebView::initialize();

    // Set up application
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
    parser.setApplicationDescription(QCoreApplication::translate("main", "Enroute Flight Navigation is a free nagivation app for VFR pilots,\ndeveloped as a project of Akaflieg Freiburg."));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption screenshotOption("s", QCoreApplication::translate("main", "Run simulator and generate screenshots for manual"));
    parser.addOption(screenshotOption);
    parser.addPositionalArgument("[fileName]", QCoreApplication::translate("main", "File to import."));
    parser.process(app);
    auto positionalArguments = parser.positionalArguments();
    if (positionalArguments.length() > 1) {
        parser.showHelp();
    }

#if !defined(Q_OS_ANDROID)
#warning
/*
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
*/
#endif

    // Create mobile platform adaptor. We do this before creating the application engine because this also asks for permissions
    if (positionalArguments.length() == 1) {
        GlobalObject::mobileAdaptor()->processFileOpenRequest(positionalArguments[0]);
    }
    QTimer::singleShot(4s, GlobalObject::mobileAdaptor(), &MobileAdaptor::hideSplashScreen);
#if !defined(Q_OS_ANDROID)
#warning
//    QObject::connect(&kdsingleapp, SIGNAL(messageReceived(QByteArray)), GlobalObject::mobileAdaptor(), SLOT(processFileOpenRequest(QByteArray)));
#endif

    /*
     * Set up ApplicationEngine for QML
     */
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("angle", QVariant::fromValue(Units::Angle()) );
    engine.rootContext()->setContextProperty("manual_location", MANUAL_LOCATION );
    engine.rootContext()->setContextProperty("global", new GlobalObject(&engine) );
    engine.rootContext()->setContextProperty("speed", QVariant::fromValue(Units::Speed()) );
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (parser.isSet(screenshotOption)) {
        GlobalObject::demoRunner()->setEngine(&engine);
        QTimer::singleShot(1s, GlobalObject::demoRunner(), &DemoRunner::run);
    }

    // Load GUI and enter event loop
    return QGuiApplication::exec();
}

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

#include <QCommandLineParser>
#include <QFile>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QSettings>
#include <QTranslator>

#if __has_include (<QtWebView/QtWebView>)
#include <QtWebView/QtWebView>
#endif
#if __has_include (<QApplication>)
#include <QApplication>
#endif

#if !defined(Q_OS_ANDROID) and !defined(Q_OS_IOS)
#include <kdsingleapplication.h>
#endif

#if defined(Q_OS_IOS)
#include "ios/ObjCAdapter.h"
#endif

#include "config.h"
#include "DemoRunner.h"
#include "GlobalObject.h"
#include "Librarian.h"
#include "geomaps/Airspace.h"
#include "platform/FileExchange.h"
#include "platform/PlatformAdaptor_Abstract.h"
#include "traffic/Warning.h"

using namespace std::chrono_literals;
using namespace Qt::Literals::StringLiterals;

auto main(int argc, char *argv[]) -> int
{
    // It seems that MapBoxGL does not work well with threaded rendering, so we disallow that.
    qputenv("QSG_RENDER_LOOP", "basic");
    // Might help with ANRs under Android
    qputenv("QT_ANDROID_DISABLE_ACCESSIBILITY", "1");

    // Register types
    qRegisterMetaType<GeoMaps::Airspace>();
    qRegisterMetaType<Platform::FileExchange_Abstract::FileFunction>();
    qRegisterMetaType<Traffic::Warning>();

    // Required by the maplibre plugin to QtLocation
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

// Set up application
#if __has_include (<QtWebView/QtWebView>)
    QtWebView::initialize();
#endif
#if defined(Q_OS_ANDROID) or defined(Q_OS_IOS)
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
    QGuiApplication::setDesktopFileName(QStringLiteral("de.akaflieg_freiburg.enroute"));
#endif
    QCoreApplication::setOrganizationName(QStringLiteral("Akaflieg Freiburg"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("akaflieg_freiburg.de"));
    QCoreApplication::setApplicationName(QStringLiteral("enroute flight navigation"));
    QCoreApplication::setApplicationVersion(QStringLiteral(ENROUTE_VERSION_STRING));
    QGuiApplication::setWindowIcon(QIcon(u":/icons/appIcon.png"_s));

    // Install translators
    auto preferredLanguage = GlobalObject::platformAdaptor()->language();
    auto* enrouteTranslator = new QTranslator(&app);
    if (enrouteTranslator->load(QStringLiteral(":i18n/enroute_%1.qm").arg(preferredLanguage)))
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
    QCommandLineOption const googlePlayScreenshotOption(
        QStringLiteral("sg"),
        QCoreApplication::translate("main",
                                    "Run simulator and generate screenshots for GooglePlay"));
    parser.addOption(googlePlayScreenshotOption);
    QCommandLineOption const iosScreenshotOption(
        QStringLiteral("si"),
        QCoreApplication::translate("main", "Run simulator and generate screenshots for iOS"));
    parser.addOption(iosScreenshotOption);
    QCommandLineOption const manualScreenshotOption(
        QStringLiteral("sm"),
        QCoreApplication::translate("main",
                                    "Run simulator and generate screenshots for the manual"));
    parser.addOption(manualScreenshotOption);
    QCommandLineOption const extractStringOption(
        u"string"_s,
        QCoreApplication::translate(
            "main", "look up string using Librarian::getStringFromRessource and print it to stdout"),
        QCoreApplication::translate("main", "string name"));
    parser.addOption(extractStringOption);
    parser.addPositionalArgument(QStringLiteral("[fileName]"), QCoreApplication::translate("main", "File to import."));
    parser.process(app);

    auto positionalArguments = parser.positionalArguments();
    if (positionalArguments.length() > 1)
    {
        parser.showHelp(-1);
    }
    QString const stringName = parser.value(extractStringOption);
    if (!stringName.isEmpty())
    {
        QTextStream out(stdout);
        out << Librarian::getStringFromRessource(stringName);
        return 0;
    }

#if !defined(Q_OS_ANDROID) and !defined(Q_OS_IOS)
    // Single application on desktops
    KDSingleApplication kdsingleapp;
    if (!kdsingleapp.isPrimaryInstance())
    {
        if (!positionalArguments.empty()) {
            kdsingleapp.sendMessage(positionalArguments[0].toUtf8());
        } else {
            kdsingleapp.sendMessage(QByteArray());
        }
        return 0;
    }
#endif

    // Create mobile platform adaptor and ask to disable to screen saver.
    GlobalObject::platformAdaptor()->disableScreenSaver();
    if (positionalArguments.length() == 1)
    {
        GlobalObject::fileExchange()->processFileOpenRequest(positionalArguments[0], {});
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
    QQuickStyle::setStyle(u"Material"_s);

    auto* engine = new QQmlApplicationEngine();
    engine->addImportPath(u":/"_s);

#if defined(Q_OS_IOS)
    engine->rootContext()->setContextProperty(QStringLiteral("manual_location"), QCoreApplication::applicationDirPath()+"/manual/");
#else
    engine->rootContext()->setContextProperty(QStringLiteral("manual_location"), MANUAL_LOCATION );
#endif
    engine->rootContext()->setContextProperty(QStringLiteral("global"), new GlobalObject(engine) );
    engine->load(u"qrc:/qml/main.qml"_s);
#if defined(Q_OS_ANDROID)
    QNativeInterface::QAndroidApplication::hideSplashScreen(1);
#endif

    if (parser.isSet(googlePlayScreenshotOption))
    {
        GlobalObject::demoRunner()->setEngine(engine);
        QTimer::singleShot(1s, GlobalObject::demoRunner(), &DemoRunner::generateGooglePlayScreenshots);
    }
    if (parser.isSet(iosScreenshotOption))
    {
        GlobalObject::demoRunner()->setEngine(engine);
        QTimer::singleShot(3s, GlobalObject::demoRunner(), &DemoRunner::generateIosScreenshots);
    }
    if (parser.isSet(manualScreenshotOption))
    {
        GlobalObject::demoRunner()->setEngine(engine);
        QTimer::singleShot(1s, GlobalObject::demoRunner(), &DemoRunner::generateManualScreenshots);
    }

    // Load GUI and enter event loop
    auto result = QGuiApplication::exec();

#if defined(Q_OS_IOS)
    exit(result);
#endif

    // Ensure that the engine does not hold objects that will interfere when we close down.
    delete engine;
    GlobalObject::clear();

    return result;
}

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

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QQmlEngine>

#include "DemoRunner.h"
#include "GlobalObject.h"
#include "GlobalSettings.h"
#include "Librarian.h"
#include "dataManagement/DataManager.h"
#include "dataManagement/SSLErrorHandler.h"
#include "geomaps/GeoMapProvider.h"
#include "geomaps/WaypointLibrary.h"
#include "navigation/Clock.h"
#include "navigation/Navigator.h"
#include "notam/NotamProvider.h"
#include "notification/NotificationManager.h"
#include "platform/FileExchange.h"
#include "platform/PlatformAdaptor.h"
#include "positioning/PositionProvider.h"
#include "traffic/FlarmnetDB.h"
#include "traffic/PasswordDB.h"
#include "traffic/TrafficDataProvider.h"
#include "weather/WeatherDataProvider.h"

bool isConstructingOrDeconstructing {false};

QPointer<Navigation::Clock> g_clock {};
QPointer<DataManagement::DataManager> g_dataManager {};
QPointer<DataManagement::SSLErrorHandler> g_sslErrorHandler {};
QPointer<DemoRunner> g_demoRunner {};
QPointer<Platform::FileExchange> g_fileExchange {};
QPointer<Traffic::FlarmnetDB> g_flarmnetDB {};
QPointer<GeoMaps::GeoMapProvider> g_geoMapProvider {};
QPointer<Librarian> g_librarian {};
QPointer<Platform::PlatformAdaptor> g_platformAdaptor {};
QPointer<Navigation::Navigator> g_navigator {};
QPointer<NOTAM::NotamProvider> g_notamProvider {};
QPointer<QNetworkAccessManager> g_networkAccessManager {};
QPointer<Notifications::NotificationManager> g_notificationManager {};
QPointer<Traffic::PasswordDB> g_passwordDB {};
QPointer<Positioning::PositionProvider> g_positionProvider {};
QPointer<GlobalSettings> g_globalSettings {};
QPointer<Traffic::TrafficDataProvider> g_trafficDataProvider {};
QPointer<GeoMaps::WaypointLibrary> g_waypointLibrary {};
QPointer<Weather::WeatherDataProvider> g_weatherDataProvider {};


template<typename T> auto GlobalObject::allocateInternal(QPointer<T>& pointer) -> T*
{
    Q_ASSERT( QCoreApplication::instance() != nullptr );
    Q_ASSERT( !isConstructingOrDeconstructing );
    if (pointer.isNull()) {
        isConstructingOrDeconstructing = true;
        pointer = new T( QCoreApplication::instance() );
        isConstructingOrDeconstructing = false;
        QQmlEngine::setObjectOwnership(pointer, QQmlEngine::CppOwnership);
        auto* gobj = qobject_cast<GlobalObject*>(pointer);
        if (gobj != nullptr) {
            gobj->deferredInitialization();
        }
    }
    return pointer;
}


GlobalObject::GlobalObject(QObject *parent) : QObject(parent)
{
}


void GlobalObject::clear()
{
    if (g_notificationManager != nullptr)
    {
        g_notificationManager->waitForSpeechEngine();
    }

    isConstructingOrDeconstructing = true;

    delete g_notamProvider;

    delete g_notificationManager;
    delete g_geoMapProvider;
    delete g_flarmnetDB;
    delete g_dataManager;

    delete g_sslErrorHandler;
    delete g_demoRunner;
    delete g_fileExchange;
    delete g_librarian;
    delete g_platformAdaptor;
    delete g_navigator;
    delete g_passwordDB;
    delete g_positionProvider;
    delete g_globalSettings;
    delete g_trafficDataProvider;
    delete g_waypointLibrary;
    delete g_weatherDataProvider;

    delete g_networkAccessManager;

    isConstructingOrDeconstructing = false;
}


auto GlobalObject::canConstruct() -> bool
{
    if (isConstructingOrDeconstructing) {
        return false;
    }
    if (QCoreApplication::instance() == nullptr ) {
        return false;
    }
    return true;
}


auto GlobalObject::fileExchange() -> Platform::FileExchange_Abstract*
{
    return allocateInternal<Platform::FileExchange>(g_fileExchange);
}


auto GlobalObject::flarmnetDB() -> Traffic::FlarmnetDB*
{
    return allocateInternal<Traffic::FlarmnetDB>(g_flarmnetDB);
}

auto GlobalObject::geoMapProvider() -> GeoMaps::GeoMapProvider*
{
    return allocateInternal<GeoMaps::GeoMapProvider>(g_geoMapProvider);
}


auto GlobalObject::clock() -> Navigation::Clock*
{
    return allocateInternal<Navigation::Clock>(g_clock);
}


auto GlobalObject::dataManager() -> DataManagement::DataManager*
{
    return allocateInternal<DataManagement::DataManager>(g_dataManager);
}


auto GlobalObject::demoRunner() -> DemoRunner*
{
    return allocateInternal<DemoRunner>(g_demoRunner);
}


auto GlobalObject::librarian() -> Librarian*
{
    return allocateInternal<Librarian>(g_librarian);
}


auto GlobalObject::platformAdaptor() -> Platform::PlatformAdaptor_Abstract*
{
    return allocateInternal<Platform::PlatformAdaptor>(g_platformAdaptor);
}


auto GlobalObject::navigator() -> Navigation::Navigator*
{
    return allocateInternal<Navigation::Navigator>(g_navigator);
}


auto GlobalObject::networkAccessManager() -> QNetworkAccessManager*
{
    return allocateInternal<QNetworkAccessManager>(g_networkAccessManager);
}


auto GlobalObject::notamProvider() -> NOTAM::NotamProvider*
{
    return allocateInternal<NOTAM::NotamProvider>(g_notamProvider);
}


auto GlobalObject::notificationManager() -> Notifications::NotificationManager*
{
    return allocateInternal<Notifications::NotificationManager>(g_notificationManager);
}


auto GlobalObject::passwordDB() -> Traffic::PasswordDB*
{
    return allocateInternal<Traffic::PasswordDB>(g_passwordDB);
}


auto GlobalObject::positionProvider() -> Positioning::PositionProvider*
{
    return allocateInternal<Positioning::PositionProvider>(g_positionProvider);
}


auto GlobalObject::globalSettings() -> GlobalSettings*
{
    return allocateInternal<GlobalSettings>(g_globalSettings);
}


auto GlobalObject::sslErrorHandler() -> DataManagement::SSLErrorHandler*
{
    return allocateInternal<DataManagement::SSLErrorHandler>(g_sslErrorHandler);
}


auto GlobalObject::trafficDataProvider() -> Traffic::TrafficDataProvider*
{
    return allocateInternal<Traffic::TrafficDataProvider>(g_trafficDataProvider);
}


auto GlobalObject::waypointLibrary() -> GeoMaps::WaypointLibrary*
{
    return allocateInternal<GeoMaps::WaypointLibrary>(g_waypointLibrary);
}


auto GlobalObject::weatherDataProvider() -> Weather::WeatherDataProvider*
{
    return allocateInternal<Weather::WeatherDataProvider>(g_weatherDataProvider);
}

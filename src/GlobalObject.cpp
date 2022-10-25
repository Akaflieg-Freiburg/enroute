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
#include "Librarian.h"
#include "Settings.h"
#include "dataManagement/DataManager.h"
#include "dataManagement/SSLErrorHandler.h"
#include "geomaps/GeoMapProvider.h"
#include "geomaps/WaypointLibrary.h"
#include "navigation/Navigator.h"
#include "platform/PlatformAdaptor.h"
#include "positioning/PositionProvider.h"
#include "traffic/FlarmnetDB.h"
#include "traffic/PasswordDB.h"
#include "traffic/TrafficDataProvider.h"
#include "weather/WeatherDataProvider.h"

#if defined(Q_OS_ANDROID)
#include "platform/Notifier_Android.h"
#endif
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
#include "platform/Notifier_Linux.h"
#endif


bool isConstructing {false};

QPointer<DataManagement::DataManager> g_dataManager {};
QPointer<DataManagement::SSLErrorHandler> g_sslErrorHandler {};
QPointer<DemoRunner> g_demoRunner {};
QPointer<Traffic::FlarmnetDB> g_flarmnetDB {};
QPointer<GeoMaps::GeoMapProvider> g_geoMapProvider {};
QPointer<Librarian> g_librarian {};
QPointer<Platform::PlatformAdaptor> g_platformAdaptor {};
QPointer<Navigation::Navigator> g_navigator {};
QPointer<QNetworkAccessManager> g_networkAccessManager {};
#if defined(Q_OS_ANDROID)
QPointer<Platform::Notifier_Android> g_notifier {};
#endif
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
QPointer<Platform::Notifier_Linux> g_notifier {};
#endif
QPointer<Traffic::PasswordDB> g_passwordDB {};
QPointer<Positioning::PositionProvider> g_positionProvider {};
QPointer<Settings> g_settings {};
QPointer<Traffic::TrafficDataProvider> g_trafficDataProvider {};
QPointer<GeoMaps::WaypointLibrary> g_waypointLibrary {};
QPointer<Weather::WeatherDataProvider> g_weatherDataProvider {};


template<typename T> auto GlobalObject::allocateInternal(QPointer<T>& pointer) -> T*
{
    Q_ASSERT( QCoreApplication::instance() != nullptr );
    Q_ASSERT( !isConstructing );

    if (pointer.isNull()) {
        isConstructing = true;
        pointer = new T( QCoreApplication::instance() );
        isConstructing = false;
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


auto GlobalObject::canConstruct() -> bool
{
    if (isConstructing) {
        return false;
    }
    if (QCoreApplication::instance() == nullptr ) {
        return false;
    }
    return true;
}


auto GlobalObject::flarmnetDB() -> Traffic::FlarmnetDB*
{
    return allocateInternal<Traffic::FlarmnetDB>(g_flarmnetDB);
}


auto GlobalObject::geoMapProvider() -> GeoMaps::GeoMapProvider*
{
    return allocateInternal<GeoMaps::GeoMapProvider>(g_geoMapProvider);
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


auto GlobalObject::platformAdaptor() -> Platform::PlatformAdaptor*
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


auto GlobalObject::notifier() -> Platform::Notifier*
{
#if defined(Q_OS_ANDROID)
    return allocateInternal<Platform::Notifier_Android>(g_notifier);
#endif
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    return allocateInternal<Platform::Notifier_Linux>(g_notifier);
#endif

}


auto GlobalObject::passwordDB() -> Traffic::PasswordDB*
{
    return allocateInternal<Traffic::PasswordDB>(g_passwordDB);
}


auto GlobalObject::positionProvider() -> Positioning::PositionProvider*
{
    return allocateInternal<Positioning::PositionProvider>(g_positionProvider);
}


auto GlobalObject::settings() -> Settings*
{
    return allocateInternal<Settings>(g_settings);
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

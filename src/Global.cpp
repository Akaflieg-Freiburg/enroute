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

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QQmlEngine>

#include "DemoRunner.h"
#include "Global.h"
#include "MobileAdaptor.h"
#include "Settings.h"
#include "dataManagement/DataManager.h"
#include "geomaps/GeoMapProvider.h"
#include "navigation/Navigator.h"
#include "platform/Notifier.h"
#include "positioning/PositionProvider.h"
#include "traffic/FlarmnetDB.h"
#include "traffic/PasswordDB.h"
#include "traffic/TrafficDataProvider.h"

bool isConstructing {false};
bool isDestructing {false};

QPointer<Traffic::FlarmnetDB> g_flarmnetDB {};
QPointer<GeoMaps::GeoMapProvider> g_geoMapProvider {};
QPointer<DataManagement::DataManager> g_mapManager {};
QPointer<MobileAdaptor> g_mobileAdaptor {};
QPointer<Navigation::Navigator> g_navigator {};
QPointer<Platform::Notifier> g_notifier {};
QPointer<Positioning::PositionProvider> g_positionProvider {};
QPointer<QNetworkAccessManager> g_networkAccessManager {};
QPointer<Traffic::PasswordDB> g_passwordDB {};
QPointer<Settings> g_settings {};
QPointer<Traffic::TrafficDataProvider> g_trafficDataProvider {};


template<typename T> auto Global::allocateInternal(QPointer<T>& pointer) -> T*
{
    Q_ASSERT( QCoreApplication::instance() != nullptr );
    Q_ASSERT( !isConstructing );
    Q_ASSERT( !isDestructing );

    if (pointer.isNull()) {
        isConstructing = true;
        pointer = new T( QCoreApplication::instance() );
        isConstructing = false;
        QQmlEngine::setObjectOwnership(pointer, QQmlEngine::CppOwnership);
    }
    return pointer;
}



Global::Global(QObject *parent) : QObject(parent)
{
}


void Global::destruct()
{
    Q_ASSERT( !isConstructing );
    Q_ASSERT( !isDestructing );

    isDestructing = true;

    delete g_flarmnetDB;
    delete g_geoMapProvider;
    delete g_mapManager;
    delete g_mobileAdaptor;
    delete g_navigator;
    delete g_networkAccessManager;
    delete g_passwordDB;
    delete g_positionProvider;
    delete g_settings;
    delete g_trafficDataProvider;

    isDestructing = false;
}


auto Global::flarmnetDB() -> Traffic::FlarmnetDB*
{
    return allocateInternal<Traffic::FlarmnetDB>(g_flarmnetDB);
}


auto Global::geoMapProvider() -> GeoMaps::GeoMapProvider*
{
    return allocateInternal<GeoMaps::GeoMapProvider>(g_geoMapProvider);
}


auto Global::dataManager() -> DataManagement::DataManager*
{
    return allocateInternal<DataManagement::DataManager>(g_mapManager);
}


auto Global::mobileAdaptor() -> MobileAdaptor*
{
    return allocateInternal<MobileAdaptor>(g_mobileAdaptor);
}


auto Global::navigator() -> Navigation::Navigator*
{
    return allocateInternal<Navigation::Navigator>(g_navigator);
}


auto Global::networkAccessManager() -> QNetworkAccessManager*
{
    return allocateInternal<QNetworkAccessManager>(g_networkAccessManager);
}


auto Global::notifier() -> Platform::Notifier*
{
    return allocateInternal<Platform::Notifier>(g_notifier);
}


auto Global::passwordDB() -> Traffic::PasswordDB*
{
    return allocateInternal<Traffic::PasswordDB>(g_passwordDB);
}


auto Global::positionProvider() -> Positioning::PositionProvider*
{
    return allocateInternal<Positioning::PositionProvider>(g_positionProvider);
}


auto Global::settings() -> Settings*
{
    return allocateInternal<Settings>(g_settings);
}

auto Global::trafficDataProvider() -> Traffic::TrafficDataProvider*
{
    return allocateInternal<Traffic::TrafficDataProvider>(g_trafficDataProvider);

}

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

#include "Global.h"
#include "MobileAdaptor.h"
#include "Settings.h"
#include "geomaps/MapManager.h"
#include "navigation/Navigator.h"

bool isConstructing {false};

QPointer<GeoMaps::MapManager> g_mapManager {};
QPointer<MobileAdaptor> g_mobileAdaptor {};
QPointer<Navigation::Navigator> g_navigator {};
QPointer<QNetworkAccessManager> g_networkAccessManager {};
QPointer<Settings> g_settings {};


template<typename T> auto Global::allocateInternal(QPointer<T>& pointer) -> T*
{
    Q_ASSERT( QCoreApplication::instance() != nullptr );
    Q_ASSERT( !isConstructing );

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


auto Global::mapManager() -> GeoMaps::MapManager*
{
    return allocateInternal<GeoMaps::MapManager>(g_mapManager);
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


auto Global::settings() -> Settings*
{
    return allocateInternal<Settings>(g_settings);
}

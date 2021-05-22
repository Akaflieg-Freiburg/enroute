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
#include "geomaps/MapManager.h"
#include "Settings.h"


bool isConstructing {false};

QPointer<GeoMaps::MapManager> g_mapManager {};
QPointer<MobileAdaptor> g_mobileAdaptor {};
QPointer<QNetworkAccessManager> g_networkAccessManager {};
QPointer<Settings> g_settings {};


Global::Global(QObject *parent) : QObject(parent)
{
}


auto Global::mapManager() -> GeoMaps::MapManager*
{
    Q_ASSERT( QCoreApplication::instance() != nullptr );
    Q_ASSERT( !isConstructing );

    if (g_mapManager.isNull()) {
        isConstructing = true;
        g_mapManager = new GeoMaps::MapManager( QCoreApplication::instance() );
        isConstructing = false;
        QQmlEngine::setObjectOwnership(g_mapManager, QQmlEngine::CppOwnership);
    }
    return g_mapManager;
}

auto Global::mobileAdaptor() -> MobileAdaptor*
{
    Q_ASSERT( QCoreApplication::instance() != nullptr );
    Q_ASSERT( !isConstructing );

    if (g_mobileAdaptor.isNull()) {
        isConstructing = true;
        g_mobileAdaptor = new MobileAdaptor( QCoreApplication::instance() );
        isConstructing = false;
        QQmlEngine::setObjectOwnership(g_mobileAdaptor, QQmlEngine::CppOwnership);
    }
    return g_mobileAdaptor;
}


auto Global::networkAccessManager() -> QNetworkAccessManager*
{
    Q_ASSERT( QCoreApplication::instance() != nullptr );
    Q_ASSERT( !isConstructing );

    if (g_networkAccessManager.isNull()) {
        isConstructing = true;
        g_networkAccessManager = new QNetworkAccessManager( QCoreApplication::instance() );
        isConstructing = false;
        QQmlEngine::setObjectOwnership(g_networkAccessManager, QQmlEngine::CppOwnership);
    }
    return g_networkAccessManager;
}


auto Global::settings() -> Settings*
{
    Q_ASSERT( QCoreApplication::instance() != nullptr );
    Q_ASSERT( !isConstructing );

    if (g_settings.isNull()) {
        isConstructing = true;
        g_settings = new Settings( QCoreApplication::instance() );
        isConstructing = false;
        QQmlEngine::setObjectOwnership(g_settings, QQmlEngine::CppOwnership);
    }
    return g_settings;
}

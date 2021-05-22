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

#include <QDebug>
#include <QQmlEngine>

#include "Global.h"
#include "geomaps/MapManager.h"
#include "MobileAdaptor.h"


QPointer<GeoMaps::MapManager> g_mapManager {};
QPointer<MobileAdaptor> g_mobileAdaptor {};


Global::Global(QObject *parent) : QObject(parent)
{

}


Global::~Global()
{
    // Delete global objects
    qWarning() << "Global destructed";
    delete g_mapManager;
    delete g_mobileAdaptor;
}


auto Global::mapManager() -> GeoMaps::MapManager*
{
    if (g_mapManager.isNull()) {
        g_mapManager = new GeoMaps::MapManager();
        QQmlEngine::setObjectOwnership(g_mapManager, QQmlEngine::CppOwnership);
    }
    return g_mapManager;
}


auto Global::mobileAdaptor() -> MobileAdaptor*
{
    if (g_mobileAdaptor.isNull()) {
        g_mobileAdaptor = new MobileAdaptor();
        QQmlEngine::setObjectOwnership(g_mobileAdaptor, QQmlEngine::CppOwnership);
    }
    return g_mobileAdaptor;
}

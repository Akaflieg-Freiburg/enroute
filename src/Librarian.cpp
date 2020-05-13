/***************************************************************************
 *   Copyright (C) 2020 by Stefan Kebekus                                  *
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

#include "Librarian.h"

#include <QStandardPaths>
#include <QtGlobal>

Librarian::Librarian(QObject *parent) : QObject(parent)
{
    auto libraryPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+"/enroute flight navigation/flight routes";
    flightRouteLibraryDir.setPath(libraryPath);
    flightRouteLibraryDir.mkpath(libraryPath);
}


QString Librarian::getStringFromRessource(const QString &name) const
{
    QFile file(name);
    file.open(QIODevice::ReadOnly);
    auto content = file.readAll();
    return QString::fromUtf8(content);
}


uint Librarian::getStringHashFromRessource(const QString &name) const
{
    return qHash(getStringFromRessource(name), 0);
}


bool Librarian::flightRouteExists(const QString &baseName) const
{
  return QFile::exists(flightRouteFullPath(baseName));
}


FlightRoute *Librarian::flightRouteGet(const QString &baseName) const
{
    auto route = new FlightRoute(nullptr, nullptr);
    if (route)
        route->load(flightRouteFullPath(baseName));
    return route;
}


QString Librarian::flightRouteFullPath(const QString &baseName) const
{
    return flightRouteLibraryDir.path()+"/"+baseName+".geojson";
}


void Librarian::flightRouteRemove(const QString &baseName) const
{
    QFile::remove(flightRouteFullPath(baseName));
}


void Librarian::flightRouteRename(const QString &oldName, const QString &newName) const
{
    QFile::rename(flightRouteFullPath(oldName), flightRouteFullPath(newName));
}


QStringList Librarian::flightRoutes(const QString &filter)
{
    QStringList filterList;
    filterList << "*.geojson";

    auto fileNames = flightRouteLibraryDir.entryList(filterList);

    QStringList fileBaseNames;
    foreach(auto fileName, fileNames)
        fileBaseNames << fileName.section('.', 0, 0);

    return permissiveFilter(fileBaseNames, filter);
}


QStringList Librarian::permissiveFilter(const QStringList &inputStrings, const QString &filter)
{
    QString simplifiedFilter = simplifySpecialChars(filter);

    QStringList result;
    foreach(auto inputString, inputStrings)
        if (simplifySpecialChars(inputString).contains(simplifiedFilter, Qt::CaseInsensitive))
            result << inputString;

    return result;
}


QString Librarian::simplifySpecialChars(const QString &string)
{
    QString cacheString = simplifySpecialChars_cache[string];
    if (!cacheString.isEmpty())
        return cacheString;

    QString normalizedString = string.normalized(QString::NormalizationForm_KD);
    return normalizedString.remove(specialChars);
}

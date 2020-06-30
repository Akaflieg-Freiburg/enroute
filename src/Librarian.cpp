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
    if (name == ":text/authors.html")
        return tr(R"html(
    <h3>Authors</h3>

    <br>

    <table>
      <tr>
        <td>
          <p>The app <strong>enroute flight navigation</strong> was written by Stefan Kebekus, flight enthusiast since 1986 and member of the Akaflieg Freiburg flight club. Stefan flies gliders and motor planes.</p>
          <h4>Address</h4>
          Stefan Kebekus<br>
          Wintererstraße 77<br>
          79104 Freiburg<br>
          Germany<br>
          <br>
          <a href='mailto:stefan.kebekus@gmail.com'>e-mail</a>
        </td>
        <td>
          <p align='center'>&nbsp;<img src='/icons/kebekus.jpg' alt='Stefan Kebekus' height='140'><br>Stefan Kebekus<br>Pic: Patrick Seeger</p>
        </td>
      </tr>
      <tr>
        <td>
          <br>
          <h3>Contributing Authors</h3>
          <br>
        </td>
      </tr>
      <tr>
        <td>
          <p>Adrien Crovato is a private pilot and aerospace engineer. He joined the development team in 2020, and contributes to the C++ and QML code base.</p>
          <br>
          <br>
          <a href='mailto:adriencrovato+code@gmail.com'>e-mail</a>
        </td>
        <td>
          <p align='center'>&nbsp;<img src='/icons/crovato.jpg' alt='Adrien Crovato' height='140'><br>Adrien Crovato</p>
        </td>
      </tr>
      <tr>
        <td>
          <p>Heiner Tholen enjoys building things, analog and digital, airborne as well as ground-based. He uses enroute as a pilot of ultralight planes. Heiner joined the enroute team mid 2020 and contributes to the C++/QML codebase.</p>
          <br>
          <br>
          <a href='mailto:ul@heinertholen.com'>e-mail</a>
        </td>
        <td>
          <p align='center'>&nbsp;<img src='/icons/tholen.jpg' alt='Heiner Tholen' height='140'><br>Heiner Tholen</p>
         </td>
      </tr>
      <tr>
        <td>
          <p>Johannes Zellner joined the development in 2020.  He contributes to the C++ and QML code base of the app and helps with bug fixing.</p>
          <br>
          <br>
          <a href='mailto:johannes@zellner.org'>e-mail</a>
        </td>
        <td>
          <p align='center'>&nbsp;<img src='/icons/zellner.jpg' alt='Johannes Zellner' height='140'><br>Johannes Zellner</p>
        </td>
      </tr>
    </table>

    <h3>Translations</h3>

    <p><strong>French:</strong> Adrien Crovato, <a href='mailto:adriencrovato+code@gmail.com'>e-mail</a>.</p>

    <p><strong>German:</strong> Markus Sachs, <a href='mailto:ms@squawk-vfr.de'>e-mail</a>. Markus flies trikes and is an enthusiastic 'Co' on everyting else that flies.</p>
    <p></p>
)html");

    if (name == ":text/whatsnew.html")
        return tr(R"html(
<p>Adrien Crovato and Heiner Tholen have joined the core development team. As a result of their efforts, there is now an option to use <strong>metric units</strong> for horizontal distances and speeds, and the <strong>Nearby</strong> page has been expanded.</p>

<p>Maps has been added for <strong>Argentina</strong> and <strong>Brazil</strong>. As usual, we provide weekly updates.</p>

<p>Markus Sachs has translated the app to <strong>German</strong>.  If you prefer English, go to the 'Settings' page where a language option has been added.</p>

<p>Michael Horbaschk has kindly written a <strong>manual</strong> for the app.</p>
)html");

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


QObject *Librarian::flightRouteGet(const QString &baseName) const
{
    auto route = new FlightRoute(nullptr, nullptr);
    if (!route)
        return nullptr;
    auto error = route->loadFromGeoJson(flightRouteFullPath(baseName));
    if (error.isEmpty())
        return route;
    delete route;
    return nullptr;
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

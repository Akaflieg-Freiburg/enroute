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
    if (name == ":text/info_enroute.html") {
        QString version(PROJECT_VERSION);
        if (!QString(GIT_COMMIT).isEmpty())
            version += QString(" • GIT #")+QString(GIT_COMMIT);
        return tr(R"html(<h3>Enroute Flight Navigation</h3>

<p>Version %1</p>

<p><strong>Enroute Flight Navigation</strong> is a free nagivation app for VFR pilots, developed as a project of Akaflieg Freiburg.</p>

<ul>
  <li>Simple, elegant and functional</li>
  <li>No ads, no commercical "pro" version</li>
  <li>No registration, no membership</li>
  <li>Does not spy on you</li>
  <li>100% Open Source, written without commercial interest</li>
</ul>

<p>Check <a href="https://akaflieg-freiburg.github.io/enroute/">the web site</a> for more information.</p>

<h3>Academic Sponsor</h3>
      
<p>The author gratefully acknowledges support by our academic sponsor, the <a href="https://www.uni-freiburg.de">University of Freiburg</a>.  The university kindly provides the infrastructure used to generate our maps, and the bandwidth required to serve them.</p>

<h3>Acknowledgements</h3>

<p>This program builds on a number of open source libraries, including <a href="https://www.qt.io">Qt</a>, <a href="https://github.com/nitroshare/qhttpengine">QHTTPEngine</a> and <a href="https://www.openssl.org">OpenSSL</a>.</p>

<p>Aeronautical data is kindly provided by the <a href="https://www.openaip.net">openAIP</a> and <a href="https://www.openflightmaps.org">open flightmaps</a> projects. Base maps are kindly provided by <a href="https://openmaptiles.org">OpenMapTiles</a>. Please refer to the documentation for more details.</p>)html").arg(version);
     }

    if (name == ":text/info_license.html") {
      return tr(R"html(<h3>License</h3>

<p>This program is licensed under the <a href="https://www.gnu.org/licenses/gpl-3.0-standalone.html">GNU General Public License V3</a> or, at your choice, any later version of this license.</p>

<h4>Third-Party software and data included in this program</h4>

<p>This program includes several libraries from the <a href="https://qt.io">Qt</a> project, licensed under the <a href="https://www.qt.io/download-open-source">GNU General Public License V3</a>. This program includes the library <a href="https://github.com/nitroshare/qhttpengine">qhttpengine</a>, which is licensed under the <a href="https://github.com/nitroshare/qhttpengine/blob/master/LICENSE.txt">MIT license</a>. This program includes the library <a href="https://openssl.org">OpenSSL</a>, licensed under the <a href="https://www.openssl.org/source/license.html">Apache License 2.0</a>.</p>

<p>This program includes versions of the <a href="https://github.com/google/roboto">Google Roboto Fonts</a>, which are licensed under the <a href="https://github.com/google/roboto/blob/master/LICENSE">Apache License 2.0</a> license. This program includes several <a href="https://github.com/google/material-design-icons">Google Material Design Icons</a>, which are licensed under the <a href="https://github.com/google/material-design-icons/blob/master/LICENSE">Apache License 2.0</a> license.</p>

<p>The style specification of the basemap is a modified version of the <a href="https://github.com/maputnik/osm-liberty">OSM liberty map design</a>, which is in turn originally derived from OSM Bright from Mapbox Open Styles. The code is licenses under the <a href="https://github.com/maputnik/osm-liberty/blob/gh-pages/LICENSE.md">BSD license</a>. The design is derived (although heavily altered) from OSM Bright from Mapbox Open Styles which is licensed under the <a href="https://github.com/maputnik/osm-liberty/blob/gh-pages/LICENSE.md">Creative Commons Attribution 3.0 license</a>. The map is displaying and styling modified data from <a href="https://github.com/openmaptiles/openmaptiles">OpenMapTiles</a> with <a href="https://github.com/openmaptiles/openmaptiles/blob/master/LICENSE.md">CC-BY 4.0 design license</a>.

<p>The map is displaying and styling data from <a href="http://www.openaip.net">openAIP</a>, which is licensed under a <a href="https://creativecommons.org/licenses/by-nc-sa/3.0/">CC BY-NC-SA license</a>. The map is also displaying and styling data from <a href="https://www.openflightmaps.org/">open flightmaps</a>, which is licensed under the <a href="https://www.openflightmaps.org/live/downloads/20150306-LCN.pdf">OFMA General Users´ License</a>.</p>)html");
    }

    if (name == ":text/participate.html") {
      return tr(R"html(<h3>Suggest features and improvements</h3>

<p>We have great plans for <strong>Enroute Flight Navigation</strong>, check our <a href="https://github.com/Akaflieg-Freiburg/enroute/projects">project list at GitHub</a> to see what's coming.  Please make further suggestions through the <a href="https://github.com/Akaflieg-Freiburg/enroute/issues">GitHub issue tracker</a> (this might require opening an account with GitHub).  If you find this difficult, then please <a href="mailto:stefan.kebekus@gmail.com">contact the author</a> directly.  We ask for your understanding that not all your suggestions can be implemented immediately.</p>

<h3>Join the project</h3>

<p>If you would like to <strong>translate the app to your native language</strong>, we would like to <a href="mailto:stefan.kebekus@gmail.com">hear from you</a>. Programming experience is not necessary, but good computer skills are required.</p>

<p>This app should run fine on <strong>Apple devices</strong>. If you would like to port the app to iOS and if you have development experience with C++/Qt programming and with Apple systems, like <a href="mailto:stefan.kebekus@gmail.com">get in touch with us</a> today.</p>

<p>If you are fluent in C++ and if you would like to <strong>help with the programming</strong>, <a href="mailto:stefan.kebekus@gmail.com">let us know</a>.</p>)html");
    }

    if (name == ":text/tooManyDownloads.html") {
      return tr(R"html(<h3>Too many maps</h3>

<p>Thank you for using Enroute Flight Navigation, we appreciate your engagement very much.</p>

<p>However, we'd like to ask you <strong>to limit yourself to 8 %1.</strong>

<p>On the one hand, the bandwidth for map downloads is kindly sponsored by the University of Freiburg, under the assumption that <strong>the costs stays within reasonable limits.</strong></p>

<p>On the other, the app will perform much better if it doesn't have to process many megabytes of map data.</p>

<p>We are trying our best to avoid a hard limit on the number of maps in the future. Please help us with that. You already have <strong>%2 maps</strong>.</p>)html");
    }

    if (name == ":text/authors.html") {
        return tr(R"html(<h3>Authors</h3>

<br>

<table>
  <tr>
    <td>
      <p>The app <strong>Enroute Flight Navigation</strong> was written by Stefan Kebekus, flight enthusiast since 1986 and member of the Akaflieg Freiburg flight club. Stefan flies gliders and motor planes.</p>
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
      <p>Heiner Tholen enjoys building things, analog and digital, airborne as well as ground-based. He uses Enroute as a pilot of ultralight planes. Heiner joined the Enroute team mid 2020 and contributes to the C++/QML codebase.</p>
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

<h3>Manual</h3>

<p>The manual has kindly been provided by Michael Horbaschk.</p>

<h3>Translations</h3>

<p><strong>French:</strong> Adrien Crovato, <a href='mailto:adriencrovato+code@gmail.com'>e-mail</a>.</p>

<p><strong>German:</strong> Markus Sachs, <a href='mailto:ms@squawk-vfr.de'>e-mail</a>. Markus flies trikes and is an enthusiastic 'Co' on everyting else that flies.</p>

<p><strong>Polish:</strong> Szymon Kocur.</p>

<p></p>)html");
     }
    
    if (name == ":text/bugReport.html") {
      return tr(R"html(<h3>Bug Report</h3>

<p>We aim to provide very high-quality software. Fixing errors is therefore always our first priority. We are grateful for every report that we get.</p>

<p>Please report errors through the <a href="https://github.com/Akaflieg-Freiburg/enroute/issues">GitHub issue tracker</a> (this might require opening an account with GitHub).  If you find this difficult, then please <a href="mailto:stefan.kebekus@gmail.com">contact the author</a> directly.</p>)html");
    }

    if (name == ":text/firstStart.html") {
        return tr(R"html(<h3>Welcome to Enroute Flight Navigation - A project of Akaflieg Freiburg</h3>

<p>Thank you for using this flight navigation app!  Before we get started, we need to point out that <strong>this app and the aviation data come with no guarantees</strong>.</p>

<p>The app is not certified to satisfy aviation standards. It may contain errors and may not work as expected.</p>

<p>The aviation data does not come from official sources. It might be incomplete, outdated or otherwise incorrect.</p>

<p><strong>This app is no substitute for proper flight preparation or good pilotage.</strong> We hope you enjoy the app and that you do find it useful.</p>

<p>Fly safely and enjoy many happy landings!</p>

<p>&#8212; Stefan Kebekus.</p>)html");
    }

    if (name == ":text/flightRouteLibraryInfo.html") {
        return tr(R"html(<p>The flight routes are stored in standard <a href="https://geojson.org">GeoJSON format</a> in the following directory.<p>

<p style="text-align:left;"><strong><a href="file:%1">%1</a></strong></p>

<p>The flight routes can be accessed by other programs, such as backup software or file synchronization apps.  This can be useful to share a flight route library with other devices.</p>)html");
    }

    if (name == ":text/missingPermissions.html") {
        return tr(R"html(<h3>Missing Permissions</h3>

<p>The app <strong>Enroute Flight Navigation</strong> will not start because some essential permissions have not been granted.  Please re-start the app and grant the required permissions.  If you have chosen to deny some permissions permanently, you may need to go to the Android Settings app to grant the permissions there.</p>

<p>Our <a href="https://akaflieg-freiburg.github.io/enroute/privacy#privileges-of-the-android-app" title="privacy policies">privacy policies</a> explain why the permissions are needed and what they are used for.</p>

<p>Fly safely and enjoy many happy landings!</p>

<p>&#8212; Stefan Kebekus.</p>)html");
    }
    
    if (name == ":text/whatsnew.html") {
        return tr(R"html(<p>Adrien Crovato has implemented METRA/TAF access. Heiner Tholen has improved the map display.</p>)html");
    }

    if (name == ":text/weatherPermissions.html") {
      return tr(R"html(<h3>Weather Permissions</h3>
      
<p>The weather feature implemented in <strong>Enroute Flight Navigation</strong> relies on data provided by the <a href https://www.aviationweather.gov/>Aviation Weather Center</a> U.S. service.</p>
      
<p>Using this feature will send your last-known location and your current route to a U.S. government website.</p>

<p>Note that <strong>Enroute Flight Navigation</strong> is not responsible for any erroneous data that could be emitted by the Aviation Weather Center.</p>)html");
    }

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
    auto error = route->loadFromGeoJSON(flightRouteFullPath(baseName));
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

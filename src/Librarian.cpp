/***************************************************************************
 *   Copyright (C) 2020-2021 by Stefan Kebekus                             *
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


auto Librarian::getStringFromRessource(const QString &name) -> QString
{
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

<p><strong>French:</strong> Adrien Crovato. Adrien has been described above.</p>

<p><strong>German:</strong> Markus Sachs. Markus flies trikes and is an enthusiastic 'Co' on everyting else that flies.</p>

<p><strong>Italian:</strong> Ivan Battistella and Luca Bertoncello. Luca flies ultralight planes since 2014 and has become a flight instructor in 2019. He programs since 1988 and works as a systems administrator in Dresden, Germany.  Ivan Battistella has recently started to fly ultralight aircraft. He works freelance as a software consultant.</p>

<p><strong>Polish:</strong> Sławek Mikuła.</p>

<h3>Alumni</h3>

<p>Szymon Kocur (Polish translation)</p>

<p></p>)html");
    }

    if (name == ":text/aviationMapMissing.html") {
        return tr(R"html(
<p>We receive our aviation data from <a href="http://openaip.net">openAIP</a>. This is a not-for-profit organisation where volunteers compile aviation data for many countries. If openAIP covers your country, we might be able to generate maps for it. First, however, we need to be reasonably sure that the openAIP data is accurate and complete. Please have a look at the <a href="http://maps.openaip.net">openAIP maps</a> and compare the display with an official aviation map of your country. Are the airfields there? Are runway lengths/orientations and frequencies correct? Are NavAids correctly displayed, with correct codes and frequencies? And what about airspaces?</p>

<p>If you are convinced that the data is good enough to be added, you can request to add the country. Please go to <a href="https://github.com/Akaflieg-Freiburg/enrouteServer/issues">this web site</a> and open an 'issue' with your request. Please tell us who you are, where you fly and how you convinced yourself that the data is good. If you find that the data is not good enough, you are welcome to join the openAIP project and help to improve the data.</p>

<p>Please understand that we program this free app in our spare time, as a service to the community. Sadly, I should point out that sending us impolite demands is not likely to give the result that you desire.</p>)html");
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

    if (name == ":text/info_enroute.html") {
        QString version(PROJECT_VERSION);
        if (!QString(GIT_COMMIT).isEmpty()) {
            version += QString(" • GIT #")+QString(GIT_COMMIT);
        }
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

<p>This program builds on a number of open source libraries, including <a href="https://https://github.com/nnaumenko/metaf">Metaf</a>, <a href="https://www.openssl.org">OpenSSL</a>, <a href="https://www.qt.io">Qt</a>, <a href="https://github.com/nitroshare/qhttpengine">QHTTPEngine</a> and <a href="https://github.com/buelowp/sunset">sunset</a>.</p>

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

    if (name == ":text/missingPermissions.html") {
        return tr(R"html(<h3>Missing Permissions</h3>

<p>The app <strong>Enroute Flight Navigation</strong> will not start because some essential permissions have not been granted.  Please re-start the app and grant the required permissions.  If you have chosen to deny some permissions permanently, you may need to go to the Android Settings app to grant the permissions there.</p>

<p>Our <a href="https://akaflieg-freiburg.github.io/enroute/privacy#privileges-of-the-android-app" title="privacy policies">privacy policies</a> explain why the permissions are needed and what they are used for.</p>

<p>Fly safely and enjoy many happy landings!</p>

<p>&#8212; Stefan Kebekus.</p>)html");
    }

    if (name == ":text/flarmSetup.md") {
        return tr(R"html(
<p>In order to display nearby traffic on the moving map, <strong>Enroute Flight
Navigation</strong> can connect to your aircraft's traffic receiver (typically a FLARM
device).</p>

<p>The author has tested the <strong>Enroute Flight Navigation</strong> with the
following traffic receivers.</p>

<ul style="margin-left:-25px;">
  <li>Air Avionics AT-1 ‘AIR Traffic’ with software version 5.</li>
</ul>

<p>Users reported success with the following traffic receivers.</p>

<ul style="margin-left:-25px;">
  <li>Stratux</li>
  <li>T-Beam</li>
</ul>
)html") + tr(R"html(
<h3>Technical Note</h3>

<p><strong>Enroute Flight Navigation</strong> expects that the traffic receiver deploys a WLAN
network via Wi-Fi and publishes a stream of NMEA sentences at the IP address
192.168.1.1, port 2000 of that network. The NMEA sentences must conform to the
specification outlined in in the document FTD-012 'Data Port Interface Control
Document (ICD)', Version 7.13, as published by FLARM Technology Ltd.</p>

)html") + tr(R"html(
<h2>Before you connect</h2>

<p>Before you try to connect this app to your traffic receiver, make sure
that the following conditions are met.</p>

<ul style="margin-left:-25px;">
  <li>Your traffic receiver has an integrated Wi-Fi interface that acts as a
  wireless access point. Bluetooth devices are currently not supported.</li>

  <li>You know the network name (=SSID) of the WLAN network deployed by your traffic
  receiver. If the network is encrypted, you also need to know the WLAN
  password.</li>

  <li>Some devices require an additional password in order to access traffic
  data. This is currently not supported. Set up your device so that no
  additional password is required.</li>
</ul>
)html") + tr(R"html(
<h2>Connecting to the traffic receiver</h2>

<p>It takes a two steps to connect <strong>Enroute Flight
Navigation</strong> to the traffic receiver for the first time. Once things
are set up properly, your device should automatically detect the traffic
receiver's WLAN network, enter the network and connect to the traffic data
stream whenever you go flying.</p>

)html") + tr(R"html(
<h3>Step 1: Enter the traffic receiver's WLAN network</h3>

<ul style="margin-left:-25px;">
  <li>Make sure that the traffic receiver has power and is switched on. In a typical
  aircraft installation, the traffic receiver is connected to the 'Avionics'
  switch and will automatically switch on. You may need to wait a minute before
  the WLAN comes online and is visible to your device.</li>

  <li>Enter the WLAN network deployed by your traffic receiver. This is usually done
  in the "WLAN Settings" of your device. Enter the WLAN password if
  required. Some devices will issue a warning that the WLAN is not connected to
  the internet. In this case, you might need to confirm that you wish to enter
  the WLAN network.</li>
</ul>

<p>Most operating systems will offer to remember the connection, so that your
device will automatically connect to this WLAN in the future. We recommend to
use this option.</p>

)html") + tr(R"html(
<h3>Step 2: Connect to the traffic data stream</h3>

<p>Open the main menu and navigate to the "Information" menu.</p>

<ul style="margin-left:-25px;">
  <li>If the entry "Traffic Receiver" is highlighted in green, then <strong>Enroute
  Flight Navigation</strong> has already found the traffic receiver in the network
  and has connected to it. Congratulations, you are done!</li>

  <li>If the entry "Traffic Receiver" is not highlighted in green, then select the
  entry. The "Traffic Receiver Status" page will open. The page explains the
  connection status in detail, and explains how to establish a connection
  manually.</li>
</ul>
)html") + tr(R"html(
<h2>Troubleshooting</h2>

<h4>The app cannot connect to the traffic data stream</h4>

<ul style="margin-left:-25px;">
  <li>If check that your device is connected to the WLAN network deployed by your
  traffic receiver.</li>
</ul>

<h4>The connection breaks down after a few seconds</h4>

<p>Most traffic receivers cannot serve more than one client and abort connections
at random if more than one device tries to access.</p>

<ul style="margin-left:-25px;">
  <li>Make sure that there no second device connected to the traffic receiver's WLAN
  network. The other device might well be in your friend's pocket!</li>

  <li>Make sure that there is no other app trying to connected to the traffic
  receiver's data stream.</li>

  <li>Many traffic receivers offer "configuration panels" that can be accessed via a
  web browser. Close all web browsers.</li>
</ul>
)html");
    }

    if (name == ":text/tooManyDownloads.html") {
        return tr(R"html(<h3>Too many maps</h3>

<p>Thank you for using Enroute Flight Navigation, we appreciate your engagement very much.</p>

<p>However, we'd like to ask you <strong>to limit yourself to 8 %1.</strong>

<p>On the one hand, the bandwidth for map downloads is kindly sponsored by the University of Freiburg, under the assumption that <strong>the costs stays within reasonable limits.</strong></p>

<p>On the other, the app will perform much better if it doesn't have to process many megabytes of map data.</p>

<p>We are trying our best to avoid a hard limit on the number of maps in the future. Please help us with that. You already have <strong>%2 maps</strong>.</p>)html");
    }

    if (name == ":text/weatherPermissions.html") {
        return tr(R"html(<h3>Privacy Warning</h3>
      
<p>Like most other programs, this app uses weather data provided by the <a href='https://www.aviationweather.gov'/>Aviation Weather Center</a>, a website of the United States government.</p>

<p>In order to request up-to-date weather information, the app needs to <strong>send your location and your current route to the Aviation Weather Center</strong> at regular intervals. If you agree to this, you can enable the weather feature by clicking on the button below. You can disable the feature at any time using the three-dot menu at the top of this screen.</p>

<p><strong>We have no control over data collected by the Aviation Weather Center. We do not guarantee correctness of the weather information in any way!</strong></p>)html");
    }

    if (name == ":text/whatsnew.html") {
        return (R"MD(
The app can now connect to your aircraft's traffic receiver
(typically a FLARM device) and show nearby traffic on the
moving map. Open the menu and go to **Information/Traffic
Reveicer** to try it out.

This feature is new. Your feedback is very welcome!
)MD");
    }

    QFile file(name);
    file.open(QIODevice::ReadOnly);
    auto content = file.readAll();
    return QString::fromUtf8(content);
}


auto Librarian::getStringHashFromRessource(const QString &name) const -> uint
{
    return qHash(getStringFromRessource(name), 0);
}


auto Librarian::flightRouteExists(const QString &baseName) const -> bool
{
    return QFile::exists(flightRouteFullPath(baseName));
}


auto Librarian::flightRouteGet(const QString &baseName) const -> QObject *
{
    auto *route = new FlightRoute(nullptr, nullptr);
    if (route == nullptr) {
        return nullptr;
    }
    auto error = route->loadFromGeoJSON(flightRouteFullPath(baseName));
    if (error.isEmpty()) {
        return route;
    }
    delete route;
    return nullptr;
}


auto Librarian::flightRouteFullPath(const QString &baseName) const -> QString
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


auto Librarian::flightRoutes(const QString &filter) -> QStringList
{
    QStringList filterList;
    filterList << "*.geojson";

    auto fileNames = flightRouteLibraryDir.entryList(filterList);

    QStringList fileBaseNames;
    foreach(auto fileName, fileNames)
        fileBaseNames << fileName.section('.', 0, -2);

    return permissiveFilter(fileBaseNames, filter);
}


auto Librarian::permissiveFilter(const QStringList &inputStrings, const QString &filter) -> QStringList
{
    QString simplifiedFilter = simplifySpecialChars(filter);

    QStringList result;
    foreach(auto inputString, inputStrings)
        if (simplifySpecialChars(inputString).contains(simplifiedFilter, Qt::CaseInsensitive)) {
            result << inputString;
        }

    return result;
}


auto Librarian::simplifySpecialChars(const QString &string) -> QString
{
    QString cacheString = simplifySpecialChars_cache[string];
    if (!cacheString.isEmpty()) {
        return cacheString;
    }

    QString normalizedString = string.normalized(QString::NormalizationForm_KD);
    return normalizedString.remove(specialChars);
}

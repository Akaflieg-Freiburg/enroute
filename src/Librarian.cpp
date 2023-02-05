/***************************************************************************
 *   Copyright (C) 2020-2023 by Stefan Kebekus                             *
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
#include "navigation/FlightRoute.h"

#include <QNetworkAccessManager>
#include <QStandardPaths>
#include <QSysInfo>
#include <QtGlobal>


Librarian::Librarian(QObject *parent) : QObject(parent)
{

    // This app used to store flight routes in QStandardPaths::GenericDataLocation. However, Android 11
    // no longer allows this "Scoped Storage". We will therefore move our files from
    // QStandardPaths::GenericDataLocation to QStandardPaths::AppDataLocation, which is still writable
    // since we set "requestlegacystorage" in the manifest file and target Android 10. See
    // https://developer.android.com/training/data-storage/use-cases#opt-out-in-production-app
    auto oldlibraryPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+"/enroute flight navigation/flight routes";
    auto libraryPath = directory(Routes);
    QDir d(oldlibraryPath);
    foreach(auto elt, d.entryList( QStringList(), QDir::Files)) {
        if (QFile::copy(oldlibraryPath+"/"+elt, libraryPath+"/"+elt)) {
            QFile::remove(oldlibraryPath+"/"+elt);
        }
    }
    d.rmdir(oldlibraryPath);
}


auto Librarian::getStringFromRessource(const QString &name) -> QString
{
    if (name == u"appUpdateRequired"_qs)
    {
        return tr("<p>This version of <strong>Enroute Flight Navigation</strong> is outdated and will no longer receive map updates. "
                  "<strong>Please update this app at your earliest convencience.</strong></p>");
    }

    if (name == u":text/authors.html"_qs)
    {
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
</table>
)html") + tr(R"html(
<h3>Manual</h3>

<p>The manual has kindly been provided by Michael Horbaschk.</p>
)html") + tr(R"html(
<h3>Translations</h3>
)html") + (R"html(
<ul style="margin-left:-25px;">
)html") + tr(R"html(
<li><strong>French:</strong> Adrien Crovato and Luca Riva. Both are private pilots and aerospace engineers. Luca is also doing aerobatics.</li>
)html") + tr(R"html(
<li><strong>German:</strong> Markus Sachs. Markus flies trikes and is an enthusiastic 'Co' on everyting else that flies.</li>
)html") + tr(R"html(
<li><strong>Italian:</strong> Ivan Battistella and Antonio Fardella.  Antonio is a naval aviator with a passion for everything challenging.</li>
)html") + tr(R"html(
<li><strong>Polish:</strong> Sławek Mikuła.</li>
)html") + (R"html(
</ul>
)html") + tr(R"html(
<h3>Alumni</h3>
)html") + (R"html(
<ul style="margin-left:-25px;">
)html") + tr(R"html(
<li>Luca Bertoncello (Italian translation)</li>
)html") + tr(R"html(
<li>Adrien Crovato (Integration of weather information)</li>
)html") + tr(R"html(
<li>Szymon Kocur (Polish translation)</li>
)html") + tr(R"html(
<li>Heiner Tholen (User interface)</li>
)html") + tr(R"html(
<li>Johannes Zellner (Geoid correction for altitude)</li>
)html") + (R"html(
</ul>
<p></p>)html");
    }

    if (name == u":text/aviationMapMissing.html"_qs)
    {
        return tr(R"html(
<p>We receive our aviation data from <a href="http://openaip.net">openAIP</a>. This is a not-for-profit organisation where volunteers compile aviation data for many countries. If openAIP covers your country, we might be able to generate maps for it. First, however, we need to be reasonably sure that the openAIP data is accurate and complete. Please have a look at the <a href="http://maps.openaip.net">openAIP maps</a> and compare the display with an official aviation map of your country. Are the airfields there? Are runway lengths/orientations and frequencies correct? Are NavAids correctly displayed, with correct codes and frequencies? And what about airspaces?</p>

<p>If you are convinced that the data is good enough to be added, you can request to add the country. Please go to <a href="https://github.com/Akaflieg-Freiburg/enrouteServer/issues">this web site</a> and open an 'issue' with your request. Please tell us who you are, where you fly and how you convinced yourself that the data is good. If you find that the data is not good enough, you are welcome to join the openAIP project and help to improve the data.</p>

<p>Please understand that we program this free app in our spare time, as a service to the community. Sadly, I should point out that sending us impolite demands is not likely to give the result that you desire.</p>)html");
    }

    if (name == u":text/firstStart.html"_qs)
    {
        return tr(R"html(<h3>Welcome to Enroute Flight Navigation - A project of Akaflieg Freiburg</h3>

<p>Thank you for using this flight navigation app!  Before we get started, we need to point out that <strong>this app and the aviation data come with no guarantees</strong>.</p>

<p>The app is not certified to satisfy aviation standards. It may contain errors and may not work as expected.</p>

<p>The aviation data does not come from official sources. It might be incomplete, outdated or otherwise incorrect.</p>

<p><strong>This app is no substitute for proper flight preparation or good pilotage.</strong> We hope you enjoy the app and that you do find it useful.</p>

<p>Fly safely and enjoy many happy landings!</p>

<p>&#8212; Stefan Kebekus.</p>)html");
    }

    if (name == u":text/flightRouteLibraryInfo.html"_qs)
    {
        return tr(R"html(<p>The flight routes are stored in standard <a href="https://geojson.org">GeoJSON format</a> in the following directory.<p>

<p style="text-align:left;"><strong><a href="file:%1">%1</a></strong></p>

<p>The flight routes can be accessed by other programs, such as backup software or file synchronization apps.  This can be useful to share a flight route library with other devices.</p>)html");
    }

    if (name == u":text/info_enroute.html"_qs)
    {
        QString version(QStringLiteral(PROJECT_VERSION));
        if (!QStringLiteral(GIT_COMMIT).isEmpty())
        {
            version += QStringLiteral(" • GIT #")+QStringLiteral(GIT_COMMIT);
        }
        return tr(R"html(<h3>Enroute Flight Navigation</h3>

<p>Version %1</p>

<p><strong>Enroute Flight Navigation</strong> is a free nagivation app for VFR pilots, developed as a project of Akaflieg Freiburg.</p>

<ul style="margin-left:-25px;">
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

    if (name == u":text/info_license.html"_qs)
    {
        QFile file(QStringLiteral(":license_overview.html"));
        file.open(QIODevice::ReadOnly);
        auto content = QString::fromUtf8(file.readAll());

        return tr(R"html(
<h3>License</h3>

<p>
  The program <strong>Enroute Flight Navigation</strong>
  is licensed under the <a
  href="https://www.gnu.org/licenses/gpl-3.0-standalone.html">GNU
  General Public License V3</a> or, at your choice, any later
  version of this license.
</p>
)html")
                + tr(R"html(
<h4>Software and data included in the program</h4>

<p>
  Depending on platform and configuration, the following
  components might be included in the installation of
  <strong>Enroute Flight Navigation</strong>.
</p>

<ul style="margin-left:-25px;">
%1
</ul>
)html").arg(content);
    }

    if (name == u":text/missingPermissions.html"_qs)
    {
        return tr(R"html(<h3>Missing Permissions</h3>

<p>The app <strong>Enroute Flight Navigation</strong> will not start because some essential permissions have not been granted.  Please re-start the app and grant the required permissions.  If you have chosen to deny some permissions permanently, you may need to go to the Android Settings app to grant the permissions there.</p>

<p>Our <a href="https://akaflieg-freiburg.github.io/enroute/privacy#privileges-of-the-android-app" title="privacy policies">privacy policies</a> explain why the permissions are needed and what they are used for.</p>

<p>Fly safely and enjoy many happy landings!</p>

<p>&#8212; Stefan Kebekus.</p>)html");
    }

    if (name == u":text/privacy.html"_qs)
    {
        return "<h3>" + tr("Privacy Policies") + "</h3>"
                + "<p>" + tr("This app does not send any data to us or other parties. We do "
                             "not collect data and we do not track our users. However, this "
                             "app accesses internet sites that may not be under our control "
                             "and may keep access logs.") + "</p>"
                + "<ul style='margin-left:-25px;'>"
                + "<li>" + tr("The app connects to servers at the University of Freiburg to "
                              "check for updates and to download maps and data.") + "</li>"
                + "<li>" + tr("The app downloads METAR and TAF data for your locaction and "
                              "your intended route from the Aviation Weather Center, a website "
                              "of the United States government.") + "</li>"
                + "</ul>"
                + "<h3>" + tr("Privacy-relevant permissions requested by this app") + "</h3>"
                + "<p>" + tr("Enroute Flight Navigation needs to access your precise location. "
                             "The app uses this data to show your position on the moving "
                             "map and to provide relevant aeronautical information for your location.") + "</p>"
                + "<p>" + tr("Please grant permission to access the precise location when prompted.") + "</p>"
                + "<h3>" + tr("Responsible") + "</h3>"
                + "Stefan Kebekus<br>Wintererstraße 77<br>79104 Freiburg im Breisgau<br>Germany<br>";
    }

    if (name == u":text/tooManyDownloads.html"_qs)
    {
        return tr(R"html(<h3>Too many maps</h3>

<p>Thank you for using Enroute Flight Navigation, we appreciate your engagement very much.</p>

<p>However, we'd like to ask you <strong>to download only the maps that you really need.</strong>

<p>On the one hand, the bandwidth for map downloads is kindly sponsored by the University of Freiburg, under the assumption that <strong>the costs stays within reasonable limits.</strong></p>

<p>On the other, the app will perform much better if it doesn't have to process many megabytes of map data.</p>

<p>We are trying our best to avoid a hard limit on the number of maps in the future. Please help us with that.</p>)html");
    }

    if (name == u":text/whatsnew.html"_qs)
    {
        return tr("<p>We have updated the technology base underlying this app. Please report any issues!</p>")
                + "<p>" + tr("Our aviation maps will now receive daily updates, provided that new data is available.") + "</p>";
        // + tr("<p>We could need help with French translations. If you would like to help, if speak French and know how to use the GIT version control system, please let us know.</p>");
    }

    QFile file(name);
    file.open(QIODevice::ReadOnly);
    auto content = file.readAll();
    return QString::fromUtf8(content);

}


auto Librarian::getStringHashFromRessource(const QString &name) -> Units::ByteSize
{
    return qHash(getStringFromRessource(name), (size_t)0);
}


auto Librarian::exists(Librarian::Library library, const QString &baseName) -> bool
{
    return QFile::exists(fullPath(library, baseName));
}


auto Librarian::get(Librarian::Library library, const QString &baseName) -> QObject *
{
    if (library == Routes)
    {
        auto *route = new Navigation::FlightRoute();
        if (route == nullptr)
        {
            return nullptr;
        }
        auto error = route->load(fullPath(Routes, baseName));
        if (error.isEmpty())
        {
            return route;
        }
        delete route;
        return nullptr;
    }

    return nullptr;
}


auto Librarian::fullPath(Librarian::Library library, const QString &baseName) -> QString
{
    switch (library)
    {
    case Aircraft:
        return directory(library)+"/"+baseName+".json";
    case Routes:
        return directory(library)+"/"+baseName+".geojson";
    }
    return {};
}


void Librarian::remove(Librarian::Library library, const QString& baseName) 
{
    QFile::remove(fullPath(library, baseName));
}


void Librarian::rename(Librarian::Library library, const QString &oldName, const QString &newName) 
{
    QFile::rename(fullPath(library, oldName), fullPath(library, newName));
}


auto Librarian::directory(Library library) -> QString
{
    QString path;
    switch (library)
    {
    case Aircraft:
        path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/aircraft";
        break;
    case Routes:
        path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/flight routes";
        break;
    }
    QDir().mkpath(path);
    return path;
}


auto Librarian::entries(Library library, const QString &filter) -> QStringList
{
    QStringList filterList;
    filterList << QStringLiteral("*");

    QDir dir(directory(library));
    auto fileNames = dir.entryList(filterList, QDir::Files);

    QStringList fileBaseNames;
    foreach(auto fileName, fileNames)
    {
        fileBaseNames << fileName.section('.', 0, -2);
    }

    return permissiveFilter(fileBaseNames, filter);
}


auto Librarian::permissiveFilter(const QStringList &inputStrings, const QString &filter) -> QStringList
{
    QString simplifiedFilter = simplifySpecialChars(filter);

    QStringList result;
    foreach(auto inputString, inputStrings)
    {
        if (simplifySpecialChars(inputString).contains(simplifiedFilter, Qt::CaseInsensitive))
        {
            result << inputString;
        }
    }

    return result;
}


auto Librarian::simplifySpecialChars(const QString &string) -> QString
{
    QString cacheString = simplifySpecialChars_cache[string];
    if (!cacheString.isEmpty())
    {
        return cacheString;
    }

    QString normalizedString = string.normalized(QString::NormalizationForm_KD);
    return normalizedString.remove(specialChars);
}

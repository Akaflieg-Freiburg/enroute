/***************************************************************************
 *   Copyright (C) 2020-2024 by Stefan Kebekus                             *
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
    QDir const dir(oldlibraryPath);
    foreach(auto elt, dir.entryList( QStringList(), QDir::Files)) {
        if (QFile::copy(oldlibraryPath+"/"+elt, libraryPath+"/"+elt)) {
            QFile::remove(oldlibraryPath+"/"+elt);
        }
    }
    dir.rmdir(oldlibraryPath);
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
        return "<p>"+tr("The app <strong>Enroute Flight Navigation</strong> was written by Stefan Kebekus, flight enthusiast since 1986 and member of the Akaflieg Freiburg flight club. Stefan flies gliders and motor planes.")+"</p>"
               + "<p><strong>"+tr("Address")+"</strong>: Stefan Kebekus, Wintererstraße 77, 79104 Freiburg, Germany · <a href='mailto:stefan.kebekus@gmail.com'>stefan.kebekus@gmail.com</a></p>"
               + "<h3>" + tr("Contributions") + "</h3>"
               + "<p><strong>" + tr("iOS Version") + "</strong> " + tr("The app has been ported to iOS by Simon Schneider. Simon is currently student pilot (PPL) and member of the Akaflieg Freiburg flight club, just like Stefan.") + "</p>"
               + "<p><strong>" + tr("Programming") + "</strong> " + tr("Heinz Blöchinger has helped us with OpenAir support. After 15 years of alpine gliding, Heinz has fulfilled a big dream and now flies helicopters.") + "</p>"
               + tr(R"html(
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
<li><strong>Spanish:</strong> Luca Riva.</li>
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
)html")
               + "<li>Michael Horbaschk (" + tr("Manual") + ")</li>"
               + tr(R"html(
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

    if (name == u":text/privacy.html"_qs)
    {
        return "<p>" +
               tr("We do not collect our users' personal data.") + " " +
               tr("We do not store any personally identifiable information.") + " " +
               tr("However, to ensure functionality, the app must transmit the following data items to servers on the internet.") +
               "</p>" +

               "<h3>" +
               tr("1. Map and Data Download Server") +
               "</h3>" +

               "<p>" +
               tr("The app regularly checks for updates and allows downloading maps and data from a <a href='https://cplx.vm.uni-freiburg.de/storage'>server at the University of Freiburg</a> to your device.") + " " +
               tr("Your device's IP address must be transmitted to the server to provide this functionality.") + " " +
               tr("Knowledge of the IP address is necessary for the server to respond.") + " " +
               tr("However, the server does not store any personal data about you in its log files.") + " " +
               tr("In particular, it does not store the IP address of your device in its log files.") + " " +
               tr("We can assure this because the server is under our control.") + "</p>" +

               "<h3>" +
               tr("2. METAR and TAF Server") +
               "</h3>" +

               "<p>" +
               tr("The app shows METARs and TAFs for airfields near your current location and your currently planned route. ") + " " +
               tr("It also shows METARs and TAFs for all waypoints you open in the app.") + " " +
               tr("To provide this functionality, a request containing the following data is transmitted to a <a href='https://cplx.vm.uni-freiburg.de/storage'>server at the University of Freiburg</a>:") +
               "</p>" +

               "<ul style='margin-left:-25px;'>" +
               "<li>" + tr("Your current location") + "</li>" +
               "<li>" + tr("The currently planned route") + "</li>" +
               "<li>" + tr("Waypoint coordinates") + "</li>" +
               "<li>" + tr("Your device's IP address") + "</li>" +
               "</ul>" +

               "<p>" +
               tr("Knowledge of the IP address is necessary for the server to respond.") + " " +
               tr("However, the server does not store any personal data about you in its log files.") + " " +
               tr("In particular, it does not store the IP address of your device in its log files.") + " " +
               tr("We can assure this because the server is under our control.") + " " +
               tr("The server cannot read any other data from your device in the process.") +
               "</p>" +

               "<p>" +
               tr("The server forwards the request to a web service at the %1 but hides your IP address, so the web service will never see it.").arg(u"<a href='https://www.aviationweather.gov'>Aviation Weather Center</a>"_qs) + " " +
               tr("The web services at the Aviation Weather Center are operated by the US government and are beyond our control.") + " " +
               tr("Detailed information can be found at %1.").arg(u"<a href='https://www.aviationweather.gov/dataserver'>www.aviationweather.gov/dataserver</a>"_qs) +
               "</p>" +

               "<h3>" +
               tr("3. NOTAM Server") +
               "</h3>" +

               "<p>" +
               tr("The app shows NOTAMs for places near your current location and your currently planned route.") + " " +
               tr("It also shows NOTAMs for all waypoints you open in the app.") + " " +
               tr("To provide this functionality, a request containing the following data is transmitted to a <a href='https://cplx.vm.uni-freiburg.de/storage'>server at the University of Freiburg</a>:") + " " +
               "</p>" +

               "<ul style='margin-left:-25px;'>" +
               "<li>" + tr("Your current location") + "</li>" +
               "<li>" + tr("The currently planned route") + "</li>" +
               "<li>" + tr("Waypoint coordinates") + "</li>" +
               "<li>" + tr("Your device's IP address") + "</li>" +
               "</ul>" +

               + "<p>" +
               tr("Knowledge of the IP address is necessary for the server to respond.") + " " +
               tr("However, the server does not store any personal data about you in its log files.") + " " +
               tr("In particular, it does not store the IP address of your device in its log files.") + " " +
               tr("We can assure this because the server is under our control.") + " " +
               tr("The server cannot read any other data from your device in the process.") +
               "</p>" +

               + "<p>" +
               tr("The server forwards the request to a web service at the %1 but hides your IP address, so the web service will never see it.").arg(u"<a href='https://api.faa.gov'>Federal Aviation Administration</a>"_qs) + " " +
               tr("The web services at the Federal Aviation Administration are operated by the US government and are beyond our control.") + " " +
               tr("Detailed information can be found at %1.").arg(u"<a href='https://api.faa.gov/s'>api.faa.gov/s</a>"_qs) +
               "</p>" +

               "<h3>" +
               tr("Responsible") +
               "</h3> " +

               "<p>" +
               "Stefan Kebekus, Wintererstraße 77, 79104 Freiburg im Breisgau, Germany" +
               "</p>";
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
        QString result;
        result += "<p>" + tr("As requested by the search-and-rescue community, <strong>Enroute Flight Navigation</strong> is now able to accept location shares from Google Maps.") + "</p>";
        result += "<p>" + tr("For enhanced privacy, <strong>Enroute Flight Navigation</strong> routes requests for METAR/TAF and NOTAM data through our servers. "
                             "This hides your IP address from the web services that provide the data.")+ "</p>";
        result += "<p>" + tr("We need help with promotional graphics for the app stores and with explainer videos. "
                             "If you are a graphic/video artist and would like to help, then please be in touch.") + "</p>";
        return result;
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


QString Librarian::import(Librarian::Library library, const QString& fileName)
{
    if (library != Librarian::Routes)
    {
        return u"Importing into libraries other than routs is not yet implemented."_qs;
    }

    QString myFileName = fileName;
    if (fileName.startsWith(u"file://"_qs))
    {
        myFileName = fileName.mid(7);
    }

    Navigation::FlightRoute route;
    auto errorMsg = route.load(myFileName);
    if (!errorMsg.isEmpty())
    {
        return errorMsg;
    }

    auto baseName = QFileInfo(myFileName).baseName();
    auto savePath = fullPath(library, baseName);
    if (QFile::exists(savePath))
    {
        for(int i=1; ; i++)
        {
            auto newBaseName = u"%1 (%2)"_qs.arg(baseName).arg(i);
            savePath = fullPath(library, newBaseName);
            if (!QFile::exists(savePath))
            {
                break;
            }
        }
    }

    errorMsg = route.save(savePath);
    if (!errorMsg.isEmpty())
    {
        return errorMsg;
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

    QDir const dir(directory(library));
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
    QString const simplifiedFilter = simplifySpecialChars(filter);

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
    QString cacheString = simplifySpecialChars_cache.value(string);
    if (!cacheString.isEmpty())
    {
        return cacheString;
    }

    QString normalizedString = string.normalized(QString::NormalizationForm_KD);
    return normalizedString.remove(specialChars);
}

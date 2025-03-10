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

#include <QNetworkAccessManager>
#include <QStandardPaths>
#include <QSysInfo>
#include <QtGlobal>

#include "config.h"
#include "Librarian.h"
#include "navigation/FlightRoute.h"

using namespace Qt::Literals::StringLiterals;


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
    if (name == u"appUpdateRequired"_s)
    {
        return tr("<p>This version of <strong>Enroute Flight Navigation</strong> is outdated and will no longer receive map updates. "
                  "<strong>Please update this app at your earliest convencience.</strong></p>");
    }

    if (name == u":text/authors.html"_s)
    {
        return "<p>"+tr("The app <strong>Enroute Flight Navigation</strong> was written by Stefan Kebekus, flight enthusiast since 1986 and member of the Akaflieg Freiburg flight club. Stefan flies gliders and motor planes.")+"</p>"
               + "<p><strong>"+tr("Address")+"</strong>: Stefan Kebekus, Wintererstraße 77, 79104 Freiburg, Germany · <a href='mailto:stefan.kebekus@gmail.com'>stefan.kebekus@gmail.com</a></p>"
               + "<h3>" + tr("Contributions") + "</h3>"
               + "<ul style='margin-left:-25px;'>"
               + "<li>"
               + "<strong>" + tr("iOS Version") + ":</strong> " + tr("The app has been ported to iOS by Simon Schneider, who also maintains the iOS port. Simon received his PPL license in 2024. Like Stefan, he is a member of the Akaflieg Freiburg flight club.")
               + "</li>"
               + "<li>"
               + "<strong>" + tr("Programming") + ":</strong> " + tr("Heinz Blöchinger has helped us with file import functionality. After 15 years of alpine gliding, Heinz has fulfilled a big dream and now flies helicopters.")
               + "</li>"
               + "<li>"
               + "<strong>" + tr("Programming") + ":</strong> " + tr("Christian Engelhardt started the implementation of height density calculation. Christian is a PPL pilot in southern Germany, studied electrical engineering and working as an Embedded SW Engineer.")
               + "</li>"
               + "<li>"
               + "<strong>" + tr("Programming") + ":</strong> " + tr("Tom Linz completed height density calculation. He received his PPL license in late 2024. Tom works as a development engineer for safety systems.")
               + "</li>"
               + "</ul>"
               + tr(R"html(
<h3>Translations</h3>
)html") + (R"html(
<ul style="margin-left:-25px;">
)html") + tr(R"html(
<li><strong>French:</strong> Adrien Crovato and Luca Riva. Both are private pilots and aerospace engineers. Luca is also doing aerobatics.</li>
)html") + tr(R"html(
<li><strong>German:</strong> Markus Sachs. Markus flies trikes and is an enthusiastic 'Co' on everyting else that flies.</li>
)html") + tr(R"html(
<li><strong>Italian:</strong> Ivan Battistella and Antonio Fardella.  Antonio is an ultralight pilot with a passion for everything challenging.</li>
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

    if (name == u":text/aviationMapMissing.html"_s)
    {
        return tr(R"html(
<p>We receive our aviation data from <a href="http://openaip.net">openAIP</a>. This is a not-for-profit organisation where volunteers compile aviation data for many countries. If openAIP covers your country, we might be able to generate maps for it. First, however, we need to be reasonably sure that the openAIP data is accurate and complete. Please have a look at the <a href="http://maps.openaip.net">openAIP maps</a> and compare the display with an official aviation map of your country. Are the airfields there? Are runway lengths/orientations and frequencies correct? Are NavAids correctly displayed, with correct codes and frequencies? And what about airspaces?</p>

<p>If you are convinced that the data is good enough to be added, you can request to add the country. Please go to <a href="https://github.com/Akaflieg-Freiburg/enrouteServer/issues">this web site</a> and open an 'issue' with your request. Please tell us who you are, where you fly and how you convinced yourself that the data is good. If you find that the data is not good enough, you are welcome to join the openAIP project and help to improve the data.</p>

<p>Please understand that we program this free app in our spare time, as a service to the community. Sadly, I should point out that sending us impolite demands is not likely to give the result that you desire.</p>)html");
    }

    if (name == u":text/info_enroute.html"_s)
    {
        QString version(QStringLiteral(ENROUTE_VERSION_STRING));
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

    if (name == u":text/info_license.html"_s)
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

    if (name == u":text/privacy.html"_s)
    {
        return  "<p>" +
               tr("This Privacy Policy outlines the data handling practices for the app <strong>Enroute Flight Navigation</strong>.") + " " +
               tr("We prioritize your privacy and do not collect or store personally identifiable information.") + " " +
               tr("However, for the app to function properly, it must communicate with certain servers on the Internet.") + " " +
               tr("The following sections list the servers that <strong>Enroute Flight Navigation</strong> communicates with and explain the communication purposes.") + " " +
               "</p>" +

               "<p>" +
               tr("By using Enroute Flight Navigation, you agree to this Privacy Policy.") + " " +
               tr("We may update this policy periodically, and any changes will be posted within the app.") +
               "</p>" +

               "<h3>" +
               tr("1. Data and Anonymization Server (enroute-data)") +
               "</h3>" +

               "<p>" +
               tr("<strong>Enroute Flight Navigation</strong> regularly communicates with the server <strong>enroute-data.akaflieg-freiburg.de</strong> (referred to as <strong>enroute-data</strong>), operated by Hetzner Online GmbH on behalf of Akaflieg Freiburg.") + " " +
               tr("The server's primary role is to facilitate the app's functionality while maintaining user privacy.") + " " +
               "</p>" +

               "<h4>" +
               tr("1.1. Data Collected") +
               "</h4>" +

               "<p>" +
               tr("In every communication with <strong>enroute-data</strong>, your device's IP address is transmitted to the server.") + " " +
               tr("The server needs to know the IP address to respond.") +
               "</p>" +

               "<p>" +
               tr("The server stores the following data items in its log files.") +
               "</p>" +

               "<ul style='margin-left:-25px;'>" +
               "<li>" + tr("<strong>Pseudonymized IP Address</strong>: The last byte of your IP address is replaced with a random number, ensuring it cannot identify you.") + "</li>" +
               "<li>" + tr("<strong>Access Data</strong>: Date and time of access, error codes, and data sent.") + "</li>" +
               "<li>" + tr("<strong>Software Information</strong>: The name of the web browser or software in use if the visitor’s software provides this information to the server.") + " " +
               tr("<strong>Enroute Flight Navigation</strong> does not provide this information and sends a standard text ('Mozilla/5.0') instead.") + "</li>" +
               "<li>" + tr("<strong>Operating System:</strong> The operating system's name if the visitor’s software provides this information to the server.") + " " +
               tr("<strong>Enroute Flight Navigation</strong> does not provide this information. ") + "</li>" +
               "</ul>" +

               "<h4>" +
               tr("1.2. Communication: Data Download") +
               "</h4>" +

               "<p>" +
               tr("The app checks for updates and downloads data from <strong>enroute-data</strong> to provide current maps and aviation data.") + " " +
               tr("Your IP address is transmitted in the process.") +
               "</p>" +

               "<h4>" +
               tr("1.3. Communication: NOTAM, METAR and TAF ") + " " +
               "</h4>" +

               "<p>" +
               tr("<strong>Enroute Flight Navigation</strong> shows NOTAMs, METARs, and TAFs for airfields near your current location and your currently planned route.") + " " +
               tr("It also shows NOTAMs, METARs, and TAFs for all waypoints you open in the app.") + " " +
               tr("To provide this functionality, requests are transmitted to the server <strong>enroute-data</strong> at regular intervals and whenever new data is requested.") + " " +
               tr("In addition to your device's IP address, the following data items will be sent.") + " " +
               "</p>" +

               "<ul style='margin-left:-25px;'>" +
               "<li>" + tr("Your current location") + "</li>" +
               "<li>" + tr("The currently planned route") + "</li>" +
               "<li>" + tr("Waypoint coordinates") + "</li>" +
               "</ul>" +

               "<p>" +
               tr("The server forwards requests for NOTAMs to a Federal Aviation Administration web service but hides your IP address, so the service will never see it.") + " " +
               tr("The Federal Aviation Administration's web services are operated by the US government.") + " " +
               tr("We do not control the data handling practices of these external services.") + " " +
               tr("Detailed information can be found at <strong>api.faa.gov</strong>.") +
               "</p>" +

               "<h3>" +
               tr("Other Servers ") +
               "</h3>" +

               "<p>" +
               tr("At the user's request, <strong>Enroute Flight Navigation</strong> may display external websites in an embedded browser window or ask the operating system to open external apps such as Google Maps.") + " " +
               tr("These external sites and apps are beyond our control and may collect their own data.") +
               "</p>" +

               "<p>" +
               tr("Users expect web browsers to follow hyperlinks immediately but may not expect the same behavior elsewhere in the app.") + " " +
               tr("To account for these expectations, <strong>Enroute Flight Navigation</strong> operates as follows.") +
               "</p>" +

               "<ul style='margin-left:-25px;'>" +
               "<li>" + tr("<strong>Embedded Browser Windows</strong>: Clicking a hyperlink is considered authorization to open the external site.") + "</li>"
               "<li>" + tr("<strong>Outside Embedded Browser Windows</strong>: The app will ask for explicit user authorization before opening any external site or app.") + "</li>"
               "</ul>" +

               "<h3>" +
               tr("Responsible") +
               "</h3> " +

               "<p>" +
               "Stefan Kebekus, Wintererstraße 77, 79104 Freiburg im Breisgau, Germany" +
               "</p>";
    }

    if (name == u":text/whatsnew.html"_s)
    {
        QString result;
        result += "<p>"
                  + tr("Enroute Flight Navigation now offers ICAO and Glider Charts for Switzerland. "
                       "To download these maps, open the main menu and go to Library/Maps and Data. "
                       "We thank the swiss Federal Office of Topography and the Federal Office of Civil Aviation for making the maps publicly available. "
                       "Use these maps for information only. The <a href='https://www.geo.admin.ch/en/general-terms-of-use-fsdi'>license conditions</a> do not allow operational use.")
                  + "</p>";
        result += "<p>"
                  + tr("<strong>Technology Preview:</strong> Enroute Flight Navigation is now able to connect to traffic data receivers via Bluetooth Low Energy. "
                       "Please try the new feature and send us your feedback!") +
                  + "</p>";
        /*
        result += "<p>" + tr("Users with nonstandard hardware can now configure IP adresses for their traffic data receivers. "
                             "Serial port connections are also supported.") + "</p>";
        */
        result += "<p>"
                  + tr("We need help with promotional graphics for the app stores and with explainer videos. "
                       "If you are a graphic/video artist and would like to help, then please be in touch.")
                  + "</p>";
        return result;
    }

    QFile file(name);
    file.open(QIODevice::ReadOnly);
    auto content = file.readAll();
    return QString::fromUtf8(content);
}


Units::ByteSize Librarian::getStringHashFromRessource(const QString &name)
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
        return u"Importing into libraries other than routs is not yet implemented."_s;
    }

    QString myFileName = fileName;
    if (fileName.startsWith(u"file://"_s))
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
            auto newBaseName = u"%1 (%2)"_s.arg(baseName).arg(i);
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

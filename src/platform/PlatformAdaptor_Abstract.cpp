/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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

#include <QClipboard>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>

#include "config.h"
#include "notam/NOTAMProvider.h"
#include "platform/PlatformAdaptor_Abstract.h"
#include "qimage.h"

using namespace Qt::Literals::StringLiterals;


Platform::PlatformAdaptor_Abstract::PlatformAdaptor_Abstract(QObject *parent)
    : GlobalObject(parent)
{
}


QString Platform::PlatformAdaptor_Abstract::clipboardText()
{
    if (qGuiApp == nullptr)
    {
        return {};
    }
    return qGuiApp->clipboard()->text();
}


void Platform::PlatformAdaptor_Abstract::openSatView(const QGeoCoordinate& coordinate)
{
    auto url = u"https://www.google.com/maps/@?api=1&map_action=map&center="_s
               + QString::number(coordinate.latitude())
               + u"%2C"_s
               + QString::number(coordinate.longitude())
               + u"&zoom=15&basemap=satellite"_s;
    QDesktopServices::openUrl(url);
}


QString Platform::PlatformAdaptor_Abstract::systemInfo()
{
    QString result;

    result += u"<h3>App</h3>\n"_s;
    result += u"<table>\n"_s;
    result += u"<tr></tr>\n"_s;
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_s.arg("Enroute Version", QStringLiteral(ENROUTE_VERSION_STRING));
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_s.arg("GIT ", QStringLiteral(GIT_COMMIT));
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_s.arg("Qt", QLibraryInfo::version().toString());
    result += u"</table><br>\n"_s;

    result += u"<h3>System</h3>\n"_s;
    result += u"<table>\n"_s;
    result += u"<tr></tr>\n"_s;
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_s.arg("Build ABI", QSysInfo::buildAbi());
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_s.arg("Build CPU", QSysInfo::buildCpuArchitecture());
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_s.arg("Current CPU", QSysInfo::currentCpuArchitecture());
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_s.arg("Kernel Type", QSysInfo::kernelType());
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_s.arg("Kernel Version", QSysInfo::kernelVersion());
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_s.arg("System Name", QSysInfo::prettyProductName());
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_s.arg("System Type", QSysInfo::productType());
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_s.arg("System Version", QSysInfo::productVersion());
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_s.arg("Locale", language());
    result += u"</table><br>\n"_s;

    QString const updateCheckTimeStamp = QSettings()
        .value(QStringLiteral("DataManager/MapListTimeStamp"))
        .toDateTime()
        .toString("dd-MM-yyyy hh:mm:ss t");
    result += u"<h3>Data</h3>\n"_s;
    result += u"<table>\n"_s;
    result += u"<tr></tr>\n"_s;
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_s.arg("Last Map Update Check", updateCheckTimeStamp);
    auto lastNotamUpdate = notamProvider()->lastUpdate();
    if (lastNotamUpdate.isValid())
    {
        result += u"<tr><td>%1<td><td>%2<td></tr>\n"_s.arg("Last NOTAM download", lastNotamUpdate.toString("dd-MM-yyyy hh:mm:ss t"));
    }
    else
    {
        result += u"<tr><td>%1<td><td>%2<td></tr>\n"_s.arg("Last NOTAM download", "NONE");
    }
    result += u"</table><br>\n"_s;

    return result;
}


QString Platform::PlatformAdaptor_Abstract::language()
{
    return QLocale::system().name().left(2);
}


void Platform::PlatformAdaptor_Abstract::saveScreenshot(const QImage& image, const QString& path)
{
    image.save(path);
}

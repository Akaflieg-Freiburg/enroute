/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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

#include <QLibraryInfo>
#include <QLocale>


#include "platform/PlatformAdaptor_Abstract.h"
#include "qimage.h"


Platform::PlatformAdaptor_Abstract::PlatformAdaptor_Abstract(QObject *parent)
    : GlobalObject(parent)
{
}


QString Platform::PlatformAdaptor_Abstract::systemInfo()
{
    QString result;

    result += u"<h3>App</h3>\n"_qs;
    result += u"<table>\n"_qs;
    result += u"<tr></tr>\n"_qs;
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_qs.arg("Enroute Version", QStringLiteral(PROJECT_VERSION));
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_qs.arg("GIT ", QStringLiteral(GIT_COMMIT));
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_qs.arg("Qt", QLibraryInfo::version().toString());
    result += u"</table><br>\n"_qs;

    result += u"<h3>System</h3>\n"_qs;
    result += u"<table>\n"_qs;
    result += u"<tr></tr>\n"_qs;
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_qs.arg("Build ABI", QSysInfo::buildAbi());
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_qs.arg("Build CPU", QSysInfo::buildCpuArchitecture());
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_qs.arg("Current CPU", QSysInfo::currentCpuArchitecture());
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_qs.arg("Kernel Type", QSysInfo::kernelType());
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_qs.arg("Kernel Version", QSysInfo::kernelVersion());
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_qs.arg("System Name", QSysInfo::prettyProductName());
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_qs.arg("System Type", QSysInfo::productType());
    result += u"<tr><td>%1<td><td>%2<td></tr>\n"_qs.arg("System Version", QSysInfo::productVersion());
    result += u"</table><br>\n"_qs;

    return result;
}


QString Platform::PlatformAdaptor_Abstract::language()
{
    return QLocale::system().name().left(2);
}


void Platform::PlatformAdaptor_Abstract::saveScreenshot(const QImage& image, QString path)
{
    image.save(path);
}

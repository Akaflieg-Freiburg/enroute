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

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>

#include "GlobalObject.h"
#include "Settings.h"


Settings::Settings(QObject *parent)
    : QObject(parent)
{
    // Save some values
    settings.setValue("lastVersion", PROJECT_VERSION);
}


auto Settings::acceptedWeatherTermsStatic() -> bool
{
    return GlobalObject::settings()->acceptedWeatherTerms();
}


auto Settings::hideUpperAirspacesStatic() -> bool
{
    return GlobalObject::settings()->hideUpperAirspaces();
}


void Settings::setAcceptedTerms(int terms)
{
    if (terms == acceptedTerms()) {
        return;
    }
    settings.setValue("acceptedTerms", terms);
    emit acceptedTermsChanged();
}


void Settings::setAcceptedWeatherTerms(bool terms)
{
    if (terms == acceptedWeatherTerms()) {
        return;
    }
    settings.setValue("acceptedWeatherTerms", terms);
    emit acceptedWeatherTermsChanged();
}


void Settings::setHideGlidingSectors(bool hide)
{
    if (hide == hideGlidingSectors()) {
        return;
    }
    settings.setValue("Map/hideGlidingSectors", hide);
    emit hideGlidingSectorsChanged();
}


void Settings::setHideUpperAirspaces(bool hide)
{
    if (hide == hideUpperAirspaces()) {
        return;
    }
    settings.setValue("Map/hideUpperAirspaces", hide);
    emit hideUpperAirspacesChanged();
}


void Settings::setIgnoreSSLProblems(bool ignore)
{
    if (ignore == ignoreSSLProblems()) {
        return;
    }
    settings.setValue("ignoreSSLProblems", ignore);
    emit ignoreSSLProblemsChanged();
}


void Settings::setLastWhatsNewHash(uint lwnh)
{
    if (lwnh == lastWhatsNewHash()) {
        return;
    }
    settings.setValue("lastWhatsNewHash", lwnh);
    emit lastWhatsNewHashChanged();
}


void Settings::setLastWhatsNewInMapsHash(uint lwnh)
{
    if (lwnh == lastWhatsNewInMapsHash()) {
        return;
    }
    settings.setValue("lastWhatsNewInMapsHash", lwnh);
    emit lastWhatsNewInMapsHashChanged();
}


auto Settings::mapBearingPolicy() const -> Settings::MapBearingPolicyValues
{
    auto intVal = settings.value("Map/bearingPolicy", 0).toInt();
    if (intVal == 0) {
        return NUp;
    }
    if (intVal == 1) {
        return TTUp;
    }
    return UserDefinedBearingUp;
}


void Settings::setMapBearingPolicy(MapBearingPolicyValues policy)
{
    if (policy == mapBearingPolicy()) {
        return;
    }

    switch(policy){
    case NUp:
        settings.setValue("Map/bearingPolicy", 0);
        break;
    case TTUp:
        settings.setValue("Map/bearingPolicy", 1);
        break;
    default:
        settings.setValue("Map/bearingPolicy", 2);
        break;
    }
    emit mapBearingPolicyChanged();
}


auto Settings::nightMode() const -> bool
{
    return settings.value("Map/nightMode", false).toBool();
}


void Settings::setNightMode(bool newNightMode)
{
    if (newNightMode == nightMode()) {
        return;
    }

    settings.setValue("Map/nightMode", newNightMode);
    emit nightModeChanged();
}

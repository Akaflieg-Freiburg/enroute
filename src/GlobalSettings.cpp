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

#include "GlobalSettings.h"


// Static instance of this class. Do not analyze, because of many unwanted warnings.
#ifndef __clang_analyzer__
QPointer<GlobalSettings> globalSettingsStatic {};
#endif


GlobalSettings::GlobalSettings(QObject *parent)
    : QObject(parent)
{
    installTranslators();
}

#warning destructor not called
GlobalSettings::~GlobalSettings()
{
    // Save some values
    settings.setValue("lastVersion", PROJECT_VERSION);
}


auto GlobalSettings::acceptedWeatherTermsStatic() -> bool
{
    // Find out that unit system we should use
    auto *globalSettings = GlobalSettings::globalInstance();
    if (globalSettings != nullptr) {
        return globalSettings->acceptedWeatherTerms();
    }
    // Fallback in the very unlikely case that no global object exists
    return false;
}


auto GlobalSettings::globalInstance() -> GlobalSettings *
{
#ifndef __clang_analyzer__
    if (globalSettingsStatic.isNull()) {
        globalSettingsStatic = new GlobalSettings();
    }
    return globalSettingsStatic;
#else
    return nullptr;
#endif
}


auto GlobalSettings::hideUpperAirspacesStatic() -> bool
{
    // Find out that unit system we should use
    auto *globalSettings = GlobalSettings::globalInstance();
    if (globalSettings != nullptr) {
        return globalSettings->hideUpperAirspaces();
    }
    // Fallback in the very unlikely case that no global object exists
    return false;
}


void GlobalSettings::setAcceptedTerms(int terms)
{
    if (terms == acceptedTerms()) {
        return;
    }
    settings.setValue("acceptedTerms", terms);
    emit acceptedTermsChanged();
}


void GlobalSettings::setAcceptedWeatherTerms(bool terms)
{
    if (terms == acceptedWeatherTerms()) {
        return;
    }
    settings.setValue("acceptedWeatherTerms", terms);
    emit acceptedWeatherTermsChanged();
}


void GlobalSettings::setHideUpperAirspaces(bool hide)
{
    if (hide == hideUpperAirspaces()) {
        return;
    }
    settings.setValue("Map/hideUpperAirspaces", hide);
    emit hideUpperAirspacesChanged();
}


void GlobalSettings::setLastWhatsNewHash(uint lwnh)
{
    if (lwnh == lastWhatsNewHash()) {
        return;
    }
    settings.setValue("lastWhatsNewHash", lwnh);
    emit lastWhatsNewHashChanged();
}


auto GlobalSettings::mapBearingPolicy() const -> GlobalSettings::MapBearingPolicyValues
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


void GlobalSettings::setMapBearingPolicy(MapBearingPolicyValues policy)
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


auto GlobalSettings::nightMode() const -> bool
{
    return settings.value("Map/nightMode", false).toBool();
}


void GlobalSettings::setNightMode(bool newNightMode)
{
    if (newNightMode == nightMode()) {
        return;
    }

    settings.setValue("Map/nightMode", newNightMode);
    emit nightModeChanged();
}


void GlobalSettings::setUseMetricUnits(bool unitHorizKmh)
{
    if (unitHorizKmh == useMetricUnits()) {
        return;
    }

    settings.setValue("System/useMetricUnits", unitHorizKmh);
    emit useMetricUnitsChanged();
}


auto GlobalSettings::useMetricUnitsStatic() -> bool
{
    // Find out that unit system we should use
    auto *globalSettings = GlobalSettings::globalInstance();
    if (globalSettings != nullptr) {
        return globalSettings->useMetricUnits();
    }
    // Fallback in the very unlikely case that no global object exists
    return false;
}


void GlobalSettings::installTranslators()
{
    // Remove existing translators
    if (enrouteTranslator != nullptr) {
        QCoreApplication::removeTranslator(enrouteTranslator);
        delete enrouteTranslator;
    }

    // If desired, install new translators
    QLocale::setDefault(QLocale::system());

    enrouteTranslator = new QTranslator(this);
    enrouteTranslator->load(QString(":enroute_%1.qm").arg(QLocale::system().name().left(2)));
    QCoreApplication::installTranslator(enrouteTranslator);
}

/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

#include "Settings.h"


//
// Constructor and destructor
//

Settings::Settings(QObject *parent)
    : QObject(parent)
{
    // Save some values
    settings.setValue(QStringLiteral("lastVersion"), PROJECT_VERSION);

    // Convert old setting to new system
    if (settings.contains(QStringLiteral("Map/hideUpperAirspaces"))) {
        auto hide = settings.value(QStringLiteral("Map/hideUpperAirspaces"), false).toBool();
        if (hide) {
            setAirspaceAltitudeLimit( Units::Distance::fromFT(10000) );
        } else {
            setAirspaceAltitudeLimit( Units::Distance::fromFT(qInf()) );
        }
        settings.remove(QStringLiteral("Map/hideUpperAirspaces"));
    }
}


//
// Getter Methods
//

auto Settings::airspaceAltitudeLimit() const -> Units::Distance
{
    auto aspAlttLimit = Units::Distance::fromFT( settings.value(QStringLiteral("Map/airspaceAltitudeLimit_ft"), qQNaN()).toDouble() );
    if (aspAlttLimit < airspaceAltitudeLimit_min) {
        aspAlttLimit = airspaceAltitudeLimit_min;
    }
    if (aspAlttLimit > airspaceAltitudeLimit_max) {
        aspAlttLimit = Units::Distance::fromFT( qInf() );
    }
    return aspAlttLimit;
}


auto Settings::lastValidAirspaceAltitudeLimit() const -> Units::Distance
{
    auto result = Units::Distance::fromFT(settings.value(QStringLiteral("Map/lastValidAirspaceAltitudeLimit_ft"), 99999).toInt() );
    return qBound(airspaceAltitudeLimit_min, result, airspaceAltitudeLimit_max);
}


auto Settings::mapBearingPolicy() const -> Settings::MapBearingPolicy
{
    auto intVal = settings.value(QStringLiteral("Map/bearingPolicy"), 0).toInt();
    if (intVal == 0) {
        return NUp;
    }
    if (intVal == 1) {
        return TTUp;
    }
    return UserDefinedBearingUp;
}


//
// Setter Methods
//

void Settings::setAcceptedTerms(int terms)
{
    if (terms == acceptedTerms()) {
        return;
    }
    settings.setValue(QStringLiteral("acceptedTerms"), terms);
    emit acceptedTermsChanged();
}


void Settings::setAcceptedWeatherTerms(bool terms)
{
    if (terms == acceptedWeatherTerms()) {
        return;
    }
    settings.setValue(QStringLiteral("acceptedWeatherTerms"), terms);
    emit acceptedWeatherTermsChanged();
}


void Settings::setAirspaceAltitudeLimit(Units::Distance newAirspaceAltitudeLimit)
{
    if (newAirspaceAltitudeLimit < airspaceAltitudeLimit_min) {
        newAirspaceAltitudeLimit = airspaceAltitudeLimit_min;
    }
    if (!newAirspaceAltitudeLimit.isFinite() || (newAirspaceAltitudeLimit > airspaceAltitudeLimit_max)) {
        newAirspaceAltitudeLimit = Units::Distance();
    }

    if (newAirspaceAltitudeLimit != airspaceAltitudeLimit()) {
        settings.setValue(QStringLiteral("Map/airspaceAltitudeLimit_ft"), newAirspaceAltitudeLimit.toFeet());
        emit airspaceAltitudeLimitChanged();
    }

    if (newAirspaceAltitudeLimit.isFinite() &&
            (newAirspaceAltitudeLimit != lastValidAirspaceAltitudeLimit())) {
        settings.setValue(QStringLiteral("Map/lastValidAirspaceAltitudeLimit_ft"), newAirspaceAltitudeLimit.toFeet());
        emit lastValidAirspaceAltitudeLimitChanged();
    }
}


void Settings::setHideGlidingSectors(bool hide)
{
    if (hide == hideGlidingSectors()) {
        return;
    }
    settings.setValue(QStringLiteral("Map/hideGlidingSectors"), hide);
    emit hideGlidingSectorsChanged();
}


void Settings::setHillshading(bool show)
{
    if (show == hillshading()) {
        return;
    }
    settings.setValue(QStringLiteral("Map/hillshading"), show);
    emit hillshadingChanged();
}


void Settings::setIgnoreSSLProblems(bool ignore)
{
    if (ignore == ignoreSSLProblems()) {
        return;
    }
    settings.setValue(QStringLiteral("ignoreSSLProblems"), ignore);
    emit ignoreSSLProblemsChanged();
}


void Settings::setLastWhatsNewHash(size_t lwnh)
{
    if (lwnh == lastWhatsNewHash()) {
        return;
    }
    settings.setValue(QStringLiteral("lastWhatsNewHash"), QVariant::fromValue(lwnh));
    emit lastWhatsNewHashChanged();
}


void Settings::setLastWhatsNewInMapsHash(size_t lwnh)
{
    if (lwnh == lastWhatsNewInMapsHash()) {
        return;
    }
    settings.setValue(QStringLiteral("lastWhatsNewInMapsHash"), QVariant::fromValue(lwnh));
    emit lastWhatsNewInMapsHashChanged();
}


void Settings::setMapBearingPolicy(MapBearingPolicy policy)
{
    if (policy == mapBearingPolicy()) {
        return;
    }

    switch(policy){
    case NUp:
        settings.setValue(QStringLiteral("Map/bearingPolicy"), 0);
        break;
    case TTUp:
        settings.setValue(QStringLiteral("Map/bearingPolicy"), 1);
        break;
    default:
        settings.setValue(QStringLiteral("Map/bearingPolicy"), 2);
        break;
    }
    emit mapBearingPolicyChanged();
}


void Settings::setNightMode(bool newNightMode)
{
    if (newNightMode == nightMode()) {
        return;
    }

    settings.setValue(QStringLiteral("Map/nightMode"), newNightMode);
    emit nightModeChanged();
}


void Settings::setPositioningByTrafficDataReceiver(bool newPositioningByTrafficDataReceiver)
{
    if (newPositioningByTrafficDataReceiver == positioningByTrafficDataReceiver()) {
        return;
    }

    settings.setValue(QStringLiteral("positioningByTrafficDataReceiver"), newPositioningByTrafficDataReceiver);
    emit positioningByTrafficDataReceiverChanged();
}


void Settings::setShowAltitudeAGL(bool newShowAltitudeAGL)
{
    if (newShowAltitudeAGL == showAltitudeAGL())
    {
        return;
    }
    settings.setValue(QStringLiteral("showAltitudeAGL"), newShowAltitudeAGL);
    emit showAltitudeAGLChanged();
}

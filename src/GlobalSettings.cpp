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

#include <QCoreApplication>

#include "config.h"
#include "GlobalSettings.h"


//
// Constructor and destructor
//


GlobalSettings::GlobalSettings(QObject *parent)
    : QObject(parent)
{
    QCoreApplication::processEvents();

    // Save some values
    m_settings.setValue(QStringLiteral("lastVersion"), ENROUTE_VERSION_STRING);

    // Read values
    m_positioningByTrafficDataReceiver = m_settings.value(QStringLiteral("positioningByTrafficDataReceiver"), false).toBool();

    // Convert old setting to new system
    if (m_settings.contains(QStringLiteral("Map/hideUpperAirspaces"))) {
        auto hide = m_settings.value(QStringLiteral("Map/hideUpperAirspaces"), false).toBool();
        if (hide) {
            setAirspaceAltitudeLimit( Units::Distance::fromFT(10000) );
        } else {
            setAirspaceAltitudeLimit( Units::Distance::fromFT(qInf()) );
        }
        m_settings.remove(QStringLiteral("Map/hideUpperAirspaces"));
    }
}


//
// Getter Methods
//

auto GlobalSettings::airspaceAltitudeLimit() const -> Units::Distance
{
    auto aspAlttLimit = Units::Distance::fromFT( m_settings.value(QStringLiteral("Map/airspaceAltitudeLimit_ft"), qQNaN()).toDouble() );
    if (aspAlttLimit < airspaceAltitudeLimit_min) {
        aspAlttLimit = airspaceAltitudeLimit_min;
    }
    if (aspAlttLimit > airspaceAltitudeLimit_max) {
        aspAlttLimit = Units::Distance::fromFT( qInf() );
    }
    return aspAlttLimit;
}


auto GlobalSettings::fontSize() const -> int
{
    auto fontSize = m_settings.value(QStringLiteral("fontSize"), 14).toInt();
    return qBound(14, fontSize, 20);
}


auto GlobalSettings::lastValidAirspaceAltitudeLimit() const -> Units::Distance
{
    auto result = Units::Distance::fromFT(m_settings.value(QStringLiteral("Map/lastValidAirspaceAltitudeLimit_ft"), 99999).toInt() );
    return qBound(airspaceAltitudeLimit_min, result, airspaceAltitudeLimit_max);
}


auto GlobalSettings::mapBearingPolicy() const -> GlobalSettings::MapBearingPolicy
{
    auto intVal = m_settings.value(QStringLiteral("Map/bearingPolicy"), 0).toInt();
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

void GlobalSettings::setAcceptedTerms(int terms)
{
    if (terms == acceptedTerms()) {
        return;
    }
    m_settings.setValue(QStringLiteral("acceptedTerms"), terms);
    emit acceptedTermsChanged();
}


void GlobalSettings::setAlwaysOpenExternalWebsites(bool alwaysOpen)
{
    if (alwaysOpen == alwaysOpenExternalWebsites()) {
        return;
    }
    m_settings.setValue(QStringLiteral("alwaysOpenExternalWebsites"), alwaysOpen);
    emit alwaysOpenExternalWebsitesChanged();
}


void GlobalSettings::setAirspaceAltitudeLimit(Units::Distance newAirspaceAltitudeLimit)
{
    if (newAirspaceAltitudeLimit < airspaceAltitudeLimit_min) {
        newAirspaceAltitudeLimit = airspaceAltitudeLimit_min;
    }
    if (!newAirspaceAltitudeLimit.isFinite() || (newAirspaceAltitudeLimit > airspaceAltitudeLimit_max)) {
        newAirspaceAltitudeLimit = Units::Distance();
    }

    if (newAirspaceAltitudeLimit != airspaceAltitudeLimit()) {
        m_settings.setValue(QStringLiteral("Map/airspaceAltitudeLimit_ft"), newAirspaceAltitudeLimit.toFeet());
        emit airspaceAltitudeLimitChanged();
    }

    if (newAirspaceAltitudeLimit.isFinite() &&
            (newAirspaceAltitudeLimit != lastValidAirspaceAltitudeLimit())) {
        m_settings.setValue(QStringLiteral("Map/lastValidAirspaceAltitudeLimit_ft"), newAirspaceAltitudeLimit.toFeet());
        emit lastValidAirspaceAltitudeLimitChanged();
    }
}


void GlobalSettings::setExpandNotamAbbreviations(bool newExpandNotamAbbreviations)
{
    if (newExpandNotamAbbreviations == expandNotamAbbreviations())
    {
        return;
    }
    m_settings.setValue(QStringLiteral("expandNotamAbbreviations"), newExpandNotamAbbreviations);
    emit expandNotamAbbreviationsChanged();
}


void GlobalSettings::setFontSize(int newFontSize)
{
    newFontSize = qBound(14, newFontSize, 20);


    if (newFontSize == fontSize())
    {
        return;
    }
    m_settings.setValue(QStringLiteral("fontSize"), newFontSize);
    emit fontSizeChanged();
}


void GlobalSettings::setHideGlidingSectors(bool hide)
{
    if (hide == hideGlidingSectors())
    {
        return;
    }
    m_settings.setValue(QStringLiteral("Map/hideGlidingSectors"), hide);
    emit hideGlidingSectorsChanged();
}


void GlobalSettings::setIgnoreSSLProblems(bool ignore)
{
    if (ignore == ignoreSSLProblems())
    {
        return;
    }
    m_settings.setValue(QStringLiteral("ignoreSSLProblems"), ignore);
    emit ignoreSSLProblemsChanged();
}


void GlobalSettings::setLastWhatsNewHash(Units::ByteSize lwnh)
{
    if (lwnh == lastWhatsNewHash())
    {
        return;
    }
    m_settings.setValue(QStringLiteral("lastWhatsNewHash"), QVariant::fromValue((size_t)lwnh));
    emit lastWhatsNewHashChanged();
}


void GlobalSettings::setLastWhatsNewInMapsHash(Units::ByteSize lwnh)
{
    if (lwnh == lastWhatsNewInMapsHash())
    {
        return;
    }
    m_settings.setValue(QStringLiteral("lastWhatsNewInMapsHash"), QVariant::fromValue((size_t)lwnh));
    emit lastWhatsNewInMapsHashChanged();
}


void GlobalSettings::setPrivacyHash(Units::ByteSize newHash)
{
    if (newHash == privacyHash())
    {
        return;
    }
    m_settings.setValue(QStringLiteral("privacyHash"), QVariant::fromValue((size_t)newHash));
    emit privacyHashChanged();
}


void GlobalSettings::setMapBearingPolicy(MapBearingPolicy policy)
{
    if (policy == mapBearingPolicy())
    {
        return;
    }

    switch(policy){
    case NUp:
        m_settings.setValue(QStringLiteral("Map/bearingPolicy"), 0);
        break;
    case TTUp:
        m_settings.setValue(QStringLiteral("Map/bearingPolicy"), 1);
        break;
    default:
        m_settings.setValue(QStringLiteral("Map/bearingPolicy"), 2);
        break;
    }
    emit mapBearingPolicyChanged();
}


void GlobalSettings::setNightMode(bool newNightMode)
{
    if (newNightMode == nightMode())
    {
        return;
    }

    m_settings.setValue(QStringLiteral("Map/nightMode"), newNightMode);
    emit nightModeChanged();
}


void GlobalSettings::setPositioningByTrafficDataReceiver(bool newPositioningByTrafficDataReceiver)
{
    m_settings.setValue(QStringLiteral("positioningByTrafficDataReceiver"), newPositioningByTrafficDataReceiver);
    m_positioningByTrafficDataReceiver = newPositioningByTrafficDataReceiver;
}


void GlobalSettings::setShowAltitudeAGL(bool newShowAltitudeAGL)
{
    if (newShowAltitudeAGL == showAltitudeAGL())
    {
        return;
    }
    m_settings.setValue(QStringLiteral("showAltitudeAGL"), newShowAltitudeAGL);
    emit showAltitudeAGLChanged();
}


void GlobalSettings::setVoiceNotifications(uint newVoiceNotifications)
{
    if (newVoiceNotifications == voiceNotifications())
    {
        return;
    }
    m_settings.setValue(QStringLiteral("voiceNotifications"), newVoiceNotifications);
    emit voiceNotificationsChanged();
}

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

#pragma once

#include <QFile>
#include <QLocale>
#include <QObject>
#include <QPointer>
#include <QSettings>

#include "units/Distance.h"


/*! \brief Global Settings Manager
 *
 * This class holds a few data items and exposes them via QObject properties, so
 * that they can be used in QML.  All data stored in this class is saved via
 * QSettings on destruction.
 *
 * There exists one static instance of this class, which can be accessed via the
 * Global functions.  No other instance of this class should be used.
 *
 * The methods in this class are reentrant, but not thread safe.
 */

class Settings : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Settings(QObject *parent = nullptr);

    /*! \brief Possible map bearing policies */
    enum MapBearingPolicyValues
    {
        NUp, /*!< North is up. */
        TTUp, /*!< True Track is up.  */
        UserDefinedBearingUp /*!< User-defined bearing is up. */
    };
    Q_ENUM(MapBearingPolicyValues)

    /*! \brief Find out if Terms & Conditions have been accepted
     *
     * This property says which version of our "terms and conditions" have been
     * accepted by the user; this is used to determine whether the
     * first-use-dialog should be shown.  If nothing has been accepted yet, 0 is
     * returned.
     */
    Q_PROPERTY(int acceptedTerms READ acceptedTerms WRITE setAcceptedTerms NOTIFY acceptedTermsChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property acceptedTerms
     */
    int acceptedTerms() const { return settings.value(QStringLiteral("acceptedTerms"), 0).toInt(); }

    /*! \brief Setter function for property of the same name
     *
     * @param terms Property acceptedTerms
     */
    void setAcceptedTerms(int terms);

    /*! \brief Find out if Weather Terms have been accepted
     *
     * This property says if the user has agreed to share its location and route
     * with aviationweather.gov (US government website providing weather data).
     * If nothing has been accepted yet, false is returned.
     */
    Q_PROPERTY(bool acceptedWeatherTerms READ acceptedWeatherTerms WRITE setAcceptedWeatherTerms NOTIFY acceptedWeatherTermsChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property acceptedWeatherTerms
     */
    bool acceptedWeatherTerms() const { return settings.value(QStringLiteral("acceptedWeatherTerms"), false).toBool(); }

    /*! \brief Getter function for property of the same name
     *
     * This function differs from acceptedWeatherTerms() only in that it is static.
     *
     * @returns Property acceptedWeatherTerms
     */
    static bool acceptedWeatherTermsStatic();

    /*! \brief Setter function for property of the same name
     *
     * @param terms Property acceptedWeatherTerms
     */
    void setAcceptedWeatherTerms(bool terms);

    /*! \brief Airspace altitude limit for map display
     *
     * This property holds an altitude. The moving map will ony display airspaces whose lower
     * boundary is lower than this altitude. The altitude value lies in the range [airspaceHeightLimit_min, airspaceHeightLimit_max]
     * or takes a non-finite value to indicate that all airspaces shall be shown.
     */
    Q_PROPERTY(Units::Distance airspaceAltitudeLimit READ airspaceAltitudeLimit WRITE setAirspaceAltitudeLimit NOTIFY airspaceAltitudeLimitChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property airspaceAltitudeLimit
     */
    Units::Distance airspaceAltitudeLimit() const;

    /*! \brief Setter function for property of the same name
     *
     *  If newAirspaceAltitudeLimit is less than airspaceHeightLimit_min, then
     *  airspaceHeightLimit_min will be set. If newAirspaceAltitudeLimit is higher than
     *  airspaceHeightLimit_max, then airspaceHeightLimit_max will be set.
     *
     * @param newAirspaceAltitudeLimit Property airspaceAltitudeLimit
     */
    void setAirspaceAltitudeLimit(Units::Distance newAirspaceAltitudeLimit);

    /*! \brief Minimum acceptable value for property airspaceAltitudeLimit
     *
     *  This is currently set to 3.000 ft
     */
    Q_PROPERTY(Units::Distance airspaceAltitudeLimit_min MEMBER airspaceAltitudeLimit_min CONSTANT)
    static constexpr Units::Distance airspaceAltitudeLimit_min = Units::Distance::fromFT(3000);

    /*! \brief Maximum acceptable value for property airspaceAltitudeLimit
     *
     *  This is currently set to 15.000 ft
     */
    Q_PROPERTY(Units::Distance airspaceAltitudeLimit_max MEMBER airspaceAltitudeLimit_max CONSTANT)
    static constexpr Units::Distance airspaceAltitudeLimit_max = Units::Distance::fromFT(15000);

    /*! \brief Last finite value of airspaceAltitudeLimit */
    Q_PROPERTY(Units::Distance lastValidAirspaceAltitudeLimit READ lastValidAirspaceAltitudeLimit NOTIFY lastValidAirspaceAltitudeLimitChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property hasTranslation
     */
    Units::Distance lastValidAirspaceAltitudeLimit() const;

    /*! \brief True if translation files exist for the system language */
    Q_PROPERTY(bool hasTranslation READ hasTranslation CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property hasTranslation
     */
    bool hasTranslation() const { return QFile::exists(QStringLiteral(":enroute_%1.qm").arg(QLocale::system().name().left(2))); }

    /*! \brief Hide gliding sectors */
    Q_PROPERTY(bool hideGlidingSectors READ hideGlidingSectors WRITE setHideGlidingSectors NOTIFY hideGlidingSectorsChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property hideGlidingSectors
     */
    bool hideGlidingSectors() const { return settings.value(QStringLiteral("Map/hideGlidingSectors"), true).toBool(); }

    /*! \brief Setter function for property of the same name
     *
     * @param hide Property hideGlidingSectors
     */
    void setHideGlidingSectors(bool hide);

    /*! \brief Ignore SSL securitry problems */
    Q_PROPERTY(bool ignoreSSLProblems READ ignoreSSLProblems WRITE setIgnoreSSLProblems NOTIFY ignoreSSLProblemsChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property ignoreSSLProblems
     */
    bool ignoreSSLProblems() const { return settings.value(QStringLiteral("ignoreSSLProblems"), false).toBool(); }

    /*! \brief Setter function for property of the same name
     *
     * @param ignore Property ignoreSSLProblems
     */
    void setIgnoreSSLProblems(bool ignore);

    /*! \brief Hash of the last "what's new" message that was shown to the user
     *
     * This property is used in the app to determine if the message has been
     * shown or not.
     */
    Q_PROPERTY(uint lastWhatsNewHash READ lastWhatsNewHash WRITE setLastWhatsNewHash NOTIFY lastWhatsNewHashChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property lastWhatsNewHash
     */
    uint lastWhatsNewHash() const { return settings.value(QStringLiteral("lastWhatsNewHash"), 0).toUInt(); }

    /*! \brief Getter function for property of the same name
     *
     * @param lwnh Property lastWhatsNewHash
     */
    void setLastWhatsNewHash(uint lwnh);

    /*! \brief Hash of the last "what's new in maps" message that was shown to the user
     *
     * This property is used in the app to determine if the message has been
     * shown or not.
     */
    Q_PROPERTY(uint lastWhatsNewInMapsHash READ lastWhatsNewInMapsHash WRITE setLastWhatsNewInMapsHash NOTIFY lastWhatsNewInMapsHashChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property lastWhatsNewInMapsHash
     */
    uint lastWhatsNewInMapsHash() const { return settings.value(QStringLiteral("lastWhatsNewInMapsHash"), 0).toUInt(); }

    /*! \brief Getter function for property of the same name
     *
     * @param lwnh Property lastWhatsNewInMapsHash
     */
    void setLastWhatsNewInMapsHash(uint lwnh);

    /*! \brief Map bearing policy */
    Q_PROPERTY(MapBearingPolicyValues mapBearingPolicy READ mapBearingPolicy WRITE setMapBearingPolicy NOTIFY mapBearingPolicyChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property mapBearingPolicy
     */
    MapBearingPolicyValues mapBearingPolicy() const;

    /*! \brief Setter function for property of the same name
     *
     * @param policy Property mapBearingPolicy
     */
    void setMapBearingPolicy(MapBearingPolicyValues policy);

    /*! \brief Night mode */
    Q_PROPERTY(bool nightMode READ nightMode WRITE setNightMode NOTIFY nightModeChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property night mode
     */
    bool nightMode() const;

    /*! \brief Setter function for property of the same name
     *
     * @param newNightMode Property nightMode
     */
    void setNightMode(bool newNightMode);

signals:
    /*! \brief Notifier signal */
    void acceptedTermsChanged();

    /*! \brief Notifier signal */
    void acceptedWeatherTermsChanged();

    /*! \brief Notifier signal */
    void airspaceAltitudeLimitChanged();

    /*! \brief Notifier signal */
    void hideGlidingSectorsChanged();

    /*! \brief Notifier signal */
    void ignoreSSLProblemsChanged();

    /*! \brief Notifier signal */
    void lastValidAirspaceAltitudeLimitChanged();

    /*! \brief Notifier signal */
    void lastWhatsNewHashChanged();

    /*! \brief Notifier signal */
    void lastWhatsNewInMapsHashChanged();

    /*! \brief Notifier signal */
    void mapBearingPolicyChanged();

    /*! \brief Notifier signal */
    void nightModeChanged();

private:
    Q_DISABLE_COPY_MOVE(Settings)

    QSettings settings;
};

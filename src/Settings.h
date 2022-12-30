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
    /*! \brief Possible map bearing policies */
    enum MapBearingPolicy
    {
        NUp, /*!< North is up. */
        TTUp, /*!< True Track is up.  */
        UserDefinedBearingUp /*!< User-defined bearing is up. */
    };
    Q_ENUM(MapBearingPolicy)


    //
    // Constructor and destructor
    //

    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Settings(QObject *parent = nullptr);


    //
    // Properties
    //

    /*! \brief Find out if Terms & Conditions have been accepted
     *
     * This property says which version of our "terms and conditions" have been
     * accepted by the user; this is used to determine whether the
     * first-use-dialog should be shown.  If nothing has been accepted yet, 0 is
     * returned.
     */
    Q_PROPERTY(int acceptedTerms READ acceptedTerms WRITE setAcceptedTerms NOTIFY acceptedTermsChanged)

    /*! \brief Find out if Weather Terms have been accepted
     *
     * This property says if the user has agreed to share its location and route
     * with aviationweather.gov (US government website providing weather data).
     * If nothing has been accepted yet, false is returned.
     */
    Q_PROPERTY(bool acceptedWeatherTerms READ acceptedWeatherTerms WRITE setAcceptedWeatherTerms NOTIFY acceptedWeatherTermsChanged)

    /*! \brief Airspace altitude limit for map display
     *
     * This property holds an altitude. The moving map will ony display airspaces whose lower
     * boundary is lower than this altitude. The altitude value lies in the range [airspaceHeightLimit_min, airspaceHeightLimit_max]
     * or takes a non-finite value to indicate that all airspaces shall be shown.
     */
    Q_PROPERTY(Units::Distance airspaceAltitudeLimit READ airspaceAltitudeLimit WRITE setAirspaceAltitudeLimit NOTIFY airspaceAltitudeLimitChanged)

    /*! \brief Minimum acceptable value for property airspaceAltitudeLimit
     *
     *  This is currently set to 3.000 ft
     */
    Q_PROPERTY(Units::Distance airspaceAltitudeLimit_min MEMBER airspaceAltitudeLimit_min CONSTANT)

    /*! \brief Maximum acceptable value for property airspaceAltitudeLimit
     *
     *  This is currently set to 15.000 ft
     */
    Q_PROPERTY(Units::Distance airspaceAltitudeLimit_max MEMBER airspaceAltitudeLimit_max CONSTANT)

    /*! \brief Hide gliding sectors */
    Q_PROPERTY(bool hideGlidingSectors READ hideGlidingSectors WRITE setHideGlidingSectors NOTIFY hideGlidingSectorsChanged)

    /*! \brief Show hillshades on moving msp */
    Q_PROPERTY(bool hillshading READ hillshading WRITE setHillshading NOTIFY hillshadingChanged)

    /*! \brief Ignore SSL security problems */
    Q_PROPERTY(bool ignoreSSLProblems READ ignoreSSLProblems WRITE setIgnoreSSLProblems NOTIFY ignoreSSLProblemsChanged)

    /*! \brief Last finite value of airspaceAltitudeLimit */
    Q_PROPERTY(Units::Distance lastValidAirspaceAltitudeLimit READ lastValidAirspaceAltitudeLimit NOTIFY lastValidAirspaceAltitudeLimitChanged)

    /*! \brief Hash of the last "what's new" message that was shown to the user
     *
     * This property is used in the app to determine if the message has been
     * shown or not.
     */
    Q_PROPERTY(size_t lastWhatsNewHash READ lastWhatsNewHash WRITE setLastWhatsNewHash NOTIFY lastWhatsNewHashChanged)

    /*! \brief Hash of the last "what's new in maps" message that was shown to the user
     *
     * This property is used in the app to determine if the message has been
     * shown or not.
     */
    Q_PROPERTY(size_t lastWhatsNewInMapsHash READ lastWhatsNewInMapsHash WRITE setLastWhatsNewInMapsHash NOTIFY lastWhatsNewInMapsHashChanged)

    /*! \brief Map bearing policy */
    Q_PROPERTY(MapBearingPolicy mapBearingPolicy READ mapBearingPolicy WRITE setMapBearingPolicy NOTIFY mapBearingPolicyChanged)

    /*! \brief Night mode */
    Q_PROPERTY(bool nightMode READ nightMode WRITE setNightMode NOTIFY nightModeChanged)

    /*! \brief Show Altitude AGL */
    Q_PROPERTY(bool showAltitudeAGL READ showAltitudeAGL WRITE setShowAltitudeAGL NOTIFY showAltitudeAGLChanged)

    /*! \brief Use traffic data receiver for positioning */
    Q_PROPERTY(bool positioningByTrafficDataReceiver READ positioningByTrafficDataReceiver WRITE setPositioningByTrafficDataReceiver NOTIFY positioningByTrafficDataReceiverChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property of the same name
     *
     * @returns Property acceptedTerms
     */
    [[nodiscard]] auto acceptedTerms() const -> int { return settings.value(QStringLiteral("acceptedTerms"), 0).toInt(); }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property acceptedWeatherTerms
     */
    [[nodiscard]] auto acceptedWeatherTerms() const -> bool { return settings.value(QStringLiteral("acceptedWeatherTerms"), false).toBool(); }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property airspaceAltitudeLimit
     */
    [[nodiscard]] auto airspaceAltitudeLimit() const -> Units::Distance;

    /*! \brief Getter function for property of the same name
     *
     * @returns Property hideGlidingSectors
     */
    [[nodiscard]] auto hideGlidingSectors() const -> bool { return settings.value(QStringLiteral("Map/hideGlidingSectors"), true).toBool(); }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property hillShading
     */
    [[nodiscard]] auto hillshading() const -> bool { return settings.value(QStringLiteral("Map/hillshading"), true).toBool(); }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property ignoreSSLProblems
     */
    [[nodiscard]] auto ignoreSSLProblems() const -> bool { return settings.value(QStringLiteral("ignoreSSLProblems"), false).toBool(); }

    /*! \brief Getter function for property with the same name
     *
     * @returns Property hasTranslation
     */
    [[nodiscard]] auto lastValidAirspaceAltitudeLimit() const -> Units::Distance;

    /*! \brief Getter function for property of the same name
     *
     * @returns Property lastWhatsNewHash
     */
    [[nodiscard]] auto lastWhatsNewHash() const -> size_t { return settings.value(QStringLiteral("lastWhatsNewHash"), 0).toUInt(); }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property lastWhatsNewInMapsHash
     */
    [[nodiscard]] auto lastWhatsNewInMapsHash() const -> size_t { return settings.value(QStringLiteral("lastWhatsNewInMapsHash"), 0).toUInt(); }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property mapBearingPolicy
     */
    [[nodiscard]] auto mapBearingPolicy() const -> MapBearingPolicy;

    /*! \brief Getter function for property of the same name
     *
     * @returns Property night mode
     */
    [[nodiscard]] auto nightMode() const -> bool { return settings.value(QStringLiteral("Map/nightMode"), false).toBool(); }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property positioningByTrafficDataReceiver
     */
    [[nodiscard]] auto positioningByTrafficDataReceiver() const -> bool { return settings.value(QStringLiteral("positioningByTrafficDataReceiver"), false).toBool(); }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property positioningByTrafficDataReceiver
     */
    [[nodiscard]] auto showAltitudeAGL() const -> bool { return settings.value(QStringLiteral("showAltitudeAGL"), false).toBool(); }


    //
    // Setter Methods
    //

    /*! \brief Setter function for property of the same name
     *
     * @param terms Property acceptedTerms
     */
    void setAcceptedTerms(int terms);

    /*! \brief Setter function for property of the same name
     *
     * @param terms Property acceptedWeatherTerms
     */
    void setAcceptedWeatherTerms(bool terms);

    /*! \brief Setter function for property of the same name
     *
     *  If newAirspaceAltitudeLimit is less than airspaceHeightLimit_min, then
     *  airspaceHeightLimit_min will be set. If newAirspaceAltitudeLimit is higher than
     *  airspaceHeightLimit_max, then airspaceHeightLimit_max will be set.
     *
     * @param newAirspaceAltitudeLimit Property airspaceAltitudeLimit
     */
    void setAirspaceAltitudeLimit(Units::Distance newAirspaceAltitudeLimit);

    /*! \brief Setter function for property of the same name
     *
     * @param hide Property hideGlidingSectors
     */
    void setHideGlidingSectors(bool hide);

    /*! \brief Setter function for property of the same name
     *
     * @param hide Property hillshading
     */
    void setHillshading(bool show);

    /*! \brief Setter function for property of the same name
     *
     * @param ignore Property ignoreSSLProblems
     */
    void setIgnoreSSLProblems(bool ignore);

    /*! \brief Getter function for property of the same name
     *
     * @param lwnh Property lastWhatsNewHash
     */
    void setLastWhatsNewHash(size_t lwnh);

    /*! \brief Getter function for property of the same name
     *
     * @param lwnh Property lastWhatsNewInMapsHash
     */
    void setLastWhatsNewInMapsHash(size_t lwnh);

    /*! \brief Setter function for property of the same name
     *
     * @param policy Property mapBearingPolicy
     */
    void setMapBearingPolicy(MapBearingPolicy policy);

    /*! \brief Setter function for property of the same name
     *
     * @param newNightMode Property nightMode
     */
    void setNightMode(bool newNightMode);

    /*! \brief Setter function for property of the same name
     *
     * @param newPositioningByTrafficDataReceiver Property positioningByTrafficDataReceiver
     */
    void setPositioningByTrafficDataReceiver(bool newPositioningByTrafficDataReceiver);

    /*! \brief Setter function for property of the same name
     *
     * @param newShowAltitudeAGL Property showAltitudeAGL
     */
    void setShowAltitudeAGL(bool newShowAltitudeAGL);


    //
    // Constants
    //

    static constexpr Units::Distance airspaceAltitudeLimit_min = Units::Distance::fromFT(3000);
    static constexpr Units::Distance airspaceAltitudeLimit_max = Units::Distance::fromFT(15000);

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
    void hillshadingChanged();

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

    /*! \brief Notifier signal */
    void positioningByTrafficDataReceiverChanged();

    /*! \brief Notifier signal */
    void showAltitudeAGLChanged();

private:
    Q_DISABLE_COPY_MOVE(Settings)

    QSettings settings;
};

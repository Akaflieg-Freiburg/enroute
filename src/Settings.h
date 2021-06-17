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

#pragma once

#include <QFile>
#include <QLocale>
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QTranslator>


/*! \brief Global Settings Manager
 *
 * This class holds a few data items and exposes them via QObject properties, so
 * that they can be used in QML.  All data stored in this class is saved via
 * QSettings on destruction.
 *
 * There exists one static instance of this class, which can be accessed via the
 * method globalInstance().  No other instance of this class should be used.
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

    /*! \brief Standard deconstructor */
    ~Settings() override;

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
     * It uses the globalInstance() to retrieve data.
     *
     * @returns Property acceptedWeatherTerms
     */
    static bool acceptedWeatherTermsStatic();

    /*! \brief Setter function for property of the same name
     *
     * @param terms Property acceptedWeatherTerms
     */
    void setAcceptedWeatherTerms(bool terms);

    /*! \brief True if translation files exist for the system language */
    Q_PROPERTY(bool hasTranslation READ hasTranslation CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property hasTranslation
     */
    bool hasTranslation() const { return QFile::exists(QStringLiteral(":enroute_%1.qm").arg(QLocale::system().name().left(2))); }

    /*! \brief Hide airspaces with lower bound FL100 or above */
    Q_PROPERTY(bool hideUpperAirspaces READ hideUpperAirspaces WRITE setHideUpperAirspaces NOTIFY hideUpperAirspacesChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property hideUpperAirspaces
     */
    bool hideUpperAirspaces() const { return settings.value(QStringLiteral("Map/hideUpperAirspaces"), false).toBool(); }

    /*! \brief Getter function for property of the same name
     *
     * This function differs from hideUpperAirspaces() only in that it is static.
     * It uses the globalInstance() to retrieve data.
     *
     * @returns Property hideUpperAirspaces
     */
    static bool hideUpperAirspacesStatic();

    /*! \brief Setter function for property of the same name
     *
     * @param hide Property hideUpperAirspaces
     */
    void setHideUpperAirspaces(bool hide);

    /*! \brief Hash of the last "what's new message that was shown to the user
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

    /*! \brief Set to true is app should be shown in English rather than the
     * system language */
    Q_PROPERTY(bool useMetricUnits READ useMetricUnits WRITE setUseMetricUnits NOTIFY useMetricUnitsChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property useMetricUnits
     */
    bool useMetricUnits() const { return settings.value(QStringLiteral("System/useMetricUnits"), false).toBool(); }

    /*! \brief Getter function for property of the same name
     *
     * This function differs from useMetricUnits() only in that it is static.
     * It uses the globalInstance() to retrieve data.
     *
     * @returns Property useMetricUnits
     */
    static bool useMetricUnitsStatic();

    /*! \brief Setter function for property of the same name
     *
     * Setting this property will switch the horizontal speed unit to km/h
     * instead of kt.
     *
     * @param unitHorizKmh Property unitHorizKmh
     */
    void setUseMetricUnits(bool unitHorizKmh);

    /*! \brief Removes/Installs global application translators
     *
     * This method can be used to change the GUI language on the fly.
     * It removes all existing translators and installs new ones.
     *
     * @param localeName Name of the locale (such as "de") or an empty string for the system locale.
     */
    void installTranslators(const QString &localeName={});

signals:
    /*! Notifier signal */
    void acceptedTermsChanged();

    /*! Notifier signal */
    void acceptedWeatherTermsChanged();

    /*! Notifier signal */
    void hideUpperAirspacesChanged();

    /*! Notifier signal */
    void lastWhatsNewHashChanged();

    /*! Notifier signal */
    void mapBearingPolicyChanged();

    /*! Notifier signal */
    void nightModeChanged();

    /*! Notifier signal */
    void useMetricUnitsChanged();

private:
    Q_DISABLE_COPY_MOVE(Settings)

    QPointer<QTranslator> enrouteTranslator {nullptr};

    QSettings settings;
};

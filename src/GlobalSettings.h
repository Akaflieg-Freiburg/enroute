/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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
#include <QObject>
#include <QPointer>
#include <QSettings>
#include <QTranslator>


/*! \brief Global Settings Manager
 *
 * This class holds a few data items and exposes them via QObject properties, so
 * that they can be used in QML.  All data stored in this class is saved via
 * QSettings on destruction.  Only one instance of this class should exist at
 * any given time.
 *
 * The methods in this class are reentrant, but not thread safe.
 */

class GlobalSettings : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit GlobalSettings(QObject *parent = nullptr);

    /*! \brief Standard deconstructor */
    ~GlobalSettings() override;

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
    int acceptedTerms() const { return settings.value("acceptedTerms", 0).toInt(); }

    /*! \brief Setter function for property of the same name
     *
     * @param terms Property acceptedTerms
     */
    void setAcceptedTerms(int terms);

    /*! \brief Find out if Weather Terms have been accepted
     *
     * This property says if the user has agreed to share its location and route
     * with aviationweather.gov (US government website providing weather data).
     * If nothing has been accepted yet, false is eturned.
     */
    Q_PROPERTY(bool acceptedWeatherTerms READ acceptedWeatherTerms WRITE setAcceptedWeatherTerms NOTIFY acceptedWeatherTermsChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property acceptedWeatherTerms
     */
    bool acceptedWeatherTerms() const { return settings.value("acceptedWeatherTerms", false).toBool(); }

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
    bool hasTranslation() const { return QFile::exists(QString(":enroute_%1.qm").arg(QLocale::system().name().left(2))); }

    /*! \brief Hash of the last "what's new message that was shown to the user
     *
     * This property is used in the app to determine if the message has been shown or not.
     */
    Q_PROPERTY(uint lastWhatsNewHash READ lastWhatsNewHash WRITE setLastWhatsNewHash NOTIFY lastWhatsNewHashChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property lastWhatsNewHash
     */
    uint lastWhatsNewHash() const { return settings.value("lastWhatsNewHash", 0).toUInt(); }

    /*! \brief Getter function for property of the same name
     *
     * @param lwnh Property lastWhatsNewHash
     */
    void setLastWhatsNewHash(uint lwnh);

    /*! \brief Hide airspaces with lower bound FL100 or above */
    Q_PROPERTY(bool autoFlightDetection READ autoFlightDetection WRITE setAutoFlightDetection NOTIFY autoFlightDetectionChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property autoFlightDetection
     */
    bool autoFlightDetection() const { return settings.value("Map/autoFlightDetection", true).toBool(); }

    /*! \brief Setter function for property of the same name
     *
     * @param autoDetect Property autoFlightDetection
     */
    void setAutoFlightDetection(bool autoDetect);

    /*! \brief Hide airspaces with lower bound FL100 or above */
    Q_PROPERTY(bool hideUpperAirspaces READ hideUpperAirspaces WRITE setHideUpperAirspaces NOTIFY hideUpperAirspacesChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property hideUpperAirspaces
     */
    bool hideUpperAirspaces() const { return settings.value("Map/hideUpperAirspaces", false).toBool(); }

    /*! \brief Setter function for property of the same name
     *
     * @param hide Property hideUpperAirspaces
     */
    void setHideUpperAirspaces(bool hide);

    /*! \brief Set to true is app should be shown in English rather than the system language */
    Q_PROPERTY(bool useMetricUnits READ useMetricUnits WRITE setUseMetricUnits NOTIFY useMetricUnitsChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property useMetricUnits
     */
    bool useMetricUnits() const { return settings.value("System/useMetricUnits", false).toBool(); }

    /*! \brief Setter function for property of the same name
     *
     * Setting this property will switch the horrizontal speed unit to km/h instead of kt.
     *
     * @param unitHorrizKmh Property unitHorrizKmh
     */
    void setUseMetricUnits(bool unitHorrizKmh);

    /*! \brief Set to true is app should be shown in English rather than the system language */
    Q_PROPERTY(bool preferEnglish READ preferEnglish WRITE setPreferEnglish NOTIFY preferEnglishChanged)

    /*! \brief Getter function for property of the same name
     *
     * @returns Property preferEnglish
     */
    bool preferEnglish() const { return settings.value("System/preferEnglish", false).toBool(); }

    /*! \brief Setter function for property of the same name
     *
     * Setting this property will install/remove system-wide translators.
     *
     * @param preferEng Property preferEng
     */
    void setPreferEnglish(bool preferEng);

signals:
    /*! Notifier signal */
    void acceptedTermsChanged();

    /*! Notifier signal */
    void acceptedWeatherTermsChanged();

    /*! Notifier signal */
    void autoFlightDetectionChanged();

    /*! Notifier signal */
    void hideUpperAirspacesChanged();

    /*! Notifier signal */
    void lastWhatsNewHashChanged();

    /*! Notifier signal */
    void preferEnglishChanged();

    /*! Notifier signal */
    void useMetricUnitsChanged();

private:
    Q_DISABLE_COPY_MOVE(GlobalSettings)

    // Removes/Installs global application translators, according to the settings value "System/preferEnglish"
    void installTranslators();

    QPointer<QTranslator> enrouteTranslator {nullptr};

    QSettings settings;
};

/***************************************************************************
 *   Copyright (C) 2020 by Stefan Kebekus                                  *
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

#include "Meteorologist.h"


/*! \brief This class represents a weather station
 *
 * This is a very simple class that represents a weather station. Weather stations
 * are uniquely identified by their ICAO code. Depending on available data, they
 * hold pointers to the latest METAR and TAF reports.
 */
class Meteorologist::Station : public QObject {
    Q_OBJECT

    friend class Meteorologist;
    friend class Meteorologist::METAR;
    friend class Meteorologist::TAF;
public:
    /*! \brief Standard constructor
     *
     * This standard constructor creates an weather station invalid.
     * Valid weather stations can only be created by instances of the
     * Meteorologist class.
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Station(QObject *parent = nullptr);

    // Standard destructor
    ~Station() = default;

    /*! Geographical coordinate of the station reporting this METAR
     *
     * If the station coordinate is unknown, the property contains an invalid coordinate.
     */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property coordiante
     */
    QGeoCoordinate coordinate() const;

    /*! \brief The station ID
     *
     * The ID of the weather station is usually the ICAO designator of the
     * aerodrome on which the station is located.
     */
    Q_PROPERTY(QString ICAOCode READ ICAOCode CONSTANT)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property id
     */
    QString ICAOCode() const
    {
        return _ICAOCode;
    }

    /*! Indicates if the station is valid */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property isValid
     */
    bool isValid() const
    {
        return (_ICAOCode.length() == 4);
    }

    /*! \brief Last METAR provided by this station
     *
     * This property holds a pointer to the last METAR provided by this station, which can be a nullptr if no data is available.
     * The METAR instance is owned by an instance of Meteorologist, and can be deleted or
     * updated by the Meteorologist anytime.
     */
    Q_PROPERTY(Meteorologist::METAR *metar READ metar NOTIFY metarChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property metar
     */
    Meteorologist::METAR *metar() const
    {
        return _metar;
    }

    /*! \brief Last TAF provided by this station
     *
     * This property holds a pointer to the last TAF provided by this station, which can be a nullptr if no data is available.
     * The TAF instance is owned by an instance of Meteorologist, and can be deleted or
     * updated by the Meteorologist anytime.
     */
    Q_PROPERTY(Meteorologist::TAF *taf READ taf NOTIFY tafChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property taf
     */
    Meteorologist::TAF *taf() const
    {
        return _taf;
    }

signals:
    /* \brief Notifier signal */
    void metarChanged();

    /* \brief Notifier signal */
    void tafChanged();

private:
    Q_DISABLE_COPY_MOVE(Station)

    // This constructor is only meant to be called by instances of the Meteorologist class
    explicit Station(const QString &id, QObject *parent);

    // Sets the METAR message. The signal metarChanged() will be emitted if appropriate.
    void setMETAR(Meteorologist::METAR *metar);

    // Sets the METAR message. The signal tafChanged() will be emitted if appropriate.
    void setTAF(Meteorologist::TAF *taf);

    /*! \brief Converts the time into a human readable string */
    static QString decodeTime(const QVariant &time);

    /*! \brief Converts the wind data into a human readable string */
    static QString decodeWind(const QVariant &windd, const QVariant &winds, const QVariant &windg = QVariant("0"));

    /*! \brief Converts the visibility into a human readable string */
    static QString decodeVis(const QVariant &vis);

    /*! \brief Converts the temperature/dewpoint into a human readable string */
    static QString decodeTemp(const QVariant &temp);

    /*! \brief Converts the QNH (pressure) into a human readable string */
    static QString decodeQnh(const QVariant &altim);

    /*! \brief Converts the weather into a human readable string */
    static QString decodeWx(const QVariant &wx);

    /*! \brief Converts the clouds into a human readable string */
    static QString decodeClouds(const QVariantList &clouds);


    /*! \brief The station ID */
    QString _ICAOCode;

    /*! \brief METAR */
    QPointer<Meteorologist::METAR> _metar;

    /*! \brief TAF */
    QPointer<Meteorologist::TAF> _taf;
};

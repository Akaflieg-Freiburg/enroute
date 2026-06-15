/***************************************************************************
 *   Copyright (C) 2026 by Quentin Bossard                                 *
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

#include <QDateTime>
#include <QList>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPointF>
#include <QQmlEngine>
#include <QVariantList>

#include "weather/Wind.h"

namespace Weather {

/*! \brief Spatially- and altitude-resolved wind field from the local forecast server.
 *
 * Fetches a JSON wind grid from the same server as ForecastMapProvider and
 * exposes interpolated wind at any (lat, lon, altitude). Falls back to invalid
 * wind when the grid is empty or stale, so callers can revert to manual wind.
 *
 * JSON schema (time-indexed):
 *   {
 *     "reference_time": "2026-06-15T12:00:00Z",
 *     "times": ["2026-06-15T12:00:00Z", ...],
 *     "levels_ft": [0, 3300, 6500, 10000],
 *     "grid": [ { "lat":48.0, "lon":7.5,
 *                 "u":[[u_t0_l0,...],...],   // [time][level]
 *                 "v":[[...],...] }, ... ]
 *   }
 * u/v are wind components in m/s (meteorological convention). Lookups
 * interpolate trilinearly: bilinear in lat/lon, linear in altitude and time.
 */
class WindFieldProvider : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit WindFieldProvider(QObject* parent = nullptr);
    static Weather::WindFieldProvider* create(QQmlEngine*, QJSEngine*);

    /*! \brief Application-wide singleton instance, for C++ callers */
    static Weather::WindFieldProvider* instance();

    //
    // Properties
    //

    /*! \brief True when no valid (non-stale) wind field is loaded */
    Q_PROPERTY(bool isStale READ isStale NOTIFY dataChanged)

    /*! \brief True when a non-empty grid has been parsed (regardless of staleness) */
    Q_PROPERTY(bool hasData READ hasData NOTIFY dataChanged)

    /*! \brief Valid-time of the loaded field, e.g. "15 Jun 12:00Z", empty if none */
    Q_PROPERTY(QString validTimeLabel READ validTimeLabel NOTIFY dataChanged)

    /*! \brief Available altitude levels in feet, ascending */
    Q_PROPERTY(QList<int> levelsFt READ levelsFt NOTIFY dataChanged)

    /*! \brief Grid point coordinates as a model for the map wind-barb layer.
     *
     *  Each entry is a map with "lat" and "lon" (degrees). Empty when no data.
     */
    Q_PROPERTY(QVariantList gridPoints READ gridPoints NOTIFY dataChanged)

    //
    // Getters
    //

    [[nodiscard]] bool isStale() const;
    [[nodiscard]] bool hasData() const { return !m_grid.isEmpty(); }
    [[nodiscard]] QString validTimeLabel() const;
    [[nodiscard]] QList<int> levelsFt() const { return m_levelsFt; }
    [[nodiscard]] QVariantList gridPoints() const;

    /*! \brief Returns true when usable (non-empty, non-stale) data is present */
    [[nodiscard]] bool isUsable() const { return hasData() && !isStale(); }

    //
    // Wind lookup
    //

    /*! \brief Interpolated wind at a location, altitude and time.
     *
     *  Bilinear in lat/lon, linear in altitude and time (each clamped to the
     *  available range). Returns an invalid Wind when no usable data.
     */
    [[nodiscard]] Q_INVOKABLE Weather::Wind windAt(double lat, double lon, double altFt, const QDateTime& time) const;

    /*! \brief As windAt(), using the time nearest "now" (for the map layer) */
    [[nodiscard]] Q_INVOKABLE Weather::Wind windAt(double lat, double lon, double altFt) const;

    /*! \brief Interpolated (u, v) wind components in KNOTS at a location/altitude/time.
     *
     *  Meteorological convention: u eastward, v northward. Returns a point with
     *  NaN components when no usable data. Used for vector averaging.
     */
    [[nodiscard]] QPointF uvKnotsAt(double lat, double lon, double altFt, const QDateTime& time) const;

    /*! \brief Construct a Weather::Wind from (u, v) components in knots */
    [[nodiscard]] static Weather::Wind windFromUV(double uKnots, double vKnots);

public slots:
    /*! \brief Fetch wind.json from the configured forecast server. */
    void refresh();

signals:
    void dataChanged();

private:
    struct GridPoint {
        double lat {};
        double lon {};
        QList<QList<double>> u; // m/s, [time][level]
        QList<QList<double>> v; // m/s, [time][level]
    };

    void parse(const QByteArray& bytes);

    // (u, v) in m/s for one grid point, interpolated in time then altitude
    [[nodiscard]] QPointF uvAtCorner(const GridPoint& p, int ti, double tFrac, double altFt) const;

    QNetworkAccessManager m_nam;

    QDateTime        m_referenceTime;
    QList<QDateTime> m_times;        // ascending
    QList<int>       m_levelsFt;     // ascending
    QList<GridPoint> m_grid;

    // Grid geometry (assumes a regular lat/lon grid)
    QList<double> m_lats;           // sorted unique, ascending
    QList<double> m_lons;           // sorted unique, ascending
};

} // namespace Weather

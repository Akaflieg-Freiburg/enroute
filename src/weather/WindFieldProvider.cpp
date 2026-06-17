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

#include "weather/WindFieldProvider.h"
#include "weather/ForecastMapProvider.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QUrl>
#include <QVariantMap>
#include <QtMath>

using namespace Qt::Literals::StringLiterals;

namespace {
constexpr double MPS_TO_KN = 1.94384;

// Lower-bound bracket index in a sorted ascending list: returns i such that
// list[i] <= value <= list[i+1], clamped to [0, n-2]. Returns -1 if list < 2.
// Quantize lat/lon to 1e-3° and pack into a collision-free key
qint64 makeCellKey(double lat, double lon)
{
    const qint64 la = llround(lat * 1000.0);
    const qint64 lo = llround(lon * 1000.0) + 500000; // keep positive, < 1e6
    return la * 1000000LL + lo;
}

int bracket(const QList<double>& sorted, double value)
{
    const int n = sorted.size();
    if (n < 2) {
        return -1;
    }
    if (value <= sorted.first()) {
        return 0;
    }
    if (value >= sorted.last()) {
        return n - 2;
    }
    for (int i = 0; i < n - 1; ++i) {
        if (value >= sorted[i] && value <= sorted[i + 1]) {
            return i;
        }
    }
    return n - 2;
}
} // namespace


// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

Weather::WindFieldProvider::WindFieldProvider(QObject* parent)
    : QObject(parent)
{
    // Load the cached wind field so data is available offline at startup
    QFile f(cacheFilePath());
    if (f.open(QIODevice::ReadOnly)) {
        parse(f.readAll());
    }

    // Re-fetch when the server URL becomes available (e.g. loaded from QSettings after construction)
    connect(ForecastMapProvider::instance(), &ForecastMapProvider::serverUrlChanged,
            this, &WindFieldProvider::refresh);
}

QString Weather::WindFieldProvider::cacheFilePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
           + u"/meteo_france/wind.json"_s;
}

void Weather::WindFieldProvider::saveToCache(const QByteArray& bytes) const
{
    const QString path = cacheFilePath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    // Atomic write: tmp → rename, so a partial write never replaces good data
    const QString tmp = path + u".tmp"_s;
    QFile f(tmp);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(bytes);
        f.close();
        QFile::rename(tmp, path);
    }
}

Weather::WindFieldProvider* Weather::WindFieldProvider::create(QQmlEngine*, QJSEngine*)
{
    return instance();
}

Weather::WindFieldProvider* Weather::WindFieldProvider::instance()
{
    static WindFieldProvider theInstance;
    return &theInstance;
}


// ---------------------------------------------------------------------------
// Getters
// ---------------------------------------------------------------------------

bool Weather::WindFieldProvider::isStale() const
{
    if (m_grid.isEmpty() || m_times.isEmpty()) {
        return true;
    }
    // Stale once "now" is past the last forecast step (no more coverage)
    return QDateTime::currentDateTimeUtc() > m_times.last();
}

QString Weather::WindFieldProvider::validTimeLabel() const
{
    if (m_times.isEmpty()) {
        return {};
    }
    return m_times.first().toUTC().toString(u"dd MMM HH:mm'Z'"_s);
}

qint64 Weather::WindFieldProvider::cellKey(double lat, double lon)
{
    return makeCellKey(lat, lon);
}

QVariantList Weather::WindFieldProvider::gridPoints() const
{
    QVariantList out;
    out.reserve(m_grid.size());
    for (const auto& p : m_grid) {
        out << QVariantMap{{u"lat"_s, p.lat}, {u"lon"_s, p.lon}};
    }
    return out;
}


// ---------------------------------------------------------------------------
// Refresh
// ---------------------------------------------------------------------------

void Weather::WindFieldProvider::refresh()
{
    const QString serverUrl = ForecastMapProvider::instance()->serverUrl();
    if (serverUrl.isEmpty()) {
        return;
    }

    const QUrl base(serverUrl);
    if (base.isLocalFile()) {
        QFile f(base.toLocalFile() + u"/wind.json"_s);
        if (f.open(QIODevice::ReadOnly)) {
            parse(f.readAll());
        }
        return;
    }

    auto* reply = m_nam.get(QNetworkRequest(QUrl(serverUrl + u"/wind.json"_s)));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            const QByteArray bytes = reply->readAll();
            saveToCache(bytes);
            parse(bytes);
        }
    });
}


// ---------------------------------------------------------------------------
// Parsing
// ---------------------------------------------------------------------------

void Weather::WindFieldProvider::parse(const QByteArray& bytes)
{
    const auto doc = QJsonDocument::fromJson(bytes);
    if (!doc.isObject()) {
        return;
    }
    const auto root = doc.object();

    const QDateTime referenceTime = QDateTime::fromString(root[u"reference_time"_s].toString(), Qt::ISODate);

    QList<QDateTime> times;
    for (const auto& v : root[u"times"_s].toArray()) {
        times << QDateTime::fromString(v.toString(), Qt::ISODate).toUTC();
    }

    QList<int> levels;
    for (const auto& v : root[u"levels_ft"_s].toArray()) {
        levels << v.toInt();
    }

    auto readSeries = [](const QJsonArray& outer) {
        QList<QList<double>> series;
        for (const auto& step : outer) {
            QList<double> perLevel;
            for (const auto& x : step.toArray()) {
                perLevel << x.toDouble();
            }
            series << perLevel;
        }
        return series;
    };

    QList<GridPoint> grid;
    QList<double> lats;
    QList<double> lons;
    for (const auto& v : root[u"grid"_s].toArray()) {
        const auto o = v.toObject();
        GridPoint p;
        p.lat = o[u"lat"_s].toDouble();
        p.lon = o[u"lon"_s].toDouble();
        p.u = readSeries(o[u"u"_s].toArray());
        p.v = readSeries(o[u"v"_s].toArray());
        grid << p;
        if (!lats.contains(p.lat)) {
            lats << p.lat;
        }
        if (!lons.contains(p.lon)) {
            lons << p.lon;
        }
    }

    if (grid.isEmpty() || levels.isEmpty() || times.isEmpty()) {
        return;
    }

    std::sort(lats.begin(), lats.end());
    std::sort(lons.begin(), lons.end());

    m_referenceTime = referenceTime;
    m_times         = times;
    m_levelsFt      = levels;
    m_grid          = grid;
    m_lats          = lats;
    m_lons          = lons;

    // Build the O(1) cell index for corner lookups
    m_index.clear();
    m_index.reserve(m_grid.size());
    for (int i = 0; i < m_grid.size(); ++i) {
        m_index.insert(makeCellKey(m_grid[i].lat, m_grid[i].lon), i);
    }

    emit dataChanged();
}


// ---------------------------------------------------------------------------
// Interpolation
// ---------------------------------------------------------------------------

QPointF Weather::WindFieldProvider::uvAtCorner(const GridPoint& p, int ti, double tFrac, double altFt) const
{
    const int nLev = m_levelsFt.size();
    if (nLev == 0 || ti < 0 || ti >= p.u.size()) {
        return {qQNaN(), qQNaN()};
    }

    // Time-interpolate the per-level arrays into a single profile (m/s)
    const int tj = qMin(ti + 1, int(p.u.size()) - 1);
    QList<double> uLev;
    QList<double> vLev;
    uLev.reserve(nLev);
    vLev.reserve(nLev);
    for (int l = 0; l < nLev; ++l) {
        uLev << p.u[ti].value(l) * (1 - tFrac) + p.u[tj].value(l) * tFrac;
        vLev << p.v[ti].value(l) * (1 - tFrac) + p.v[tj].value(l) * tFrac;
    }

    if (nLev == 1) {
        return {uLev[0], vLev[0]};
    }

    // Altitude-interpolate the profile
    QList<double> levelsD;
    levelsD.reserve(nLev);
    for (int lvl : m_levelsFt) {
        levelsD << double(lvl);
    }
    const int i = bracket(levelsD, altFt);
    if (i < 0) {
        return {uLev[0], vLev[0]};
    }
    const double a0 = levelsD[i];
    const double a1 = levelsD[i + 1];
    const double t  = (a1 > a0) ? qBound(0.0, (altFt - a0) / (a1 - a0), 1.0) : 0.0;
    return {uLev[i] * (1 - t) + uLev[i + 1] * t,
            vLev[i] * (1 - t) + vLev[i + 1] * t};
}

QPointF Weather::WindFieldProvider::uvKnotsAt(double lat, double lon, double altFt, const QDateTime& time) const
{
    if (!isUsable()) {
        return {qQNaN(), qQNaN()};
    }

    // Bracket time (m_times ascending). Clamp outside the range.
    int ti = 0;
    double tFrac = 0.0;
    const int nT = m_times.size();
    if (nT >= 2 && time.isValid()) {
        if (time <= m_times.first()) {
            ti = 0;
            tFrac = 0.0;
        } else if (time >= m_times.last()) {
            ti = nT - 1;
            tFrac = 0.0;
        } else {
            for (int k = 0; k < nT - 1; ++k) {
                if (time >= m_times[k] && time <= m_times[k + 1]) {
                    ti = k;
                    const qint64 span = m_times[k].secsTo(m_times[k + 1]);
                    tFrac = (span > 0) ? double(m_times[k].secsTo(time)) / double(span) : 0.0;
                    break;
                }
            }
        }
    }

    const int iLat = bracket(m_lats, lat);
    const int iLon = bracket(m_lons, lon);
    if (iLat < 0 || iLon < 0) {
        return {qQNaN(), qQNaN()};
    }

    const double lat0 = m_lats[iLat];
    const double lat1 = m_lats[iLat + 1];
    const double lon0 = m_lons[iLon];
    const double lon1 = m_lons[iLon + 1];

    auto corner = [this](double la, double lo) -> const GridPoint* {
        const int idx = m_index.value(makeCellKey(la, lo), -1);
        return (idx >= 0) ? &m_grid[idx] : nullptr;
    };
    const GridPoint* c00 = corner(lat0, lon0);
    const GridPoint* c01 = corner(lat0, lon1);
    const GridPoint* c10 = corner(lat1, lon0);
    const GridPoint* c11 = corner(lat1, lon1);
    if ((c00 == nullptr) || (c01 == nullptr) || (c10 == nullptr) || (c11 == nullptr)) {
        return {qQNaN(), qQNaN()};
    }

    const double tx = (lon1 > lon0) ? qBound(0.0, (lon - lon0) / (lon1 - lon0), 1.0) : 0.0;
    const double ty = (lat1 > lat0) ? qBound(0.0, (lat - lat0) / (lat1 - lat0), 1.0) : 0.0;

    const QPointF uv00 = uvAtCorner(*c00, ti, tFrac, altFt);
    const QPointF uv01 = uvAtCorner(*c01, ti, tFrac, altFt);
    const QPointF uv10 = uvAtCorner(*c10, ti, tFrac, altFt);
    const QPointF uv11 = uvAtCorner(*c11, ti, tFrac, altFt);

    // Bilinear blend (m/s)
    const QPointF top = uv00 * (1 - tx) + uv01 * tx;
    const QPointF bot = uv10 * (1 - tx) + uv11 * tx;
    const QPointF uv  = top * (1 - ty) + bot * ty;

    return {uv.x() * MPS_TO_KN, uv.y() * MPS_TO_KN};
}

Weather::Wind Weather::WindFieldProvider::windFromUV(double uKnots, double vKnots)
{
    Weather::Wind w;
    if (!qIsFinite(uKnots) || !qIsFinite(vKnots)) {
        return w;
    }

    const double speedKn = std::hypot(uKnots, vKnots);
    // Meteorological "direction from": atan2(-u, -v), 0° = from north
    double dirDeg = qRadiansToDegrees(std::atan2(-uKnots, -vKnots));
    if (dirDeg < 0) {
        dirDeg += 360.0;
    }

    w.setSpeed(Units::Speed::fromKN(speedKn));
    w.setDirectionFrom(Units::Angle::fromDEG(dirDeg));
    return w;
}

Weather::Wind Weather::WindFieldProvider::windAt(double lat, double lon, double altFt, const QDateTime& time) const
{
    const QPointF uv = uvKnotsAt(lat, lon, altFt, time);
    return windFromUV(uv.x(), uv.y());
}

Weather::Wind Weather::WindFieldProvider::windAt(double lat, double lon, double altFt) const
{
    return windAt(lat, lon, altFt, QDateTime::currentDateTimeUtc());
}

bool Weather::WindFieldProvider::loadFromJson(const QByteArray& bytes)
{
    parse(bytes);
    return hasData();
}

QDateTime Weather::WindFieldProvider::nearestStep(const QDateTime& time) const
{
    if (m_times.isEmpty()) {
        return {};
    }
    if (!time.isValid()) {
        return m_times.first();
    }
    const QDateTime* best = &m_times.first();
    qint64 bestDiff = qAbs(m_times.first().secsTo(time));
    for (const auto& t : m_times) {
        const qint64 d = qAbs(t.secsTo(time));
        if (d < bestDiff) {
            bestDiff = d;
            best = &t;
        }
    }
    return *best;
}

Weather::Wind Weather::WindFieldProvider::windAtStep(double lat, double lon, double altFt, const QDateTime& time) const
{
    // Snap to the nearest forecast step: no time interpolation
    return windAt(lat, lon, altFt, nearestStep(time));
}

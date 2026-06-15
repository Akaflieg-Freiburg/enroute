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

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrl>
#include <QtMath>

using namespace Qt::Literals::StringLiterals;

namespace {
constexpr double MPS_TO_KN = 1.94384;

// Lower-bound bracket index in a sorted ascending list: returns i such that
// list[i] <= value <= list[i+1], clamped to [0, n-2]. Returns -1 if list < 2.
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
    if (m_grid.isEmpty() || !m_validTime.isValid()) {
        return true;
    }
    return m_validTime.secsTo(QDateTime::currentDateTimeUtc()) > staleSeconds;
}

QString Weather::WindFieldProvider::validTimeLabel() const
{
    if (!m_validTime.isValid()) {
        return {};
    }
    return m_validTime.toUTC().toString(u"dd MMM HH:mm'Z'"_s);
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
            parse(reply->readAll());
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

    QDateTime validTime = QDateTime::fromString(root[u"valid_time"_s].toString(), Qt::ISODate);

    QList<int> levels;
    for (const auto& v : root[u"levels_ft"_s].toArray()) {
        levels << v.toInt();
    }

    QList<GridPoint> grid;
    QList<double> lats;
    QList<double> lons;
    for (const auto& v : root[u"grid"_s].toArray()) {
        const auto o = v.toObject();
        GridPoint p;
        p.lat = o[u"lat"_s].toDouble();
        p.lon = o[u"lon"_s].toDouble();
        for (const auto& uu : o[u"u"_s].toArray()) {
            p.u << uu.toDouble();
        }
        for (const auto& vv : o[u"v"_s].toArray()) {
            p.v << vv.toDouble();
        }
        grid << p;
        if (!lats.contains(p.lat)) {
            lats << p.lat;
        }
        if (!lons.contains(p.lon)) {
            lons << p.lon;
        }
    }

    if (grid.isEmpty() || levels.isEmpty()) {
        return;
    }

    std::sort(lats.begin(), lats.end());
    std::sort(lons.begin(), lons.end());

    m_validTime = validTime;
    m_levelsFt  = levels;
    m_grid      = grid;
    m_lats      = lats;
    m_lons      = lons;

    emit dataChanged();
}


// ---------------------------------------------------------------------------
// Interpolation
// ---------------------------------------------------------------------------

QPointF Weather::WindFieldProvider::uvAtLevelInterp(const GridPoint& p, double altFt) const
{
    const int nLev = m_levelsFt.size();
    if (nLev == 0 || p.u.isEmpty()) {
        return {qQNaN(), qQNaN()};
    }
    if (nLev == 1) {
        return {p.u.value(0), p.v.value(0)};
    }

    // Bracket altitude between two levels (levels are ascending)
    QList<double> levelsD;
    levelsD.reserve(nLev);
    for (int lvl : m_levelsFt) {
        levelsD << double(lvl);
    }
    const int i = bracket(levelsD, altFt);
    if (i < 0) {
        return {p.u.value(0), p.v.value(0)};
    }

    const double a0 = levelsD[i];
    const double a1 = levelsD[i + 1];
    const double t  = (a1 > a0) ? qBound(0.0, (altFt - a0) / (a1 - a0), 1.0) : 0.0;

    const double u = p.u.value(i) * (1 - t) + p.u.value(i + 1) * t;
    const double v = p.v.value(i) * (1 - t) + p.v.value(i + 1) * t;
    return {u, v};
}

QPointF Weather::WindFieldProvider::uvKnotsAt(double lat, double lon, double altFt) const
{
    if (!isUsable()) {
        return {qQNaN(), qQNaN()};
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

    // Look up the 4 corner grid points
    auto corner = [this](double la, double lo) -> const GridPoint* {
        for (const auto& p : m_grid) {
            if (qFuzzyCompare(p.lat, la) && qFuzzyCompare(p.lon, lo)) {
                return &p;
            }
        }
        return nullptr;
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

    const QPointF uv00 = uvAtLevelInterp(*c00, altFt);
    const QPointF uv01 = uvAtLevelInterp(*c01, altFt);
    const QPointF uv10 = uvAtLevelInterp(*c10, altFt);
    const QPointF uv11 = uvAtLevelInterp(*c11, altFt);

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

Weather::Wind Weather::WindFieldProvider::windAt(double lat, double lon, double altFt) const
{
    const QPointF uv = uvKnotsAt(lat, lon, altFt);
    return windFromUV(uv.x(), uv.y());
}

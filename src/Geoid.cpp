/*!
 * Copyright (C) 2020 by Johannes Zellner, johannes@zellner.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "Geoid.h"
#include <QIODevice>
#include <QDataStream>
#include <QDebug>
#include <QFile>

#include <QtMath>
#include <cmath>

Geoid::Geoid() : egm(nullptr)
{
    QFile file(":/WW15MGH.DAC");

    if (!file.open(QIODevice::ReadOnly) || file.size() != (egm96_size * 2)) {
        qDebug() << "Geoid::Geoid failed to open WW15MGH.DAC";
        return;
    }

    QDataStream data(&file);
    data.setByteOrder(QDataStream::BigEndian);

    egm = new qint16[egm96_size];

    for (int nread = 0; !data.atEnd() && nread < egm96_size; nread++) {
        data >> egm[nread];
    }

    file.close();
}

Geoid::~Geoid()
{
    if (valid())
        delete egm;
}

// 90 >= latitude >= -90
// 0 <= longitude < 360
//
// we do a simple bilinear interpolation between the four surrounding data points
// according to Numerical Recipies in C++ 3.6 "Interpolation in Tow or More Dimensions".
//
qreal Geoid::operator()(qreal latitude, qreal longitude)
{
    if (!valid())
        return 0;

    auto real_row = []  (qreal lat) -> qreal { return (90 - lat) * 4; };        // [0; 720] from north to south
    auto real_col = []  (qreal lon) -> qreal { return fmod(lon, 360) * 4; };    // [0; 1440[
    auto row      = [&] (qreal lat) -> int   { return qFloor(real_row(lat)); }; // [0; 720]
    auto col      = [&] (qreal lon) -> int   { return qFloor(real_col(lon)); }; // [0; 1440[

    auto geoid = [&] (qreal lat, qreal lon) -> qreal
    {
        qint32 idx = row(lat) * egm96_cols + col(lon);
        return idx >=0 && idx < egm96_size ? egm[idx] * 0.01 : 0.0;
    };

    qreal interpolated = 0;
    for (qint16 ilat : {0, 1})
    {
        qreal lat_dist = qAbs(1 - ilat - (real_row(latitude) - row(latitude))); // 1 - (distance to row)
        for (qint16 ilon : {0, 1})
        {
            qreal lon_dist = qAbs(1 - ilon - (real_col(longitude) - col(longitude))); // 1 - (distance to coloumn)
            interpolated += geoid(latitude + ilat * 0.25, longitude + ilon * 0.25) * lat_dist * lon_dist;
            // qDebug() << latitude << ", " << longitude << ", " << lat_dist << ", " << lon_dist;
        }
    }

    // qDebug() << "Geoid (" << latitude << ", " << longitude << ") = " << interpolated;
    return interpolated;
}

bool Geoid::valid() const
{
    return (egm != nullptr);
}

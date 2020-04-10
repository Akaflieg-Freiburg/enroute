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
#include <QSysInfo>
#include <QtEndian>

// reading binary geoid data was carefully optimized for speed. We read
// the binary content at once and do the byte order conversion afterwards.
// This turned out to be up to 60x faster compared to using QDataStream
// with setByteOrder(QDataStream::BigEndian) and reading the shorts one
// after the other with the QDataStream >> operator.
//
Geoid::Geoid() : egm(nullptr)
{
    QFile file(":/WW15MGH.DAC");

    int egm96_size_2 = egm96_size * 2;

    if (!file.open(QIODevice::ReadOnly) || file.size() != (egm96_size_2)) {
        qDebug() << "Geoid::Geoid failed to open WW15MGH.DAC";
        return;
    }

    egm = new qint16[egm96_size];

    int nread = file.read(reinterpret_cast<char*>(egm), egm96_size_2);
    file.close();

    if (nread != (egm96_size_2)) {
        qDebug() << "Geoid::Geoid expected to read " << egm96_size_2 << " bytes from WW15MGH.DAC but got " << nread;
        delete[] egm;
        egm = nullptr;
        return;
    }

    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian)
    {
        qFromBigEndian<qint16>(egm, egm96_size, egm);
    }
}

Geoid::~Geoid()
{
    if (valid())
        delete[] egm;
}

// 90 >= latitude >= -90
//
// we do a simple bilinear interpolation between the four surrounding data points
// according to Numerical Recipies in C++ 3.6 "Interpolation in Two or More Dimensions".
//
qreal Geoid::operator()(qreal latitude, qreal longitude)
{
    if (!valid())
        return 0;

    while (longitude < 0)
        longitude += 360.;

    // coordinate transformation from lat/lon to the data file coordinate system.
    // The returning row and col are still reals (_not_ data index integers).
    // We do not care about cyclic overflows yet, col might > 1400.
    //
    auto row = []  (qreal lat) -> qreal { return (90 - lat) * 4; }; // [0; 720] from north to south
    auto col = []  (qreal lon) -> qreal { return    lon     * 4; }; // [0; 1440[

    // integer row north and south of latitude
    //
    int north = qFloor(row(latitude));
    int south = (north + 1) < egm96_rows? (north + 1) : north;

    // integer column west and east of latitude
    //
    int west = qFloor(col(longitude)) % egm96_cols;
    int east = (west + 1) % egm96_cols;

    auto geoid = [&] (int row, int col) -> qreal
    {
        int idx = row * egm96_cols + col;
        return idx >=0 && idx < egm96_size ? egm[idx] * 0.01 : 0.0;
    };

    // here we do a bilinear interpolation between the 4 neighbouring
    // data points of the requested location.
    //
    qreal interpolated = 0;
    qreal row_dist = row(latitude) - north;
    qreal col_dist = col(longitude) - qFloor(col(longitude));
    for (int irow : {north, south})
    {
        for (int icol : {west, east})
        {
            interpolated += geoid(irow, icol) * (1 - row_dist) * (1 - col_dist);
            col_dist = 1 - col_dist;
        }
        row_dist = 1 - row_dist;
    }

    return interpolated;
}

bool Geoid::valid() const
{
    return (egm != nullptr);
}

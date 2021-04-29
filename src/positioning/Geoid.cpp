/*!
 * Copyright (C) 2020 by Johannes Zellner, johannes@zellner.org
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <QDebug>
#include <QFile>
#include <QtEndian>
#include <QtMath>

#include "positioning/Geoid.h"

// Static member variable

QVector<qint16> Positioning::Geoid::egm {};


// reading binary geoid data was carefully optimized for speed. We read the
// binary content at once and do the byte order conversion afterwards.  This
// turned out to be up to 60x faster compared to using QDataStream with
// setByteOrder(QDataStream::BigEndian) and reading the shorts one after the
// other with the QDataStream >> operator.


void Positioning::Geoid::readEGM()
{
    QFile file(QStringLiteral(":/WW15MGH.DAC"));

    qint64 egm96_size_2 = egm96_size * 2;

    if (!file.open(QIODevice::ReadOnly) || file.size() != (egm96_size_2))
    {
        qDebug() << "Geoid::Geoid failed to open " << file.fileName();
        return;
    }

    egm.resize(egm96_size);

    qint64 nread = file.read(static_cast<char*>(static_cast<void*>(egm.data())), egm96_size_2);

    file.close();

    if (nread != egm96_size_2)
    {
        qDebug() << "Geoid::Geoid expected " << egm96_size_2
                 << " bytes from " << file.fileName() << " but got " << nread;
        egm.clear();
        return;
    }

    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        qFromBigEndian<qint16>(egm.data(), egm96_size, egm.data());
    }

}

// 90 >= latitude >= -90
//
// we do a simple bilinear interpolation between the four surrounding data
// points according to Numerical Recipies in C++ 3.6 "Interpolation in Two or
// More Dimensions".
//
auto Positioning::Geoid::separation(const QGeoCoordinate& coord) -> AviationUnits::Distance
{
    // Paranoid safety checks
    if (!coord.isValid()) {
        return AviationUnits::Distance::fromM( qQNaN() );
    }

    // Read EGM vector if this has not been done already
    if (egm.empty()) {
        readEGM();
        if (egm.empty()) {
            return AviationUnits::Distance::fromM( qQNaN() );
        }
    }

    // Get lat/long
    auto latitude = coord.latitude();
    auto longitude = coord.longitude();
    while (longitude < 0) {
        longitude += 360.;
    }

    // coordinate transformation from lat/lon to the data file coordinate
    // system.  The returning row and col are still reals (_not_ data index
    // integers).  We do not care about cyclic overflows yet, col might > 1400.
    //
    auto row = []  (double lat) -> double { return (90 - lat) * 4; }; // [0; 720] from north to south
    auto col = []  (double lon) -> double { return    lon     * 4; }; // [0; 1440[

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
        return idx >=0 && idx < egm96_size ? egm.at(idx) * 0.01 : 0.0;
    };

    // here we do a bilinear interpolation between the 4 neighbouring data
    // points of the requested location.
    //
    double interpolated = 0;
    double row_dist = row(latitude) - north;
    double col_dist = col(longitude) - qFloor(col(longitude));
    for (int irow : {north, south})
    {
        for (int icol : {west, east})
        {
            interpolated += geoid(irow, icol) * (1 - row_dist) * (1 - col_dist);
            col_dist = 1 - col_dist;
        }
        row_dist = 1 - row_dist;
    }

    return AviationUnits::Distance::fromM( interpolated );
}

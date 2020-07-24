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

#include <QObject>

#warning Docu

class WeatherReport : public QObject {
    Q_OBJECT

public:
#warning Docu
    explicit WeatherReport(const QMultiMap<QString, QVariant> &metar, const QMultiMap<QString, QVariant> &taf, QObject *parent = nullptr);

    // Standard destructor
    ~WeatherReport() = default;

    Q_PROPERTY(QString id READ id CONSTANT)
    QString id() const { return _id; }

    Q_PROPERTY(QString cat READ cat CONSTANT)
    QString cat() const { return _cat; }

    Q_PROPERTY(QList<QString> metar READ metar CONSTANT)
    QList<QString> metar() const { return _metar; }

    Q_PROPERTY(QList<QString> taf READ taf CONSTANT)
    QList<QString> taf() const { return _taf; }

private:
    Q_DISABLE_COPY_MOVE(WeatherReport)

    QString _id;
    QString _cat;
    QList<QString> _metar;
    QList<QString> _taf;

    QString decodeTime(const QVariant &time);
    QString decodeWind(const QVariant &windd, const QVariant &winds);
    QString decodeVis(const QVariant &vis);
    QString decodeTemp(const QVariant &temp);
    QString decodeQnh(const QVariant &altim);
    QString decodeWx(const QVariant &wx);
    QString decodeClouds(const QVariantList &clouds);
};

/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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

#include <QDateTime>
#include <QGeoCoordinate>
#include <QGeoCircle>
#include <QQmlEngine>

#include "geomaps/Waypoint.h"
#include "notam/Notam.h"

namespace NOTAM {


class NotamList {
    Q_GADGET
    QML_VALUE_TYPE(notamList)

public:
    NotamList();
    NotamList(const QByteArray& jsonData, const QGeoCircle& region);


    Q_PROPERTY(QList<NOTAM::Notam> notams MEMBER m_notams)
    Q_PROPERTY(QString summary READ summary)
    Q_PROPERTY(QString text READ text)

    bool covers(const GeoMaps::Waypoint& waypoint);
    NotamList restrict(const GeoMaps::Waypoint& waypoint);


    QString summary();
    QString text();

    QList<NOTAM::Notam> m_notams;
    QGeoCircle m_region;
    QDateTime m_retrieved;
};

} // namespace NOTAM

QDataStream& operator<<(QDataStream& stream, const NOTAM::NotamList &notam);
QDataStream& operator>>(QDataStream& stream, NOTAM::NotamList& notam);

// Declare meta types
Q_DECLARE_METATYPE(NOTAM::NotamList)

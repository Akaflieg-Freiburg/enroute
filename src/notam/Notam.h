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
#include <QQmlEngine>

namespace NOTAM {

/*! \brief This extremely simple class holds a the data item of a NOTAM */

class Notam {
    Q_GADGET
    QML_VALUE_TYPE(notam)

public:
    //
    // Properties
    //

    Q_PROPERTY(QDateTime start MEMBER m_start)
    Q_PROPERTY(QDateTime expiration MEMBER m_expiration)
    Q_PROPERTY(QString text MEMBER m_text)
    Q_PROPERTY(QGeoCoordinate location MEMBER m_location)

    Q_INVOKABLE [[nodiscard]] bool operator==(const NOTAM::Notam& rhs) const = default;

    QDateTime m_start {{2023, 2, 28}, {11, 0}};
    QDateTime m_expiration {{2023, 3, 28}, {11, 0}};
    QString m_text {u"All runways closed"_qs};
    QGeoCoordinate m_location {48.022778, 7.832500};
};

} // namespace NOTAM

// Declare meta types
Q_DECLARE_METATYPE(NOTAM::Notam)

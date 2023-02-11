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
    void read(const QJsonObject& jsonObject);
    QString richText() const;

    Q_PROPERTY(QDateTime effectiveStart MEMBER m_effectiveStart)
    Q_PROPERTY(QDateTime effectiveEnd MEMBER m_effectiveEnd)
    Q_PROPERTY(QString text MEMBER m_text)
    Q_PROPERTY(QString richText READ richText)
    Q_PROPERTY(QString icaoLocation MEMBER m_icaoLocation)
    Q_PROPERTY(QGeoCoordinate coordinates MEMBER m_coordinates)

    Q_INVOKABLE [[nodiscard]] bool operator==(const NOTAM::Notam& rhs) const = default;

    QDateTime m_effectiveStart;
    QDateTime m_effectiveEnd;
    QString m_text;
    QString m_icaoLocation;
    QGeoCoordinate m_coordinates;
};

} // namespace NOTAM

// Declare meta types
Q_DECLARE_METATYPE(NOTAM::Notam)

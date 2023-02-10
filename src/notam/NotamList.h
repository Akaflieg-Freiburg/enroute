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

#include "notam/Notam.h"

namespace NOTAM {

/*! \brief This extremely simple class holds a the data item of a NOTAM */

class NotamList {
    Q_GADGET
    QML_VALUE_TYPE(notamList)

public:
    //
    // Properties
    //

    Q_PROPERTY(bool complete MEMBER m_complete)
    Q_PROPERTY(QList<NOTAM::Notam> notams MEMBER m_notams)
    Q_PROPERTY(QString summary READ summary)
    Q_PROPERTY(QString text READ text)

    static QString summary() ;
    static QString text() ;

    bool m_complete {false};
    QList<NOTAM::Notam> m_notams;
};

} // namespace NOTAM

// Declare meta types
Q_DECLARE_METATYPE(NOTAM::NotamList)

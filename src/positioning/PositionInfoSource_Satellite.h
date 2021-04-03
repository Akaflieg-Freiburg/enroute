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

#include <QGeoPositionInfoSource>
#include <QPointer>

#include "positioning/Geoid.h"
#include "positioning/PositionInfoSource_Abstract.h"


namespace Positioning {

/*! \brief Satellite Navigator
 *
 *  This class is a thin wrapper around QGeoPositionInfoSource. It constructs a
 *  default QGeoPositionInfoSource and forwards the data provided by that source
 *  via the PositionInfoSource_Abstract interface that it implements.
 */

class PositionInfoSource_Satellite : public PositionInfoSource_Abstract
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit PositionInfoSource_Satellite(QObject *parent = nullptr);

private slots:
    void onPositionUpdated(const QGeoPositionInfo &info);

    void updateStatusString();

private:
    Q_DISABLE_COPY_MOVE(PositionInfoSource_Satellite)

    Positioning::Geoid geoid;

    QPointer<QGeoPositionInfoSource> source {nullptr};
};

}

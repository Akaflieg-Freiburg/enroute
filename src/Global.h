/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

class MobileAdaptor;

namespace GeoMaps {
class MapManager;
};


/*! \brief Global Objects Storage
 *
 * This class holds objects that are defined application-globally.
 *
 * The methods in this class are reentrant, but not thread safe.
 */

class Global : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Global(QObject *parent = nullptr);

    /*! \brief Standard deconstructor */
#warning docu
    ~Global() override;

    /*! \brief Pointer to appplication-wide static GeoMaps::MapManager object
     *
     *  This property holds a pointer to an application-wide static object.
     *  The pointer is guaranteed to be valid.
     *  The object is owned by this class and must not be deleted.
     *  The property will never change, unless an instance of Global gets destructed.
     */
    Q_INVOKABLE static GeoMaps::MapManager* mapManager();

    Q_INVOKABLE static MobileAdaptor* mobileAdaptor();

private:
    Q_DISABLE_COPY_MOVE(Global)
};

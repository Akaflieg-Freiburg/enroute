/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

#include "dataManagement/DataManager.h"


namespace DataManagement {

/*! \brief Informs the user that updates of geographic maps are peding
 *
 *  This class informs the user by notification if updates of geographic maps are peding.
 *  The implementation tries to ensure that update notifications never appear in flight, and
 *  only when an active internet connection exists.
 */

class UpdateNotifier : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     *  @param parent Pointer to the parent object, which must be a DataManager.
     */
    explicit UpdateNotifier(DataManager* parent);

private:
    // Notify if map updates pending, else close the notification
    void updateNotification();

    Q_DISABLE_COPY_MOVE(UpdateNotifier)
};

};

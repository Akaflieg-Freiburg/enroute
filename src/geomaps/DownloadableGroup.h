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

#include "DownloadableGroupWatcher.h"

namespace GeoMaps {

/*! \brief Manages a set of downloadable objects

  This class inherits from DownloadableGroupWatcher, but has a public
  constructor and two methods (addToGroup, removeFromGroup) that allow to
  manipulate the group.
 */

class DownloadableGroup : public DownloadableGroupWatcher
{
    Q_OBJECT

public:
    /*! \brief Constructs an empty group
      
      @param parent The standard QObject parent pointer.
    */
    explicit DownloadableGroup(QObject *parent=nullptr);

    /*! \brief Adds a Downloadable to the group

      This method adds a Downloadable object to the group.

      The DownloadableGroup does not take ownership of the Downloadable, and it
      is safe to delete the Downloadable after it has been added, without
      removing it first. It is perfectly fine for Downloadable objects to be
      members of several groups.

      @param downloadable Pointer to the Downloadable to be added.
    */
    void addToGroup(Downloadable *downloadable);

    /*! \brief Remove a downloadable from the group

      This method removes a Downloadable from the group.

      @param downloadable Pointer to the Downloadable to be removed.
     */
    void removeFromGroup(Downloadable *downloadable);

private:
    Q_DISABLE_COPY_MOVE(DownloadableGroup)
};

};

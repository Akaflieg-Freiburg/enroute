/***************************************************************************
 *   Copyright (C) 2022 by Stefan Kebekus                                  *
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

#include "MapSet.h"


DataManagement::MapSet::MapSet(QObject *parent)
    : QObject(parent) {

}

auto DataManagement::MapSet::downloading() const -> bool
{
    return baseMap->downloading() || terrainMap->downloading();
}


auto DataManagement::MapSet::updatable() const -> bool
{
    if (baseMap->updatable() || terrainMap->updatable())
    {
        return true;
    }
    if (baseMap->hasFile() || terrainMap->hasFile())
    {
        if (!baseMap->hasFile())
        {
            return true;
        }
        if (!terrainMap->hasFile())
        {
            return true;
        }
    }

    return false;
}

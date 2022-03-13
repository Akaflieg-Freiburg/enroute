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

#include "RemainingRouteInfo.h"


//
// Constructors and destructors
//

Navigation::RemainingRouteInfo::RemainingRouteInfo()
{
}

bool Navigation::operator==(const Navigation::RemainingRouteInfo& A, const Navigation::RemainingRouteInfo& B)
{
    return ((A.nextWP == B.nextWP) &&
            (A.nextWP_DIST == B.nextWP_DIST) &&
            (A.nextWP_ETE == B.nextWP_ETE) &&
            (A.nextWP_ETA == B.nextWP_ETA) &&

            (A.finalWP == B.finalWP) &&
            (A.finalWP_DIST == B.finalWP_DIST) &&
            (A.finalWP_ETE == B.finalWP_ETE) &&
            (A.finalWP_ETA == B.finalWP_ETA));
}

//
// Getter Methods
//

// ...


//
// Methods
//

// ...


/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus  *
 *   stefan.kebekus@gmail.com  *
 * *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or *
 *   (at your option) any later version.   *
 * *
 *   This program is distributed in the hope that it will be useful,   *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of*
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the *
 *   GNU General Public License for more details.  *
 * *
 *   You should have received a copy of the GNU General Public License *
 *   along with this program; if not, write to the *
 *   Free Software Foundation, Inc.,   *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. *
 ***************************************************************************/

#include "notam/NotamList.h"


QString NOTAM::NotamList::summary() 
{
return {u"One notam • Data possibly incomplete"_qs};
}

QString NOTAM::NotamList::text() 
{
   return QStringLiteral(R"html(<pre>
EDDS AIRCRAFT STANDS LIMITED	P0648/23
From 24 Jan 2023 12:28 until 25 Mar 2023 23:59
EDDS MIL: MIL RAMP GROUNDING POINTS UNRELIABLE.

EDDS AERONAUTICAL INFORMATION SERVICE (AIS) HOURS OF SERVICE CHANGED	P0604/23
From 01 Feb 2023 05:00 until 25 Mar 2023 21:00
STUTTGART ARMY AFLD (SAAF) BASE OPERATION HOURS CHANGED TO
0500-2100 MON-FRI, CLSD ON WEEKENDS EXCEPT PPR APPROVAL.
AFTER HOURS CALL 0173-1652634.

EDGG EDDS VOR/DME	A3607/22
From 08 Jul 2022 08:55 until PERM
STUTTGART DVOR/DME STG 116.85MHZ/CH115Y
RADIAL 166 NOT USABLE.
DVOR-PART: BEYOND 38NM AND BLW 7000FT AMSL
DME-PART: BEYOND 43NM AND BLW 5400FT AMSL.
</pre>)html");
}

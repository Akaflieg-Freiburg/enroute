/***************************************************************************
 *   Copyright (C) 2026 by Stefan Kebekus                                  *
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

#include <QByteArray>
#include <QList>

#include "flightlog/Flight.h"

namespace Flightlog {

/*! \brief ForeFlight Logbook Import CSV export format for the flight log
 *
 *  Produces the exact two-table CSV structure expected by ForeFlight's
 *  logbook import feature (magic row, blank, Aircraft Table, blank,
 *  Flights Table).
 * 
 *  see https://support.foreflight.com/hc/en-us/articles/215998368-Is-there-a-properly-formatted-sample-logbook-available-for-viewing
 *  see https://support.foreflight.com/hc/article_attachments/35060835400727
 */
class FlightLogExportForeFlight
{
public:
    FlightLogExportForeFlight() = delete;

    /*! \brief Generate ForeFlight CSV content for a list of flights
     *
     *  @param flights The flights to include
     *  @returns CSV content as UTF-8, or empty if @p flights is empty
     */
    [[nodiscard]] static auto toCSV(const QList<Flight>& flights) -> QByteArray;
};

} // namespace Flightlog

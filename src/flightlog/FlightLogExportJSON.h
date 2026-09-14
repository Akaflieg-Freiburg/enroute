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
#include <QJsonObject>
#include <QList>
#include <QString>

#include "flightlog/Flight.h"

namespace Flightlog {

/*! \brief JSON export format for the flight log
 *
 *  This is a pure export/interchange format — used when the user shares or
 *  exports flights as JSON — independent of how the flight log is actually
 *  persisted (see FlightLogStorage). Its shape must stay stable regardless
 *  of the persistence backend.
 */
class FlightLogExportJSON
{
public:
    FlightLogExportJSON() = delete;

    /*! \brief Serialize a single flight to JSON
     *
     *  @param flight The flight to serialize
     *  @returns A JSON object representing the flight
     */
    [[nodiscard]] static auto toJSON(const Flight& flight) -> QJsonObject;

    /*! \brief Deserialize a single flight from JSON
     *
     *  @param json A JSON object as produced by toJSON()
     *  @returns A Flight constructed from the JSON data
     */
    [[nodiscard]] static auto fromJSON(const QJsonObject& json) -> Flight;

    /*! \brief Generate JSON content for a list of flights
     *
     *  @param flights The flights to include
     *  @returns JSON content, or empty if @p flights is empty
     */
    [[nodiscard]] static auto toJSON(const QList<Flight>& flights) -> QByteArray;

    /*! \brief Parse a list of flights from JSON content
     *
     *  @param content JSON content, as produced by toJSON(const QList<Flight>&)
     *  @returns The parsed flights, or an empty list if @p content isn't a
     *  valid flight log
     */
    [[nodiscard]] static auto fromJSON(const QByteArray& content) -> QList<Flight>;


    //
    // Static methods for file-type detection (e.g. by a future FileExchange sniffer)
    //

    /*! \brief Check if a file contains valid flight log JSON data
     *
     *  Reads the file and checks whether it is a JSON document with the
     *  {"content": "flightLog", ...} envelope written by toJSON(). Does not
     *  check the validity of individual flight entries, only the envelope.
     *
     *  @param fileName Name of a file
     *  @param info If non-null, receives a translated description of why
     *  the file was rejected
     *  @returns True if the file is likely to contain a flight log
     */
    [[nodiscard]] static auto isValid(const QString& fileName, QString* info = nullptr) -> bool;
};

} // namespace Flightlog

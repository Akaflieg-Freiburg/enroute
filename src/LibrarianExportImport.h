/***************************************************************************
 *   Copyright (C) 2020-2026 by Stefan Kebekus                             *
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

#include <QJsonArray>
#include <QObject>


/*! \brief Backup export and import for the library
 *
 *  This class handles serialization and deserialization of library data
 *  (aircraft, routes, waypoints, maps) for the backup feature.
 *
 *  The backup format is intentionally decoupled from internal data structures.
 *  All sections use a clean, human-readable JSON format with explicit field
 *  names.  Domain classes are accessed exclusively through their public APIs;
 *  no knowledge of internal file formats is required here.
 */

class LibrarianExportImport : public QObject {
    Q_OBJECT

public:
    /*! \brief Export library backup and share/save it via FileExchange
     *
     *  Creates a JSON backup of all aircraft, routes, waypoints, and a list
     *  of installed maps, then hands it to FileExchange for sharing or
     *  saving.
     *
     *  @returns Empty string on success, the string "abort" if the user
     *  cancelled, or a translated error message otherwise
     */
    static QString exportAndShareBackup();

    /*! \brief Import complete library backup from raw data
     *
     *  Parses a JSON backup and restores aircraft, routes, and waypoints.
     *  The maps section is informational and is not imported.
     *
     *  @param content QByteArray containing the backup JSON data
     *
     *  @returns Empty string on success, translated error message on failure
     */
    static QString importFullBackup(const QByteArray& content);

    /*! \brief Import complete library backup from a file
     *
     *  Convenience wrapper that opens a file and calls importFullBackup().
     *
     *  @param fileName Path or URL to the backup file
     *
     *  @returns Empty string on success, translated error message on failure
     */
    static QString importFullBackupFromFile(const QString& fileName);

private:
    Q_DISABLE_COPY_MOVE(LibrarianExportImport)

    // Helpers that build the individual JSON arrays for the backup
    static QJsonArray createAircraftJsonArray(QStringList& errors);
    static QJsonArray createRoutesJsonArray(QStringList& errors);
    static QJsonArray createWaypointsJsonArray(QStringList& errors);
    static QJsonArray createMapsJsonArray();

    //! Backup format version.  Increment only for breaking changes.
    static constexpr int backupFormatVersion = 1;
};

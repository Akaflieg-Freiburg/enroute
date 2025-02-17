/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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

#include <QFile>
#include <QStandardPaths>

#include "geomaps/VAC.h"


namespace GeoMaps
{

/*! \brief Library of visual approach charts
 *
 * This class collects visual approach charts that the user has installed. The
 * list is automatically loaded on startup, and saved every time that a change
 * is made.
 */

class VACLibrary : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    /*! \brief Constructor
     *
     *  @param parent Standard QObject parent
     */
    VACLibrary(QObject *parent = nullptr);

    /*! \brief Destructor */
    ~VACLibrary() override;

    //
    // Properties
    //

    /*! \brief True if library is empty. */
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY dataChanged)

    /*! \brief List of all VACs installed
     *
     * This property holds the list of all installed VACs, sorted alphabetically
     * by name.
     */
    Q_PROPERTY(QList<GeoMaps::VAC> vacs READ vacs NOTIFY dataChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property of the same name
     *
     * @returns Property isEmpty
     */
    [[nodiscard]] bool isEmpty() const { return m_vacs.isEmpty(); }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property vacs
     */
    [[nodiscard]] QList<GeoMaps::VAC> vacs();



    //
    // Methods
    //

    /*! \brief Removes all VACs
     *
     *  This method also deletes the associated files.
     */
    Q_INVOKABLE void clear();

    /*! \brief Obtain VACs from the library
     *
     *  @param name Name of the VAC
     *
     *  @returns Returns an invalid, default-constructed VAC if the name does
     *  not exist in the library.
     */
    [[nodiscard]] Q_INVOKABLE GeoMaps::VAC get(const QString& name);

    /*! \brief Import trip kit
     *
     *  @param fileName Name of the trip kit file
     *
     *  @returns A localized error message, or an empty string on success
     */
    [[nodiscard]] Q_INVOKABLE QString importTripKit(const QString& fileName);

    /*! \brief Import VAC
     *
     *  This method copies the file 'fileName' to the library directory. It does
     *  not take ownership of the file, and does not delete the file.
     *
     *  @param fileName Name of a graphics file
     *
     *  @param name Name under which the VAC is available in the library. If
     *  left empty, a default name is assigned.
     *
     *  @returns A localized error message, or an empty string on success
     */
    [[nodiscard]] Q_INVOKABLE QString importVAC(const QString& fileName, const QString& name = {});

    /*! \brief Remove one VACs
     *
     *  @param name Name of the VAC
     *
     *  This method also deletes the associated file.
     */
    Q_INVOKABLE void remove(const QString& name);

    /*! \brief Rename a VACs
     *
     *  @param oldName Name of the VAC to be removed
     *
     *  @param newName New name of the VAC
     *
     *  @returns A localized error message, or an empty string on success
     */
    [[nodiscard]] Q_INVOKABLE QString rename(const QString& oldName, const QString& newName);

    /*! \brief List of all VACs installed
     *
     * This method returns the list of all installed VACs, sorted by distance to
     * position (closest waypoints first).
     *
     * @param position Geographic position used for sorting
     *
     * @param filter List of words
     *
     * @returns List of all VACs installed
     */
    [[nodiscard]] Q_INVOKABLE QVector<GeoMaps::VAC> vacsByDistance(const QGeoCoordinate& position, const QString& filter);

signals:
    /*! \brief Notifier signal */
    void dataChanged();

    /*! \brief Progress report when importing a trip kit.
     *
     *  This signal is emitted when TripKits are imported. At the end of the
     *  import, the precise value 1.0 is emitted.
     *
     *  @param percent A number between 0.0 and 1.0.
     */
    void importTripKitStatus(double percent);

private:
    Q_DISABLE_COPY_MOVE(VACLibrary)

    // This method cleans the VAC directory. It deletes all VAC from m_vacs that
    // have no raster image files. It looks for unmanaged raster image files and
    // either imports them or deletes them.
    void janitor();

    // This method saves m_vacs to m_dataFile.
    void save();

    QVector<GeoMaps::VAC> m_vacs;
    QString m_vacDirectory {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/VAC"};
    QFile m_dataFile {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/VAC.data"};

};

} // namespace GeoMaps


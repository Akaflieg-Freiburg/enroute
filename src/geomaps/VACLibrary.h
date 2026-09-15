/***************************************************************************
 *   Copyright (C) 2024-2025 by Stefan Kebekus                             *
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

#include <QAbstractListModel>
#include <QFile>
#include <QProperty>
#include <QStandardPaths>
#include <QTimer>

#include "geomaps/VAC.h"


namespace GeoMaps
{

/*! \brief Library of visual approach charts
 *
 * This class collects visual approach charts that the user has installed. The
 * list is automatically loaded on startup, and saved every time that a change
 * is made.
 *
 * In addition to manually imported charts, this class presents the charts
 * contained in the VAC collections managed by DataManagement::DataManager.
 * These charts are not stored in the library data file, cannot be renamed or
 * removed individually, and their raster data is extracted on demand; see
 * materialize().
 *
 * The library is a list model with one row per chart, sorted by section and
 * name, and with the roles 'vac', 'name', 'section' and 'center'. Changes are
 * announced with row-granular model signals, and with the coarse signal
 * vacsChanged() for consumers of the property 'vacs'. In QML, bind a view to
 * this singleton through a NameFilterProxyModel or a DistanceSortProxyModel.
 */

class VACLibrary : public QAbstractListModel
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

    /*! \brief Model roles */
    enum Role : int {
        /*! \brief The chart, of type GeoMaps::VAC */
        VacRole = Qt::UserRole + 1,

        /*! \brief Name of the chart, a QString */
        NameRole,

        /*! \brief Section of the chart, a QString; see GeoMaps::VAC::section() */
        SectionRole,

        /*! \brief Center of the chart, a QGeoCoordinate */
        CenterRole
    };

    //
    // Properties
    //

    /*! \brief True if the library contains manually imported charts. */
    Q_PROPERTY(bool hasManuallyImported READ hasManuallyImported BINDABLE bindableHasManuallyImported NOTIFY hasManuallyImportedChanged)

    /*! \brief True if library is empty. */
    Q_PROPERTY(bool isEmpty READ isEmpty BINDABLE bindableIsEmpty NOTIFY isEmptyChanged)

    /*! \brief List of all VACs installed
     *
     * This property holds the list of all installed VACs, in model order:
     * sorted by section, then by name.
     */
    Q_PROPERTY(QList<GeoMaps::VAC> vacs READ vacs NOTIFY vacsChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property of the same name
     *
     * @returns Property hasManuallyImported
     */
    [[nodiscard]] bool hasManuallyImported() const { return m_hasManuallyImported.value(); }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property hasManuallyImported
     */
    [[nodiscard]] QBindable<bool> bindableHasManuallyImported() const { return &m_hasManuallyImported; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property isEmpty
     */
    [[nodiscard]] bool isEmpty() const { return m_isEmpty.value(); }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property isEmpty
     */
    [[nodiscard]] QBindable<bool> bindableIsEmpty() const { return &m_isEmpty; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property vacs
     */
    [[nodiscard]] QList<GeoMaps::VAC> vacs() const { return m_rows; }


    //
    // Model API
    //

    /*! \brief Re-implemented from QAbstractListModel
     *
     *  @param parent Parent index, invalid for the list itself
     *
     *  @returns Number of charts, or zero for a valid parent
     */
    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /*! \brief Re-implemented from QAbstractListModel
     *
     *  @param index Model index
     *
     *  @param role One of the roles in VACLibrary::Role
     *
     *  @returns Data for the role, or an invalid QVariant
     */
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /*! \brief Re-implemented from QAbstractListModel
     *
     *  @returns Names of the roles in VACLibrary::Role
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;


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

    /*! \brief Obtain a VAC whose fileName points to a raster image file
     *
     *  Charts from VAC collections have their member 'fileName' set to the
     *  name of the collection file. This method extracts the raster image of
     *  such a chart to a cache directory and returns a copy of the VAC whose
     *  'fileName' points to the extracted image, ready for display in the
     *  moving map. Manually imported charts are returned unchanged.
     *
     *  @param vac VAC to materialize
     *
     *  @returns A VAC whose fileName points to a raster image file, or the
     *  unchanged input on error.
     */
    [[nodiscard]] Q_INVOKABLE GeoMaps::VAC materialize(const GeoMaps::VAC& vac);

    /*! \brief Import VAC
     *
     *  This method copies the file 'fileName' to the library directory. It does
     *  not take ownership of the file, and does not delete the file.
     *
     *  @param vac VAC to be imported
     *
     *  @returns A localized error message, or an empty string on success
     */
    [[nodiscard]] Q_INVOKABLE QString importVAC(GeoMaps::VAC vac);

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

    /*! \brief List of all VACs that contain a given point.
     *
     * This method returns the list of all installed VACs that contain the given
     * point.
     *
     * @param position Geographic position
     *
     * @returns List of all VACs containing the given point.
     */
    [[nodiscard]] Q_INVOKABLE QVector<GeoMaps::VAC> vacs4Point(const QGeoCoordinate& position);

signals:
    /*! \brief Notifier signal */
    void hasManuallyImportedChanged();

    /*! \brief Notifier signal */
    void isEmptyChanged();

    /*! \brief Notifier signal */
    void vacsChanged();

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
    // either imports them or moves them to the subdirectory "unrecognised".
    void janitor();

    // This method re-reads the chart index from all VAC collection files
    // managed by DataManagement::DataManager, rebuilds m_collectionVacs and
    // deletes stale entries from the extraction cache.
    void updateCollections();

    // This method saves m_vacs to m_dataFileName, atomically.
    void save();

    // This method returns the absolute path of a given VAC. Needed for iOS
    // after App Update. See GeoMaps::VACLibrary::janitor
    QString absolutePathForVac(const GeoMaps::VAC&);
    QString absolutePathForVac(const QString& name);

    // Replaces the manually imported charts and rebuilds the model rows
    void setManualVacs(const QVector<GeoMaps::VAC>& vacs);

    // Replaces the charts from VAC collections and rebuilds the model rows
    void setCollectionVacs(const QVector<GeoMaps::VAC>& vacs);

    // Recomputes m_rows from m_vacs and m_collectionVacs, announces the
    // difference with row-granular model signals, updates the derived
    // properties and emits vacsChanged() if anything changed.
    void rebuildRows();

    // Sort order of the model rows: by section, then by name, then by file
    // name, so that distinct charts never compare equal
    static bool lessThan(const GeoMaps::VAC& first, const GeoMaps::VAC& second);

    // Source data: manually imported charts and charts from VAC collections.
    // Mutations go through setManualVacs() and setCollectionVacs(): modify a
    // local copy, then assign it back in a single call.
    QVector<GeoMaps::VAC> m_vacs;
    QVector<GeoMaps::VAC> m_collectionVacs;

    // Model rows: m_vacs and m_collectionVacs together, sorted with lessThan()
    QVector<GeoMaps::VAC> m_rows;

    // Derived properties, updated by rebuildRows()
    Q_OBJECT_BINDABLE_PROPERTY(GeoMaps::VACLibrary, bool, m_isEmpty, &GeoMaps::VACLibrary::isEmptyChanged)
    Q_OBJECT_BINDABLE_PROPERTY(GeoMaps::VACLibrary, bool, m_hasManuallyImported, &GeoMaps::VACLibrary::hasManuallyImportedChanged)

    QString m_vacDirectory {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/VAC"_s};
    QString m_cacheDirectory {QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + u"/VAC"_s};
    QString m_dataFileName {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/VAC.data"_s};

    // Compresses multiple change notifications from DataManager into a single
    // call to updateCollections()
    QTimer m_updateCollectionsTimer;

};

} // namespace GeoMaps


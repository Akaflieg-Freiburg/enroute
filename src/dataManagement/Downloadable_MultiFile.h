/***************************************************************************
 *   Copyright (C) 2022-2023 by Stefan Kebekus                             *
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

#include <QQmlEngine>

#include "dataManagement/Downloadable_Abstract.h"

namespace DataManagement {


/*! \brief Group of closely related downloadable items
 *
 *  This class implements a group of Downloadable_Abstract objects.
 */

class Downloadable_MultiFile : public Downloadable_Abstract {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /*! \brief Update Policy */
    enum UpdatePolicy {
        SingleUpdate, /*!< \brief Update children that are updatable */
        MultiUpdate   /*!< \brief Update children that are updatable. If one local file exist, download all other files on update */
    };
    Q_ENUM(UpdatePolicy)

    /*! \brief Standard constructor
     *
     *  @param updatePolicy Update policy of this instance
     *
     *  @param parent The standard QObject parent pointer.
     */
    explicit Downloadable_MultiFile(DataManagement::Downloadable_MultiFile::UpdatePolicy updatePolicy, QObject *parent = nullptr);


    //
    // PROPERTIES
    //

    // Repeated from Downloadable_Abstract to keep QML happy
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)
    Q_PROPERTY(QStringList files READ files NOTIFY filesChanged)
    Q_PROPERTY(Units::ByteSize updateSize READ updateSize NOTIFY updateSizeChanged)

    /*! \brief List of Downloadables in this group
     *
     *  This property holds a list of direct children of the instance.  The list is sorted by section and object name, and never contains a zero pointer.
     */
    Q_PROPERTY(QList<DataManagement::Downloadable_Abstract*> downloadables READ downloadables NOTIFY downloadablesChanged)

    // Repeated from Downloadable_Abstract, to avoid QML warning
    Q_PROPERTY(bool hasFile READ hasFile NOTIFY hasFileChanged)


    //
    // Getter Methods
    //

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     *  @returns Property description
     */
    [[nodiscard]] auto description() -> QString override;

    /*! \brief Getter function for the property with the same name
     *
     *   @returns Property downloadables
     */
    [[nodiscard]] auto downloadables() -> QList<DataManagement::Downloadable_Abstract*>;

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     * @returns Property downloading
     */
    [[nodiscard]] auto downloading() -> bool override { return m_downloading; }

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     * @returns Property files
     */
    [[nodiscard]] auto files() -> QStringList override { return m_files; }

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     * @returns Property hasFile
     */
    [[nodiscard]] auto hasFile() -> bool override { return m_hasFile; }

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     * @returns Property infoText
     */
    [[nodiscard]] auto infoText() -> QString override;

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property remoteFileSize
     */
    [[nodiscard]] auto remoteFileSize() -> qint64 override { return m_remoteFileSize; }

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     * @returns Property updateSize
     */
    [[nodiscard]] auto updateSize() -> Units::ByteSize override { return m_updateSize; }


    //
    // Methods
    //

    /*! \brief Add a Downloadable_SingleFile to this Downloadable_MultiFile
     *
     *  This method copies the values of map->objectName and map->section into *this object.
     *  As a result, these properties are always set to match those of the last map added.
     *  This instance does not take ownership of the map.
     *
     *  It is perfectly possible that a map is a child of more than one Downloadable_MultiFile.
     *
     *  @param map Map to be added
     */
    Q_INVOKABLE void add(DataManagement::Downloadable_Abstract* map);

    /*! \brief Add 'Downloadable_SingleFile's to this Downloadable_MultiFile
     *
     *  This method differs from add(DataManagement::Downloadable_Abstract* map) only in that
     *  it adds several maps, but emits notifier signals only once.
     *
     *  @param maps Maps to be added
     */
    Q_INVOKABLE void add(const QVector<DataManagement::Downloadable_Abstract*>& maps);

    /*! \brief Removes all children */
    Q_INVOKABLE void clear();

    /*! \brief Implementation of pure virtual method from Downloadable_Abstract */
    Q_INVOKABLE void deleteFiles() override;

    /*! \brief Downloadables for a given location
     *
     *   @param location QGeoCoordinate with a location
     *
     *   @returns list of downloadables whose bounding box contains the given location
     */
    [[nodiscard]] Q_INVOKABLE QList<DataManagement::Downloadable_Abstract*> downloadables4Location(const QGeoCoordinate& location);

    /*! \brief Downloadables, sorted by distance to a given location
     *
     *   @param location QGeoCoordinate with a location
     *
     *   @returns list of downloadables whose bounding box contains the given location
     */
    [[nodiscard]] Q_INVOKABLE QList<DataManagement::Downloadable_Abstract*> downloadablesByDistance(const QGeoCoordinate& location);

    /*! \brief Remove a Downloadable_SingleFile from this Downloadable_MultiFile
     *
     *  @param map Map to be removed
     */
    Q_INVOKABLE void remove(DataManagement::Downloadable_Abstract* map);

    /*! \brief Implementation of pure virtual method from Downloadable_Abstract */
    Q_INVOKABLE void startDownload() override;

    /*! \brief Implementation of pure virtual method from Downloadable_Abstract */
    Q_INVOKABLE void stopDownload() override;

    /*! \brief Implementation of pure virtual method from Downloadable_Abstract */
    Q_INVOKABLE void update() override;

signals:
    /*! \brief Notifier signal */
    void downloadablesChanged();

private:
    // Re-evaluate members when the properties of a member changes
    void evaluateDownloading();
    void evaluateFiles();
    void evaluateHasFile();
    void evaluateRemoteFileSize();
    void evaluateUpdateSize();

    bool m_downloading {false};
    QStringList m_files;
    bool m_hasFile {false};
    qint64 m_remoteFileSize {-1};
    qint64 m_updateSize {0};

    QVector<QPointer<DataManagement::Downloadable_Abstract>> m_downloadables;
    DataManagement::Downloadable_MultiFile::UpdatePolicy m_updatePolicy;
};

} // namespace DataManagement

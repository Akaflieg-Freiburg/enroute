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

#ifndef DOWNLOADABLEGROUP_H
#define DOWNLOADABLEGROUP_H


#include "Downloadable.h"


/*! \brief Manages a set of downloadable objects
 *
 * This convenience class collects signals and properties from a set of
 * Downloadable objects, and forwards summarized information.
 */

class DownloadableGroup : public QObject
{
    Q_OBJECT

public:
    /*! \brief Constructs an empty group
     *
     * @param parent The standard QObject parent pointer.
     */
    explicit DownloadableGroup(QObject *parent=nullptr);

    // No copy constructor
    DownloadableGroup(DownloadableGroup const&) = delete;

    // No assign operator
    DownloadableGroup& operator =(DownloadableGroup const&) = delete;

    // No move constructor
    DownloadableGroup(DownloadableGroup&&) = delete;

    // No move assignment operator
    DownloadableGroup& operator=(DownloadableGroup&&) = delete;

    /*! \brief Adds a Downloadable to the group
     *
     * This method adds a Downloadable object to the group. The signals
     * 'downloadingChanged' and 'updatableChanged' will be emitted if required.
     *
     * The DownloadableGroup does not take ownership of the Downloadable, and it
     * is safe to delete the Downloadable after it has been added, without
     * removing it first. It is perfectly fine for Downloadable objects to be
     * members of several groups.
     *
     * @param downloadable Pointer to the Downloadable to be added.
     */
    void addToGroup(Downloadable *downloadable);

    /*! \brief Remove a downloadable from the group
     *
     * This method removes a Downloadable from the group. The signals
     * 'downloadingChanged' and 'updatableChanged' will be emitted if required.
     *
     * @param downloadable Pointer to the Downloadable to be removed.
     */
    void removeFromGroup(Downloadable *downloadable);

    /*! \brief List of downloadables in this group
     *
     * This property holds the list of downloadables in the group. The
     * downloadables are sorted alphabetically in ascending order, first be
     * section() and then secondly by file name. The nullptr is never contained
     * in the list.
     */
    Q_PROPERTY(QList<Downloadable *> downloadables READ downloadables NOTIFY downloadablesChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property downloadables
     */
    QList<Downloadable *> downloadables() const;

    /*! \brief Indicates whether a download process is currently running
     *
     * By definition, an empty group is not downloading
     */
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property downloading
     */
    bool downloading() const;

    /*! \brief Names of all files that have been downloaded by any of the
     *  Downloadbles in this group
     */
    Q_PROPERTY(QStringList files READ files NOTIFY filesChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property localFiles
     */
    QStringList files() const;

    /*! \brief Indicates any one of Downloadable objects is known to be updatable
     *
     * By definition, an empty group is not updatable
     */
    Q_PROPERTY(bool updatable READ updatable NOTIFY updatableChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property updatable
     */
    bool updatable() const;

signals:
    /*! \brief Notifier signal for property downloading */
    void downloadingChanged(bool);

    /*! \brief Notifier signal for the property updatable */
    void updatableChanged();

    /*! \brief Notifier signal for the property localFiles */
    void filesChanged();

    /*! \brief Emitted if the content of one of the local files changes.
     *
     * This signal is emitted if one of the downloadables in this group emits
     * the signal localFileContentChanged().
     */
    void localFileContentChanged();

    /*! \brief Notifier signal for the property downloadables */
    void downloadablesChanged();

private slots:
    void elementChanged();

    // Remove all instances of nullptr from _downloadables
    void cleanUp();

private:
    bool _cachedDownloading; // Cached value for the 'downloading' property
    bool _cachedUpdatable;   // Cached value for the 'updatable' property

    // List of QPointers to the Downloadable objects in this group
    QList<QPointer<Downloadable>> _downloadables;
};

#endif // DOWNLOADABLEGROUP_H

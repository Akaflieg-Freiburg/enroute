/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

#include <QTimer>

#include "Downloadable_SingleFile.h"

namespace DataManagement
{

/*! \brief Watches a group of Downloadable objects
   *
   *  This convenience class collects signals and properties from a set of
   *  Downloadable objects, and forwards summarized information.
   */

class DownloadableGroupWatcher : public QObject
{
    Q_OBJECT

public:
    //
    // PROPERTIES
    //

    /*! \brief List of Downloadables in this group
     *
     *  This property holds the list of Downloadable objects in the group. The
     *  Downloadable objects are sorted alphabetically in ascending order, first
     *  by section() and then secondly by file name. The nullptr is never
     *  contained in the list.
     */
    Q_PROPERTY(QVector<QPointer<DataManagement::Downloadable_Abstract>> downloadables READ downloadables NOTIFY downloadablesChanged)

    /*! \brief List of Downloadables in this group, as a list of QObjects
     *
     *  This property is identical to downloadables, but returns the pointers to
     *  the Downloadable objects in the form of a QObjectList
     */
    Q_PROPERTY(QVector<QObject*> downloadablesAsObjectList READ downloadablesAsObjectList NOTIFY downloadablesChanged)

    /*! \brief List of Downloadable objects in this group that have local files
     *
     *  This property holds the list of Downloadable objects in the group that
     *  have local files. The Downloadable objects are sorted alphabetically in
     *  ascending order, first by section() and then secondly by object. The
     *  nullptr is never contained in the list.
     */
    Q_PROPERTY(QVector<QPointer<DataManagement::Downloadable_SingleFile>> downloadablesWithFile READ downloadablesWithFile NOTIFY downloadablesWithFileChanged)

    /*! \brief Indicates whether a download process is currently running
     *
     *  This is true if there exists an object in the group that is currently
     *  downloading.  By definition, an empty group is not downloading
     */
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)

    /*! \brief Names of all files that have been downloaded by any of the
     *  Downloadble objects in this group
     */
    Q_PROPERTY(QStringList files READ files NOTIFY filesChanged)

    /*! \brief True iF one of the Downloadable objects has a local file */
    Q_PROPERTY(bool hasFile READ hasFile NOTIFY hasFileChanged)

    /*! \brief Indicates that one of Downloadable objects is updatable
     *
     *  By definition, an empty group is not updatable
     */
    Q_PROPERTY(bool updatable READ updatable NOTIFY updatableChanged)

    /*! \brief Gives an estimate for the download size for all updates in this group */
    Q_PROPERTY(qsizetype updateSize READ updateSize NOTIFY updateSizeChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *   @returns Property downloadables
     */
    [[nodiscard]] auto downloadables() const -> QVector<QPointer<DataManagement::Downloadable_Abstract>>;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property downloadables
     */
    [[nodiscard]] auto downloadablesAsObjectList() const -> QVector<QObject*>;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property downloadablesWithFiles
     */
    [[nodiscard]] auto downloadablesWithFile() const -> QVector<QPointer<DataManagement::Downloadable_SingleFile>>;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property downloading
     */
    [[nodiscard]] auto downloading() const -> bool;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property files
     */
    [[nodiscard]] auto files() const -> QStringList;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property hasFile
     */
    [[nodiscard]] auto hasFile() const -> bool;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property updatable
     */
    [[nodiscard]] auto updatable() const -> bool;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property updateSize
     */
    [[nodiscard]] auto updateSize() const -> qsizetype;


    //
    // Methods
    //

    /*! \brief Kill pending emission of signal localFileContentChanged_delayed */
    void killLocalFileContentChanged_delayed()
    {
        emitLocalFileContentChanged_delayedTimer.stop();
    }

    /*! \brief Number of files that are either downloaded or currently
     *  downloading.
     *
     *  @returns int nFilesTotal
     */
    Q_INVOKABLE [[nodiscard]] int numberOfFilesTotal() const;

    /*! \brief Deletes all local files
     *
     *  This method call deleteFile() on all Downloadable objects in this group.
     */
    Q_INVOKABLE void deleteAllFiles();

    /*! Update all updatable downloadable objects */
    void updateAll();

signals:
    /*! \brief Notifier signal for property downloading */
    void downloadablesWithFileChanged(QVector<QPointer<DataManagement::Downloadable_SingleFile>>);

    /*! \brief Notifier signal for property downloading */
    void downloadingChanged(bool);

    /*! \brief Notifier signal for the property localFiles */
    void filesChanged(QStringList);

    /*! \brief Notifier signal for the property localFiles */
    void hasFileChanged(bool);

    /*! \brief Notifier signal for the property updatable */
    void updatableChanged(bool);

    /*! \brief Notifier signal for the property updatable */
    void updateSizeChanged(qsizetype);

    /*! \brief Emitted if the content of one of the local files changes.
     *
     *  This signal is emitted if one of the downloadables in this group emits
     *  the signal localFileContentChanged().
     */
    void localFileContentChanged();

    /*! \brief Emitted some time after the content of one of the local files
     *  changes.
     *
     *  This signal is in principle identical to localFileContentChanged(), but
     *  is emitted with a delay.  The DownloadableGroupWatch waits with the
     *  emission of this signal for two seconds. In addition it waits until
     *  there are no running download processes anymore.
     */
    void localFileContentChanged_delayed();

    /*! \brief Notifier signal for the property downloadables */
    void downloadablesChanged();

protected slots:
    // This slot is called whenever a Downloadable in this group changes. It
    // compares if one of the _cached… values has changed and emits the
    // appropriate notification signals.
    void checkAndEmitSignals();

    // Remove all instances of nullptr from _downloadables
    void cleanUp();

protected:
    /*! \brief Constructs an empty group
     *
     *  @param parent The standard QObject parent pointer.
     */
    explicit DownloadableGroupWatcher(QObject *parent = nullptr);

    // List of QPointers to the Downloadable objects in this group
    QList<QPointer<Downloadable_Abstract>> _downloadables;

private:
    Q_DISABLE_COPY_MOVE(DownloadableGroupWatcher)

    // Provisions to provide the signal localFileContentChanged_delayed
    void emitLocalFileContentChanged_delayed();

    QTimer emitLocalFileContentChanged_delayedTimer;

    bool _cachedDownloading {false};                                 // Cached value for the 'downloading' property
    QVector<QPointer<Downloadable_SingleFile>> _cachedDownloadablesWithFile {}; // Cached value for the 'downloadablesWithFiles' property
    QStringList _cachedFiles{};                                     // Cached value for the 'files' property
    bool _cachedHasFile {false};                                     // Cached value for the 'hasLocalFile' property
    bool _cachedUpdatable {false};                                   // Cached value for the 'updatable' property
    qsizetype _cachedUpdateSize {};                                    // Cached value for the 'updateSize' property
};

};

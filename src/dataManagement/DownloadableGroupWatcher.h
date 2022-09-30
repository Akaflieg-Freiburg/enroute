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

class DownloadableGroupWatcher : public Downloadable_Abstract
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

    /*! \brief Names of all files that have been downloaded by any of the
     *  Downloadble objects in this group
     */
    Q_PROPERTY(QStringList files READ files NOTIFY filesChanged)



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
    [[nodiscard]] auto downloading() -> bool override;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property files
     */
    [[nodiscard]] auto files() const -> QStringList;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property hasFile
     */
    [[nodiscard]] auto hasFile() -> bool override;

    /*! \brief Implementation of pure virtual getter method from Downloadable_Abstract
     *
     *  @returns Property infoText
     */
    [[nodiscard]] auto infoText() -> QString override;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property updateSize
     */
    [[nodiscard]] auto updateSize() -> qint64 override;



    //
    // Methods
    //

    /*! \brief Implementation of pure virtual method from Downloadable_Abstract
     *
     * This method deletes the local files.
     */
    Q_INVOKABLE void deleteFiles() override;

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

    /*! \brief Implementation of pure virtual method from Downloadable_Abstract */
    Q_INVOKABLE void startDownload() override;

    /*! \brief Implementation of pure virtual method from Downloadable_Abstract */
    Q_INVOKABLE void stopDownload() override ;

    /*! \brief Implementation of pure virtual method from Downloadable_Abstract */
    Q_INVOKABLE void update() override;


signals:
    /*! \brief Notifier signal for property downloading */
    void downloadablesWithFileChanged(QVector<QPointer<DataManagement::Downloadable_SingleFile>>);

    /*! \brief Notifier signal for the property localFiles */
    void filesChanged(QStringList);

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
    QList<QPointer<Downloadable_Abstract>> m_downloadables;

private:
    Q_DISABLE_COPY_MOVE(DownloadableGroupWatcher)

    // Provisions to provide the signal localFileContentChanged_delayed
    void emitLocalFileContentChanged_delayed();

    QTimer emitLocalFileContentChanged_delayedTimer;

    bool m_cachedDownloading {false};                                 // Cached value for the 'downloading' property
    QVector<QPointer<Downloadable_SingleFile>> m_cachedDownloadablesWithFile {}; // Cached value for the 'downloadablesWithFiles' property
    QStringList m_cachedFiles {};                                     // Cached value for the 'files' property
    bool m_cachedHasFile {false};                                     // Cached value for the 'hasLocalFile' property
    qsizetype m_cachedUpdateSize {};                                    // Cached value for the 'updateSize' property
};

};

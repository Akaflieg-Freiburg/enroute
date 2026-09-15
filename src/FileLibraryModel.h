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

#include <QAbstractListModel>
#include <QFileSystemWatcher>
#include <QQmlEngine>
#include <QTimer>


/*! \brief List of the entries of a file-based library
 *
 *  A file-based library is a directory that holds one file per entry, all
 *  with the same suffix. This model lists the base names of these files under
 *  the role 'name', sorted case-insensitively.
 *
 *  The directory is watched with a QFileSystemWatcher, so that the model
 *  follows every writer, including those that do not go through Librarian.
 *  Changes to the directory are coalesced and applied with row-granular model
 *  signals. Writers inside the app call refresh() to apply their changes
 *  immediately.
 *
 *  Instances are owned by Librarian, see Librarian::aircraftModel and
 *  Librarian::routesModel. In QML, bind a view to one of them through a
 *  NameFilterProxyModel.
 */

class FileLibraryModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use Librarian.aircraftModel or Librarian.routesModel")

public:
    /*! \brief Model roles */
    enum Role : int {
        /*! \brief Base name of the entry, a QString */
        NameRole = Qt::UserRole + 1
    };

    /*! \brief Standard constructor
     *
     *  @param directory Directory holding the library files. Created if it
     *  does not exist.
     *
     *  @param suffix Suffix of the library files, including the dot, such as
     *  ".json"
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit FileLibraryModel(QString directory, QString suffix, QObject* parent = nullptr);

    // Default destructor
    ~FileLibraryModel() override = default;


    //
    // Getter Methods
    //

    /*! \brief Base names of the library entries
     *
     *  @returns Names, in model order
     */
    [[nodiscard]] QStringList names() const { return m_names; }


    //
    // Model API
    //

    /*! \brief Re-implemented from QAbstractListModel
     *
     *  @param parent Parent index, invalid for the list itself
     *
     *  @returns Number of entries, or zero for a valid parent
     */
    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /*! \brief Re-implemented from QAbstractListModel
     *
     *  @param index Model index
     *
     *  @param role One of the roles in FileLibraryModel::Role
     *
     *  @returns Data for the role, or an invalid QVariant
     */
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /*! \brief Re-implemented from QAbstractListModel
     *
     *  @returns Names of the roles in FileLibraryModel::Role
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

public slots:
    /*! \brief Rescans the directory
     *
     *  Compares the directory content with the model and announces the
     *  difference with row-granular signals. This is called automatically when
     *  the directory changes; writers inside the app call it to apply their
     *  changes without delay.
     */
    void refresh();

private:
    Q_DISABLE_COPY_MOVE(FileLibraryModel)

    // Base names of the library files in the directory, sorted with lessThan()
    [[nodiscard]] QStringList scan() const;

    // Sort order of the model: case-insensitive, with a case-sensitive
    // tie-break so that distinct names never compare equal
    static bool lessThan(const QString& first, const QString& second);

    QString m_directory;
    QString m_suffix;

    // Model content, sorted with lessThan()
    QStringList m_names;

    QFileSystemWatcher m_watcher;

    // Directory changes arrive in bursts, e.g. when a file is saved
    // atomically. The timer coalesces them into one refresh().
    QTimer m_debounce;
};

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

#include <QDir>
#include <algorithm>

#include "FileLibraryModel.h"

using namespace Qt::Literals::StringLiterals;


FileLibraryModel::FileLibraryModel(QString directory, QString suffix, QObject* parent)
    : QAbstractListModel(parent), m_directory(std::move(directory)), m_suffix(std::move(suffix))
{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    m_debounce.setSingleShot(true);
    m_debounce.setInterval(100);
    connect(&m_debounce, &QTimer::timeout, this, &FileLibraryModel::refresh);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this]() { m_debounce.start(); });

    refresh();
}


//
// Model API
//

int FileLibraryModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return static_cast<int>(m_names.size());
}


QVariant FileLibraryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || (index.row() < 0) || (index.row() >= m_names.size()))
    {
        return {};
    }

    switch (role)
    {
    case NameRole:
    case Qt::DisplayRole:
        return m_names.at(index.row());
    default:
        return {};
    }
}


QHash<int, QByteArray> FileLibraryModel::roleNames() const
{
    return {{NameRole, "name"}};
}


//
// Slots
//

void FileLibraryModel::refresh()
{
    // The watcher forgets a directory once it is deleted. Make sure that the
    // directory exists and is watched.
    if (!m_watcher.directories().contains(m_directory))
    {
        QDir().mkpath(m_directory);
        m_watcher.addPath(m_directory);
    }

    auto const newNames = scan();

    // Both m_names and newNames are sorted with lessThan(). Merge them and
    // announce every run of vanished or new names as one row operation.
    int i = 0;          // Row in m_names
    qsizetype j = 0;    // Index in newNames
    while ((i < m_names.size()) || (j < newNames.size()))
    {
        // Run of names that are no longer in the directory
        if ((i < m_names.size()) && ((j >= newNames.size()) || lessThan(m_names.at(i), newNames.at(j))))
        {
            int count = 1;
            while ((i + count < m_names.size()) && ((j >= newNames.size()) || lessThan(m_names.at(i + count), newNames.at(j))))
            {
                ++count;
            }
            beginRemoveRows({}, i, i + count - 1);
            m_names.remove(i, count);
            endRemoveRows();
            continue;
        }

        // Run of names that are new in the directory
        if ((j < newNames.size()) && ((i >= m_names.size()) || lessThan(newNames.at(j), m_names.at(i))))
        {
            int count = 1;
            while ((j + count < newNames.size()) && ((i >= m_names.size()) || lessThan(newNames.at(j + count), m_names.at(i))))
            {
                ++count;
            }
            beginInsertRows({}, i, i + count - 1);
            for (int k = 0; k < count; ++k)
            {
                m_names.insert(i + k, newNames.at(j + k));
            }
            endInsertRows();
            i += count;
            j += count;
            continue;
        }

        // Same name in both lists
        ++i;
        ++j;
    }
}


//
// Private Methods
//

QStringList FileLibraryModel::scan() const
{
    QDir const dir(m_directory);
    auto const fileNames = dir.entryList({u"*"_s + m_suffix}, QDir::Files);

    QStringList names;
    names.reserve(fileNames.size());
    for (auto const& fileName : fileNames)
    {
        if (fileName.endsWith(m_suffix, Qt::CaseSensitive))
        {
            names.append(fileName.chopped(m_suffix.size()));
        }
    }
    std::ranges::sort(names, &FileLibraryModel::lessThan);
    return names;
}


bool FileLibraryModel::lessThan(const QString& first, const QString& second)
{
    auto const insensitive = QString::compare(first, second, Qt::CaseInsensitive);
    if (insensitive != 0)
    {
        return insensitive < 0;
    }
    return QString::compare(first, second, Qt::CaseSensitive) < 0;
}

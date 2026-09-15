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

#include "GlobalObject.h"
#include "Librarian.h"
#include "ui/NameFilterProxyModel.h"


Ui::NameFilterProxyModel::NameFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}


//
// Setter Methods
//

void Ui::NameFilterProxyModel::setFilter(const QString& newFilter)
{
    if (m_filter == newFilter)
    {
        return;
    }

    beginFilterChange();
    m_filter = newFilter;
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    invalidateRowsFilter();
#endif
    emit filterChanged();
}


void Ui::NameFilterProxyModel::setFilterRoleName(const QString& newFilterRoleName)
{
    if (m_filterRoleName == newFilterRoleName)
    {
        return;
    }

    beginFilterChange();
    m_filterRoleName = newFilterRoleName;
    m_filterRole = roleForName(m_filterRoleName);
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    invalidateRowsFilter();
#endif
    emit filterRoleNameChanged();
}


//
// Methods
//

void Ui::NameFilterProxyModel::setSourceModel(QAbstractItemModel* newSourceModel)
{
    QSortFilterProxyModel::setSourceModel(newSourceModel);
    m_filterRole = roleForName(m_filterRoleName);
}


bool Ui::NameFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (m_filter.isEmpty() || (m_filterRole < 0))
    {
        return true;
    }

    auto* source = sourceModel();
    if (source == nullptr)
    {
        return true;
    }

    auto const text = source->index(sourceRow, 0, sourceParent).data(m_filterRole).toString();
    return GlobalObject::librarian()->matches(text, m_filter);
}


int Ui::NameFilterProxyModel::roleForName(const QString& roleName) const
{
    auto* source = sourceModel();
    if (source == nullptr)
    {
        return -1;
    }
    return source->roleNames().key(roleName.toUtf8(), -1);
}

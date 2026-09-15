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

#include <limits>

#include "ui/DistanceSortProxyModel.h"


Ui::DistanceSortProxyModel::DistanceSortProxyModel(QObject* parent)
    : NameFilterProxyModel(parent)
{
}


//
// Setter Methods
//

void Ui::DistanceSortProxyModel::setReferenceCoordinate(const QGeoCoordinate& newReferenceCoordinate)
{
    if (m_referenceCoordinate == newReferenceCoordinate)
    {
        return;
    }
    m_referenceCoordinate = newReferenceCoordinate;
    emit referenceCoordinateChanged();

    if (!m_referenceCoordinate.isValid())
    {
        return;
    }
    if (m_sortReference.isValid() && (m_sortReference.distanceTo(m_referenceCoordinate) <= m_resortDistance))
    {
        return;
    }
    m_sortReference = m_referenceCoordinate;
    invalidate();
}


void Ui::DistanceSortProxyModel::setCoordinateRoleName(const QString& newCoordinateRoleName)
{
    if (m_coordinateRoleName == newCoordinateRoleName)
    {
        return;
    }
    m_coordinateRoleName = newCoordinateRoleName;
    m_coordinateRole = roleForName(m_coordinateRoleName);
    emit coordinateRoleNameChanged();
    invalidate();
}


void Ui::DistanceSortProxyModel::setResortDistance(double newResortDistance)
{
    if (m_resortDistance == newResortDistance)
    {
        return;
    }
    m_resortDistance = newResortDistance;
    emit resortDistanceChanged();
}


//
// Methods
//

void Ui::DistanceSortProxyModel::setSourceModel(QAbstractItemModel* newSourceModel)
{
    NameFilterProxyModel::setSourceModel(newSourceModel);
    m_coordinateRole = roleForName(m_coordinateRoleName);

    // Activate sorting. QSortFilterProxyModel::sort() returns early when the
    // sort column is unchanged, and sortColumn() is -1 for a new proxy, so this
    // call is what switches dynamic sorting on.
    sort(0);
}


bool Ui::DistanceSortProxyModel::lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight) const
{
    if (m_sortReference.isValid() && (m_coordinateRole >= 0))
    {
        auto const leftDistance = distance(sourceLeft);
        auto const rightDistance = distance(sourceRight);
        if (leftDistance != rightDistance)
        {
            return leftDistance < rightDistance;
        }
    }
    return sourceLeft.row() < sourceRight.row();
}


double Ui::DistanceSortProxyModel::distance(const QModelIndex& sourceIndex) const
{
    auto const coordinate = sourceIndex.data(m_coordinateRole).value<QGeoCoordinate>();
    if (!coordinate.isValid())
    {
        return std::numeric_limits<double>::infinity();
    }
    return m_sortReference.distanceTo(coordinate);
}

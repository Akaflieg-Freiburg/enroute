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

#include <QGeoCoordinate>

#include "ui/NameFilterProxyModel.h"

namespace Ui {

/*! \brief NameFilterProxyModel that orders rows by distance to a reference position
 *
 *  This proxy model filters like NameFilterProxyModel and, in addition, orders
 *  the accepted rows by distance to the property 'referenceCoordinate', closest
 *  first. The distance is computed from the source model role
 *  'coordinateRoleName', which must hold a QGeoCoordinate.
 *
 *  Position feeds update at a high rate. To avoid re-sorting the list on every
 *  update, the order is recomputed only when the reference coordinate has moved
 *  by more than 'resortDistance' since the last sort. While the reference
 *  coordinate is invalid, the order of the source model is kept.
 */

class DistanceSortProxyModel : public NameFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    /*! \brief Standard constructor
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit DistanceSortProxyModel(QObject* parent = nullptr);

    // Default destructor
    ~DistanceSortProxyModel() override = default;


    //
    // Properties
    //

    /*! \brief Position that distances are measured from
     *
     *  Invalid coordinates are accepted and leave the current order unchanged.
     */
    Q_PROPERTY(QGeoCoordinate referenceCoordinate READ referenceCoordinate WRITE setReferenceCoordinate NOTIFY referenceCoordinateChanged)

    /*! \brief Name of the source model role holding the row's QGeoCoordinate
     *
     *  Defaults to "center". Rows keep the source order if the source model has
     *  no role of this name.
     */
    Q_PROPERTY(QString coordinateRoleName READ coordinateRoleName WRITE setCoordinateRoleName NOTIFY coordinateRoleNameChanged)

    /*! \brief Movement of the reference coordinate, in metres, that triggers a re-sort
     *
     *  Defaults to 1000 metres.
     */
    Q_PROPERTY(double resortDistance READ resortDistance WRITE setResortDistance NOTIFY resortDistanceChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property referenceCoordinate
     */
    [[nodiscard]] QGeoCoordinate referenceCoordinate() const { return m_referenceCoordinate; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property coordinateRoleName
     */
    [[nodiscard]] QString coordinateRoleName() const { return m_coordinateRoleName; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property resortDistance
     */
    [[nodiscard]] double resortDistance() const { return m_resortDistance; }


    //
    // Setter Methods
    //

    /*! \brief Setter function for the property with the same name
     *
     *  @param newReferenceCoordinate Property referenceCoordinate
     */
    void setReferenceCoordinate(const QGeoCoordinate& newReferenceCoordinate);

    /*! \brief Setter function for the property with the same name
     *
     *  @param newCoordinateRoleName Property coordinateRoleName
     */
    void setCoordinateRoleName(const QString& newCoordinateRoleName);

    /*! \brief Setter function for the property with the same name
     *
     *  @param newResortDistance Property resortDistance
     */
    void setResortDistance(double newResortDistance);


    //
    // Methods
    //

    /*! \brief Re-implemented from QAbstractProxyModel
     *
     *  Sets the source model and activates sorting.
     *
     *  @param newSourceModel Source model
     */
    void setSourceModel(QAbstractItemModel* newSourceModel) override;

signals:
    /*! \brief Notification signal for property with the same name */
    void referenceCoordinateChanged();

    /*! \brief Notification signal for property with the same name */
    void coordinateRoleNameChanged();

    /*! \brief Notification signal for property with the same name */
    void resortDistanceChanged();

protected:
    /*! \brief Re-implemented from QSortFilterProxyModel
     *
     *  @param sourceLeft Index in the source model
     *
     *  @param sourceRight Index in the source model
     *
     *  @returns True if the row of sourceLeft is closer to the reference
     *  coordinate than the row of sourceRight
     */
    [[nodiscard]] bool lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight) const override;

private:
    Q_DISABLE_COPY_MOVE(DistanceSortProxyModel)

    // Distance from the reference coordinate to the coordinate of a source
    // row. Infinite if the row has no valid coordinate.
    [[nodiscard]] double distance(const QModelIndex& sourceIndex) const;

    QGeoCoordinate m_referenceCoordinate;

    // Reference coordinate that the current order is based on
    QGeoCoordinate m_sortReference;

    QString m_coordinateRoleName {QStringLiteral("center")};

    // Role matching m_coordinateRoleName in the current source model, or -1
    int m_coordinateRole {-1};

    double m_resortDistance {1000.0};
};

} // namespace Ui

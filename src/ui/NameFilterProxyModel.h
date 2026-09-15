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

#include <QQmlEngine>
#include <QSortFilterProxyModel>

namespace Ui {

/*! \brief Makes QSortFilterProxyModel known to the QML type system
 *
 *  QtQml.Models registers QAbstractProxyModel, but not QSortFilterProxyModel.
 *  Without this anonymous registration, qmltyperegistrar cannot resolve the
 *  prototype chain of the proxy models below, and qmllint does not know their
 *  inherited properties, such as 'sourceModel'.
 */
struct QSortFilterProxyModelForeign
{
    Q_GADGET
    QML_FOREIGN(QSortFilterProxyModel)
    QML_ANONYMOUS
};


/*! \brief Filters a list model by a fuzzy name match
 *
 *  This proxy model accepts those rows of its source model whose entry for the
 *  role 'filterRoleName' matches the property 'filter', in the sense of
 *  Librarian::matches(). The order of the source model is preserved. Typical
 *  use in QML:
 *
 *  @code
 *  ListView {
 *      model: NameFilterProxyModel {
 *          sourceModel: WaypointLibrary
 *          filter: filterField.filter
 *      }
 *  }
 *  @endcode
 */

class NameFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    /*! \brief Standard constructor
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit NameFilterProxyModel(QObject* parent = nullptr);

    // Default destructor
    ~NameFilterProxyModel() override = default;


    //
    // Properties
    //

    /*! \brief Filter string
     *
     *  Rows are accepted if Librarian::matches() holds for the filter and the
     *  text of the role 'filterRoleName'. The empty string accepts all rows.
     */
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)

    /*! \brief Name of the source model role that the filter is matched against
     *
     *  Defaults to "name". Rows are accepted unconditionally if the source model
     *  has no role of this name.
     */
    Q_PROPERTY(QString filterRoleName READ filterRoleName WRITE setFilterRoleName NOTIFY filterRoleNameChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property filter
     */
    [[nodiscard]] QString filter() const { return m_filter; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property filterRoleName
     */
    [[nodiscard]] QString filterRoleName() const { return m_filterRoleName; }


    //
    // Setter Methods
    //

    /*! \brief Setter function for the property with the same name
     *
     *  @param newFilter Property filter
     */
    void setFilter(const QString& newFilter);

    /*! \brief Setter function for the property with the same name
     *
     *  @param newFilterRoleName Property filterRoleName
     */
    void setFilterRoleName(const QString& newFilterRoleName);


    //
    // Methods
    //

    /*! \brief Re-implemented from QAbstractProxyModel
     *
     *  @param newSourceModel Source model
     */
    void setSourceModel(QAbstractItemModel* newSourceModel) override;

signals:
    /*! \brief Notification signal for property with the same name */
    void filterChanged();

    /*! \brief Notification signal for property with the same name */
    void filterRoleNameChanged();

protected:
    /*! \brief Re-implemented from QSortFilterProxyModel
     *
     *  @param sourceRow Row in the source model
     *
     *  @param sourceParent Parent index in the source model
     *
     *  @returns True if the row matches the filter
     */
    [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

    /*! \brief Looks up a role by name in the source model
     *
     *  @param roleName Role name, as found in QAbstractItemModel::roleNames()
     *
     *  @returns The role, or -1 if there is no source model or no role of that
     *  name
     */
    [[nodiscard]] int roleForName(const QString& roleName) const;

private:
    Q_DISABLE_COPY_MOVE(NameFilterProxyModel)

    QString m_filter;
    QString m_filterRoleName {QStringLiteral("name")};

    // Role matching m_filterRoleName in the current source model, or -1
    int m_filterRole {-1};
};

} // namespace Ui

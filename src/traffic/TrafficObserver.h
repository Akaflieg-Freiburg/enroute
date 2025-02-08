/***************************************************************************
 *   Copyright (C) 2025 by Stefan Kebekus                                  *
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

#include <QProperty>

#include "traffic/TrafficFactor_Abstract.h"


namespace Traffic {

/*! \brief Provides list of traffic, sorted by relevance
 *
 *  This class monitors the global TrafficDataProvider to produce list of all
 *  traffic factors, sorted by relevance. Holding an instance of this class is
 *  relatively expensive because it needs to update and sort whenever any data
 *  in the TrafficDataProvider changes. Instances should therefore be deleted as
 *  soon as they are no longer used.
 */

class TrafficObserver : public QObject {
    Q_OBJECT
    QML_ELEMENT

public:
    //
    // Constructors and destructors
    //

    /*! \brief Standard constructor
     *
     * @param parent Standard QObject parent
     */
    explicit TrafficObserver(QObject* parent=nullptr);

    /*! \brief Standard destructor */
    ~TrafficObserver() override = default;


    //
    // Properties
    //

    /*! List of current traffic, sorted by relevance */
    Q_PROPERTY(QList<Traffic::TrafficFactor_Abstract*> traffic READ traffic BINDABLE bindableTraffic)


    //
    // Getter Methods
    //

    /*! \brief Getter method for property of the same name
     *
     * @returns Property traffic
     */
    [[nodiscard]] QList<Traffic::TrafficFactor_Abstract*> traffic() {return m_traffic.value();}

    /*! \brief Getter method for property of the same name
     *
     * @returns Property traffic
     */
    [[nodiscard]] QBindable<QList<Traffic::TrafficFactor_Abstract*>> bindableTraffic() {return &m_traffic;}

private:
    Q_DISABLE_COPY_MOVE(TrafficObserver)

    QProperty<QList<Traffic::TrafficFactor_Abstract*>> m_traffic;
};

} // namespace Weather

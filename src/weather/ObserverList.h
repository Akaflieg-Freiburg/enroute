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

#include "weather/Observer.h"

namespace Weather {

/*! \brief Up-to-date list of all METAR/TAF stations
 *
 *  This class holds an up-to-date list of all METAR/TAF stations, frequently
 *  updated and sorted by distance to the current position. The class is
 *  relatively expensive, as the constant updates will take some computation
 *  time. It makes sense to hold instances only when required and to delete them
 *  as soon as possible.
 */

class ObserverList : public QObject {
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
    explicit ObserverList(QObject* parent=nullptr);

    /*! \brief Standard destructor */
    ~ObserverList() override = default;


    //
    // Properties
    //

    /*! \brief List of METAR/TAF stations
     *
     *  This property holds the last METAR for the waypoint, sorted by distance
     *  to the current positionq.
     */
    Q_PROPERTY(QList<Weather::Observer*> observers READ observers BINDABLE bindableObservers)


    //
    // Getter Methods
    //

    /*! \brief Getter method for property of the same name
     *
     * @returns Property observers
     */
    [[nodiscard]] QList<Weather::Observer*> observers() {return m_observers.value();}

    /*! \brief Getter method for property of the same name
     *
     * @returns Property observers
     */
    [[nodiscard]] QBindable<QList<Weather::Observer*>> bindableObservers() const {return &m_observers;}


private:
    Q_DISABLE_COPY_MOVE(ObserverList)

    // Observers by ICAO-Id. This map is used to store Observer instances.
    QMap<QString,Weather::Observer*> m_observersByID;

    // Currently active observers. This list is updated by a binding that
    // watches GlobalObject::weatherDataProvider, creates observer instances if
    // needed and stores them in m_observersByID.
    QProperty<QList<Weather::Observer*>> m_unsortedObservers;

    // Currently active observers, sorted by distance. This list is updated by a
    // binding that watches m_unsortedObservers and the current position.
    QProperty<QList<Weather::Observer*>> m_observers;
};

} // namespace Weather


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

/*! \brief This class represents a weather Observer that issues METAR or TAF
 * report
 *
 * This is a very simple value-base class that holds a waypoint and queries the
 * WeatherDataProvider for METAR and TAF for this waypoint.
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
     * This standard constructor creates a WeatherObserver with an invalid waypoint.
     */
    explicit ObserverList(QObject* parent=nullptr);

    /*! \brief Standard destructor */
    ~ObserverList() = default;


    //
    // Properties
    //

    Q_PROPERTY(QList<Weather::Observer*> observers READ observers BINDABLE bindableObservers)
    [[nodiscard]] QList<Weather::Observer*> observers() {return m_observers.value();}
    [[nodiscard]] QBindable<QList<Weather::Observer*>> bindableObservers() const {return &m_observers;}


private:
    Q_DISABLE_COPY_MOVE(ObserverList)

    QMap<QString,Weather::Observer*> m_observersByID;

    QProperty<QList<Weather::Observer*>> m_unsortedObservers;
    QProperty<QList<Weather::Observer*>> m_observers;
};

} // namespace Weather


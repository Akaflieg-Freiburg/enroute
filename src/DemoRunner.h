/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#include <QQmlApplicationEngine>


/*! \brief Remote controls the app and takes screenshot images
 *
 * This class remote controls the app.  It sets up a traffic data receiver simulator,
 * feeds it with data and controls the GUI, in order to generate a sequence of screenshots,
 * which can then be used in the manual and as propaganda material.
 */

class DemoRunner : public QObject {
    Q_OBJECT

public:
    /*! \brief Creates a new DemoRunner
     *
     * This constructor creates a new DemoRunner instance.
     *
     * @param parent The standard QObject parent
     */
    explicit DemoRunner(QObject *parent = nullptr);

    // Standard destructor
    ~DemoRunner() override = default;

    /*! \brief Set pointer to QQmlApplicationEngine
     *
     *  To work, the instance needs to know the QQmlApplicationEngine that runs the GUI.q
     *
     *  @param engine Pointer to QQmlApplicationEngine
     */
    void setEngine(QQmlApplicationEngine* engine)
    {
        m_engine = engine;
    }

public slots:
    // Begin to remote-control the app
    void generateGooglePlayScreenshots();

    // Begin to remote-control the app
    void generateManualScreenshots();

signals:
    /*! \brief Emitted to indicate that the GUI should open the "Add Waypoint" dialog */
    void requestOpenFlightRouteAddWPDialog();

    /*! \brief Emitted to indicate that the GUI should open the "Aircraft" page */
    void requestOpenAircraftPage();

    /*! \brief Emitted to indicate that the GUI should open the "Nearby" page */
    void requestOpenNearbyPage();

    /*! \brief Emitted to indicate that the GUI should open the "Weather" page */
    void requestOpenWeatherPage();

    /*! \brief Emitted to indicate that the GUI should open the "Route & Wind" page */
    void requestOpenRoutePage();

    /*! \brief Emitted to indicate that the GUI return to the main page */
    void requestClosePages();

private:
    Q_DISABLE_COPY_MOVE(DemoRunner)

    QPointer<QQmlApplicationEngine> m_engine;
};

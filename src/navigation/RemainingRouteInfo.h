/***************************************************************************
 *   Copyright (C) 2022 by Stefan Kebekus                                  *
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

#include "geomaps/Waypoint.h"
#include "units/Distance.h"
#include "units/Time.h"


namespace Navigation {

class Navigator;

/*! \brief Info about remaining route
 *
 *  This class bundles information about the remainder of the present flight. This includes
 *  next waypoint, final waypoint, as well as distance, ETE and ETA.
 *
 *  Consumers of this class can expect the properties nextWP* to be valid if status equals OnRoute.
 *  The properties finalWP* need not be valid in this case. They are valid only if the next
 *  waypoint differs from the final waypoint of the route.
 */

class RemainingRouteInfo {
    Q_GADGET

    /*! \brief Comparison */
    friend bool operator==(const Navigation::RemainingRouteInfo&, const Navigation::RemainingRouteInfo&);
    friend class Navigation::Navigator;

public:
    /*! \brief Status */
    enum Status {
        /*! \brief No valid route has been set */
        NoRoute,

        /*! \brief Valid route has been set, but no position info available */
        PositionUnknown,

        /*! \brief Current position is further than Leg::nearThreshold nm away from route */
        OffRoute,

        /*! \brief Current position is closer than Leg::nearThreshold to destination */
        NearDestination,

        /*! \brief Currently travelling along the route */
        OnRoute
    };
    Q_ENUM(Status)

    //
    // PROPERTIES
    //

    /*! \brief Next waypoint in the route */
    Q_PROPERTY(GeoMaps::Waypoint nextWP MEMBER nextWP CONSTANT)

    /*! \brief Distance to next waypoint in the route */
    Q_PROPERTY(Units::Distance nextWP_DIST MEMBER nextWP_DIST CONSTANT)

    /*! \brief ETE for flight to next waypoint in the route */
    Q_PROPERTY(Units::Time nextWP_ETE MEMBER nextWP_ETE CONSTANT)

    /*! \brief ETA for flight to next waypoint in the route */
    Q_PROPERTY(QDateTime nextWP_ETA MEMBER nextWP_ETA CONSTANT)

    /*! \brief ETA for flight to next waypoint in the route, in UTC and as a string */
    Q_PROPERTY(QString nextWP_ETAAsUTCString READ nextWP_ETAAsUTCString CONSTANT)

    /*! \brief Final waypoint in the route */
    Q_PROPERTY(GeoMaps::Waypoint finalWP MEMBER finalWP CONSTANT)

    /*! \brief Distance to final waypoint in the route */
    Q_PROPERTY(Units::Distance finalWP_DIST MEMBER finalWP_DIST CONSTANT)

    /*! \brief ETE for flight to final waypoint in the route */
    Q_PROPERTY(Units::Time finalWP_ETE MEMBER finalWP_ETE CONSTANT)

    /*! \brief ETA for flight to final waypoint in the route */
    Q_PROPERTY(QDateTime finalWP_ETA MEMBER finalWP_ETA CONSTANT)

    /*! \brief ETA for flight to final waypoint in the route, in UTC and as a string */
    Q_PROPERTY(QString finalWP_ETAAsUTCString READ finalWP_ETAAsUTCString CONSTANT)

    /*! \brief Status */
    Q_PROPERTY(Navigation::RemainingRouteInfo::Status status MEMBER status CONSTANT)


    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property nextWP_ETAAsUTCString
     */
    QString nextWP_ETAAsUTCString() const {
        if (nextWP_ETE.isFinite()) {
            return nextWP_ETA.toString("H:mm");
        }
        return "-:--";
    }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property nextWP_ETAAsUTCString
     */
    QString finalWP_ETAAsUTCString() const {
        if (finalWP_ETE.isFinite()) {
            return finalWP_ETA.toString("H:mm");
        }
        return "-:--";
    }

private:
    Status status {NoRoute};

    GeoMaps::Waypoint nextWP {};
    Units::Distance nextWP_DIST {};
    Units::Time nextWP_ETE {};
    QDateTime nextWP_ETA {};

    GeoMaps::Waypoint finalWP {};
    Units::Distance finalWP_DIST {};
    Units::Time finalWP_ETE {};
    QDateTime finalWP_ETA {};
};


/*! \brief Comparison */
bool operator==(const Navigation::RemainingRouteInfo&, const Navigation::RemainingRouteInfo&);


}

// Declare meta types
Q_DECLARE_METATYPE(Navigation::RemainingRouteInfo)

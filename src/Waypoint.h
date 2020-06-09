/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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
#include <QMap>
#include <QJsonObject>
#include <QVariant>


/*! \brief Waypoint, such as an airfield, a navaid station or a reporting point.
  
  This class represents a waypoint feature of a GeoJSON file, as described in
  [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
  In essence, it contains two bits of data:
  
  - The coordinate of the waypoint.
  
  - A QMap<QString, QVariant> that represents the waypoint properties found in
    the GeoJSON file.
*/

class Waypoint : public QObject
{
    Q_OBJECT

public:
    /*! \brief Constructs an invalid way point

    @param parent The standard QObject parent pointer
  */
    explicit Waypoint(QObject *parent = nullptr);

    /*! \brief Constructs a waypoint by copying data from another waypoint

    @param other Waypoint whose data is copied
    
    @param parent The standard QObject parent pointer
  */
    explicit Waypoint(const Waypoint &other, QObject *parent = nullptr);

    /*! \brief Constructs a way point from a coordinate

    The waypoint constructed is generic. It will have property TYP="WP" and
    property CAT="WP".

    @param coordinate Geographical position of the waypoint
    
    @param parent The standard QObject parent pointer
  */
    explicit Waypoint(const QGeoCoordinate& coordinate, QObject *parent = nullptr);

    /*! \brief Constructs a waypoint from a GeoJSON object

    This method constructs a Waypoint from a GeoJSON description. The GeoJSON
    file specification is found
    [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
    
    @param geoJSONObject GeoJSON Object that describes the waypoint
    
    @param parent The standard QObject parent pointer
  */
    explicit Waypoint(const QJsonObject &geoJSONObject, QObject *parent = nullptr);

    // Standard destructor
    ~Waypoint() = default;

    /*! \brief Check property existence

      Recall that this class represents a waypoint feature of a GeoJSON file, as
      described in
      [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
      This method allows to check for the existence of individual members described in this
      file.

      @param name The name of the member. This is a string such as "CAT", "TYP",
      "NAM", etc

      @returns True is property exists
     */
    Q_INVOKABLE bool contains(const QString& name) const { return _properties.contains(name); }

    /*! \brief Coordinate of the waypoint

    If the coordinate is invalid, this waypoint should not be used
  */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate CONSTANT)

    /*! \brief Getter function for property with the same name

    @returns Property coordinate
  */
    QGeoCoordinate coordinate() const { return _coordinate; }

    /*! \brief Extended name of the Waypoints

    This method returns a string of the form "Karlsruhe (DVOR-DME)"
  */
    Q_PROPERTY(QString extendedName READ extendedName CONSTANT)

    /*! \brief Getter function for property with the same name

    @returns Property extendedName
  */
    Q_INVOKABLE QString extendedName() const;

    /*! \brief Retrieve member by name

    Recall that this class represents a waypoint feature of a GeoJSON file, as
    described in
    [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
    This method allows to retrieve the individual members described in this
    file.
    
    @param name The name of the member. This is a string such as "CAT", "TYP",
    "NAM", etc

    @returns Value of the member
   */
    Q_INVOKABLE QVariant get(const QString& name) const { return _properties.value(name); }

    /* \brief Validity

    This is a simple shortcut for coordinate().isValid
  */
    bool isValid() const { return _coordinate.isValid(); }

    /* \brief Name of the waypoint as rich text */
    Q_PROPERTY(QString richTextName READ richTextName CONSTANT)

    /*! \brief Getter function for property with the same name

    @returns Property richTextName
  */
    QString richTextName() const;

    /* \brief Description of waypoint details as an HTML table */
    Q_PROPERTY(QList<QString> tabularDescription READ tabularDescription CONSTANT)

    /*! \brief Getter function for property with the same name

    @returns Property tabularDescription
  */
    QList<QString> tabularDescription() const;

    /*! \brief Serialization to GeoJSON object

    This method serialises the waypoint route as a GeoJSON object. The object
    conforms to the specification outlined
    [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
    The waypoint can be restored with the obvious constructor
    
    @returns QJsonObject describing the waypoint
  */
    QJsonObject toJSON() const;

    /*! \brief Description of the way from a given position to the waypoint

    @param position Position
    @param useMetricUnits if true, render distance in km, else in NM

    @returns a string of the form "65.2 NM • TC 276°"
  */
    Q_INVOKABLE QString wayFrom(const QGeoCoordinate& position, bool useMetricUnits) const;

private:
    Q_DISABLE_COPY_MOVE(Waypoint)

    QGeoCoordinate _coordinate;
    QMultiMap<QString, QVariant> _properties;
};

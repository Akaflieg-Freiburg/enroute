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


#include <qhttpengine/handler.h>


namespace GeoMaps {


/*! \brief Implementation of QHttpEngine::Handler that serves GeoJSON from GeoMapProvider
 *
 *  This class serves the GeoJSON provided by GeoMapProvider at the URL path "aviationData.geojson".
 */

class GeoJSONHandler : public QHttpEngine::Handler
{
  Q_OBJECT
  
public:
  /*! \brief Create a new  GeoJSON handler
   *
   *  This constructor sets up a new GeoJSON handler.
   *
   *  @param parent The standard QObject parent
   */
  explicit GeoJSONHandler(QObject* parent = nullptr);
  
protected:
  /*
   * @brief Reimplementation of
   * [Handler::process()](QHttpEngine::Handler::process)
   */
  void process(QHttpEngine::Socket* socket, const QString& path) override;
  
private:
  Q_DISABLE_COPY_MOVE(GeoJSONHandler)
};

} // namespace GeoMaps

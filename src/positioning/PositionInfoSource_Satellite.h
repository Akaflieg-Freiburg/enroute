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

#include <QGeoPositionInfoSource>
#include <QLocale>
#include <QTimer>

#include "positioning/PositionInfoSource_Abstract.h"
#include "GlobalSettings.h"
#include "positioning/Geoid.h"
#include "positioning/PositionInfo.h"
#include "units/Distance.h"

namespace Positioning {

/*! \brief Satellite Navigator

  This class is a thin wrapper around QGeoPositionInfoSource.  The main
  differences to QGeoPositionInfoSource are the following.

  - This class the data in formats suitable for aviation purposes.

  - This class has a well-defined timeout if no data has been received for more
    than two minutes.

  - The signal statusChanged() reliably lets you know if the status changes. The
    QGeoPositionInfoSource, in contrast, only reports errors. It does not report
    explicitly when error conditions are lifted.

  Once constructed, the health status of the satellite navigation subsystem can
  be queried using the status property. If all is well, the class receives
  regular satellite navigation data packets (aka 'fixes'), in the form a
  QGeoPositionInfo object, and the signal 'update' is emitted.  The
  QGeoPositionInfo is considered valid for one minute, and can be accessed via
  the method lastFix(), or via a multitude of properties that present the data
  in formats suitable for GUI applications.  If no new PositionInfoSource_Satellite fixes come in for
  one minute, the QGeoPositionInfo will cleared, and the status is set to
  Timeout until new data arrives.

  There exists one static instance of this class, which can be accessed via the
  method globalInstance().  No other instance of this class should be used.

  The methods in this class are reentrant, but not thread safe.
*/

class PositionInfoSource_Satellite : public PositionInfoSource_Abstract
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
   *
   * @param parent The standard QObject parent pointer
   */
    explicit PositionInfoSource_Satellite(QObject *parent = nullptr);

    /*! \brief Standard deconstructor */
    ~PositionInfoSource_Satellite() override;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property statusAsString
     */
    QString statusString() const;

private slots:
    // Connected to source, in order to receive error information
    void error(QGeoPositionInfoSource::Error newSourceStatus);

    void onPositionUpdated_Sat(const QGeoPositionInfo &info);

private:
    Q_DISABLE_COPY_MOVE(PositionInfoSource_Satellite)

    Positioning::Geoid* _geoid {nullptr};

    // Set according to the status of *source. We need to replicate the
    // information stored in source because the status of the source can change
    // from "error" to "ok" without notification. This data field is then used to
    // check if something changed, and to emit the signal "statusUpdate" when
    // appropriate.
    QGeoPositionInfoSource::Error sourceStatus {QGeoPositionInfoSource::AccessError};

    QGeoPositionInfoSource *source;
};

}

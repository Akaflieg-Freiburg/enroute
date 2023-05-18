/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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

#include "notification/Notification.h"

namespace Notifications {

/*! \brief Notification for available map & data updates
 *
 *  This implementation of Notifications::Notification sets proper button texts,
 *  reacts to button clicks and deletes itself in flight and whenever a map and
 *  data update starts.
 */


class Notification_DataUpdateAvailable : public Notification
{
    Q_OBJECT

public:
    //
    // Constructors and destructors
    //

    /*! \brief Standard constructor
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit Notification_DataUpdateAvailable(QObject* parent = nullptr);

    // No default constructor, always want a parent
    explicit Notification_DataUpdateAvailable() = delete;

    /*! \brief Standard destructor */
    ~Notification_DataUpdateAvailable() = default;

public slots:
    /*! \brief Reimplemented from Notifications::Notification */
    virtual void onButton1Clicked() override;

private slots:
    // Check if this notification is still useful and delete it if not.
    void update();
};

} // namespace Notifications

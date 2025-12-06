/***************************************************************************
 *   Copyright (C) 2019-2025 by Stefan Kebekus                             *
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

#include "PlatformAdaptor_Abstract.h"

// Forward declarations for IOKit types
class IONotificationPort;

namespace Platform {

/*! \brief Template implementation of PlatformAdaptor */

class PlatformAdaptor : public PlatformAdaptor_Abstract
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    /*! \brief Standard constructor
     *
     * @param parent Standard QObject parent pointer
     */
    explicit PlatformAdaptor(QObject *parent = nullptr);

    // No default constructor, important for QML singleton
    explicit PlatformAdaptor() = delete;

    // factory function for QML singleton
    static Platform::PlatformAdaptor* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::platformAdaptor();
    }

    ~PlatformAdaptor() override;

private:
    Q_DISABLE_COPY_MOVE(PlatformAdaptor)

    // Members used to track changes of serial port devices
    IONotificationPort* m_notifyPort {nullptr};
    unsigned int m_addedIterator {0};
    unsigned int m_removedIterator {0};
    void setupIOKitNotifications();
    static void deviceChanged(void *refCon, unsigned int iterator);
};

} // namespace Platform

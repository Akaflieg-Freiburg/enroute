/***************************************************************************
 *   Copyright (C) 2021-2022 by Stefan Kebekus                             *
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

#include "platform/Notifier_Abstract.h"

namespace Platform {

/*! \brief Template implementation of Notifier */

class Notifier: public Notifier_Abstract
{
    Q_OBJECT

public:
    // Constructor
    explicit Notifier(QObject* parent = nullptr);

    // Destructor
    ~Notifier() = default;


public slots:
    // Implementation of pure virtual function
    Q_INVOKABLE void hideNotification(Platform::Notifier_Abstract::NotificationTypes notificationType) override;

    // Implementation of pure virtual function
    void showNotification(Platform::Notifier_Abstract::NotificationTypes notificationType, const QString& text, const QString& longText) override;


protected:
    /*! \brief Implements virtual method from GlobalObject */
    void deferredInitialization() override {};


private:
    Q_DISABLE_COPY_MOVE(Notifier)
};

} // namespace Platform

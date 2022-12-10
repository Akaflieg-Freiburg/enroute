/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

#include <QQmlEngine>

#include "platform/SafeInsets_Abstract.h"

namespace Platform {

/*! \brief Implementation of SafeInsets for Android devices */


class SafeInsets : public Platform::SafeInsets_Abstract
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Properties need to be repeated, or else the Qt CMake macros cannot find them.
    Q_PROPERTY(double bottom READ bottom NOTIFY bottomChanged)
    Q_PROPERTY(double left READ left NOTIFY leftChanged)
    Q_PROPERTY(double right READ right NOTIFY rightChanged)
    Q_PROPERTY(double top READ top NOTIFY topChanged)


public:
    /*! \brief Standard constructor
     *
     * @param parent Standard QObject parent pointer
     */
    explicit SafeInsets(QObject *parent = nullptr);

    ~SafeInsets() override = default;


private slots:
    /*! \brief Implements virtual method from SafeInsets_Abstract */
    void updateSafeInsets();

private:
    Q_DISABLE_COPY_MOVE(SafeInsets)
};

} // namespace Platform

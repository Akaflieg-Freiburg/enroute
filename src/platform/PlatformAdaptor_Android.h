/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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

#include <QObject>
#include <QTimer>
#include <QtGlobal>

#include "platform/PlatformAdaptor_Abstract.h"

namespace Platform {

/*! \brief Implementation of PlatformAdaptor for Android devices */


class PlatformAdaptor : public Platform::PlatformAdaptor_Abstract
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent Standard QObject parent pointer
     */
    explicit PlatformAdaptor(QObject *parent = nullptr);

    ~PlatformAdaptor() override = default;


    //
    // Methods
    //

    /*! \brief Implements a pure virtual method from PlatformAdaptor_Abstract
     *
     *  @returns see PlatformAdaptor_Abstract
     */
    QString currentSSID() override;

    /*! \brief Implements a pure virtual method from PlatformAdaptor_Abstract */
    void disableScreenSaver() override;

    /*! \brief Implements s pure virtual method from PlatformAdaptor_Abstract
     *
     *  @param lock see PlatformAdaptor_Abstract
     */
    void lockWifi(bool lock) override;

    /*! \brief Re-implements a virtual method from PlatformAdaptor_Abstract
     *
     * @param coordinate Location whose sat view should be shown.
     */
    void openSatView(const QGeoCoordinate& coordinate) override;

    /*! \brief Information about the system, in HTML format
     *
     * @returns Info string
     */
    QString systemInfo() override;

    /*! \brief Implements a pure virtual method from PlatformAdaptor_Abstract */
    void vibrateBrief() override;

    /*! \brief Implements a pure virtual method from PlatformAdaptor_Abstract */
    void vibrateLong() override;

    /*! \brief Implements a pure virtual method from PlatformAdaptor_Abstract */
    void onGUISetupCompleted() override;


protected:
    /*! \brief Re-implements a virtual method from PlatformAdaptor_Abstract */
    void deferredInitialization() override;

private:
    Q_DISABLE_COPY_MOVE(PlatformAdaptor)

    bool splashScreenHidden {false};
};

} // namespace Platform

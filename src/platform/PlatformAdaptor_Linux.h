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

#include <QtGlobal>
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)

#include <QObject>
#include <QTimer>

#include "PlatformAdaptor_Abstract.h"

namespace Platform {

/*! \brief Implementation of PlatformAdaptor for Linux desktop devices */

class PlatformAdaptor : public PlatformAdaptor_Abstract
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

    /*! \brief Implements pure virtual method from PlatformAdaptor_Abstract */
    Q_INVOKABLE QString currentSSID() override;

    /*! \brief Implements pure virtual method from PlatformAdaptor_Abstract */
    void disableScreenSaver() override;

    /*! \brief Implements pure virtual method from PlatformAdaptor_Abstract */
    Q_INVOKABLE bool hasMissingPermissions() override;

    /*! \brief Implements pure virtual method from PlatformAdaptor_Abstract */
    Q_INVOKABLE void importContent() override;

    /*! \brief Implements pure virtual method from PlatformAdaptor_Abstract */
    Q_INVOKABLE void lockWifi(bool lock) override;

    /*! \brief Implements pure virtual method from PlatformAdaptor_Abstract */
    void requestPermissionsSync() override;

    /*! \brief Implements pure virtual method from PlatformAdaptor_Abstract */
    Q_INVOKABLE QString shareContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) override;

    /*! \brief Implements pure virtual method from PlatformAdaptor_Abstract */
    Q_INVOKABLE void vibrateBrief() override;

    /*! \brief Implements pure virtual method from PlatformAdaptor_Abstract */
    Q_INVOKABLE QString viewContent(const QByteArray& content, const QString& mimeType, const QString& fileNameTemplate) override;

public slots:
    /*! \brief Implements pure virtual method from PlatformAdaptor_Abstract */
    Q_INVOKABLE void onGUISetupCompleted() override;

private:
    Q_DISABLE_COPY_MOVE(PlatformAdaptor)
};

} // namespace Platform

#endif // defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)

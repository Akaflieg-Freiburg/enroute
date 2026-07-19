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

#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QWindow>
#include <QtGlobal>

#include "platform/PlatformAdaptor_Abstract.h"

namespace Platform {

/*! \brief Implementation of PlatformAdaptor for Android devices */


class PlatformAdaptor : public Platform::PlatformAdaptor_Abstract
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

    ~PlatformAdaptor() override = default;


    //
    // Getter Methods
    //

    /*! \brief Re-implements a virtual method from PlatformAdaptor_Abstract
     *
     *  @returns Property imeBottomInset
     */
    [[nodiscard]] double imeBottomInset() const override { return m_imeBottomInset; }


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
     *  @returns QVector with connection infos
     */
    QVector<Traffic::ConnectionInfo> serialPortConnectionInfos() override;

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

private slots:
    // Poll the window inset occupied by the virtual keyboard and update the
    // property imeBottomInset. Connected to the QInputMethod signals in the
    // constructor.
    void updateImeBottomInset();

private:
    Q_DISABLE_COPY_MOVE(PlatformAdaptor)

    bool splashScreenHidden {false};
    double m_imeBottomInset {0.0};
    QPointer<QWindow> m_watchedWindow;
};

} // namespace Platform

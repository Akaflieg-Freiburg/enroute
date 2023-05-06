/***************************************************************************
 *   Copyright (C) 2022 by Stefan Kebekus                                  *
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

#include "ios/ObjCAdapter.h"

#include <QGuiApplication>
#include <QJniObject>
#include <QScreen>
#include <QTimer>
#include <chrono>

#include "platform/SafeInsets_iOS.h"

using namespace std::chrono_literals;

// This class is a QML singleton. For cooperation with JNICALL
// Java_de_akaflieg_1freiburg_enroute_MobileAdaptor_onWindowSizeChanged,
// this pointer is inizialized in the constructor of the SafeInsets object.
QPointer<Platform::SafeInsets> safeInsetsinstance = nullptr;

Platform::SafeInsets::SafeInsets(QObject* parent)
    : Platform::SafeInsets_Abstract(parent)
{

    connect(QGuiApplication::primaryScreen(), &QScreen::orientationChanged, this, &SafeInsets::updateSafeInsets);
    connect(QGuiApplication::inputMethod(), &QInputMethod::keyboardRectangleChanged, this, &SafeInsets::updateSafeInsets);
    connect(QGuiApplication::inputMethod(), &QInputMethod::visibleChanged, this, &SafeInsets::updateSafeInsets);

    auto* timer = new QTimer(this);
    timer->setInterval(50ms);
    timer->setSingleShot(true);
    connect(QGuiApplication::primaryScreen(), &QScreen::orientationChanged, timer, qOverload<>(&QTimer::start));
    connect(QGuiApplication::inputMethod(), &QInputMethod::keyboardRectangleChanged, timer, qOverload<>(&QTimer::start));
    connect(QGuiApplication::inputMethod(), &QInputMethod::visibleChanged, timer, qOverload<>(&QTimer::start));

    connect(timer, &QTimer::timeout, this, &SafeInsets::updateSafeInsets);

    updateSafeInsets();

    // Initialize global pointer
    safeInsetsinstance = this;
    m_top = ObjCAdapter::safeAreaTopInset();
    m_bottom = ObjCAdapter::safeAreaBottomInset();
    m_left = ObjCAdapter::safeAreaLeftInset();
    m_right = ObjCAdapter::safeAreaRightInset();
}

void Platform::SafeInsets::updateSafeInsets()
{
    auto bottom {m_bottom};
    auto left {m_left};
    auto right {m_right};
    auto top {m_top};
    auto wHeight {m_wHeight};
    auto wWidth {m_wWidth};

    top = ObjCAdapter::safeAreaTopInset();
    bottom = ObjCAdapter::safeAreaBottomInset();
    left = ObjCAdapter::safeAreaLeftInset();
    right = ObjCAdapter::safeAreaRightInset();

    // Update properties and emit notification signals
    if (bottom != m_bottom)
    {
        m_bottom = bottom;
        emit bottomChanged();
    }
    if (left != m_left)
    {
        m_left = left;
        emit leftChanged();
    }
    if (right != m_right)
    {
        m_right = right;
        emit rightChanged();
    }
    if (top != m_top)
    {
        m_top = top;
        emit topChanged();
    }
    if (wHeight != m_wHeight)
    {
        m_wHeight = wHeight;
        emit wHeightChanged();
    }
    if (wWidth != m_wWidth)
    {
        m_wWidth = wWidth;
        emit wWidthChanged();
    }
}

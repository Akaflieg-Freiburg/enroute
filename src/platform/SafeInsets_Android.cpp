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

#include <QGuiApplication>
#include <QJniObject>
#include <QScreen>
#include <QTimer>

#include "platform/SafeInsets_Android.h"


Platform::SafeInsets::SafeInsets(QObject* parent)
    : Platform::SafeInsets_Abstract(parent)
{
    connect(QGuiApplication::primaryScreen(), &QScreen::orientationChanged, this, &SafeInsets::updateSafeInsets);
    connect(QGuiApplication::inputMethod(), &QInputMethod::keyboardRectangleChanged, this, &SafeInsets::updateSafeInsets);
    connect(QGuiApplication::inputMethod(), &QInputMethod::visibleChanged, this, &SafeInsets::updateSafeInsets);

    // The signals above are not always emitted reliably, perhaps because of
    // a delay caused by the virtual keyboard animation. To be on the safe side,
    // we re-check the safe insets again after one second.
    auto* timer = new QTimer(this);
    timer->setInterval(1000);
    timer->setSingleShot(true);
    connect(QGuiApplication::primaryScreen(), &QScreen::orientationChanged, timer, qOverload<>(&QTimer::start));
    connect(QGuiApplication::inputMethod(), &QInputMethod::keyboardRectangleChanged, timer, qOverload<>(&QTimer::start));
    connect(QGuiApplication::inputMethod(), &QInputMethod::visibleChanged, timer, qOverload<>(&QTimer::start));

    connect(timer, &QTimer::timeout, this, &SafeInsets::updateSafeInsets);

    updateSafeInsets();
}


void Platform::SafeInsets::updateSafeInsets()
{
    auto bottom {_bottom};
    auto left {_left};
    auto right {_right};
    auto top {_top};

    auto devicePixelRatio = QGuiApplication::primaryScreen()->devicePixelRatio();
    if ( qIsFinite(devicePixelRatio) && (devicePixelRatio > 0.0))
    {
        double inset = 0.0;

        inset = static_cast<double>(QJniObject::callStaticMethod<jdouble>("de/akaflieg_freiburg/enroute/MobileAdaptor", "safeInsetBottom"));
        if ( qIsFinite(inset) && (inset >= 0.0) )
        {
            bottom = inset/devicePixelRatio;
        }

        inset = static_cast<double>(QJniObject::callStaticMethod<jdouble>("de/akaflieg_freiburg/enroute/MobileAdaptor", "safeInsetLeft"));
        if ( qIsFinite(inset) && (inset >= 0.0) )
        {
            left = inset/devicePixelRatio;
        }

        inset = static_cast<double>(QJniObject::callStaticMethod<jdouble>("de/akaflieg_freiburg/enroute/MobileAdaptor", "safeInsetRight"));
        if ( qIsFinite(inset) && (inset >= 0.0) )
        {
            right = inset/devicePixelRatio;
        }

        inset = static_cast<double>(QJniObject::callStaticMethod<jdouble>("de/akaflieg_freiburg/enroute/MobileAdaptor", "safeInsetTop"));
        if ( qIsFinite(inset) && (inset >= 0.0) )
        {
            top = inset/devicePixelRatio;
        }
    }

    // Update properties and emit notification signals
    if (bottom != _bottom)
    {
        _bottom = bottom;
        emit bottomChanged();
    }
    if (left != _left)
    {
        _left = left;
        emit leftChanged();
    }
    if (right != _right)
    {
        _right = right;
        emit rightChanged();
    }
    if (top != _top)
    {
        _top = top;
        emit topChanged();
    }
}

/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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
#include <chrono>

#include "platform/SafeInsets_Android.h"

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

    // The signals above are not always emitted reliably, perhaps because of
    // a delay caused by the virtual keyboard animation. To be on the safe side,
    // we re-check the safe insets again after one second.
    auto* timer = new QTimer(this);
    timer->setInterval(1s);
    timer->setSingleShot(true);
    connect(QGuiApplication::primaryScreen(), &QScreen::orientationChanged, timer, qOverload<>(&QTimer::start));
    connect(QGuiApplication::inputMethod(), &QInputMethod::keyboardRectangleChanged, timer, qOverload<>(&QTimer::start));
    connect(QGuiApplication::inputMethod(), &QInputMethod::visibleChanged, timer, qOverload<>(&QTimer::start));

    connect(timer, &QTimer::timeout, this, &SafeInsets::updateSafeInsets);

    updateSafeInsets();

    // Initialize global pointer
    safeInsetsinstance = this;
}

void Platform::SafeInsets::updateSafeInsets()
{
    auto bottom {m_bottom};
    auto left {m_left};
    auto right {m_right};
    auto top {m_top};
    auto wHeight {m_wHeight};
    auto wWidth {m_wWidth};

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

        inset = static_cast<double>(QJniObject::callStaticMethod<jdouble>("de/akaflieg_freiburg/enroute/MobileAdaptor", "windowHeight"));
        if ( qIsFinite(inset) && (inset >= 0.0) )
        {
            wHeight = inset/devicePixelRatio;
        }

        inset = static_cast<double>(QJniObject::callStaticMethod<jdouble>("de/akaflieg_freiburg/enroute/MobileAdaptor", "windowWidth"));
        if ( qIsFinite(inset) && (inset >= 0.0) )
        {
            wWidth = inset/devicePixelRatio;
        }

    }

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


//
// C Methods
//


extern "C" {

JNIEXPORT void JNICALL Java_de_akaflieg_1freiburg_enroute_MobileAdaptor_onWindowSizeChanged(JNIEnv* /*unused*/, jobject /*unused*/)
{
    if (!safeInsetsinstance.isNull())
    {
        QTimer::singleShot(0, safeInsetsinstance, &Platform::SafeInsets::updateSafeInsets);
    }
}

}

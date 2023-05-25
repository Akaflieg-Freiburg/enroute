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

#pragma once

#include <QQmlEngine>
#include <QQuickItem>

#include "GlobalObject.h"


namespace Platform {

/*! \brief Interface to platform-specific functionality
 *
 * This pure virtual class is an interface to capabilities of mobile devices
 * (e.g. vibration) that need platform-specific code to operate.  The files
 * PlatformAdaptor_XXX.(h|cpp) implement a child class PlatformAdaptor that
 * contains the actual implementation.
 *
 * Child classes need to implement all pure virtual functions, and need to
 * provide the following additional functionality.
 *
 * - Child classes need to monitor the Wi-Fi network and emit the signal
 *   wifiConnected() whenever a new Wi-Fi connection becomes available. The app
 *   will then check if a traffic data receiver is active in the network.
 *
 * - If supported by the platform, child classes need to react to requests by
 *   the platform to open a file (e.g. a GeoJSON file containing a flight
 *   route). Once a request is received, the method processFileRequest() should
 *   be called.
 */

class PlatformAdaptor_Abstract : public GlobalObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(PlatformAdaptor)
    QML_SINGLETON

public:
    /*! \brief Standard constructor
     *
     * @param parent Standard QObject parent pointer
    */
    explicit PlatformAdaptor_Abstract(QObject* parent = nullptr);

    // No default constructor, important for QML singleton
    explicit PlatformAdaptor_Abstract() = delete;

    ~PlatformAdaptor_Abstract() override = default;

    // factory function for QML singleton
    static Platform::PlatformAdaptor_Abstract* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::platformAdaptor();
    }


    //
    // Methods
    //

    /*! \brief Checks if all required permissions have been granted
     *
     * Depending on the platform, the app needs to ask for permissions to
     * operate properly. This method can be used to check if all permissions
     * have been granted. It returns an empty string if all permissions are
     * there, and a human-readable, translated explanation of missing
     * permissions otherwise.
     *
     * @returns Empty string or explanation
    */
    Q_INVOKABLE virtual QString checkPermissions() = 0;

    /*! \brief SSID of current Wi-Fi network
     *
     * @returns The SSID of the current Wi-Fi networks, an empty string if the
     * device is not connected to a Wi-Fi or a generic string if the SSID cannot
     * be determined.
     */
    Q_INVOKABLE virtual QString currentSSID() = 0;

    /*! \brief Disable the screen saver
     *
     * On platforms that support this, this method shall disable to screen
     * saver, so that the display does not switch off automatically.  This is
     * meant to ensure that the display remains on while the app is in use (e.g.
     * while the pilot is following a non-standard traffic pattern).
     */
    Q_INVOKABLE virtual void disableScreenSaver() = 0;

    /*! \brief Lock connection to Wi-Fi network
     *
     * If supported by the platform, this method is supposed to lock the current
     * Wi-Fi connection, that is, to prevent the device from dropping the
     * connection or shutting down the Wi-Fi interface.
     *
     * The app calls that method after connecting to a traffic data receiver, in
     * order to ensure that traffic data is continuously received.
     *
     * @param lock If true, then lock the network. If false, then release the
     * lock.
     */
    Q_INVOKABLE virtual void lockWifi(bool lock) = 0;

    /*! \brief Request permissions
     *
     * On some platforms, the app needs to ask for permissions to perform
     * certain functions (e.g. receive location, query Wi-Fi status, …). This
     * method is called before the GUI is set up and must request ALL
     * permissions that the app will ever use. The method is meant to run
     * synchroneously and shall return only once all permissions have been
     * granted (or not). 
     */
    Q_INVOKABLE virtual void requestPermissionsSync() = 0;

    /*! \brief Workaround for QTBUG-80790
     *
     *  This method is empty except on iOS, where it installs a special event
     *  filter for a QQuickItem. The event filter avoids problematic behavior
     *  where the virtual keyboard pushes up the whole qml page. Details for
     *  this problem and the present workaround are described here.
     *
     *  https://stackoverflow.com/questions/34716462/ios-sometimes-keyboard-pushes-up-the-whole-qml-page
     *
     *  https://bugreports.qt.io/browse/QTBUG-80790
     *
     *  @param item QQuickItem where the event filter is to be intalled.
     */
    Q_INVOKABLE virtual void setupInputMethodEventFilter(QQuickItem* item)
    {
        Q_UNUSED(item)
    }

    /*! \brief Information about the system, in HTML format
     *
     * @returns Info string
     */
    Q_INVOKABLE virtual QString systemInfo();

    /*! \brief Make the device briefly vibrate
     *
     *  On platforms that support this, make the device briefly vibrate if
     *  haptic feedback is enabled in the system settings.
     */
    Q_INVOKABLE virtual void vibrateBrief() = 0;

    /*! \brief Make the device vibrate for a longer period
     *
     *  On platforms that support this, make the device vibrate for a longer time,
     *  if haptic feedback is enabled in the system settings. This is used to
     *  when showing notifications.
     */
    Q_INVOKABLE virtual void vibrateLong() = 0;

    /*! \brief Language code that is to be used in the GUI
     *
     *  This method returns a two-letter language code (such as "en" or "de") that
     *  describes the language that is to be used in the GUI.
     */
    Q_INVOKABLE virtual QString language();

public slots:
    /*! \brief Signal handler: GUI setup completed
     *
     *  This method is called as soon as the GUI setup is completed. On Android,
     *  this method is used to hide the splash screen and to show the app.
     *
     *  The implementation should guarentee that nothing bad happens if the
     *  method is called more than once.
     */
    virtual void onGUISetupCompleted() = 0;


signals:
    /*! \brief Emitted when a new WiFi connections becomes available
     *
     *  This signal is emitted when a new WiFi connection becomes available.
     */
    void wifiConnected();

private:
    Q_DISABLE_COPY_MOVE(PlatformAdaptor_Abstract)

};

} // namespace Platform

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

public:
    /*! \brief Standard constructor
     *
     * @param parent Standard QObject parent pointer
    */
    explicit PlatformAdaptor_Abstract(QObject *parent = nullptr);

    ~PlatformAdaptor_Abstract() override = default;


    //
    // Properties
    //

    /*! \brief Safe inset at bottom of screen, so as to avoid system status bars and display cutouts */
    Q_PROPERTY(double safeInsetBottom READ safeInsetBottom NOTIFY safeInsetBottomChanged)

    /*! \brief Safe inset at left of screen, so as to avoid system status bars and display cutouts */
    Q_PROPERTY(double safeInsetLeft READ safeInsetLeft NOTIFY safeInsetLeftChanged)

    /*! \brief Safe inset at right of screen, so as to avoid system status bars and display cutouts */
    Q_PROPERTY(double safeInsetRight READ safeInsetRight NOTIFY safeInsetRightChanged)

    /*! \brief Safe inset at top of screen, so as to avoid system status bars and display cutouts */
    Q_PROPERTY(double safeInsetTop READ safeInsetTop NOTIFY safeInsetTopChanged)



    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property safeInsetBottom
     */
    double safeInsetBottom() const {return _safeInsetBottom;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property safeInsetLeft
     */
    double safeInsetLeft() const {return _safeInsetLeft;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property safeInsetRight
     */
    double safeInsetRight() const {return _safeInsetRight;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property safeInsetTop
     */
    double safeInsetTop() const {return _safeInsetTop;}


    //
    // Methods

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
    virtual void disableScreenSaver() = 0;

    /*! \brief Checks if all required permissions have been granted
     *
     * Depending on the platform, the app needs to ask for permissions to
     * operate properly. This method can be used to check if all permissions
     * have been granted.
     *
     * @returns 'True' if all required permissions have been granted.
    */
    Q_INVOKABLE virtual bool hasRequiredPermissions() = 0;

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
    virtual void lockWifi(bool lock) = 0;

    /*! \brief Request permissions
     *
     * On some platforms, the app needs to ask for permissions to perform
     * certain functions (e.g. receive location, query Wi-Fi status, …). This
     * method is called before the GUI is set up and must request ALL
     * permissions that the app will ever use. The method is meant to run
     * synchroneously and shall return only once all permissions have been
     * granted (or not). 
     */
    virtual void requestPermissionsSync() = 0;

    /*! \brief Make the device briefly vibrate
     *
     * On platforms that support this, make the device briefly vibrate if haptic
     * feedback is enabled in the system settings.
     */
    Q_INVOKABLE virtual void vibrateBrief() = 0;


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
    /*! \brief Notifier signal */
    void safeInsetBottomChanged();

    /*! \brief Notifier signal */
    void safeInsetLeftChanged();

    /*! \brief Notifier signal */
    void safeInsetRightChanged();

    /*! \brief Notifier signal */
    void safeInsetTopChanged();

    /*! \brief Emitted when a new WiFi connections becomes available
     *
     *  This signal is emitted when a new WiFi connection becomes available.
     */
    void wifiConnected();

private:
    Q_DISABLE_COPY_MOVE(PlatformAdaptor_Abstract)

    // Computes the values of the member variables _safeInset* and emits the notification signals as appropriate.
    void updateSafeInsets();

    // Member variables
    double _safeInsetBottom {0.0};
    double _safeInsetLeft {0.0};
    double _safeInsetRight {0.0};
    double _safeInsetTop {0.0};
};

} // namespace Platform

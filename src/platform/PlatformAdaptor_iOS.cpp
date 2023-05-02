/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#include "platform/PlatformAdaptor_iOS.h"
#include "ios/ObjCAdapter.h"

// This is a template file without actual implementation.


Platform::PlatformAdaptor::PlatformAdaptor(QObject *parent)
    : PlatformAdaptor_Abstract(parent)
{
    // Standard constructor. Recall that the constructor must not call virtual functions.
    // If you need virtual functions, use the methode deferredInitialization below.
#warning Not implemented
}


/*void Platform::PlatformAdaptor::deferredInitialization()
{
    // This method is called immediately after the instance has been constructed.
    // It can be used to implement initialization that calls virtual methods.
#warning Not implemented
}
*/


//
// Methods
//

auto Platform::PlatformAdaptor::currentSSID() -> QString
{
    // This method must return the SSID of the current Wi-Fi connection, or an empty string
    // if there is no Wi-Fi or if the SSID cannot be determined

#warning Not implemented
    return {};
}


void Platform::PlatformAdaptor::disableScreenSaver()
{
    // If supported by the platform, this method shall disable the screensaver.
    // Experience has shown that the screensaver will typically switch the display off when the pilot it trying to follow
    // a complicated traffic pattern or control zone procedure.
    ObjCAdapter::disableScreenSaver();
}



void Platform::PlatformAdaptor::lockWifi(bool lock)
{
    // If supported by the platform, this method shall (un)lock the Wi-Fi connection.
    // Some platforms (such as Android) will switch Wi-Fi off after a few minutes of idle time, in order to save battery.
    // To ensure that the app continuously receives traffic data, this function is calls whenever a
    // connection to a traffic data receiver has been established.
#warning Not implemented
}


void Platform::PlatformAdaptor::onGUISetupCompleted()
{
    // This method is called once the GUI has been set up. The Android-specific implementes uses this method to hide the
    // splash screen.
#warning Not implemented
}

QString Platform::PlatformAdaptor::checkPermissions()
{
    // This method is called once the GUI has been set up. The Android-specific implementes uses this method to hide the
    // splash screen.
    QString string = "";
    if (!ObjCAdapter::hasLocationPermission()) {
        string += "Location";
    }
    if (!ObjCAdapter::hasNotificationPermission()) {
        string += "Notification";
    }
    return string;
}


void Platform::PlatformAdaptor::requestPermissionsSync()
{
    // Most mobile platforms require that the app asks for permission to do tasks such as showing a notification
    // or accessing location information. This method must request the necessary permissions. It will be called before the GUI is set up and is meant to run synchroneously.
    // Once the method returns, the app will check if all permissions are there, or else refuse to run.
    ObjCAdapter::requestNotificationPermission();
}


void Platform::PlatformAdaptor::vibrateBrief()
{
    // If supported by the platform, give short haptic feedback. Experience has shown that this is helpful
    // in aircraft situations where the pilot often has only one free hand and cannot concentrate on the device screen.
    ObjCAdapter::vibrateBrief();
}

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

#include "platform/PlatformAdaptor_MacOS.h"

#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <CoreFoundation/CoreFoundation.h>


Platform::PlatformAdaptor::PlatformAdaptor(QObject *parent)
    : PlatformAdaptor_Abstract(parent)
{
    setupIOKitNotifications();
}

Platform::PlatformAdaptor::~PlatformAdaptor()
{
    if (m_addedIterator != 0)
    {
        IOObjectRelease(m_addedIterator);
        m_addedIterator = 0;
    }

    if (m_removedIterator != 0)
    {
        IOObjectRelease(m_removedIterator);
        m_removedIterator = 0;
    }

    if (m_notifyPort != nullptr)
    {
        CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource(m_notifyPort);
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);
        IONotificationPortDestroy(m_notifyPort);
        m_notifyPort = nullptr;
    }
}


void Platform::PlatformAdaptor::setupIOKitNotifications()
{
    m_notifyPort = IONotificationPortCreate(kIOMainPortDefault);
    if (m_notifyPort == nullptr)
    {
        qWarning() << "Platform::PlatformAdaptor::setupIOKitNotifications()" << "m_notifyPort == nullptr";
        return;
    }

    CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource(m_notifyPort);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);

    // Register for device addition
    CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOSerialBSDServiceValue);
    if (matchingDict == nullptr)
    {
        qWarning() << "Platform::PlatformAdaptor::setupIOKitNotifications()" << "matchingDict == nullptr";
        return;
    }
    CFDictionarySetValue(matchingDict, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));
    auto kr = IOServiceAddMatchingNotification(m_notifyPort, kIOFirstMatchNotification, matchingDict,
                                               deviceChanged, this, &m_addedIterator);
    if (kr == KERN_SUCCESS)
    {
        deviceChanged(this, m_addedIterator);
    }

    // Register for device removal
    matchingDict = IOServiceMatching(kIOSerialBSDServiceValue);
    if (matchingDict == nullptr)
    {
        qWarning() << "Platform::PlatformAdaptor::setupIOKitNotifications()" << "matchingDict == nullptr";
        return;
    }
    CFDictionarySetValue(matchingDict, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));
    kr = IOServiceAddMatchingNotification(m_notifyPort, kIOTerminatedNotification, matchingDict,
                                          deviceChanged, this, &m_removedIterator);
    if (kr == KERN_SUCCESS)
    {
        deviceChanged(this, m_removedIterator);
    }
}


void Platform::PlatformAdaptor::deviceChanged(void *refCon, unsigned int iterator)
{
    io_service_t device = 0;

    // Consume all devices in the iterator
    while ((device = IOIteratorNext(iterator)) != 0)
    {
        IOObjectRelease(device);
    }

    // Emit signal that something changed
    auto *monitor = static_cast<Platform::PlatformAdaptor*>(refCon);
    monitor->serialPortsChanged();
}

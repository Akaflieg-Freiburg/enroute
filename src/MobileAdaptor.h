/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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


#ifndef MOBILEADAPTOR_H
#define MOBILEADAPTOR_H

#include <QObject>

/*! \brief Interface to platform-specific capabilities of mobile devices
 *
 * This class is an interface to capabilities of mobile devices (e.g. vibration) that need platform-specific code to operate. On desktop platforms, the methods of this class generally do nothing.
 *
 */

class MobileAdaptor : public QObject
{
    Q_OBJECT

public:
    explicit MobileAdaptor(QObject *parent = nullptr);

public slots:
    /*! \brief On Android, hides the android splash screen. On other platforms, this does nothing.
     *
     * The implementation ensures that QtAndroid::hideSplashScreen is called (only once, regardless of how often this slot is used).
     */
    void hideSplashScreen();

    /*! \brief On Android, make the device briefly vibrate. On other platforms, this does nothing. */
    void vibrateBrief();

    /*! \brief On Android, disables the screen lock: if set to 'on', the screen will never switch off while the app is shown to the user. On other platforms, this does nothing. */
    void disableScreenLock(bool on);

private:
    bool splashScreenHidden {false};
};

#endif // ANDROIDADAPTOR_H

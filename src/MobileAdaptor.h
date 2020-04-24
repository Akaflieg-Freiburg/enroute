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


#pragma once

#include <QObject>

/*! \brief Interface to platform-specific capabilities of mobile devices
  
  This class is an interface to capabilities of mobile devices (e.g. vibration)
  that need platform-specific code to operate. On desktop platforms, the methods
  of this class generally do nothing.
*/

class MobileAdaptor : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor

    On Android, this constructor disables the screen lock.

    @param parent Standard QObject parent pointer
    */
    explicit MobileAdaptor(QObject *parent = nullptr);

    ~MobileAdaptor();

public slots:
    /*! \brief Hides the android splash screen.

    On Android, hides the android splash screen. On other platforms, this does
    nothing. The implementation ensures that QtAndroid::hideSplashScreen is
    called (only once, regardless of how often this slot is used).
    */
    void hideSplashScreen();

    /*! \brief Make the device briefly vibrate

    On Android, make the device briefly vibrate. On other platforms, this does
    nothing.
    */
    void vibrateBrief();

    /*! \brief Shows a notifaction, indicating that a download is in progress

    @param show If set to 'true', a notification will be shown. If set to
    'false', any existing notification will be withdrawn
    */
    void showDownloadNotification(bool show);

private:
    Q_DISABLE_COPY_MOVE(MobileAdaptor)
  
    bool splashScreenHidden {false};
};

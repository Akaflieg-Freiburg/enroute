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


package de.akaflieg_freiburg.enroute;

import android.content.Context;
import android.os.Bundle;
import android.os.Vibrator;

import de.akaflieg_freiburg.enroute.Geoid;

public class MobileAdaptor extends de.akaflieg_freiburg.enroute.ShareActivity
{
    private static MobileAdaptor m_instance;
    private static Vibrator m_vibrator;
    private static Geoid m_geoid = null;

    public MobileAdaptor()
    {
        m_instance = this;
    }

    /* Vibrate once, very briefly */
    public static void vibrateBrief()
    {
        if (m_vibrator == null)
            m_vibrator = (Vibrator) m_instance.getSystemService(Context.VIBRATOR_SERVICE);
        m_vibrator.vibrate(20);
    }

    /**
     * Called when the activity is starting.
     *
     * We have to initialize Geoid here (and not in the constructor above)
     * as system services are not available to Activities before onCreate().
     */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        m_geoid = new Geoid();
    }

    /**
     * Called when the activity is resumed.
     *
     * In the case where permission for fine location wasn't allowed, m_geoid
     * as created in onCreate() couldn't register itself as NMEA listener.
     * If during startup of enroute the user then grants permission to fine
     * location, we can use onResume() to let the m_geoid instance register
     * itself as NMEA listener.
     *
     * This is useful and required for the first usage of enroute only.
     * In all later startups the user should have granted permission to
     * fine location already and m_geoid will register itself as NMEA listener
     * during creation in onCreate() already.
     */
    @Override
    public void onResume()
    {
        super.onResume();
        m_geoid.maybeAddAsNmeaListener();
    }
}

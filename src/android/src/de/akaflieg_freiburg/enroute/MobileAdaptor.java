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
    private static Geoid geoid_ = null;

    public MobileAdaptor()
    {
        m_instance = this;
    }

    /**
     * Called when the activity is starting.
     *
     * We have to initialize Geoid here (and not in the constructor above)
     * as system services are not available to Activities before onCreate().
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        geoid_ = new Geoid(this);
    }

    /* Vibrate once, very briefly */
    
    public static void vibrateBrief()
    {
        if (m_vibrator == null)
            m_vibrator = (Vibrator) m_instance.getSystemService(Context.VIBRATOR_SERVICE);
        m_vibrator.vibrate(20);
    }
}

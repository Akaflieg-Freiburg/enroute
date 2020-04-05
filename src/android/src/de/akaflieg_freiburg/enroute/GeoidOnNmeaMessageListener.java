/*
 * Copyright (C) 2020 by Johannes Zellner <johannes@zellner.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


package de.akaflieg_freiburg.enroute;

import android.location.OnNmeaMessageListener;
import android.location.LocationManager;

import de.akaflieg_freiburg.enroute.Geoid;

/**
 * implements the "newer" OnNmeaMessageListener interface which was introduced in API level 24.
 *
 * the purpose of this implementation is to support devices as of API level 24 or later.
 */
public class GeoidOnNmeaMessageListener extends Geoid implements OnNmeaMessageListener
{
    /**
     * add ourselves as OnNmeaMessageListener to the LocationManager.
     *
     * @param locationManager the LocationManager we add ourselves to
     */
    public void addToLocationManager(LocationManager locationManager)
    {
        locationManager.addNmeaListener(this);
    }

    /**
     * Called whenever NMEA messages arrive.
     *
     * Implements the OnNmeaMessageListener.onNmeaMessage().
     * Processing is done in Geoid.processNmeaMessge().
     *
     * @param message the NMEA message
     * @param timestamp of the location fix, as reported by the GNSS chipset in milliseconds
     */
    public void onNmeaMessage(String message, long timestamp /* milliseconds */)
    {
        processNmeaMessage(message, timestamp);
    }
}

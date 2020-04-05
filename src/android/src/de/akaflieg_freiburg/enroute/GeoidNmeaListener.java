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

import android.location.GpsStatus.NmeaListener;
import android.location.LocationManager;

import de.akaflieg_freiburg.enroute.Geoid;

/**
 * implements the "old" NmeaListener interface which was deprecated in API level 24.
 *
 * the purpose of this implementation is to support android devices running
 * API lower than 24 (Nougat).
 */
public class GeoidNmeaListener extends Geoid implements NmeaListener
{
    /**
     * add ourselves as NmeaListener to the LocationManager.
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
     * Implements the GpsStatus.NmeaListener.onNmeaReceived().
     * Processing is done in Geoid.processNmeaMessge().
     *
     * @param timestamp of the location fix, as reported by the GNSS chipset in milliseconds
     * @param message the NMEA message
     */
    public void onNmeaReceived(long timestamp /* milliseconds */, String message)
    {
        processNmeaMessage(message, timestamp);
    }
}

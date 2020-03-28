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

import android.content.Context;

import android.location.LocationManager;
import android.location.OnNmeaMessageListener;

public class Geoid implements OnNmeaMessageListener
{
    private float geoid_cached = 0;
    private long timestamp_for_cached_value = 0;

    private static final int GGA_PRECISION = 8;
    private static final int GGA_ALTITUDE = 9;
    private static final int GGA_GEOID = 11;

    public Geoid(Context context)
    {
        LocationManager locationManager = (LocationManager)context.getSystemService(Context.LOCATION_SERVICE);
        locationManager.addNmeaListener(this);
    }

    /**
     * Called whenever NMEA messages arrive.
     *
     * Implements the OnNmeaMessageListener.onNmeaMessage().
     * Get geoid correction from GGA messages, see...
     *
     * - https://www.gpsinformation.org/dale/nmea.htm#GGA
     * - https://de.wikipedia.org/wiki/NMEA_0183
     *
     * Updates the object's geoid_cached variable.
     *
     * @param message the NMEA message
     * @param timestamp of the location fix, as reported by the GNSS chipset in milliseconds
     */
    public void onNmeaMessage(String message, long timestamp /* milliseconds */)
    {
        // onNmeaMessage may get a few NMEA messages per second.
        // As the geoid correction varies quite slowly with distance
        // (or with time -- unless we fly a rocket with warp drive)
        // it is sufficient to parse the messages only every 10 seconds
        // or longer.
        //
        if (timestamp - timestamp_for_cached_value < 10000 || message == null)
        {
            return;
        }

        String[] tokens = message.split(",");

        if (tokens.length >= 12 && (tokens[0].equalsIgnoreCase("$GPGGA") || tokens[0].equalsIgnoreCase("$GNGGA")))
        {
            if (tokens[GGA_PRECISION].length() == 0 || tokens[GGA_GEOID].length() == 0)
            {
                // shortcut for empty tokens like
                // $GPGGA,,,,,,0,,,,,,,,
                //
                return;
            }

            // see https://de.wikipedia.org/wiki/NMEA_0183
            // $GNGGA,212716.00,4850.676296,N,01005.195966,E,1,09,1.0,446.2,M,47.9,M,,*76
            //                                                    prc, alt , ,geoid
            //                                                     8 ,  9  , , 11
            //
            try {

                float precision = Float.valueOf(tokens[GGA_PRECISION]);
                if (precision > 20)
                {
                    return; // precision is not sufficient
                }

                float geoid = Float.valueOf(tokens[GGA_GEOID]);
                timestamp_for_cached_value = timestamp;
                geoid_cached = geoid;

                /*
                Log.d("Geoid", "geoid = " + Float.toString(geoid_cached) +
                               ", alt = " + tokens[GGA_ALTITUDE] + " m" +
                               ", " + Float.toString(Float.valueOf(tokens[GGA_ALTITUDE]) * 3.28084f) + " ft");
                 */

        } catch (NumberFormatException e)
            {
                return;
            }
        }
    }

    /**
     * get cached geoid correction.
     *
     * @returns the geoid correction
     */
    public float geoid() {
        return geoid_cached;
    }
}

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

import android.Manifest;
import android.support.v4.content.ContextCompat;
import android.content.pm.PackageManager;
import android.util.Log;

import org.qtproject.qt5.android.QtNative;

public class Geoid implements OnNmeaMessageListener
{
    /**
     * native method to send file data to Qt - implemented in Cpp via JNI.
     */
    public static native void set(float newSeparation);

    private long last_valid_timestamp = 0;
    private boolean registered = false;

    private static final int GGA_PRECISION = 8;
    private static final int GGA_ALTITUDE = 9;
    private static final int GGA_GEOID = 11;

    /**
     * Geoid constructor.
     *
     * we add ourselves to the location manager to listen on NMEA messages.
     */
    public Geoid()
    {
        maybeAddAsNmeaListener();
    }

    /**
     * try to add ourselves to the location manager as NMEA listener.
     *
     * We add ourselves as NMEA listener only if permission to
     * ACCESS_FINE_LOCATION has been granted.
     */
    public void maybeAddAsNmeaListener() {

        if (registered) {
            return;
        }

        if (ContextCompat.checkSelfPermission(QtNative.activity(), Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            // Permission is not granted
            Log.d("Geoid", "ACCESS_FINE_LOCATION not granted.");
        } else {

            try {
                LocationManager locationManager = (LocationManager)QtNative.activity().getSystemService(Context.LOCATION_SERVICE);
                locationManager.addNmeaListener(this);
            } catch (Exception e) {
                Log.d("Geoid", e.getMessage());
            }

            registered = true;
        }
    }

    /**
     * Called whenever NMEA messages arrive.
     *
     * Implements the OnNmeaMessageListener.onNmeaMessage().
     * Get geoid separation from GGA messages, see...
     *
     * - https://www.gpsinformation.org/dale/nmea.htm#GGA
     * - https://de.wikipedia.org/wiki/NMEA_0183
     *
     * sends the geoidal separation to c++ via the native method set().
     *
     * @param message the NMEA message
     * @param timestamp of the location fix, as reported by the GNSS chipset in milliseconds
     */
    public void onNmeaMessage(String message, long timestamp /* milliseconds */)
    {
        // onNmeaMessage may get a few NMEA messages per second.
        // As the geoid separation varies quite slowly with distance
        // (or with time -- unless we fly a rocket with warp drive)
        // it is sufficient to parse the messages only every 10 seconds
        // or longer.
        //
        if (timestamp - last_valid_timestamp < 10000 || message == null)
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
                last_valid_timestamp = timestamp;
                set(geoid); // set geoidal separation in c++ class via jni native call

                /*
                Log.d("Geoid", "geoid = " + Float.toString(geoid) +
                               ", alt = " + tokens[GGA_ALTITUDE] + " m" +
                               ", " + Float.toString(Float.valueOf(tokens[GGA_ALTITUDE]) * 3.28084f) + " ft");
                 */

        } catch (NumberFormatException e)
            {
                return;
            }
        }
    }
}

/*!
* Copyright (C) 2020 by Johannes Zellner, johannes@zellner.org
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation; either version 3 of the License, or (at your option) any later
* version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place - Suite 330, Boston, MA  02111-1307, USA.
*/

package de.akaflieg_freiburg.enroute;

import android.content.ContentResolver;
import android.content.Intent;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.webkit.MimeTypeMap;
import androidx.documentfile.provider.DocumentFile;

import org.qtproject.qt.android.QtNative;
import org.qtproject.qt.android.bindings.QtActivity;

import java.io.File;
import java.lang.String;

/**
 * This class handles different sorts of "incoming" intents with "file URL's"
 * whose data is then sent back to Qt using the static native method
 * setDataReceived().
 *
 * This class should be used as main activity class.
 */
public class ShareActivity extends QtActivity 
{
    /**
     * native method to send file data to Qt - implemented in Cpp via JNI.
     */
    public static native void setFileReceived(String fileName, String unmingled);
    public static native void setTextReceived(String text);
    public static native void onOpenUSBRequestReceived(String device);

    private static boolean isIntentPending = false;
    private static boolean isInitialized = false;
    private static String TAG = "ShareActivity";
    private static File tmpDir;

    @Override
    public void onDestroy() 
    {
        super.onDestroy();
    }

    /**
     * Called when the activity is starting.
     *
     * This is where most initialization should go. We don't process any intent
     * here as the app may not be fully initialized yet.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
        Log.d(TAG, "onCreate");

        super.onCreate(savedInstanceState);
        UsbSerialHelper.initialize(this);

        // check if the App was started from another Android App via Intent
        Intent theIntent = getIntent();

        if (theIntent != null) 
        {
            String theAction = theIntent.getAction();
            if (theAction != null) 
            {
                // QML UI not ready yet delay processIntent();
                isIntentPending = true;
            }
        }
    }

    /**
     * Called when the activity is brought into foreground.
     *
     * When the activity is re-launched while at the top of the activity stack
     * instead of a new instance of the activity being started, onNewIntent()
     * will be called on the _existing_ instance with the Intent that was used
     * to re-launch it.
     *
     * Note that getIntent() still returns the original Intent. We use use
     * setIntent(Intent) to update it to this new Intent.
     *
     * In our case onNewIntent() will be called to handle the SEND and VIEW
     * intents from other apps e.g. if other apps want to SEND us or open their
     * gpx content.
     */
    @Override
    public void onNewIntent(Intent intent) 
    {
        Log.d(TAG, "onNewIntent");

        super.onNewIntent(intent);
        setIntent(intent);

        if (isInitialized) 
        {
            // process intent if all is initialized and Qt / QML can handle
            // results
            processIntent();
        }
        else
        {
            // postpone processing of intent until checkPendingIntents() is
            // called from main app (which signals that the main app is ready)
            isIntentPending = true;
        }
    }

    /**
     * Called from c++ to process pending intents.
     *
     * Calling this method from c++ indicates that QML/Qt is initialized and
     * ready for getting results from processIntent(). Check if intents are
     * pending and process them.
     */
    public void checkPendingIntents(String tmpDir) 
    {
        Log.d(TAG, "checkPendingIntents");

        isInitialized = true;
        this.tmpDir = new File(tmpDir);

        if (isIntentPending) 
        {
            isIntentPending = false;
            processIntent();
        }
    }

    /**
     * process the (pending) Intent if Action is SEND or VIEW.
     *
     * Extract intent URL from intent and process it further in setUriReceived().
     */
    private void processIntent() 
    {
        Log.d(TAG, "processIntent()");

        Intent intent = getIntent();
        Uri intentUri;
        if (intent.getAction() == null)
        {
            Log.d(TAG, "processIntent intent.getAction() == null");
            return;
        }

        Log.d(TAG, "processIntent() " + intent.getAction());

        // we are listening to android.intent.action.SEND or VIEW (see Manifest)
        if (intent.getAction().equals("android.intent.action.VIEW"))
        {
            intentUri = intent.getData();
            if (intentUri == null)
            {
                Log.d(TAG, "processIntent VIEW: intent.getData() == null");
                return;
            }

            // Handle geo: URIs (e.g. from Organic Maps) — these are not files
            if ("geo".equals(intentUri.getScheme())) 
            {
                setTextReceived(intentUri.toString());
                return;
            }

            DocumentFile docFile = DocumentFile.fromSingleUri(this, intentUri);
            // getName() can return null if the URI has no backing content provider
            String name = (docFile != null && docFile.getName() != null) ? docFile.getName() : "";
            setFileReceived(intentUri.toString(), name);
            return;
        }

        if (intent.getAction().equals("android.intent.action.SEND"))
        {
            // EXTRA_STREAM holds a Uri, not a String; getStringExtra() would return null
            Uri streamUri = intent.getParcelableExtra(Intent.EXTRA_STREAM);
            if (streamUri != null)
            {
                DocumentFile docFile = DocumentFile.fromSingleUri(this, streamUri);
                // getName() can return null if the URI has no backing content provider
                String name = (docFile != null && docFile.getName() != null) ? docFile.getName() : "";
                setFileReceived(streamUri.toString(), name);
                return;
            }

            if (intent.getStringExtra(Intent.EXTRA_TEXT) != null)
            {
                setTextReceived(intent.getStringExtra(Intent.EXTRA_TEXT));
                return;
            }
        }

        if (intent.getAction().equals(UsbManager.ACTION_USB_DEVICE_ATTACHED)) 
        {
            UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
            if (device == null) 
            {
                Log.d("enroute flight navigation", "processIntent USB device attached: device is null");
                return;
            }
            com.hoho.android.usbserial.driver.UsbSerialDriver driver =
                com.hoho.android.usbserial.driver.UsbSerialProber.getDefaultProber().probeDevice(device);
            if (driver != null) 
            {
                // Get product name, fallback to device name
                String deviceName = device.getProductName();
                if (deviceName == null || deviceName.trim().isEmpty()) 
                {
                    deviceName = device.getDeviceName();
                }
                Log.d("enroute flight navigation", "processIntent USB device attached: " + deviceName);
                onOpenUSBRequestReceived(deviceName);
                return;
            }
        }
    }
}

/*!
 * Copyright (C) 2020 by Johannes Zellner, johannes@zellner.org
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

import android.content.ContentResolver;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.webkit.MimeTypeMap;

import org.qtproject.qt5.android.QtNative;
import org.qtproject.qt5.android.bindings.QtActivity;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.lang.String;


/**
 * This class handles different sorts of "incoming" intents
 * with "file URL's" whose data is then sent back to Qt using
 * the static native method setDataReceived().
 *
 * This class should be used as main activity class.
 */
public class ShareActivity extends QtActivity {

    /**
     * native method to send file data to Qt - implemented in Cpp via JNI.
     */
    public static native void setFileReceived(String fileName);

    private static boolean isIntentPending = false;
    private static boolean isInitialized = false;
    private static String TAG = "ShareActivity";

    private static File tmpDir;

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    /**
     * Called when the activity is starting.
     *
     * This is where most initialization should go.
     * We don't process any intent here as the app may* not be fully initialized yet.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
	Log.d(TAG, "onCreate");

        super.onCreate(savedInstanceState);

        // check if the App was started from
        // another Android App via Intent
        //
        Intent theIntent = getIntent();

        if (theIntent != null) {

            String theAction = theIntent.getAction();

            if (theAction != null) {

                // QML UI not ready yet
                // delay processIntent();
                isIntentPending = true;
            }
        }
    }

    /**
     * Called when the activity is brought into foreground.
     *
     * When the activity is re-launched while at the top of the activity stack instead
     * of a new instance of the activity being started, onNewIntent() will be called
     * on the _existing_ instance with the Intent that was used to re-launch it.
     *
     * Note that getIntent() still returns the original Intent.
     * We use use setIntent(Intent) to update it to this new Intent.
     *
     * In our case onNewIntent() will be called to handle the SEND and VIEW intents
     * from other apps e.g. if other apps want to SEND us or open their gpx content.
     */
    // if we are opened from other apps:
    @Override
    public void onNewIntent(Intent intent) {
	Log.d(TAG, "onNewIntent");

        super.onNewIntent(intent);
        setIntent(intent);

        if (isInitialized) {

            // process intent if all is initialized and Qt / QML can handle results
            //
            processIntent();

        } else {

            // postpone processing of intent until checkPendingIntents() is
            // called from main app (which signals that the main app is ready)
            //
            isIntentPending = true;
        }
    }

    /** Called when an activity _we_ launched ourselves from IntentLauncher exits,
     * giving us the requestCode we started it with, a resultCode and any additional
     * data from it.
     *
     * The resultCode will be RESULT_CANCELED if the activity explicitly returned
     * that, didn't return any result, or crashed during its operation.
     *
     * In our case onActivityResult() will be called to handle the ACTION_OPEN_DOCUMENT
     * or ACTION_CREATE_DOCUMENT intents which wer launched from IntentLauncher to open
     * or to save to a file URL.
     * We extract the file URL from the intent and process it with setUriReceived().
     */
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
	Log.d(TAG, "onActivityResult");

        super.onActivityResult(requestCode, resultCode, intent);

        if (resultCode == RESULT_OK) {

            if (requestCode == ShareUtils.getOpenRequestCode()) {

                setUriReceived(intent.getData());

            } else if (requestCode == ShareUtils.getSaveRequestCode()) {

                copyContent(ShareUtils.getShareUri(), intent.getData());
            }
        }
    }

    /** Called from c++ to process pending intents.
      *
      * Calling this method from c++ indicates that QML/Qt is initialized
      * and ready for getting results from processIntent().
      * Check if intents are pending and process them.
      */
    public void checkPendingIntents(String tmpDir) {
	Log.d(TAG, "checkPendingIntents");

        isInitialized = true;
        this.tmpDir = new File(tmpDir);

        if (isIntentPending) {
            isIntentPending = false;
            processIntent();
        }
    }

    /** process the (pending) Intent if Action is SEND or VIEW.
     *
     * Extract intent URL from intent and process it further
     * in setUriReceived().
     */
    private void processIntent() {
	Log.d(TAG, "processIntent()");

        Intent intent = getIntent();

        Uri intentUri;
        if (intent.getAction() == null) {
            Log.d(TAG, "processIntent intent.getAction() == null");
            return;
        }
        // we are listening to android.intent.action.SEND or VIEW (see Manifest)
        if (intent.getAction().equals("android.intent.action.VIEW")) {
            intentUri = intent.getData();
        } else if (intent.getAction().equals("android.intent.action.SEND")) {
            Bundle bundle = intent.getExtras();
            intentUri = (Uri) bundle.get(Intent.EXTRA_STREAM);
        } else {
            // could be for example
            // action:anroid.intent.action.MAIN
            return;
        }

        setUriReceived(intentUri);
    }

    /** copy received URI to temporary file in cache directory and
     * send the file name to c++ by setFileReceived().
     *
     * Extract intent URL from intent and process it further
     * in setUriReceived().
     */
    private void setUriReceived(Uri src) {
	String pth = src.getPath();
	String ending = pth.substring(pth.lastIndexOf('.'));
	Log.d(TAG, "setUriReceived");

        try {
            File tmpFile = File.createTempFile("tmp", ending, tmpDir);
            copyContent(src, Uri.fromFile(tmpFile));
            setFileReceived(tmpFile.getPath());
        } catch (IOException exception) {
            Log.d(TAG, exception.getMessage());
        }
    }

    /** copy src URI to dst URI.
     */
    private void copyContent(Uri src, Uri dst) {
	Log.d(TAG, "copyContent");

        // Log.d(TAG, "ShareActivity.copyContent " + src + " --> " + dst);

        try {
            InputStream istream = getContentResolver().openInputStream(src);
            OutputStream ostream = getContentResolver().openOutputStream(dst);

            int nRead;
            byte[] data = new byte[0x10000];
            while ((nRead = istream.read(data, 0, data.length)) != -1) {
                ostream.write(data, 0, nRead);
            }

            ostream.close();
            istream.close();

        } catch (FileNotFoundException exception) {
            Log.d(TAG, exception.getMessage());
        } catch (IOException exception) {
            Log.d(TAG, exception.getMessage());
        }
    }
}

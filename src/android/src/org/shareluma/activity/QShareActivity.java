/*!
 * MIT License
 *
 * Copyright (c) 2019 Tim Seemann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// (c) 2017 Ekkehard Gentz (ekke)
// this project is based on ideas from
// http://blog.lasconic.com/share-on-ios-and-android-using-qml/
// see github project https://github.com/lasconic/ShareUtils-QML
// also inspired by:
// https://www.androidcode.ninja/android-share-intent-example/
// https://www.calligra.org/blogs/sharing-with-qt-on-android/
// https://stackoverflow.com/questions/7156932/open-file-in-another-app
// http://www.qtcentre.org/threads/58668-How-to-use-QAndroidJniObject-for-intent-setData
// OpenURL in At Android: got ideas from:
// https://github.com/BernhardWenzel/open-url-in-qt-android
// https://github.com/tobiatesan/android_intents_qt
//
// see also /COPYRIGHT and /LICENSE

package org.shareluma.activity;

import android.app.*;
import android.content.*;
import android.content.ContentResolver;
import android.content.Intent;
import android.net.Uri;
import android.os.*;
import android.util.Log;
import android.webkit.MimeTypeMap;

import org.qtproject.qt5.android.QtNative;
import org.qtproject.qt5.android.bindings.QtActivity;
import org.shareluma.utils.*;

import java.io.File;
import java.lang.String;

public class QShareActivity extends QtActivity {
    // native - must be implemented in Cpp via JNI
    // 'file' scheme or resolved from 'content' scheme:
    public static native void setFileUrlReceived(String url);

    public static native void fireActivityResult(int requestCode, int resultCode);

    public static native boolean checkFileExits(String url);

    public static boolean isIntentPending;
    public static boolean isInitialized;
    public static String workingDirPath;

    private static String TAG = "QShareActivity";

    // Use a custom Chooser without providing own App as share target !
    // see QShareUtils.java createCustomChooserAndStartActivity()
    // Selecting your own App as target could cause AndroidOS to call
    // onCreate() instead of onNewIntent()
    // and then you are in trouble because we're using 'singleInstance' as
    // LaunchMode more details: my blog at Qt
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Log.d(TAG, "onCreate QShareActivity");
        // now we're checking if the App was started from another Android App via
        // Intent
        Intent theIntent = getIntent();
        if (theIntent != null) {
            String theAction = theIntent.getAction();
            if (theAction != null) {
                // Log.d(TAG, "onCreate theAction=" + theAction);
                // QML UI not ready yet
                // delay processIntent();
                isIntentPending = true;
            }
        }
    } // onCreate

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy QShareActivity");
        super.onDestroy();
    }

    // we start Activity with result code
    // to test JNI with QAndroidActivityResultReceiver you must comment or rename
    // this method here - otherwise you'll get wrong request or result codes
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // Check which request we're responding to
        // Log.d(TAG, "onActivityResult requestCode: " + requestCode);
		// if (resultCode == RESULT_OK) {
		//     Log.d(TAG, "onActivityResult - resultCode: SUCCESS");
		// } else {
		//     Log.d(TAG, "onActivityResult - resultCode: CANCEL");
		// }
        // hint: result comes back too fast for Action SEND
        // if you want to delete/move the File add a Timer w 500ms delay
        // see Example App main.qml - delayDeleteTimer
        // if you want to revoke permissions for older OS
        // it makes sense also do this after the delay
        fireActivityResult(requestCode, resultCode);
    }

    // if we are opened from other apps:
    @Override
    public void onNewIntent(Intent intent) {
        Log.d(TAG, "onNewIntent");
        super.onNewIntent(intent);
        setIntent(intent);
        // Intent will be processed, if all is initialized and Qt / QML can handle
        // the event
        if (isInitialized) {
            processIntent();
        } else {
            isIntentPending = true;
        }
    } // onNewIntent

    public void checkPendingIntents(String workingDir) {
        isInitialized = true;
        workingDirPath = workingDir;
        Log.d(TAG, "checkPendingIntents: " + workingDirPath);
        if (isIntentPending) {
            isIntentPending = false;
            //  Log.d(TAG, "checkPendingIntents: true");
            processIntent();
        }
    } // checkPendingIntents

    // process the Intent if Action is SEND or VIEW
    private void processIntent() {
        Intent intent = getIntent();
        // Log.d(TAG, " process Intent");

        Uri intentUri;
        String intentScheme;
        String intentAction;
        if (intent.getAction() == null) {
            return;
        }
        // we are listening to android.intent.action.SEND or VIEW (see Manifest)
        if (intent.getAction().equals("android.intent.action.VIEW")) {
            intentAction = "VIEW";
            intentUri = intent.getData();
        } else if (intent.getAction().equals("android.intent.action.SEND")) {
            intentAction = "SEND";
            Bundle bundle = intent.getExtras();
            intentUri = (Uri) bundle.get(Intent.EXTRA_STREAM);
        } else {
            Log.d(TAG, "Intent unknown action:" + intent.getAction());
            return;
        }
        // Log.d(TAG, " action: " + intentAction);
        if (intentUri == null) {
            Log.d(TAG, " Intent URI: is null");
            return;
        }

        // Log.d(TAG, "Intent URI:" + intentUri.toString());

        // content or file
        intentScheme = intentUri.getScheme();
        if (intentScheme == null) {
            Log.d(TAG, "Intent URI Scheme: is null");
            return;
        }
        if (intentScheme.equals("file")) {
            // URI as encoded string
            //  Log.d(TAG, "Intent File URI: " + intentUri.toString());
            setFileUrlReceived(intentUri.toString());
            // we are done Qt can deal with file scheme
            return;
        }
        if (!intentScheme.equals("content")) {
            Log.d(TAG, "Intent URI unknown scheme: " + intentScheme);
            return;
        }
        // ok - it's a content scheme URI
        // we will try to resolve the Path to a File URI
        // if this won't work or if the File cannot be opened,
        // we'll try to copy the file into our App working dir via InputStream
        // hopefully in most cases PathResolver will give a path

        // you need the file extension, MimeType or Name from ContentResolver ?
        // here's HowTo get it:
        // Log.d(TAG, " Intent Content URI: " + intentUri.toString());
        ContentResolver cR = this.getContentResolver();
        MimeTypeMap mime = MimeTypeMap.getSingleton();
        String fileExtension = mime.getExtensionFromMimeType(cR.getType(intentUri));
        // Log.d(TAG, "Intent extension: " + fileExtension);
        String mimeType = cR.getType(intentUri);
        // Log.d(TAG, " Intent MimeType: " + mimeType);
        String name = QShareUtils.getContentName(cR, intentUri);
        String filePath;
        filePath = QSharePathResolver.getRealPathFromURI(this, intentUri);
        if (filePath == null) {
            // Log.d(TAG, "QSharePathResolver: filePath is NULL");
        } else {
            //  Log.d(TAG, "QSharePathResolver:" + filePath);
            // to be safe check if this File Url really can be opened by Qt
            // there were problems with MS office apps on Android 7
            if (checkFileExits(filePath)) {
                setFileUrlReceived(filePath);
                // we are done Qt can deal with file scheme
                return;
            }
        }

        // trying the InputStream way:
        filePath = QShareUtils.createFile(cR, intentUri, workingDirPath);
        if (filePath == null) {
            Log.d(TAG, "Intent FilePath: is NULL");
            return;
        }
        setFileUrlReceived(filePath);
    } // processIntent

} // class QShareActivity

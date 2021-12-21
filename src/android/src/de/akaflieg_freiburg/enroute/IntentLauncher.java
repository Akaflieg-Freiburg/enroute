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

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.net.Uri;
import android.os.Parcelable;
import android.support.v4.app.ShareCompat;
import android.support.v4.content.FileProvider;
import android.util.Log;
import android.content.ComponentName;
import android.content.SharedPreferences;

import org.qtproject.qt.android.QtNative;

import java.io.File;
import java.lang.String;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * The purpose of the class IntentLauncher is to create either a
 * VIEW, SEND, OPEN or SAVE intent and then to call startActivity()
 * or startActivityForResult().
 *
 * All public static methods are called from the c++ class Share.cpp.
 */
public class IntentLauncher {

    // reference Authority as defined in AndroidManifest.xml
    private static String AUTHORITY = "de.akaflieg_freiburg.enroute";
    private static String TAG = "IntentLauncher";

    protected IntentLauncher() {
    }

    /**
     * SEND (== "share") a file to another application.
     *
     * The receiving app is _not_ supposed to handle the mime type but
     * it is rather supposed to _transport_ the file.
     * Examples for receiving apps are email, messenger or file managers.
     *
     * @param filePath the path of the file to send.
     * @param mimeType the mime type of the file to send.
     *
     * @return true if the intent was launched otherwise false
     */
    public static boolean sendFile(String filePath, String mimeType) {
        return sendOrViewFile(filePath, mimeType, Intent.ACTION_SEND);
    }

    /**
     * VIEW (== "open") a file in another application.
     *
     * The receiving app is supposed to handle the mime type.
     *
     * @param filePath the path of the file to send.
     * @param mimeType the mime type of the file to send.
     *
     * @return true if the intent was launched otherwise false
     */
    public static boolean viewFile(String filePath, String mimeType) {
        return sendOrViewFile(filePath, mimeType, Intent.ACTION_VIEW);
    }

    /**
     * open file from file system.
     *
     * Creates an ACTION_OPEN_DOCUMENT intent and does the necessary settings.
     * Fires the intent with the blocking startActivityForResult().
     * This will launch a file chooser.
     * The result from the file chooser -- either a file URL or "CANCELLED" --
     * will be handled by the ShareActivity class.
     *
     * This methods blocks until the file selection has completed and
     * the file URL has been sent back to Qt from the ShareActivity class.
     *
     * @param mimeType the mime type of the file to send.
     *
     * @return true if the intent was launched otherwise false
     */
    public static boolean openFile(String mimeType) {
        return openOrSave(null, mimeType, Intent.ACTION_OPEN_DOCUMENT, ShareUtils.getOpenRequestCode());
    }

    /**
     * save file.
     *
     * here we create a ACTION_CREATE_DOCUMENT intent.
     * Copying the file content to the created file will be done by
     * ShareActivity.copyFile() after ShareActivity.onActivityResult()
     * received the intent.
     *
     * @param filePath the path of the file to send.
     * @param mimeType the mime type of the file to send.
     *
     * @return true if the intent was launched otherwise false
     */
    public static boolean saveFile(String filePath, String mimeType) {

        Uri uri = fileToUri(filePath);

        if (uri == null) {
            return false;
        }

        // we save the source filePath so that it can be later retrieved by
        // ShareActivity.onActivityResult().
        //
        ShareUtils.setShareUri(uri);

        return openOrSave(new File(filePath).getName(), mimeType, Intent.ACTION_CREATE_DOCUMENT, ShareUtils.getSaveRequestCode());
    }

    /**
     * common implementation for both the sendFile() and viewFile() methods.
     *
     * Creates an appropriate intent and does the necessary settings.
     * Creates a chooser with the intent and calls startActivity() to fire the intent.
     *
     * This method does _not_ block until the content has been share with another
     * app but rather returns immediately (if action == 0). Therefore the receiving
     * apps can handle the file URL while enroute stays active an is not blocked.
     * Effectively this means that both the sending and the receiving app will run
     * concurrently.
     *
     * @param filePath the path of the file to send.
     * @param mimeType the mime type of the file to send.
     * @param action either Intent.ACTION_SEND or Intent.ACTION_VIEW
     *
     * @return true if the intent was launched otherwise false
     */
    private static boolean sendOrViewFile(String filePath, String mimeType, String action) {

        if (QtNative.activity() == null) {
            return false;
        }

        // Intent intent = new Intent();
        // using v4 support library create the Intent from ShareCompat
        Intent intent = ShareCompat.IntentBuilder.from(QtNative.activity()).getIntent();
        intent.setAction(action);

        Uri uri = fileToUri(filePath);

        if (uri == null) {
            return false;
        }

        if (mimeType == null || mimeType.isEmpty()) {
            mimeType = QtNative.activity().getContentResolver().getType(uri);
        }

        if (action == Intent.ACTION_SEND) {
            intent.putExtra(Intent.EXTRA_STREAM, uri);
            intent.setType(mimeType);
        } else {
            intent.setDataAndType(uri, mimeType);
        }

        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);

        return customStartActivity(intent);
    }

    /**
     * create uri from file path.
     *
     * android content provoiders operate with Uri's which represent the
     * content scheme (e.g. file:// or content://). This methods converts
     * the absolute file path in the sharing directory into a valid Uri.
     *
     * @param filePath the absolut path of the file in the cache directory.
     *
     * @return Uri the corresponding uri
     */
    private static Uri fileToUri(String filePath) {

        File fileToShare = new File(filePath);

        // Using FileProvider you must get the URI from FileProvider using your AUTHORITY
        // Uri uri = Uri.fromFile(imageFileToShare);
        Uri uri;
        try {
            return FileProvider.getUriForFile(QtNative.activity(), AUTHORITY, fileToShare);
        } catch (IllegalArgumentException e) {
            Log.d(TAG, "error" + e.getMessage());
            return null;
        }
    }

    /**
     * create a custom chooser and start activity.
     *
     * The custom chooser takes care of not sharing with or sending to the own app.
     * This is done by first finding matching apps which can handle the intent
     * and then blacklisting the own app by Intent.EXTRA_EXCLUDE_COMPONENTS.
     *
     * The chooser intent is then started with startActivity() which will return
     * immediately and not wait for the chooser result. This way, both enroute and
     * the receiving app can run concurrently and enroute is _not_ blocked until
     * the receiving app terminates.
     *
     * @param theIntent intent to be handled
     *
     * @return true if the intent was launched otherwise false
     */
    private static boolean customStartActivity(Intent theIntent) {

        final Context context = QtNative.activity();
        final PackageManager packageManager = context.getPackageManager();

        // MATCH_DEFAULT_ONLY: Resolution and querying flag. if set, only filters
        // that support the CATEGORY_DEFAULT will be considered for matching. Check
        // if there is a default app for this type of content.
        ResolveInfo defaultAppInfo =
                packageManager.resolveActivity(theIntent, PackageManager.MATCH_DEFAULT_ONLY);
        if (defaultAppInfo == null) {
            Log.d(TAG, "PackageManager cannot resolve Activity");
            return false;
        }

        // Retrieve all apps for our intent. Check if there are any apps returned
        List<ResolveInfo> appInfoList =
                packageManager.queryIntentActivities(theIntent, PackageManager.MATCH_DEFAULT_ONLY);
        if (appInfoList.isEmpty()) {
            Log.d(TAG, "appInfoList.isEmpty");
            return false;
        }
        // Log.d(TAG, "appInfoList: " + appInfoList.size());

        // Sort in alphabetical order
        Collections.sort(appInfoList, new Comparator<ResolveInfo>() {
            @Override
            public int compare(ResolveInfo first, ResolveInfo second) {
                String firstName = first.loadLabel(packageManager).toString();
                String secondName = second.loadLabel(packageManager).toString();
                return firstName.compareToIgnoreCase(secondName);
            }
        });


        // find own package and blacklist it
        //
        List<ComponentName> blacklistedComponents = new ArrayList<ComponentName>();
        for (ResolveInfo info : appInfoList) {

            String pkgName = info.activityInfo.packageName;
            String clsName = info.activityInfo.name;
            // we don't want to share with our own app so blacklist it
            //
            if (pkgName.equals(context.getPackageName())) {
                blacklistedComponents.add(new ComponentName(pkgName, clsName));
            }
        }

        Intent chooserIntent = Intent.createChooser(theIntent, null /* title */);
        chooserIntent.putExtra(Intent.EXTRA_EXCLUDE_COMPONENTS, blacklistedComponents.toArray(new Parcelable[] {}));
        chooserIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);

        // Verify that the intent will resolve to an activity
        // do NOT use startActivityForResult as it will block
        // enroute until the receiving app terminates.
        //
        if (chooserIntent.resolveActivity(QtNative.activity().getPackageManager()) != null) {
            QtNative.activity().startActivity(chooserIntent);
            return true;
        }
        return false;
    }

    /**
     * common implementation for both the openFile() and saveFile() methods.
     *
     * Creates an appropriate intent and does the necessary settings.
     * Creates a chooser with the intent and calls startActivity() to fire the intent.
     *
     * This method will block until the user has chosen to open or save a file.
     * On return of the file chooser ShareActivity.onActivityResult() will do
     * the actual opening or saving of the file.
     *
     * @param fileName the (suggested) name of the file (null for OPEN action)
     * @param mimeType the mime type of the file to open or save
     * @param action either Intent.ACTION_CREATE_DOCUMENT or Intent.ACTION_OPEN_DOCUMENT
     * @param requestCode request code common to IntentLauncher and ShareActivity
     *
     * @return true if the intent was launched otherwise false
     */
    private static boolean openOrSave(String fileName, String mimeType, String action, int requestCode) {

        if (QtNative.activity() == null) {
            return false;
        }

        // Intent intent = new Intent();
        // using v4 support library create the Intent from ShareCompat
        Intent intent = ShareCompat.IntentBuilder.from(QtNative.activity()).getIntent();
        intent.setAction(action);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType(mimeType);

        if (fileName != null) {
            intent.putExtra(Intent.EXTRA_TITLE, fileName);
        }

        // Optionally, specify a URI for the directory that should be opened in
        // the system file picker when your app creates the document.
        // intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, pickerInitialUri);
        // Log.d(TAG, "getExternalCacheDir() = " + context.getExternalCacheDir());

        if (intent.resolveActivity(QtNative.activity().getPackageManager()) != null) {
            QtNative.activity().startActivityForResult(intent, requestCode);
            return true;
        }

        return false;
    }
}

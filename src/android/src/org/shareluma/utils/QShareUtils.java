/*!
 * MIT License
 *
 * Copyright (c) 2019 Tim Seemann
 * modified 2020 by Johannes Zellner
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
// https://stackoverflow.com/questions/5734678/custom-filtering-of-intent-chooser-based-on-installed-android-package-name
// see also /COPYRIGHT and /LICENSE

package org.shareluma.utils;

import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Parcelable;
import android.provider.MediaStore;
import android.support.v4.app.ShareCompat;
import android.support.v4.content.FileProvider;
import android.util.Log;

import android.content.ComponentName;

import org.qtproject.qt5.android.QtNative;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.String;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class QShareUtils {
    // reference Authority as defined in AndroidManifest.xml
    private static String AUTHORITY = "org.shareluma.shareutils.enroutefp";
    private static String TAG = "QShareUtils";

    protected QShareUtils() {
        // Log.d(TAG, "QShareUtils()");
    }

    public static boolean checkMimeTypeView(String mimeType) {
        if (QtNative.activity() == null) {
            return false;
        }
        Intent myIntent = new Intent();
        myIntent.setAction(Intent.ACTION_VIEW);
        // without an URI resolve always fails
        // an empty URI allows to resolve the Activity
        File fileToShare = new File("");
        Uri uri = Uri.fromFile(fileToShare);
        myIntent.setDataAndType(uri, mimeType);

        // Verify that the intent will resolve to an activity
        if (myIntent.resolveActivity(QtNative.activity().getPackageManager()) != null) {
            // Log.d(TAG, "checkMime YEP - we can go on and View");
            return true;
        } else {
            // Log.d(TAG, "checkMime sorry - no App available to View");
        }
        return false;
    }


    // Zellner: using Intent.EXTRA_EXCLUDE_COMPONENTS
    //
    public static boolean createCustomChooserAndStartActivity(Intent theIntent,
                                                              String title,
                                                              int requestId,
                                                              Uri uri) {
        final Context context = QtNative.activity();
        final PackageManager packageManager = context.getPackageManager();
        final boolean isLowerOrEqualsKitKat = Build.VERSION.SDK_INT <= Build.VERSION_CODES.KITKAT;

        // MATCH_DEFAULT_ONLY: Resolution and querying flag. if set, only filters
        // that support the CATEGORY_DEFAULT will be considered for matching. Check
        // if there is a default app for this type of content.
        ResolveInfo defaultAppInfo =
                packageManager.resolveActivity(theIntent, PackageManager.MATCH_DEFAULT_ONLY);
        if (defaultAppInfo == null) {
            Log.d(TAG, title + " PackageManager cannot resolve Activity");
            return false;
        }

        // Retrieve all apps for our intent. Check if there are any apps returned
        List<ResolveInfo> appInfoList =
                packageManager.queryIntentActivities(theIntent, PackageManager.MATCH_DEFAULT_ONLY);
        if (appInfoList.isEmpty()) {
            Log.d(TAG, title + " appInfoList.isEmpty");
            return false;
        }
        // Log.d(TAG, title + " appInfoList: " + appInfoList.size());

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

        Intent chooserIntent = Intent.createChooser(theIntent, title);
        chooserIntent.putExtra(Intent.EXTRA_EXCLUDE_COMPONENTS, blacklistedComponents.toArray(new Parcelable[] {}));

        // Verify that the intent will resolve to an activity
		// do NOT use startActivityForResult as it will block
		// enroute until the receiving app terminates.
		//
        if (chooserIntent.resolveActivity(QtNative.activity().getPackageManager()) != null) {
			/*
            if (requestId > 0) {
                QtNative.activity().startActivityForResult(chooserIntent, requestId);
            } else {
			*/
                QtNative.activity().startActivity(chooserIntent);
			/*
            }
			*/
            return true;
        }
        return false;
    }

    // I am deleting the files from shared folder when Activity was done or
    // canceled so probably I don't have to revike FilePermissions for older OS if
    // you don't delete or move the file: here's what you must done to revoke the
    // access
    public static void revokeFilePermissions(String filePath) {
        final Context context = QtNative.activity();
        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.KITKAT) {
            File file = new File(filePath);
            Uri uri = FileProvider.getUriForFile(context, AUTHORITY, file);
            context.revokeUriPermission(
                    uri,
                    Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
        }
    }

    public static boolean sendFile(String filePath, String title, String mimeType, int requestId) {
        if (QtNative.activity() == null) {
            return false;
        }

        // using v4 support library create the Intent from ShareCompat
        // Intent sendIntent = new Intent();
        Intent sendIntent = ShareCompat.IntentBuilder.from(QtNative.activity()).getIntent();
        sendIntent.setAction(Intent.ACTION_SEND);

        File fileToShare = new File(filePath);

        // Using FileProvider you must get the URI from FileProvider using your
        // AUTHORITY Uri uri = Uri.fromFile(imageFileToShare);
        Uri uri;
        try {
            uri = FileProvider.getUriForFile(QtNative.activity(), AUTHORITY, fileToShare);
        } catch (IllegalArgumentException e) {
            Log.d("error", e.getMessage());
            Log.d(TAG, "sendFile - cannot be shared: " + filePath);
            return false;
        }

        sendIntent.putExtra(Intent.EXTRA_STREAM, uri);

        if (mimeType == null || mimeType.isEmpty()) {
            // fallback if mimeType not set
            mimeType = QtNative.activity().getContentResolver().getType(uri);
            // Log.d(TAG, "sendFile guessed mimeType:" + mimeType);
        } else {
            // Log.d(TAG, "sendFile w mimeType:" + mimeType);
        }

        sendIntent.setType(mimeType);

		// Log.d(TAG, "sendFile " + filePath + " " + mimeType);
        sendIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        sendIntent.addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
        return createCustomChooserAndStartActivity(sendIntent, title, requestId, uri);
    }

    public static boolean viewFile(String filePath, String title, String mimeType, int requestId) {
        if (QtNative.activity() == null) {
            return false;
        }

        // using v4 support library create the Intent from ShareCompat
        // Intent viewIntent = new Intent();
        Intent viewIntent = ShareCompat.IntentBuilder.from(QtNative.activity()).getIntent();
        viewIntent.setAction(Intent.ACTION_VIEW);

        File fileToShare = new File(filePath);

        // Using FileProvider you must get the URI from FileProvider using your
        // AUTHORITY
        Uri uri;
        try {
            uri = FileProvider.getUriForFile(QtNative.activity(), AUTHORITY, fileToShare);
        } catch (IllegalArgumentException e) {
            // Log.d(TAG, "viewFile - cannot be shared: " + filePath);
            return false;
        }

        if (mimeType == null || mimeType.isEmpty()) {
            // fallback if mimeType not set
            mimeType = QtNative.activity().getContentResolver().getType(uri);
            // Log.d(TAG, "viewFile guessed mimeType:" + mimeType);
        } else {
            // Log.d(TAG, "viewFile w mimeType:" + mimeType);
        }

        viewIntent.setDataAndType(uri, mimeType);

		// Log.d(TAG, "viewFile " + filePath + " " + mimeType);
        viewIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        viewIntent.addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);

        return createCustomChooserAndStartActivity(viewIntent, title, requestId, uri);
    }

    public static boolean editFile(String filePath, String title, String mimeType, int requestId) {
        if (QtNative.activity() == null)
            return false;

        // using v4 support library create the Intent from ShareCompat
        // Intent editIntent = new Intent();
        Intent editIntent = ShareCompat.IntentBuilder.from(QtNative.activity()).getIntent();
        editIntent.setAction(Intent.ACTION_EDIT);

        File imageFileToShare = new File(filePath);

        // Using FileProvider you must get the URI from FileProvider using your
        // AUTHORITY Uri uri = Uri.fromFile(imageFileToShare);
        Uri uri;
        try {
            uri = FileProvider.getUriForFile(QtNative.activity(), AUTHORITY, imageFileToShare);
        } catch (IllegalArgumentException e) {
            // Log.d(TAG, "editFile - cannot be shared: " + filePath);
            return false;
        }
        // Log.d(TAG, "editFile" + uri.toString());

        if (mimeType == null || mimeType.isEmpty()) {
            // fallback if mimeType not set
            mimeType = QtNative.activity().getContentResolver().getType(uri);
            // Log.d(TAG, "editFile guessed mimeType:" + mimeType);
        } else {
            // Log.d(TAG, "editFile w mimeType:" + mimeType);
        }

        editIntent.setDataAndType(uri, mimeType);

        editIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        editIntent.addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);

        return createCustomChooserAndStartActivity(editIntent, title, requestId, uri);
    }

    public static String getContentName(ContentResolver cR, Uri uri) {
        Cursor cursor = cR.query(uri, null, null, null, null);
        cursor.moveToFirst();
        int nameIndex = cursor.getColumnIndex(MediaStore.MediaColumns.DISPLAY_NAME);
        if (nameIndex >= 0) {
            return cursor.getString(nameIndex);
        } else {
            return null;
        }
    }

    public static String createFile(ContentResolver cR, Uri uri, String fileLocation) {
        String filePath = null;
        try {
            InputStream iStream = cR.openInputStream(uri);
            if (iStream != null) {
                String name = getContentName(cR, uri);
                if (name != null) {
                    filePath = fileLocation + "/" + name;
                    // Log.d(TAG, "- create File" + filePath);
                    File f = new File(filePath);
                    FileOutputStream tmp = new FileOutputStream(f);
                    // Log.d(TAG, "- create File new FileOutputStream");

                    int bufferSize = 1024;
                    byte[] buffer = new byte[bufferSize];
                    ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
                    while (iStream.available() > 0) {
                        int n = iStream.read(buffer);
                        // write to output stream
                        outputStream.write(buffer, 0, n);
                    }
                    // write output stream to file in one chunk
                    tmp.write(outputStream.toByteArray());
                    outputStream.close();
                    tmp.close();
                    iStream.close();
                    return filePath;
                } // name
            }     // iStream
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return filePath;
        } catch (IOException e) {
            e.printStackTrace();
            return filePath;
        } catch (Exception e) {
            e.printStackTrace();
            return filePath;
        }
        return filePath;
    }
}

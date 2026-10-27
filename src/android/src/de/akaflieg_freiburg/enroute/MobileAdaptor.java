/***************************************************************************
 * Copyright (C) 2019-2025 by Stefan Kebekus, stefan.kebekus@gmail.com * Copyright (C) 2020 by
 * Johannes Zellner, johannes@zellner.org * * This program is free software; you can redistribute it
 * and/or modify * it under the terms of the GNU General Public License as published by * the Free
 * Software Foundation; either version 3 of the License, or * (at your option) any later version. *
 * * This program is distributed in the hope that it will be useful, * but WITHOUT ANY WARRANTY;
 * without even the implied warranty of * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the * GNU General Public License for more details. * * You should have received a copy of the GNU
 * General Public License * along with this program; if not, write to the * Free Software
 * Foundation, Inc., * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. *
 ***************************************************************************/

package de.akaflieg_freiburg.enroute;

import org.qtproject.qt.android.QtNative;

import android.app.PendingIntent;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.Color;
import android.net.NetworkInfo;
import android.net.NetworkInfo.State;
import android.net.Uri;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;
import android.net.wifi.WifiManager.WifiLock;
import android.os.Build;
import android.os.Bundle;
import android.os.Parcelable;
import android.os.Vibrator;
import android.provider.Settings;
import android.provider.Settings.System;
import android.util.Log;
import android.view.*;
import android.window.OnBackInvokedCallback;
import android.window.OnBackInvokedDispatcher;
import androidx.core.app.ShareCompat;
import androidx.core.content.FileProvider;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import androidx.documentfile.provider.DocumentFile;

import java.io.File;

public class MobileAdaptor extends de.akaflieg_freiburg.enroute.ShareActivity {

  public static native void onLanguageChanged();
  public static native void onWifiConnected();
  public static native void onBackInvoked();
  public static native void onWindowFocusGained();

  private static MobileAdaptor m_instance;
  private static Vibrator m_vibrator;
  private static LocaleChangedReceiver m_localeChangedReceiver;
  private static WifiLock m_wifiLock;
  private static WifiManager m_wifiManager;
  private static MulticastLock m_multicastLock;
  private static BroadcastReceiver m_wifiStateChangeReceiver;
  private OnBackInvokedCallback m_backInvokedCallback;

  // reference Authority as defined in AndroidManifest.xml
  private static String AUTHORITY = "de.akaflieg_freiburg.enroute";
  private static String TAG = "IntentLauncher";

  private static final int PICK_FILE_REQUEST = 1;
  private static final int CREATE_FILE_REQUEST = 2;

  // Request code for requestNotificationPermission(). Qt numbers its own
  // permission requests counting up from 0, and QtActivityBase forwards every
  // permission result to Qt. A request code of 0 would therefore deliver our
  // result to Qt's first pending request (for instance the location request
  // made at startup). Use a value Qt never reaches, and keep the result away
  // from Qt, see onRequestPermissionsResult().
  private static final int NOTIFICATION_PERMISSION_REQUEST_CODE = 0x7E000001;

  /**
   * Request POST_NOTIFICATIONS permission on Android 13+.
   * Safe to call from any context — uses m_instance which IS the Activity.
   */
  public static void requestNotificationPermission() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU
        && m_instance != null
        && m_instance.checkSelfPermission(android.Manifest.permission.POST_NOTIFICATIONS)
               != PackageManager.PERMISSION_GRANTED) {
      m_instance.requestPermissions(
          new String[]{android.Manifest.permission.POST_NOTIFICATIONS},
          NOTIFICATION_PERMISSION_REQUEST_CODE);
    }
  }

  @Override
  public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults)
  {
    // The result of our own notification request needs no handling:
    // notifications simply work or not. Do not forward it to Qt, which
    // would look for a pending request of its own with this code.
    if (requestCode == NOTIFICATION_PERMISSION_REQUEST_CODE)
    {
      return;
    }
    super.onRequestPermissionsResult(requestCode, permissions, grantResults);
  }

  // Tell C++ whenever the window regains focus from the OS, for instance after
  // a permission dialog closed or the user returned from the system settings.
  // Qt's application state cannot be used for this: in multi-window mode,
  // QtActivityBase.onPause() keeps the app "active", so Qt reports no change.
  @Override
  public void onWindowFocusChanged(boolean hasFocus)
  {
    super.onWindowFocusChanged(hasFocus);
    if (hasFocus)
    {
      try
      {
        onWindowFocusGained();
      }
      catch (UnsatisfiedLinkError e)
      {
        // The native library is not loaded yet, the app is still starting.
      }
    }
  }

  public MobileAdaptor() 
  {
    m_instance = this;
  }

  @Override
  public void onCreate(Bundle savedInstanceState) 
  {
    super.onCreate(savedInstanceState);

    // Get WiFi manager and stateChangeReceiver
    m_wifiManager = (WifiManager) m_instance.getSystemService(Context.WIFI_SERVICE);
    m_wifiStateChangeReceiver = new WifiStateChangeReceiver();

    // Acquire Multicast lock
    m_multicastLock = m_wifiManager.createMulticastLock("multicastLock");
    m_multicastLock.setReferenceCounted(true);
    m_multicastLock.acquire();

    // Be informed when locale changes
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) 
    {
      m_localeChangedReceiver = new LocaleChangedReceiver();
      IntentFilter filter = new IntentFilter(Intent.ACTION_LOCALE_CHANGED);
      registerReceiver(m_localeChangedReceiver, filter);
    }

    // Back handling, see forwardBackToQt() below. The predictive-back callback
    // is registered in onResume(), the legacy key is intercepted in
    // onKeyDown()/onKeyUp().
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU)
    {
      m_backInvokedCallback = new OnBackInvokedCallback() {
        @Override
        public void onBackInvoked() {
          forwardBackToQt();
        }
      };
    }
  }

  @Override
  public void onResume()
  {
    super.onResume();

    // Since the app targets API 36, Android 16+ enables predictive back by
    // default: the system no longer dispatches KEYCODE_BACK but invokes an
    // OnBackInvokedCallback. Qt's QtActivityBase.onResume() registers its own
    // callback at PRIORITY_DEFAULT. The dispatcher invokes the most recently
    // registered callback of the highest priority, so ours must be registered
    // here, after super.onResume(), to take precedence over Qt's. (Registered
    // in onCreate() it is never invoked.) Qt unregisters in onPause(); we do
    // the same.
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU && m_backInvokedCallback != null)
    {
      getOnBackInvokedDispatcher().registerOnBackInvokedCallback(
          OnBackInvokedDispatcher.PRIORITY_DEFAULT, m_backInvokedCallback);
    }
  }

  @Override
  public void onPause()
  {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU && m_backInvokedCallback != null)
    {
      getOnBackInvokedDispatcher().unregisterOnBackInvokedCallback(m_backInvokedCallback);
    }
    super.onPause();
  }

  // Legacy Back key (Android < 13, or predictive back disabled). Qt's
  // QtActivityBase.onKeyDown()/onKeyUp() would forward it synchronously, see
  // forwardBackToQt().
  @Override
  public boolean onKeyDown(int keyCode, KeyEvent event)
  {
    if (keyCode == KeyEvent.KEYCODE_BACK)
    {
      return true; // handled in onKeyUp()
    }
    return super.onKeyDown(keyCode, event);
  }

  @Override
  public boolean onKeyUp(int keyCode, KeyEvent event)
  {
    if (keyCode == KeyEvent.KEYCODE_BACK)
    {
      forwardBackToQt();
      return true;
    }
    return super.onKeyUp(keyCode, event);
  }

  // Qt 6.12 delivers the Back key to the Qt thread synchronously:
  // QtInputDelegate.keyDown() blocks the Android UI thread in
  // QWindowSystemInterface::flushWindowSystemEvents() until the Qt thread has
  // processed the key. The Qt thread, however, waits for the UI thread whenever
  // it creates the window surface, which happens on the first expose at
  // startup. A Back press while the splash screen is shown therefore
  // deadlocks both threads and ends in an ANR (Google Play reports for 3.5.4).
  // We bypass Qt's handler and hand the key to the Qt thread asynchronously.
  private void forwardBackToQt()
  {
    try
    {
      onBackInvoked();
    }
    catch (UnsatisfiedLinkError e)
    {
      // The native library is not loaded yet, the app is still starting.
    }
  }

  @Override
  public void onDestroy() {
    // Unregister back callback
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU && m_backInvokedCallback != null) {
      getOnBackInvokedDispatcher().unregisterOnBackInvokedCallback(m_backInvokedCallback);
      m_backInvokedCallback = null;
    }

    // Release WiFi lock
    if (m_wifiLock != null) {
      if (m_wifiLock.isHeld() == true) {
        m_wifiLock.release();
      }
    }

    // Release multicast lock
    if (m_multicastLock != null) {
      m_multicastLock.release();
      m_multicastLock = null;
    }

    // Unregister the WiFi state change receiver
    if (m_wifiStateChangeReceiver != null) {
      try {
        m_instance.unregisterReceiver(m_wifiStateChangeReceiver);
        m_wifiStateChangeReceiver = null;
      } catch (IllegalArgumentException e) {
        Log.d("enroute flight navigation", "Exception in onDestroy: " + e.toString());
      }
    } else {
      Log.d("enroute flight navigation", "onDestroy: m_wifiStateChangeReceiver == null");
    }

    super.onDestroy();
  }

  //
  // Static Methods
  //

  // Returns the device name
  public static String deviceName() {
    return android.os.Build.MANUFACTURER + " " + android.os.Build.PRODUCT + " ("
        + android.os.Build.MODEL + ")";
  }

  // The following methods return the safe-area insets of the window -- the
  // union of the virtual keyboard, system bars and display cutout -- in
  // physical pixels. These are the values that Android reports to this
  // window, so they are also correct in split-screen mode and in any
  // orientation, where Qt's own safe-area margins are not reliable. The
  // keyboard inset is zero while the keyboard is hidden.

  private static double safeInset(int side) {
    if (m_instance == null) {
      return 0.0;
    }
    WindowInsets insets = m_instance.getWindow().getDecorView().getRootWindowInsets();
    if (insets == null) {
      return 0.0;
    }
    if (Build.VERSION.SDK_INT >= 30) {
      android.graphics.Insets in = insets.getInsets(WindowInsets.Type.ime()
          | WindowInsets.Type.systemBars() | WindowInsets.Type.displayCutout());
      switch (side) {
        case 0: return in.left;
        case 1: return in.top;
        case 2: return in.right;
        default: return in.bottom;
      }
    }
    switch (side) {
      case 0: return insets.getSystemWindowInsetLeft();
      case 1: return insets.getSystemWindowInsetTop();
      case 2: return insets.getSystemWindowInsetRight();
      default: return insets.getSystemWindowInsetBottom();
    }
  }

  public static double safeInsetLeft() {
    return safeInset(0);
  }

  public static double safeInsetTop() {
    return safeInset(1);
  }

  public static double safeInsetRight() {
    return safeInset(2);
  }

  public static double safeInsetBottom() {
    return safeInset(3);
  }

  // The following methods return the size of the application window in
  // physical pixels, as laid out by the Android window manager. In
  // split-screen mode, this is the size of the pane that the app occupies.
  // Qt's own window size is wrong there on Android 10 and below, see
  // PlatformAdaptor_Android.cpp.

  private static double windowExtent(int axis) {
    if (m_instance == null) {
      return 0.0;
    }
    Window window = m_instance.getWindow();
    if (window == null) {
      return 0.0;
    }
    View decorView = window.getDecorView();
    return (axis == 0) ? decorView.getWidth() : decorView.getHeight();
  }

  public static double windowWidth() {
    return windowExtent(0);
  }

  public static double windowHeight() {
    return windowExtent(1);
  }

  // Choose dark status-bar icons (for light content underneath the status
  // bar) or light ones. Only the status bar is affected; the navigation bar
  // keeps the appearance that Qt derives from the application color scheme.
  // androidx maps the request to WindowInsetsController on API 30+ and to the
  // legacy decor view flags on older versions. Must run on the UI thread.
  public static void setDarkStatusBarIcons(boolean dark) {
    if (m_instance == null) {
      return;
    }
    m_instance.runOnUiThread(() -> {
      Window window = m_instance.getWindow();
      if (window == null) {
        return;
      }
      WindowInsetsControllerCompat controller =
          WindowCompat.getInsetsController(window, window.getDecorView());
      controller.setAppearanceLightStatusBars(dark);
    });
  }

  /*
   * Get the SSID of the current WIFI network, if any. Returns a string like "<unknown SSID>"
   * otherwise
   */
  public static String getSSID() {

    if (m_wifiManager == null) {
      m_wifiManager = (WifiManager) m_instance.getSystemService(Context.WIFI_SERVICE);
    }
    WifiInfo wifiInfo = m_wifiManager.getConnectionInfo();
    return wifiInfo.getSSID();

  }

  /*
   * Get the manufacturer of the current device.
   */
  public static String manufacturer() {

    return android.os.Build.MANUFACTURER;

  }

  /*
   * Acquire or release a WiFi lock
   */
  public static void lockWiFi(boolean on) {

    // Get WiFi lock w/o reference counting
    if (m_wifiLock == null) {
      m_wifiLock =
          m_wifiManager.createWifiLock(WifiManager.WIFI_MODE_FULL, "Traffic Receiver Wi-Fi Lock");
      m_wifiLock.setReferenceCounted(false);
    }

    // Paranoid safety checks
    if (m_wifiLock == null) {
      return;
    }

    // Acquire lock
    if (on == true) {

      try {
        m_wifiLock.acquire();
      } catch (Exception e) {
        Log.d("enroute flight navigation", "Exception in lockWiFi(): " + e.toString());
      }

      return;
    }

    // Release lock
    if (m_wifiLock.isHeld() == true) {
      m_wifiLock.release();
    }

  }

  /* Begin to monitor network changes */
  public static void startWiFiMonitor() {

    // Look for WiFi changes
    IntentFilter intentFilter = new IntentFilter();
    intentFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
    m_instance.registerReceiver(m_wifiStateChangeReceiver, intentFilter);

  }

  /*
   * If the system setting for haptic feedback is "on", then vibrate once briefly
   */
  public static void vibrateBrief() {

    // Get system settings for haptic feedback
    int haptic = Settings.System.getInt(m_instance.getContentResolver(),
        Settings.System.HAPTIC_FEEDBACK_ENABLED, 1);

    // If systems settings want vibrate, then do vibrate
    if (haptic != 0) {
      if (m_vibrator == null) {
        m_vibrator = (Vibrator) m_instance.getSystemService(Context.VIBRATOR_SERVICE);
      }
      m_vibrator.vibrate(20);
    }

  }

  /*
   * If the system setting for haptic feedback is "on", then vibrate once for a longer time
   */
  public static void vibrateLong() {

    // Get system settings for haptic feedback
    int haptic = Settings.System.getInt(m_instance.getContentResolver(),
        Settings.System.HAPTIC_FEEDBACK_ENABLED, 1);

    // If systems settings want vibrate, then do vibrate
    if (haptic != 0) {
      if (m_vibrator == null) {
        m_vibrator = (Vibrator) m_instance.getSystemService(Context.VIBRATOR_SERVICE);
      }
      m_vibrator.vibrate(500);
    }

  }

  /**
   * SEND (== "share") a file to another application.
   *
   * The receiving app is _not_ supposed to handle the mime type but it is rather supposed to
   * _transport_ the file. Examples for receiving apps are email, messenger or file managers.
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

  //
  // Private Methods
  //
  /**
   * common implementation for both the sendFile() and viewFile() methods.
   *
   * Creates an appropriate intent and does the necessary settings. Creates a chooser with the
   * intent and calls startActivity() to fire the intent.
   *
   * This method does _not_ block until the content has been share with another app but rather
   * returns immediately (if action == 0). Therefore the receiving apps can handle the file URL
   * while enroute stays active an is not blocked. Effectively this means that both the sending and
   * the receiving app will run concurrently.
   *
   * @param filePath the path of the file to send.
   * @param mimeType the mime type of the file to send.
   * @param action either Intent.ACTION_SEND or Intent.ACTION_VIEW
   *
   * @return true if the intent was launched otherwise false
   */
  private static boolean sendOrViewFile(String filePath, String mimeType, String action) {

    if (m_instance == null) {
      return false;
    }

    // Intent intent = new Intent();
    // using v4 support library create the Intent from ShareCompat
    Intent intent = ShareCompat.IntentBuilder.from(m_instance).getIntent();
    intent.setAction(action);

    Uri uri = fileToUri(filePath);

    if (uri == null) {
      return false;
    }

    if (mimeType == null || mimeType.isEmpty()) {
      mimeType = m_instance.getContentResolver().getType(uri);
    }

    // Remove parameters such as "charset=UTF-8". Intent resolution compares the
    // complete type string, so "text/plain;charset=UTF-8" would not reach apps
    // whose intent filters list "text/plain".
    mimeType = Intent.normalizeMimeType(mimeType);

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
   * Open a file picker to select a file.
   *
   * This method opens a file picker to select a file of a given mime type. This is necessary to
   * work around a bug in Qt which does not allow to select files with a specific extension and does
   * not support remote files.
   *
   * https://bugreports.qt.io/browse/QTBUG-118154
   *
   * @param mimeType the mime type of the file to select.
   *
   */
  public void openFilePicker(String mimeType) {
    Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
    intent.addCategory(Intent.CATEGORY_OPENABLE);
    if (!mimeType.isEmpty()) {
      intent.setType(mimeType);
    } else {
      intent.setType("*/*");
    }
    startActivityForResult(intent, PICK_FILE_REQUEST);
  }

  /**
   * Open the system 'create document' dialog (Storage Access Framework).
   *
   * This method lets the user choose a location and name for a new file. The result is reported
   * asynchronously through the native method onCreateFileResult().
   *
   * @param mimeType the mime type of the file to create.
   *
   * @param suggestedName suggested display name for the new file, including suffix.
   *
   */
  public void createFile(String mimeType, String suggestedName) {
    Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
    intent.addCategory(Intent.CATEGORY_OPENABLE);
    if (!mimeType.isEmpty()) {
      intent.setType(mimeType);
    } else {
      intent.setType("application/octet-stream");
    }
    intent.putExtra(Intent.EXTRA_TITLE, suggestedName);
    startActivityForResult(intent, CREATE_FILE_REQUEST);
  }

  /**
   * Result of file picking
   */
  public static native void setFileReceived(String fileName, String unmingled);

  /**
   * Result of the 'create document' dialog. An empty string means the user aborted.
   */
  public static native void onCreateFileResult(String uri);

  @Override
  protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    if (requestCode == PICK_FILE_REQUEST && resultCode == RESULT_OK && data != null) {
      Uri uri = data.getData();
      if (uri != null) {
        DocumentFile docFile = DocumentFile.fromSingleUri(this, uri);
        // getName() can return null if the URI has no backing content provider
        String name = (docFile != null && docFile.getName() != null) ? docFile.getName() : "";
        setFileReceived(uri.toString(), name);
      }
    }
    if (requestCode == CREATE_FILE_REQUEST) {
      // Cancellation must be reported, so that the C++ side can clear its
      // pending save buffer.
      Uri uri = (resultCode == RESULT_OK && data != null) ? data.getData() : null;
      onCreateFileResult(uri != null ? uri.toString() : "");
    }
    super.onActivityResult(requestCode, resultCode, data);
  }

  /**
   * create uri from file path.
   *
   * android content provoiders operate with Uri's which represent the content scheme (e.g. file://
   * or content://). This methods converts the absolute file path in the sharing directory into a
   * valid Uri.
   *
   * @param filePath the absolut path of the file in the cache directory.
   *
   * @return Uri the corresponding uri
   */

  private static Uri fileToUri(String filePath) {

    File fileToShare = new File(filePath);

    // Using FileProvider you must get the URI from FileProvider using your
    // AUTHORITY
    // Uri uri = Uri.fromFile(imageFileToShare);
    Uri uri;
    try {
      return FileProvider.getUriForFile(m_instance, AUTHORITY, fileToShare);
    } catch (IllegalArgumentException e) {
      Log.d(TAG, "error" + e.getMessage());
      return null;
    }
  }

  /**
   * create a chooser and start activity.
   *
   * The chooser takes care of not sharing with or sending to the own app. This is done by
   * excluding the own activity via Intent.EXTRA_EXCLUDE_COMPONENTS.
   *
   * The PackageManager is deliberately not asked beforehand whether there are apps that can handle
   * the intent. Since Android 11, package visibility filtering hides most installed apps from such
   * queries, so that the check would reject intents that installed apps can handle. The system
   * chooser is not subject to this filtering and lists all suitable apps.
   *
   * The chooser intent is then started with startActivity() which will return immediately and not
   * wait for the chooser result. This way, both enroute and the receiving app can run concurrently
   * and enroute is _not_ blocked until the receiving app terminates.
   *
   * @param theIntent intent to be handled
   *
   * @return true if the intent was launched otherwise false
   */
  private static boolean customStartActivity(Intent theIntent) {

    // We don't want to share with our own app, so exclude it. MobileAdaptor is
    // the only activity of enroute that accepts SEND and VIEW intents.
    Intent chooserIntent = Intent.createChooser(theIntent, null /* title */);
    chooserIntent.putExtra(Intent.EXTRA_EXCLUDE_COMPONENTS,
        new Parcelable[] {new ComponentName(m_instance, MobileAdaptor.class)});
    chooserIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);

    // do NOT use startActivityForResult as it will block
    // enroute until the receiving app terminates.
    try {
      m_instance.startActivity(chooserIntent);
    } catch (ActivityNotFoundException e) {
      Log.d(TAG, "Cannot start chooser: " + e.getMessage());
      return false;
    }
    return true;
  }

  //
  // Embedded classes
  //

  private class LocaleChangedReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
      onLanguageChanged();
    }
  }

  private class WifiStateChangeReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
      NetworkInfo info = intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
      // The extra is documented as nullable; an NPE here would crash the app
      if (info != null && info.getState() == State.CONNECTED) {
        onWifiConnected();
      }
    }
  }

}

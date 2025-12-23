package de.akaflieg_freiburg.enroute;

import android.content.Context;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.util.Log;

import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialProber;

import java.util.ArrayList;
import java.util.List;

public class UsbSerialHelper 
{
    private static final String TAG = "UsbSerialHelper";
    private static Context appContext;

    /**
     * Initialize the helper with application context. Call this from your main
     * Activity's onCreate() method.
     */
    public static void initialize(Context context) 
    {
        appContext = context.getApplicationContext();
    }

    /**
     * Lists all available USB serial devices. Returns an array of device
     * names/descriptions. Can be called from C++ via JNI.
     *
     * @return String array containing device names, empty array if none found
     * or on error
     */
    public static String[] listSerialDevices() 
    {
        List<String> deviceList = new ArrayList<>();
        try 
        {
            // Check if context is initialized
            if (appContext == null) 
            {
                Log.e(TAG, "Context not initialized. Call initialize() first.");
                return new String[0];
            }

            // Get USB manager
            UsbManager usbManager = (UsbManager) appContext.getSystemService(Context.USB_SERVICE);
            if (usbManager == null) 
            {
                Log.e(TAG, "Failed to get UsbManager");
                return new String[0];
            }

            // Find all available drivers from attached devices
            List<UsbSerialDriver> availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(usbManager);
            if (availableDrivers.isEmpty()) 
            {
                Log.i(TAG, "No USB serial devices found");
                return new String[0];
            }

            // Iterate through all found drivers/devices
            for (UsbSerialDriver driver : availableDrivers) 
            {
                try 
                {
                    UsbDevice device = driver.getDevice();
                    if (device == null) 
                    {
                        continue;
                    }

                    // Get product name, fallback to device name
                    String deviceName = device.getProductName();
                    if (deviceName == null || deviceName.trim().isEmpty()) {
                        deviceName = device.getDeviceName();
                    }

                    // Add additional info for disambiguation
                    String deviceInfo = String.format("%s (VID:0x%04X PID:0x%04X)",
                            deviceName != null ? deviceName : "Unknown Device",
                            device.getVendorId(),
                            device.getProductId());

                    deviceList.add(deviceInfo);
                    Log.d(TAG, "Found device: " + deviceInfo);

                } 
                catch (Exception e) 
                {
                    Log.e(TAG, "Error processing device: " + e.getMessage(), e);
                    // Continue with next device
                }
            }
        } 
        catch (SecurityException e) 
        {
            Log.e(TAG, "Security exception - missing USB permission: " + e.getMessage(), e);
        } 
        catch (Exception e) 
        {
            Log.e(TAG, "Unexpected error listing devices: " + e.getMessage(), e);
        }
        return deviceList.toArray(new String[0]);
    }

}


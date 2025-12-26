package de.akaflieg_freiburg.enroute;

import android.content.Context;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.util.Log;

import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class UsbSerialHelper 
{
    private static class ConnectionInfo 
    {
        UsbSerialPort port;
        UsbDeviceConnection connection;

        ConnectionInfo(UsbSerialPort port, UsbDeviceConnection connection) 
        {
            this.port = port;
            this.connection = connection;
        }
    }

    private static final String TAG = "UsbSerialHelper";
    private static final String ACTION_USB_PERMISSION = "de.akaflieg_freiburg.enroute.USB_PERMISSION";
    private static Context appContext;
    private static final Map<String, ConnectionInfo> connections = new HashMap<>();


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
                    deviceList.add(deviceName);
                    Log.d(TAG, "Found device: " + deviceName);
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


    /**
     * Opens a USB serial connection by device path.
     *
     * @param devicePath Device path from listSerialDevices() (e.g.,
     * "/dev/bus/usb/001/002")
     *
     * @param baudRate Baud rate (e.g., 9600, 115200)
     *
     * @param dataBits Data bits (5, 6, 7, or 8)
     *
     * @param stopBits Stop bits (1 or 2)
     *
     * @param parity Parity: 0=NONE, 1=ODD, 2=EVEN, 3=MARK, 4=SPACE
     *
     * @return true if opened successfully, false on error
     */
    public static boolean openDevice(String devicePath) 
    {
        try 
        {
            if (appContext == null) 
            {
                Log.e(TAG, "Context not initialized");
                return false;
            }

            if (devicePath == null || devicePath.isEmpty()) 
            {
                Log.e(TAG, "Invalid device path");
                return false;
            }

            // Check if already open
            if (connections.containsKey(devicePath)) 
            {
                Log.w(TAG, "Device already open: " + devicePath);
                return false;
            }

            UsbManager usbManager = (UsbManager) appContext.getSystemService(Context.USB_SERVICE);
            if (usbManager == null) 
            {
                Log.e(TAG, "Failed to get UsbManager");
                return false;
            }

            List<UsbSerialDriver> availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(usbManager);
            
            // Find the driver matching the device path
            UsbSerialDriver driver = null;
            UsbDevice device = null;
            for (UsbSerialDriver d : availableDrivers) 
            {
                UsbDevice dev = d.getDevice();
                if (dev != null && (dev.getDeviceName().equals(devicePath) || dev.getProductName().equals(devicePath))) 
                {
                    driver = d;
                    device = dev;
                    break;
                }
            }

            if (driver == null || device == null) 
            {
                Log.e(TAG, "Device not found: " + devicePath);
                return false;
            }

            // Check and request permission if needed
            if (!usbManager.hasPermission(device)) 
            {
                Log.i(TAG, "Requesting USB permission for: " + devicePath);
                    
                // Store pending request
                /*
                pendingPermissions.put(devicePath, 
                    new PendingPermissionRequest(devicePath, baudRate, dataBits, stopBits, parity));
                */
                   
                // Request permission
                android.content.Intent intent = new android.content.Intent(ACTION_USB_PERMISSION);
                intent.setPackage(appContext.getPackageName()); // Make intent explicit for Android 14+
                android.app.PendingIntent permissionIntent;
                if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.S) 
                {
                    permissionIntent = android.app.PendingIntent.getBroadcast(
                        appContext, 0, 
                        intent,
                        android.app.PendingIntent.FLAG_IMMUTABLE
                    );
                }
                else
                {
                    permissionIntent = android.app.PendingIntent.getBroadcast(
                        appContext, 0, 
                        intent,
                        0
                    );
                }
                usbManager.requestPermission(device, permissionIntent);
                return false;
            }

            // Open connection
            UsbDeviceConnection connection = usbManager.openDevice(device);
            if (connection == null) 
            {
                Log.e(TAG, "Failed to open USB device connection: " + devicePath);
                return false;
            }

            // Get the first port (most devices have only one)
            List<UsbSerialPort> ports = driver.getPorts();
            if (ports.isEmpty()) 
            {
                connection.close();
                Log.e(TAG, "No ports available on device: " + devicePath);
                return false;
            }

            UsbSerialPort port = ports.get(0);
            
            try 
            {
                port.open(connection);
                
                // Store connection
                connections.put(devicePath, new ConnectionInfo(port, connection));
                
                Log.i(TAG, "Opened device: " + devicePath);                
            } 
            catch (IOException e) 
            {
                connection.close();
                Log.e(TAG, "Failed to configure port: " + e.getMessage(), e);
                return false;
            }

        } 
        catch (Exception e) 
        {
            Log.e(TAG, "Error opening device: " + e.getMessage(), e);
            return false;
        }
        return true;
    }


    /**
     * Checks if a connection is open.
     *
     * @param devicePath Device path
     * 
     * @return true if device is open
     */
    public static boolean isOpen(String devicePath) 
    {
        ConnectionInfo info = connections.get(devicePath);
        return info != null && info.port != null;
    }


    /**
     * Sets parameters for an open USB serial connection by device path.
     *
     * @param devicePath Device path from listSerialDevices() (e.g.,
     * "/dev/bus/usb/001/002")
     *
     * @param baudRate Baud rate (e.g., 9600, 115200)
     *
     * @param dataBits Data bits (5, 6, 7, or 8)
     *
     * @param stopBits Stop bits (1 or 2)
     *
     * @param parity Parity: 0=NONE, 1=ODD, 2=EVEN, 3=MARK, 4=SPACE
     *
     * @return true if parameters were set successfully, false on error
     */
    public static boolean setParameters(String devicePath, int baudRate, int stopBits, int flowControl) 
    {
        ConnectionInfo info = connections.get(devicePath);
        if (info == null) 
        {
            Log.e(TAG, "Device not open: " + devicePath);
            return false;
        }

        try
        {
            info.port.setParameters(baudRate, 8, stopBits, UsbSerialPort.PARITY_NONE);
            if (flowControl == 0)
                info.port.setFlowControl(UsbSerialPort.FlowControl.NONE);
            else if (flowControl == 1)
                info.port.setFlowControl(UsbSerialPort.FlowControl.RTS_CTS);
            else if (flowControl == 2)
                info.port.setFlowControl(UsbSerialPort.FlowControl.XON_XOFF);
            Log.i(TAG, "Set parameters on device: " + devicePath);
        }
        catch (Exception e)
        {
            Log.e(TAG, "Error setting parameters on device: " + e.getMessage(), e);
            return false;
        }
        return true;
    }
    

    /**
     * Closes a serial port connection.
     * 
     * @param devicePath Device path from listSerialDevices()
     * 
     * @return true if closed successfully, false otherwise
     */
    public static boolean close(String devicePath) 
    {
        ConnectionInfo info = connections.remove(devicePath);
        if (info == null) 
        {
            Log.e(TAG, "Device not open: " + devicePath);
            return false;
        }

        try 
        {
            if (info.port != null) 
            {
                info.port.close();
            }
            if (info.connection != null) 
            {
                info.connection.close();
            }
            Log.i(TAG, "Closed connection: " + devicePath);
        } 
        catch (IOException e) 
        {
            Log.e(TAG, "Error closing " + devicePath + ": " + e.getMessage(), e);
            return false;
        } 
        catch (Exception e) 
        {
            Log.e(TAG, "Unexpected error closing: " + e.getMessage(), e);
            return false;
        }
        return true;
    }


    /**
     * Reads data from the serial port.
     *
     * @param devicePath Device path from listSerialDevices()
     * 
     * @param maxBytes Maximum number of bytes to read
     * 
     * @param timeoutMs Timeout in milliseconds
     * 
     * @return Bytes read, or null on error/timeout
     */
    public static byte[] read(String devicePath, int maxBytes, int timeoutMs) 
    {
        ConnectionInfo info = connections.get(devicePath);
        if (info == null) 
        {
            Log.e(TAG, "Device not open: " + devicePath);
            return null;
        }

        try 
        {
            byte[] buffer = new byte[maxBytes];
            int bytesRead = info.port.read(buffer, timeoutMs);
            
            if (bytesRead <= 0) 
            {
                return new byte[0]; // Timeout or no data
            }
            
            // Return only the bytes actually read
            byte[] result = new byte[bytesRead];
            System.arraycopy(buffer, 0, result, 0, bytesRead);
            return result;
            
        } 
        catch (IOException e) 
        {
            Log.e(TAG, "Read error on " + devicePath + ": " + e.getMessage(), e);
            return null;
        } 
        catch (Exception e) {
            Log.e(TAG, "Unexpected error reading: " + e.getMessage(), e);
            return null;
        }
    }


    /**
     * Writes data to the serial port.
     * 
     * @param devicePath Device path from listSerialDevices()
     * 
     * @param data Data to write
     * 
     * @param timeoutMs Timeout in milliseconds
     * 
     * @return Number of bytes written, or -1 on error
     */
    public static int write(String devicePath, byte[] data, int timeoutMs) 
    {
        ConnectionInfo info = connections.get(devicePath);
        if (info == null) 
        {
            Log.e(TAG, "Device not open: " + devicePath);
            return -1;
        }

        if (data == null || data.length == 0) 
        {
            return 0;
        }

        try 
        {
            info.port.write(data, timeoutMs);
            return data.length;
            
        } 
        catch (IOException e) 
        {
            Log.e(TAG, "Write error on " + devicePath + ": " + e.getMessage(), e);
            return -1;
        } 
        catch (Exception e) 
        {
            Log.e(TAG, "Unexpected error writing: " + e.getMessage(), e);
            return -1;
        }
    }

}


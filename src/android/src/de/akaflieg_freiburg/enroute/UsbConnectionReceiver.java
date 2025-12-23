package org.qtproject.example.appqtjenny_consumer;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;

public class UsbConnectionReceiver extends BroadcastReceiver
{
    @Override
    public void onReceive(Context context, Intent intent)
    {
        String action = intent.getAction();

        if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action))
        {
            UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
            if (device != null)
            {
                // Call native C++ function
                notifyUsbDeviceAttached(
                device.getDeviceName(),
                device.getVendorId(),
                device.getProductId(),
                device.getDeviceClass()
                );
            }
        }
        else if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action))
        {
            UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
            if (device != null)
            {
                notifyUsbDeviceDetached(device.getDeviceName());
            }
        }
    }

    // Native methods implemented in C++
    private native void notifyUsbDeviceAttached(String deviceName, int vendorId, int productId, int deviceClass);
    private native void notifyUsbDeviceDetached(String deviceName);
}

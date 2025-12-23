package de.akaflieg_freiburg.enroute;

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

        if (!UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action) && !UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action))
        {
            return;
        }
        UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
        if (device == null)
        {
            return;
        }
        onSerialPortConnectionsChanged();
    }

    // Native methods implemented in C++
    private native void onSerialPortConnectionsChanged();
}

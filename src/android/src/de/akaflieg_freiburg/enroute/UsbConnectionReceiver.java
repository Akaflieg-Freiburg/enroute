package de.akaflieg_freiburg.enroute;

import android.content.ActivityNotFoundException;
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

        try
        {
            onSerialPortConnectionsChanged();
        }
        catch (UnsatisfiedLinkError e)
        {
            // The native libraries are not loaded, so Enroute is not running.
            // Start it when a device was attached. Do nothing on detach: there
            // is nothing to update, and starting the app because a device was
            // unplugged would be a surprise for the user.
            if (!UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action))
            {
                return;
            }
            Intent launch = context.getPackageManager()
                    .getLaunchIntentForPackage(context.getPackageName());
            if (launch == null)
            {
                return;
            }
            launch.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            try
            {
                context.startActivity(launch);
            }
            catch (ActivityNotFoundException | SecurityException | IllegalStateException ex)
            {
                // The activity cannot be started right now, e.g. because the
                // package is being updated or the activity is disabled. An
                // exception escaping a broadcast receiver kills the process,
                // so give up quietly.
            }
        }
    }

    // Native methods implemented in C++
    private native void onSerialPortConnectionsChanged();
}

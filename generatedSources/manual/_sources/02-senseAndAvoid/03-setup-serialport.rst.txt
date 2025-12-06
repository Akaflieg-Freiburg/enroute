Connect via the Serial Port
===========================

**Enroute Flight Navigation** is able to connect to your traffic data receiver
using serial port connections. Connections via USB are also supported. Compared
with Wi-Fi, serial port connections are equally reliable, but require manual
configuration. By nature, serial ports support only point-to-point connections,
so that only one single app can access traffic data at any given time.  Pilots
and co-pilots must therefore decide who gets to see traffic data.

.. note:: **Enroute Flight Navigation** expects a stream for FLARM/NMEA sentences
    from the serial port device. **Enroute Flight Navigation** is not able to
    integrate into a CAN-Bus environment.

.. note:: Serial port devices are currently not supported on the Android 
    platform. For that reason, serial port communication is not available at 
    all on Android devices. 

.. note:: Serial port devices are not supported by the iOS platform. For that reason,
    serial port communication is not available at all on iPhone or iPad devices. 


One-time Setup
--------------

Step 0: Before You Connect
^^^^^^^^^^^^^^^^^^^^^^^^^^

Before you try to connect this app to your traffic receiver, make sure that the
following conditions are met.

- The hardware is set up.
- Your traffic receiver is switched on and broadcasts FLARM/NMEA via its serial
  port. 
- Your device is connected to the serial port and no other app uses the serial
  port connection.



Step 1: Configure a Data Connection to the Serial Port Device
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Follow the steps described in the Section :ref:`SettingsDataConnectionsPage`.
You will need to know or guess the name of the serial port on your device.


Step 2: Check Connectivity
^^^^^^^^^^^^^^^^^^^^^^^^^^

After the data connection to the serial port device has been configured in Step
1, everything else should be automatic.  To check, open the main menu and
navigate to the "Information" menu.  If the entry "Traffic Receiver" is
highlighted in green, then **Enroute Flight Navigation** has already found the
traffic receiver and has connected to it. Congratulations, you are done!

If the entry "Traffic Receiver" is not highlighted in green, then something has
gone wrong.  Open the main menu and go to "Information/Traffic Receiver".  Make
sure that your device is in discoverable mode and use the button "Reconnect".
Failing that, you are out of luck.


Daily Operations
----------------

Once things are set up properly, your device should automatically detect the
traffic receiver's Bluetooth adaptor and connect to the traffic data stream
whenever you go flying.  We recommend the following procedure.

- Connect your device to the serial port cable.
- After you power on the avionics and the traffic receiver has booted, start
  **Enroute Flight Navigation**.
- **Enroute Flight Navigation** will connect to your traffic data receiver via
  the configured serial port connection and show traffic information in the
  moving map.
- If the data connection gets lost in mid-flight, **Enroute Flight Navigation**
  will automatically try to re-connect.


Troubleshooting
---------------

Permission Issues on Linux Systems
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On Linux systems, serial ports are typically accessed through device files such
as `/dev/ttyUSB0`, `/dev/ttyACM0`, or `/dev/ttyS0`. By default, these devices
are often restricted to specific user groups, which means **Enroute Flight
Navigation** may not be able to access the serial port even though the hardware
is properly connected.

**Symptoms**

If you experience permission issues, you might see one or more of the following:

- The app cannot detect or connect to the serial port device
- Error messages about "permission denied"
- The serial port device appears in system tools but not in the app

**Solution: Add Your User to the dialout Group (Recommended)**

The most common and permanent solution is to add your user account to the group
that has permission to access serial ports. On most Linux distributions, this
group is called `dialout`.

1. Open a terminal
2. Run the following command (replace `username` with your actual username)::

     sudo usermod -a -G dialout username

3. Log out completely and log back in (or reboot your computer) for the changes
   to take effect
4. Verify the change by running: `groups` - you should see `dialout` in the list


**Distribution-Specific Notes**

- **Ubuntu/Debian**: The group is typically `dialout`
- **Fedora/RHEL/CentOS**: The group is typically `dialout` or `uucp`
- **Arch Linux**: The group is typically `uucp` or `lock`
- **openSUSE**: The group is typically `dialout`

If `dialout` doesn't work, check which group owns your serial port device by
running::

  ls -l /dev/tty*

The output will show the group name (usually the third column from the left),
and you can add your user to that group instead.


**Still Having Issues?**

If permission issues persist after following these steps:

- Make sure you completely logged out and back in after adding yourself to the
  group
- Check if SELinux or AppArmor policies are blocking access (advanced users)
- Verify the serial device is working by testing with another application like
  `minicom` or `screen`
- Check system logs with `dmesg | tail` to see if there are any hardware or
  driver issues

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

After the data connection to the Bluetooth Classic device has been configured in
Step 1, everything else should be automatic.  To check, open the main menu and
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

Connect via Bluetooth
=====================

**Enroute Flight Navigation** is able to connect to your traffic data receiver
using the Bluetooth Classic radio standard. Compared with Wi-Fi, Bluetooth
connections are less reliable and require manual configuration. We found that
many Bluetooth adaptors are build with cheap and unreliable hardware and
implement industry standards only partially, if at all.

Bluetooth Classic supports only point-to-point connections, so that only one single app
can access traffic data at any given time.  Pilots and co-pilots must 
therefore decide who gets to see traffic data and configure their devices appropriately.

.. note:: At present, **Enroute Flight Navigation** supports only Bluetooth Classic
    communication. Bluetooth Low Energy may be supported in the future if there is
    sufficient demand from the user community.

.. note:: Access to Bluetooth radio is severely limited on iOS platforms. For that reason,
    Bluetooth communication is not supported at all on iPhone or iPad devices. 


One-time Setup
--------------

Step 0: Before You Connect
^^^^^^^^^^^^^^^^^^^^^^^^^^

Before you try to connect this app to your traffic receiver, make sure that the
following conditions are met.

- Your traffic receiver supports Bluetooth Classic radio. 
- You know the Bluetooth name of your traffic receiver.
- Bluetooth is switched on in your phone.
- Bluetooth is switched on in your traffic data receiver and set to
  'discoverable mode'.
- If possible, configure your traffic data receiver to always be in
  'discoverable mode'.



Step 1: Configure a Data Connection to the Bluetooth Classic Device
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Follow the steps described in the Section :ref:`SettingsDataConnectionsPage`.


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

- Bluetooth Classic can handle only one data connection. Before boarding,
  clarify which device should connect to the traffic data receiver. Ask your
  co-pilot and all passengers to switch off Bluetooth in all other devices. Make
  sure that there are no undetected devices (e.g. in someone's baggage) that
  could interfere with your data connection.
- A few moments after you power on the avionics, the traffic receiver's
  Bluetooth adaptor will become discoverable.  Start **Enroute Flight
  Navigation** while the traffic data receiver is discoverable.
- **Enroute Flight Navigation** will connect to your traffic data receiver via the
  configured Bluetooth data connection and show traffic information in the moving map.
- If the data connection gets lost in mid-flight, **Enroute Flight Navigation**
  will automatically try to re-connect. Depending on your hardware, it might be
  necessary to restart the traffic data receiver in order to enter discoverable
  mode.

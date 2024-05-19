
.. _SettingsDataConnections Page:
.. _SettingsDataConnectionsPage:

Settings: Data Connections
==========================

This page lists all data connections that **Enroute Flight Navigation** uses to
communicate with traffic data receivers.  It shows that status of each
connection and allows adding/removing connections to Bluetooth devices that
cannot be automatically configured.

- Data connections are colored in green if **Enroute Flight Navigation**
  receives heartbeat signals from a traffic data receiver via that connection.

- Data connections are colored in red if **Enroute Flight Navigation** failed to
  open the connection.

In a typical setup, where traffic data receivers broadcast information via one
of the standard Wi-Fi channels, the default data connection allow **Enroute
Flight Navigation** to automatically detect (and connect to) all customary
devices.  In that case, no user interaction is ever required.


Configure a Data Connection to a Bluetooth Classic Device
---------------------------------------------------------

The **Enroute Flight Navigation** is able to communicate with your traffic data
receiver via the "Bluetooth Classic" radio standard.  

.. note:: The radio standard "Bluetooth Low Energy" is currently unsupported.

.. note:: Due to limitations of the iOS platform, Bluetooth is not
    supported on iPhone and iPad devices.

To avoid any ambiguity, this manual refers to the device running **Enroute
Flight Navigation** as the "phone", and to your Bluetooth-enabled traffic
data receiver as the "Bluetooth Device".  To configure a new data connection
between your phone to the Bluetooth device, proceed as follows.

- Ensure that your Bluetooth Classic device is switched on and in "discoverable"
  mode.

- Because Bluetooth devices support only one data connection, the following
  steps will fail if another phone is trying to connect to your Bluetooth
  device. Ensure that there are no other phones around that could interfere
  with your phone.  Keep in mind that other phones might be in someone
  else's bag, stowed away in a nearby car, or in the office building next door.

- Ensure that Bluetooth is switched "on" in your phone.

- Depending on the precise version of your operating system, you may need to
  pair your phone with the Bluetooth Classic device.  Pairing never hurts, so
  we recommend pairing if possible.  Note that some Bluetooth device cannot be
  paired.

- Open **Enroute Flight Navigation** on your phone, navigate to this page and
  tap on "Add Data Connection" at the bottom of the page.  A device discovery
  dialog will open.

- The device discovery dialog shows a list of all nearby Bluetooth devices.
  Please wait for a few minutes until all devices have been connected.  If
  necessary, tap on the button "Scan for Devices" to re-start the device
  discovery process.

- Choose the relevant Bluetooth device from the list. A data connection to your
  Bluetooth device has now been configured.

- **Enroute Flight Navigation** will try to connect to your Bluetooth device.
  Check the connectivity status by looking at the relevant entry in the list of
  data connections.

In the future, **Enroute Flight Navigation** will automatically detect and
connect to your traffic receiver a few minutes after it becomes visible on
Bluetooth radio.


Remove a Data Connection
------------------------

We recommended removing data connections that you will no longer use.  In order
to remove a data connection, locate the data connection in the list, tap on the
three-dot menu and choose the menu item "Remove".

.. note:: **Enroute Flight Navigation** maintains a list of default data 
    connections, which cannot be removed.

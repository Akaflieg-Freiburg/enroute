.. _SettingsDataConnections Page:
.. _SettingsDataConnectionsPage:

Settings: Data Connections
==========================

This page lists all data connections that **Enroute Flight Navigation** uses to
communicate with traffic data receivers.  It shows that status of each
connection and allows adding/removing connections to Bluetooth devices that
cannot be automatically configured.  The page also allows configuring additional
data connections through a variety of communication channels.


User Interface
--------------

The body of the page displays a list of configured connections. Tap on a
connection to obtain more detailed information.
 
- Data connections are colored in green if **Enroute Flight Navigation**
  receives heartbeat signals from a traffic data receiver via that connection.

- Data connections are colored in red if **Enroute Flight Navigation** failed to
  open the connection.

The footer of the page contains two buttons.

- The button "Reconnect" resets all configured connections and starts a new
  connection process for each. This can be useful in settings where a connection
  failed and where you do not want to wait until the next reconnection attempt
  starts automatically.

- If **Enroute Flight Navigation** cannot detect your traffic data receiver
  automatically, use the button "New Connection" to configure a connection. The
  remainder of the present manual section explains how this is done.


Configure New Data Connections
------------------------------

In a typical setup, where traffic data receivers broadcast information via one
of the standard Wi-Fi channels, the default data connections allow **Enroute
Flight Navigation** to automatically detect (and connect to) all customary
devices.  In that case, no user interaction is ever required.  There are however
settings where **Enroute Flight Navigation** cannot detect your traffic data
receiver automatically.  Depending on the communication channel, the following
subsections describe how to configure a new data connection in that case.


Bluetooth Classic
^^^^^^^^^^^^^^^^^

The **Enroute Flight Navigation** is able to communicate with your traffic data
receiver via the "Bluetooth Classic" radio standard.  The radio standard 
"Bluetooth Low Energy" is supported as a technology preview only and should not
be used in production.

.. note:: Due to limitations of the iOS platform, Bluetooth is not
    supported on iPhone and iPad devices.

To avoid any ambiguity, this manual refers to the device running **Enroute
Flight Navigation** as the "phone", and to your Bluetooth-enabled traffic
data receiver as the "Bluetooth Device".  To configure a new data connection
between your phone to the Bluetooth device, proceed as follows.

- Ensure that your Bluetooth device is switched on and in "discoverable" mode.

- Note that "Bluetooth Classic" devices support only one data connection.  If
  you use "Bluetooth Classic", the following steps will fail if another phone is
  trying to connect to your Bluetooth device. Ensure that there are no other
  phones around that could interfere with your phone.  Keep in mind that other
  phones might be in someone else's bag, stowed away in a nearby car, or in the
  office building next door.

- Ensure that Bluetooth is switched "on" in your phone.

- Depending on the precise version of your operating system, you may need to
  pair your phone with the Bluetooth device.  Pairing never hurts, so we
  recommend pairing if possible.  Note that some Bluetooth device cannot be
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


Serial Port
^^^^^^^^^^^

The **Enroute Flight Navigation** is able to communicate with your traffic data
receiver via the serial port.  Serial port communication via USB is supported.

.. note:: Due to limitations of the iOS platform, serial port communication is not
    supported on iPhone and iPad devices.

- Ensure that your traffic data receiver is switched on and connected to the
  serial port/USB input of your device.

- Open **Enroute Flight Navigation** on your phone, navigate to this page and
  tap on "New Connection" at the bottom of the page and choose "Serial Port
  Connection" from the menu.  A device discovery dialog will open.

- The device discovery dialog shows a list of all nearby serial ports in your
  device.  If necessary, tap on the button "Scan for Devices" to re-start the
  device discovery process.

- Choose the relevant serial port from the list. A data connection to that
  serial port has now been configured.  Enroute will determine the necessary
  parameter (such as bit rate) automatically.

- **Enroute Flight Navigation** will try to connect to your traffic data
  receiver via the serial port.  Check the connectivity status by looking at the
  relevant entry in the list of data connections.

In the future, **Enroute Flight Navigation** will automatically detect and
connect to your traffic receiver a few minutes after it is connected to your
device.


TCP via Wi-Fi or LAN
^^^^^^^^^^^^^^^^^^^^

The Transmission Control Protocol (TCP) is one of the main protocols of the
internet.  Traffic data receivers based on FLARM typically use TCP to transmit
traffic data via Wi-Fi and LAN networks.  To configure a TCP connection, you
need the following data.

- The internet address of the traffic data receiver in its network. This is
  typically a string of the form "192.168.1.1".

- The port number. This is a number between 0 and 65535, but most FLARM based
  devices use port 2000.

.. note:: To simplify the setup process, **Enroute Flight Navigation** includes a number
    of predefined TCP connections.  These suffice to connect to any traffic data
    receiver that we have seen.  Manual configuration of TCP connections should
    never be necessary.  If you are aware of hardware that uses an internet
    address/port combination not covered by the predefined connections, then please
    open the main menu and use the entry "Bug Report" to let us know.

To configure a new TCP data connection, proceed as follows.

- Read the manual of your traffic data receiver to find out what internet
  address and port number it uses.

- Start the traffic data receiver.

- Connect to the Wi-Fi or LAN network of your traffic data receiver.

- Open **Enroute Flight Navigation** on your phone, navigate to this page and
  tap on "New Connection" at the bottom of the page and choose "TCP Connection"
  from the menu.  A dialog will open.

- Enter the IP address and port number used by your traffic data receiver. While
  all devices that we have seen use IPv4 addresses of the form "192.168.1.1",
  IPv6 addresses and internet host names are also supported.  Tap on "OK".  A
  new data connection has been configured.

- **Enroute Flight Navigation** will try to connect to your traffic data
  receiver. Check the connectivity status by looking at the relevant entry in
  the list of data connections.

In the future, **Enroute Flight Navigation** will automatically detect and
connect to your traffic receiver a few minutes after it becomes visible on
Wi-Fi or LAN.


UDP via Wi-Fi or LAN
^^^^^^^^^^^^^^^^^^^^

The User Datagram Protocol (UDP) is one of the main protocols of the internet.
Flight simulators and traffic data receivers based on Garmin hardware typically
use UDP to transmit traffic data via Wi-Fi and LAN networks.  To configure a UDP
connection, you need the following data.

- The port number. This is a number between 0 and 65535, but most devices use
  ports 4000 or 49002.

.. note:: To simplify the setup process, **Enroute Flight Navigation** includes
    a number of predefined UDP connections.  These suffice to connect to any 
    traffic data receiver that we have seen.  Manual configuration of UDP 
    connections should never be necessary.  If you are aware of hardware that 
    uses a port not covered by the predefined connections, then please open the 
    main menu and use the entry "Bug Report" to let us know.

To configure a new TCP data connection, proceed as follows.

- Read the manual of your traffic data receiver to find out what port number it
  uses.

- Start the traffic data receiver.

- Connect to the Wi-Fi or LAN network of your traffic data receiver.

- Open **Enroute Flight Navigation** on your phone, navigate to this page and
  tap on "New Connection" at the bottom of the page and choose "UDP Connection"
  from the menu.  A dialog will open.

- Enter the port number used by your traffic data receiver.  Tap on "OK".  A new
  data connection has been configured.

- **Enroute Flight Navigation** will try to connect to your traffic data
  receiver. Check the connectivity status by looking at the relevant entry in
  the list of data connections.

In the future, **Enroute Flight Navigation** will automatically detect and
connect to your traffic receiver a few minutes after it becomes visible on
Wi-Fi or LAN.


OGN glidernet.org Connection
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The `Open Glider Network <http://wiki.glidernet.org/about>`__ is a network of
ground station and internet servers operated by volunteers. It collects FLARM
and ADS-B data and distributes this data in real time via internet service.

Starting with version 2.34.0, **Enroute Flight Navigation** is able to display
traffic data from the Open Glider Network in its moving map. To configure a data
connection to the Open Glider Network, proceed as follows.

- Open **Enroute Flight Navigation** on your phone, navigate to this page and
  tap on "New Connection" at the bottom of the page and choose "OGN
  glidernet.org Connection" from the menu.

- **Enroute Flight Navigation** will show two warning dialogs, pointing to
  technical restrictions of internet services in flight, aviation safety
  concerns, and consequences for data privacy. Read these text with care and
  click on "OK" only if you understand the implications.

In the future, **Enroute Flight Navigation** will automatically connect to the
"Open Glider Network".  For reasons of flight safety, **Enroute Flight
Navigation** will always prefer connections to proper traffic data receivers and
will switch connections as soon as a data receiver becomes available.


Remove a Data Connection
------------------------

We recommended removing data connections that you will no longer use.  In order
to remove a data connection, locate the data connection in the list, tap on the
three-dot menu and choose the menu item "Remove".  Note that default data
connections cannot be removed.

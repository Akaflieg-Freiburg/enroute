Connect via Wi-Fi
=================

Wi-Fi is the recommended method to connect **Enroute Flight Navigation** to your
traffic data receiver.  Compared with Bluetooth, Wi-Fi connections are reliable,
hassle-free and work automatically with minimal setup.


One-time Setup
--------------

Step 0: Before You Connect
^^^^^^^^^^^^^^^^^^^^^^^^^^

Before you try to connect this app to your traffic receiver, make sure that the
following conditions are met.

- Your traffic receiver has an integrated Wi-Fi interface that acts as a
  wireless access point.
- You know the network name (=SSID) of the Wi-Fi network deployed by your
  traffic receiver. If the network is encrypted, you also need to know the Wi-Fi
  password.
- Some devices require an additional password in order to access traffic data.
  If this is the case, you will need to know this password.

**Enroute Flight Navigation** supports all major protocols for traffic data
sharing, including "FLARM/NMEA" and "GDL90".  If your traffic receiver supports
FLARM/NMEA as well as GDL90 output, then configure it to always use FLARM/NMEA.
The GDL90 protocol has a number of shortcomings that **Enroute Flight
Navigation** cannot always work around.  See the Section :ref:`gdl90problems`
for more details.


Step 1: Enter the Traffic Receiver's Wi-Fi Network
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Make sure that the traffic receiver has power and is switched on. In a typical
  aircraft installation, the traffic receiver is connected to the 'Avionics'
  switch and will automatically switch on. You may need to wait a minute before
  the Wi-Fi comes online and is visible to your device.
- Enter the Wi-Fi network deployed by your traffic receiver. This is usually
  done in the "Wi-Fi Settings" of your device. Enter the Wi-Fi password if
  required. Some devices will issue a warning that the Wi-Fi is not connected to
  the internet. In this case, you might need to confirm that you wish to enter
  the Wi-Fi network.

Most operating systems will offer to remember the connection, so that your
device will automatically connect to this Wi-Fi in the future. We recommend
using this option.


Step 2: Check Connectivity
^^^^^^^^^^^^^^^^^^^^^^^^^^

After your device has entered the traffic receiver's Wi-Fi network in Step 1,
everything else should be automatic.  To check, open the main menu and navigate
to the "Information" menu.  If the entry "Traffic Receiver" is highlighted in
green, then **Enroute Flight Navigation** has already found the traffic receiver
in the network and has connected to it. Congratulations, you are done!

If the entry "Traffic Receiver" is not highlighted in green, then something has
gone wrong.  The section "Troubleshooting" below might help you find the issue.


Daily Operations
----------------

Once things are set up properly, your device should automatically detect the
traffic receiver's Wi-Fi network, enter the network and connect to the traffic
data stream whenever you go flying.  Here is a breakdown of what will happen.

- As soon as you board your aircraft and power on the avionics, the traffic
  receiver's Wi-Fi network will become visible to your device.
- In a typical scenario, your device might already be connected to a Wi-Fi in a
  nearby building. In that case, nothing will happen for the moment.  As you
  taxi to the runway, your device leaves the range of that network and
  automatically connects to the traffic receiver's Wi-Fi network as the next
  best alternative.
- Traffic information will be shown in the moving map.


Troubleshooting
---------------

The app cannot connect to the traffic data stream
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Open the main menu and navigate to the "Information" menu.  If the entry
"Traffic Receiver" is not highlighted in green, then **Enroute Flight
Navigation** does not receive any traffic data or traffic data receiver
heartbeat.  Work through the following steps to identify the issue.

**Step 1: Check Wi-Fi** 

Open the Wi-Fi settings of your device and confirm that your device is indeed
connected to the traffic data receiver's network.  If not, then reconnect.  It
might help move your device closer to the traffic data receiver's Wi-Fi antenna.

Some traffic data receivers offer a web-interface that can be accessed with your
web browser.  In that case, check if you can access the web-interface with your
browser.  Close the web browser afterwards, because some devices cannot
concurrently operate the web interface and transmit traffic data.

**Step 2: Check Connection** 

If you are sure that your device has connected to the correct Wi-Fi network,
then return to **Enroute Flight Navigation** open the main menu and go to
"Settings/Data Connections".  Look at the connections of type "TCP" and "UDP".

- If none of the TCP/UDP connections has status "Connected", then **Enroute
  Flight Navigation** cannot see your traffic data receiver in the Wi-Fi
  network.  This means that your traffic data receiver is not available at any
  of the IP address/Port combinations known to **Enroute Flight Navigation**.

  Check the manual of your traffic data receiver or Wi-Fi interface and note the
  connection type (TCP or UDP), IP address and port that the traffic data
  receiver uses.  Go back to **Enroute Flight Navigation** and check whether the
  combination of IP address and port appears in the list of data connections.  

  If a data connection for that IP address and port exists but cannot connect,
  then there is a communication issue that we cannot resolve. It might be
  interesting to check if another app is able to communicate with your traffic
  data receiver.  
  
  If a data connection for that IP address and port does not exist, then please
  do the following.

  - Contact us! We want to support all traffic data receivers on the market, and
    we will be glad to support your traffic data receiver as well. For this,
    return to the main moving map screen of **Enroute Flight Navigation**, open
    the main menu and go to "Bug Report".

  - Check if your traffic data receiver can be configured to use one of the
    supported IP address/port combinations. If possible, this will ensure that
    users of **Enroute Flight Navigation** can connect to your traffic data
    receiver without manual configuration.

  - Configure a new data connection for the IP address/port combination used by
    your traffic data receiver. The section :ref:`SettingsDataConnectionsPage`
    explains how to do that.

- If one or several of the TCP/UDP connections have status "Connected" but no
  heartbeat is received, then you are out of luck. **Enroute Flight Navigation**
  does not receive any data or cannot understand the data it receives.  Please
  contact us! We want to support all traffic data receivers on the market, and
  we will be interested to hear about your case. For this, return to the main
  moving map screen of **Enroute Flight Navigation**, open the main menu and go
  to "Bug Report".

**Step 3: [Censored Profanity]** 

If **Enroute Flight Navigation** has connected to your traffic data receiver via
a TCP/UDP connection but does not receive heartbeat of traffic data, then you
are out of luck. Please contact us, as we would like to hear about your case.
Return to the main moving map screen of **Enroute Flight Navigation**, open the
main menu and go to "Bug Report".  It might be worth checking if other apps
experience similar problems.

  
The connection breaks down frequently
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are two common causes for unstable connections.


**Limitations of your Traffic Data Receiver**

Some traffic receivers and some Wi-Fi adaptors cannot serve more than one client
concurrently and abort connections at random if more than one device tries to
access.

- Make sure that there no second device connected to the traffic receiver's
  Wi-Fi network. The other device might well be in your friend's pocket!
- Make sure that there is no other app trying to connect to the traffic
  receiver's data stream.
- Many traffic receivers offer "configuration panels" that can be accessed via a
  web browser. Close all web browsers.


**Electromagnetic Interference**

Electromagnetic interference is a major problem in many avionics installations.
This is not easily solved.  Try moving your device closer to the Wi-Fi antenna
and try to install the antenna in other locations.
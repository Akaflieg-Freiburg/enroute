.. _InfoTrafficPage:

Information/Traffic Data Receiver
=================================

The page **Traffic Data Receiver** provides status information on the data
connection between **Enroute Flight Navigation** and your traffic data receiver.
To access the page, open the main menu and choose "Information/Traffic Data
Receiver." 


Connection Status
-----------------

To ensure reliable operation, traffic receivers emit "heartbeat messages" at
frequent intervals. The text field "Connection Status" explains if heartbeat
messages are received through any of the configured data channels. If yes, this
field shows the data connection currently used and lists the types of data
presently received.


Traffic Data Receiver Status
----------------------------

If heartbeat messages are received, this field shows status messages reported by
the traffic data receiver, including results of internal self-tests of the
traffic data receiver hardware. If no heartbeat messages are received, this
field is invisible.


Traffic/Currently No Traffic
----------------------------

If heartbeat messages are received, this field lists all the traffic reported by
the traffic data receiver, sorted in order of importance. If no heartbeat
messages are received, this field is not shown.

Traffic opponents are classified as "Relevant Traffic" and "Irrelevant Traffic".
Traffic is considered relevant if the vertical distance to the traffic opponent
is less than 1,500 m and the horizontal distance is less than 20 NM. Only
relevant traffic is shown in the moving map.


Help
----

If no heartbeat is received from any traffic receiver, two buttons "Connect to a
traffic receiver" and "Connect to a flight simulator" will become visible. A
click on any of these buttons will open the appropriate page of the manual.


Page Footer
-----------

The page footer shows one or two additional buttons.

Reconnect
^^^^^^^^^

This button is visible if no heartbeat is received from any traffic receiver.
After a click on this button will disconnect all configured data connections and
attempt to reconnect.  This can be useful if **Enroute Flight Navigation** did
not automatically connect, or if you do not wish to wait for the next automatic
reconnection attempt.

Configure Data Connections
^^^^^^^^^^^^^^^^^^^^^^^^^^

A click on this button will open the page "Data Connections", described in the
section :ref:`SettingsDataConnectionsPage`. There, you can view the status of
every single data connection, and configure new connections if required.
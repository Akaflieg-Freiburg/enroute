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
frequent intervals. The text field "Connection Status" shows if heartbeat
messages are received through any of the configured data channels. If yes, this
field shows the data connection currently used and lists the types of data
presently received.


Traffic Data Receiver Status
----------------------------

If heartbeat messages are received, this field shows status messages reported by
the traffic data receiver, including results of internal self-tests of the
traffic data receiver hardware. If no heartbeat messages are received, this
field is invisible.


Position
--------

If your traffic data receiver provides its own SatNavS position, this field
shows the position reported by the traffic data receiver. If no heartbeat
messages are received, or if the traffic data receiver does not provide its own
SatNav position, this field is invisible.


True Altitude
-------------

If your traffic data receiver provides its own altitude information, this field
shows the true altitude reported by the traffic data receiver. Otherwise, this
field is invisible.

True altitude AGL or AMSL is the vertical distance from the aircraft to the
terrain or to the main sea level, respectively.

.. warning:: 
   True altitude is not the same as barometric altitude, which is the altitude
   shown on your aircraft's altimeter. Depending on weather conditions, true
   altitude and barometric altitude may differ substantially. Never use true
   altitude to judge the vertical distance from your aircraft to an airspace
   boundary.


Pressure Altitude
-----------------

If your traffic data receiver provides its own pressure altitude information,
this field shows the pressure altitude reported by the traffic data receiver.
Otherwise, this field is invisible.

Pressure altitude is the altitude in the standard atmosphere at which the
pressure is equal to the current atmospheric pressure. This is the altitude
displayed on the aircraft's altimeter when set to the standard pressure of
1013.2hPa. This is also the altitude shown on the transponder.


Traffic
-------

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
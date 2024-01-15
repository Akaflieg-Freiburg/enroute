
.. _Settings Page:
.. _SettingsPage:

Settings
========

**Enroute Flight Navigation** is designed to be simple. The number of user
settings is deliberately small. To access the user settings, open the main menu
and choose "Settings." 


Moving Map
----------

The settings grouped under "Moving Map" change the appearance of the map
display.


.. _SettingsAALimit:

Airspace Altitude Limit
^^^^^^^^^^^^^^^^^^^^^^^

If you never fly higher than 5.000ft, you will probably not be interested in
airspaces that begin above FL100. **Enroute Flight Navigation** allows you to
set an altitude limit to improve the readability of the moving map. Once set,
the app will show only airspaces below that limit. Tap on the entry “Airspace
Altitude Limit” to set or unset the altitude limit. 

Once you set an altitude limit, the moving map will display a little warning
(“Airspaces up to 9,500 ft”) to remind you that the moving map does not show all
airspaces. The app will automatically increase the limit when your aircraft
approaches the altitude limit from below.

.. warning:: Airspace boundaries are often flight levels. The true altitude of a
    flight level depends on meteorological conditions (such as the temperature 
    gradient) and is not known to **Enroute Flight Navigation**. When deciding 
    which airspace to show, the app will use an approximation. The approximation 
    might be off by 1,000ft or more in extreme weather. **Always leave an ample 
    safety margin when setting an airspace altitude limit.**
    

Gliding Sectors
^^^^^^^^^^^^^^^

In regions with high glider traffic, local regulations often allow gliders to
fly in airspaces that are otherwise difficult to access, such as control zones.
The moving map displays these “Gliding Sectors” in bright yellow. If you are not
flying a glider, the gliding sectors are probably not relevant. Hiding the
gliding sectors might improve the readability of the moving map.


Navigation Bar
--------------

These settings apply to the Navigation Bar, shown at the bottom of the moving
map screen.


Altimeter Mode
^^^^^^^^^^^^^^

Use this settings item to chose if the altimeter shows height above ground level
(AGL) or height above main sea level (AMSL). 

.. note:: In order to compute height above ground, if terrain maps for your 
  region must be installed. If terrain data is not available, the altimeter 
  field of the navigation bar will display "--".  If you are unsure if terrain 
  data is available, open the main menu and go to "Library/Maps and Data" to 
  check which maps are installed in your device.


User Interface
--------------

Large Fonts
^^^^^^^^^^^

Use this option to enlarge fonts for improved readability.


Night Mode
^^^^^^^^^^

The “Night Mode” of Enroute Flight Navigation is similar to the “Dark Mode”
found in many other apps. We designed the night mode for pilots performing VFR
flights by night, whose eyes have adapted to the darkness. Compared with other
apps, you will find that the display is quite dark indeed.


Voice Notifications
^^^^^^^^^^^^^^^^^^^

Pilots should not be looking at their mobile devices for extended periods of
time. Enroute Flight Navigation is therefore able to read notification texts in
addition to showing them on the screen. Since we expect that not everybody likes
this feature, this setting item allows switching voice notification on and off.


System
------

Primary Position Data Source
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Enroute Flight Navigation** can either use the built-in satnav receiver of
your device or a connected traffic receiver as a primary position data source.
This setting is essential if your device has reception problems or if you use
**Enroute Flight Navigation** together with a flight simulator.

- You will most likely prefer the built-in satnav receiver for actual flight.
  The built-in receiver provides one position update per second on a typical
  Android system, while traffic receivers do not always provide timely position
  updates.

- If you use **Enroute Flight Navigation** together with a flight simulator, you
  **must** choose the traffic receiver as a primary position data source. Flight
  simulators broadcast position information of simulated aircraft via Wi-Fi,
  using the same protocol that a traffic data receiver would use in a real
  plane. As long as the built-in satnav receiver is selected, all position
  information provided by your flight simulator is ignored.

.. note:: Setting a traffic receiver as a primary position data source is safe 
    even when the app is not connected to a traffic receiver. When no traffic 
    receiver is connected, **Enroute Flight Navigation** will automatically fall 
    back using the built-in satnav receiver of your device.


Ignore Network Security Errors
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This entry is visible if you have asked the app to download data via insecure
internet connections after a secure connection attempt failed. Uncheck this item
to revert to the standard policy, which enforces secure connections.


Clear Password Storage
^^^^^^^^^^^^^^^^^^^^^^

This entry is visible if you have connected to a traffic data receiver that
requires a password in addition to the Wi-Fi password and if you have asked the
app to remember the password. Tap on this entry to clear the password storage. 


Help
----

The items grouped under “Help” refer the user to this manual.

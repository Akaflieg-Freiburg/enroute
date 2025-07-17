.. _trafficInformationTools:

Traffic Information Tools
=========================

There are several "traffic information" or "anti-collision" apps on the marked
targeting general aviation pilots. These apps typically perform two functions.

- They receive traffic data via the internet and broadcast the information to
  apps like **Enroute Flight Navigation** which can then show traffic in the
  moving map, and

- The report the current position to servers on the internet, so other users of
  the system can be warned of your presence.

**Enroute Flight Navigation** integrates well with these apps.  This section
describes two of the more popular solutions, "CCAS VFR-Kollisionswarnsystem" and
"SafeSky"

.. warning:: 
  While traffic information apps have valid use cases, for instance in flight
  simulation or to give ground operators an overview of local traffic, we strongly
  recommend against using traffic data from internet services in real flight.

  - Internet connectivity is not reliable in flight. Experiments in Germany have
    shown that internet connection to fail for about half of the time, even when
    flying over densely populated areas. This problem will likely increase in the
    future, as modern 5G antennas have a stronger directional characteristic than
    traditional ones.
  
  - Experience shows that the data is frequently laggy, incomplete and often outdated.
  
  We strongly feel that no responsible pilot should ever fly without a proper
  traffic data receiver, such as a FLARM or ADS-B device.


CCAS VFR-Kollisionswarnsystem
-----------------------------

`CCAS VFR-Kollisionswarnsystem <https://ccas.aero>`_ is a free anti-collision
app that works without registration or setup, and integrates well with **Enroute
Flight Navigation**. 

.. warning::
  "CCAS VFR-Kollisionswarnsystem" wrongly reports the cabin pressure measured by
  the sensor in your device as static pressure. In closed-cabin aircraft, where 
  cabin pressure is typically not equal to static pressure, this can lead to 
  **wrong barometric altitude readings in Enroute Flight Navigation**, and in 
  turn to **involuntary airspace violations**.  
  
  Be aware that cabin pressure and static pressure disagree even in aircraft 
  without pressurized cabin.


One-time Setup
^^^^^^^^^^^^^^

There is no setup required, but it might make sense to check the integration
once before you start using "CCAS VFR-Kollisionswarnsystem" on a regular basis.

To check the integration, start the app "CCAS VFR-Kollisionswarnsystem". Then,
go to **Enroute Flight Navigation**, open the main menu in **Enroute Flight
Navigation** and navigate to the "Information" menu.  If the entry "Traffic
Receiver" is highlighted in green, then **Enroute Flight Navigation** has
connected to the app. Congratulations, you are done!


Daily Operations
^^^^^^^^^^^^^^^^

Before you start your flight, start "CCAS VFR-Kollisionswarnsystem". Everything
else is automatic.


SafeSky
-------

`SafeSky <https://www.safesky.app>`_ is a commercial anti-collision app. The
commercial premium version of SafeSky integrates with **Enroute Flight
Navigation**. 


One-time Setup
^^^^^^^^^^^^^^

The following steps configure SafeSky to forward traffic information to
**Enroute Flight Navigation**. In **Enroute Flight Navigation**, no
configuration is required. 


Step 0: Before You Connect
..........................

Traffic sharing is a premium feature of SafeSky. Before you connect, make sure
to have a valid premium subscription.


Step 1: Enable Traffic Sharing
..............................

In the main menu of Safe Sky, choose the box "Traffic Sharing". The page
"Traffic Sharing" will open. 

- Choose the option "Enable traffic sharing"
  
- Touch the field below "Enable traffic sharing" to open the list of supported
  navigation apps. Choose "Enroute" from the list and touch the button "back" to
  close the list.

- Touch the button "back" to close page "Traffic sharing". Leave the main menu
  and return to the SafeSky main page.

That's it. As soon as you choose "TAKE OFF" in SafeSky to start a flight,
SafeSky will start a background process that shares traffic data with Enroute
Flight Navigation. 


Step 2: Check Connectivity
..........................

To check the SafeSky integration, choose "TAKE OFF" in SafeSky. Then, go to
**Enroute Flight Navigation**, open the main menu in **Enroute Flight
Navigation** and navigate to the "Information" menu.  If the entry "Traffic 
Receiver" is highlighted in green, then **Enroute Flight Navigation** has 
connected to the SafeSky app. Congratulations, you are done!


Daily Operations
^^^^^^^^^^^^^^^^

Before you start your flight, open the SafeSky app and choose "TAKE OFF". Everything else is automatic.

.. _safeSkyPage:

Connect the SafeSky App
=======================

`SafeSky <https://www.safesky.app>`_ is an anti-collision app and a real-time
flight information service for all pilots flying any type of aircraft. The
commercial premium version of SafeSky integrates with **Enroute Flight
Navigation**. Once set up, the moving map of **Enroute Flight Navigation** will
show nearby traffic, similar to the map display when connected to a traffic
receiver.

.. figure:: ./SafeSkyIntegration.png
   :scale: 75 %
   :align: center
   :alt: SafeSky integration at work

   SafeSky integration at work

.. note:: To show only relevant traffic, **Enroute Flight Navigation** will
    display traffic factors only if the vertical distance is less than 1,500 m
    and the horizontal distance less than 20 nm.  The moving map of **Enroute 
    Flight Navigation** will therefore show substantially less traffic than 
    the SafeSky app.

.. warning:: **Enroute Flight Navigation** does not issue traffic warnings.  
    The app contains no collision avoidance algorithms.


Connect!
--------

The following steps configure SafeSky to forward traffic information to
**Enroute Flight Navigation**. In **Enroute Flight Navigation**, no
configuration is required. 

.. figure:: ./SafeSky.png
   :scale: 75 %
   :align: center
   :alt: SafeSky configuration screens

   SafeSky configuration screens


Step 0: Before You Connect
^^^^^^^^^^^^^^^^^^^^^^^^^^

Traffic sharing is a premium feature of SafeSky. Before you connect, make sure
to have a valid premium subscription.

Step 1: Enable Traffic Sharing
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

To check the SafeSky integration, choose "TAKE OFF" in SafeSky. Then, go to
**Enroute Flight Navigation**, open the main menu in **Enroute Flight
Navigation** and navigate to the "Information" menu.

- If the entry "Traffic Receiver" is highlighted in green, then **Enroute Flight
  Navigation** has connected to the SafeSky app. Congratulations, you are done!

- If the entry "Traffic Receiver" is not highlighted in green, then select the
  entry. The "Traffic Receiver Status" page will open. The page explains the
  connection status in detail, and explains how to establish a connection
  manually.

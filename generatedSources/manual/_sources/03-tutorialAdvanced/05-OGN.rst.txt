Receive Traffic Data via the Internet
=====================================

Starting with version 2.34.0, **Enroute Flight Navigation** is able to receive
traffic data from the `Open Glider Network <http://wiki.glidernet.org/about>`__
and display traffic data in the moving map as long as no proper traffic data
receiver is available.

.. warning::
  While OGN data can be useful in certain scenarios, we recommend against using
  traffic data from internet services in real flight.

  - Internet connectivity is not reliable in flight. Even when flying over
    populated areas, expect the internet connection to fail for about half of the
    time.

  - Experience shows that data is frequently laggy and often outdated.

  - You will not be visible to others.

  We strongly feel that no responsible pilot should ever fly without a proper
  traffic data receiver, such as a FLARM or ADS-B device.

To configure a data connection to the Open Glider Network, proceed as follows.


One-time Setup
--------------

Step 1: Configure the Connection
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Follow the steps described in the Section :ref:`SettingsDataConnectionsPage`.


Step 2: Check Connectivity
^^^^^^^^^^^^^^^^^^^^^^^^^^

After your connection has been configured, everything else should be automatic.
To check, open the main menu and navigate to the "Information" menu.  Choose the
entry "Traffic Receiver". The page "Traffic Data Receiver" will open.

On the page "Traffic Data Receiver", look for the text field "Connection
Status". If the text field is highlighted in yellow and starts with "OGN
glidernet.org APRS-IS connection", then your connections has been configured
successfully.


Daily Operations
----------------

Once things are set up properly, **Enroute Flight Navigation** will
automatically connect to the "Open Glider Network".  For reasons of flight
safety, **Enroute Flight Navigation** will always prefer connections to proper
traffic data receivers and will switch connections as soon as a data receiver
becomes available.

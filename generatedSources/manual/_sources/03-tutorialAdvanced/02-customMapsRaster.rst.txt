.. _importRaster:

Import Custom Raster Maps
=========================

**Enroute Flight Navigation** comes with a set of general-purpose base maps in
vector format, which are shown together with the aviation data in a style that
we consider suitable for most aviation purposes.  Still, there might be
situations where a user would like to use their own base maps. 

- Where available, some users might prefer to use official ICAO charts of their
  countries.
- Some users might prefer raster maps that follow a different style.

**Enroute Flight Navigation** is able to import MBTILES files containing raster
data.

.. note:: 
  Even if you decide to use raster maps, we still recommend installing our 
  regular maps, because **Enroute Flight Navigation** needs the data to provide 
  airspace and waypoint information.


MBTILES Map Data Sources
------------------------

The website `open flightmaps
<https://www.openflightmaps.org/https://www.openflightmaps.org>`_ provides
excellent aviation maps in raster format for a variety of European countries, as
well as South Africa and Namibia.  We are aware of aviation authorities that
offer ICAO raster maps in GeoTIFF format. The Section :ref:`mapconversiontools`
explains how these can be converted to the MBTILES format suitable for **Enroute
Flight Navigation**.


Import Raster Maps
------------------

Transfer the MBTILES file to your device and open the file on your device.  The
Section :ref:`importData` explains the process in detail.


Manage Imported Raster Maps
---------------------------

To manage the raster maps installed in **Enroute Flight Navigation**, open the
main menu and go to "Library/Maps and Data".


Use Imported Raster Maps
------------------------

.. |ico1| image:: ../01-gettingStarted/ic_layers.png

As soon as raster maps are installed, a button with the label |ico1| will become
visible on the moving map page.  A tap on the button will open the Raster Map
Menu, which shows all raster maps installed.  Tap on an entry to show the
relevant map inside the moving map display.  Tap on the same entry again to hide
it.

.. note:: 
  In order to avoid confusion, at most one raster map will be shown at any given 
  time.


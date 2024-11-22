.. _mapConversionTools:

Map Generation and Conversion Tools
===================================

As explained in Section :ref:`importraster`, **Enroute Flight Navigation** is
able to import and use third-party raster maps in MBTILES format.  However, most
government offices publish aviation maps in GeoTIFF format rather than MBTILES.
GeoTIFF is suitable for use in geodata information service systems, but
unsuitable for map display on low-powered devices.

This section lists several official sources for aviation maps and explains how
to convert GeoTIFF to MBTILES format.


Data Sources
------------

We are aware of websites that offer raster maps in GeoTIFF format. At present,
**Enroute Flight Navigation** cannot handle GeoTIFF files, but there are tools
that convert GeoTIFF to MBTILES.

- Official ICAO maps for Denmark are available from the danish `AIM Naviair
  <https://aim.naviair.dk/en/charts/>`_

- Official ICAO maps for Spain are available from the Spanish `Insignia Servicio
  de Información Aeronáutica
  <https://aip.enaire.es/AIP/CartasInsigniaImpresas-es.html>`_

- Official ICAO maps for Switzerland are available from the Swiss `Federal
  Office of Topography swisstopo
  <https://www.swisstopo.admin.ch/en/geodata/aero/icao.html>`_

- Official VFR raster charts are available from the `United States Federal
  Aviation Administration
  <https://www.faa.gov/air_traffic/flight_info/aeronav/digital_products/vfr/>`_


Conversion Tool: QGIS
---------------------

Users have successfully used the free tool `QGIS <https://qgis.org/en/site>`_ to
convert GeoTIFF files to MBTILES, which can then be used with **Enroute Flight
Navigation**. 

.. _QGIS-img:
.. figure:: QGIS-MainWindow.png
   :scale: 40 %
   :align: center

   QGIS Main Window

Since QGIS is a powerful tool that is not always easy to use, one user has
kindly provided the following short tutorial.

- Install QGIS on your desktop computer. On Fedora Linux, we found that the
  packages provided by the default software repository were outdated and lacked
  the necessary functionality.  We followed the installations instructions on
  the `QGIS website <https://qgis.org/en/site/forusers/download.html>`_ to
  install a current and full-featured version of the program.

- Open QGIS. Create a new project and open the GeoTIFF file in QGIS by
  dragging-and-dropping the GeoTIFF file into the QGIS window. The content of
  the GeoTIFF file should become visible.

- Choose the menu item "Project/Properties…" to open the dialog window "Project
  Properties". There, set the coordinate reference system to EPSG:3857. To
  locate the reference system, use the text field "Filter" and search for
  EPSG:3857.

- Use the menu items under "View/Panels/…" to ensure that the panels "Layer" and
  "Layer Styling" are visible. Select the layer of your GeoTIFF file and in the
  "Layer" panel.  Then, go to the "Layer Styling" panel and set "Resampling" to
  "Bilinear" for better image render quality.

- Use the menu items under "View/Panels/…" to ensure that the panel "Processing
  Toolbox" is visible. Inside the "Processing Toolbox", double-click on "Raster
  Tools→Generate XYZ Tiles (MBTILES)".  The dialog "Generate XYZ Tiles
  (MBTILES)" will open. Fill the necessary parameters, as seen in the image
  below. We found the function "Draw on Map Canvas" useful to specify the map
  extent. Pay attention to the maximum zoom level, as the time and file size
  increase significantly after zoom level 12. Depending on the size of your
  GeoTIFF and on the number of zoom levels you use, it may take a while to
  generate the MBTILES file.

.. _QGIS-Gen-img:
.. figure:: QGIS-GenerateMBTILES.png
   :scale: 40 %
   :align: center

   QGIS Generate Tiles Dialog

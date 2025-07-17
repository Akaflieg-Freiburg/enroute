Data Sources
============


.. _aeroMapData:

Map Data
--------

Aeronautical Maps
.................

Our maps available for offline use, so that the app does not require any
internet connection in flight. The maps are updated once per day, provided that
new data is available. Note, however, that we do not **guarantee** regular
updates.

The aeronautical maps are compiled from databases provided by the `openAIP
<http://openaip.net>`_ and the `open flightmaps
<https://www.openflightmaps.org/>`_ projects.  While openAIP covers most of the
world, the open flightmaps cover fewer countries but contain more detailed
information.

================================  ===============
Map Feature                       Data Origin
================================  ===============
Airfields                         openAIP
Airspace: Nature Preserve Areas   open flightmaps
Airspace: all other               openAIP
Navaids                           openAIP
Procedures (Traffic Circuits, …)  open flightmaps
Reporting Points                  open flightmaps
================================  ===============


List of maps
^^^^^^^^^^^^

For simplicity, our maps are divided in "Class 1" and "Class 2".

- Class 1 maps are compiled from `openAIP <http://openaip.net>`_ and `open
  flightmaps <https://www.openflightmaps.org/>`_ data. These maps contain
  complete information about airspaces, airfields and navaids.  In addition, the
  maps contain (mandatory) reporting points. Some of our class 1 maps also show
  traffic circuits and flight procedures for control zones.
  
- Class 2 maps are compiled from `openAIP <http://openaip.net>`_ data only. They
  contain complete information about airspaces, airfields and navaids.

Below is a complete list of the maps that we offer.

=================== ============== =======
Continent           Country        Class
=================== ============== =======
Africa              Algeria        Class 2
Africa              Botswana       Class 2
Africa              Canary Islands Class 1
Africa              Kenya          Class 2
Africa              Madagascar     Class 2
Africa              Malawi         Class 2
Africa              Mauritius      Class 2
Africa              Morocco        Class 2
Africa              Namibia        Class 1
Africa              Réunion        Class 2
Africa              South Africa   Class 1
Africa              Tunisia        Class 2
Asia                Bahrain        Class 2
Asia                Japan          Class 2
Asia                Laos           Class 2
Asia                Nepal          Class 2
Asia                Qatar          Class 2
Asia                Sri Lanka      Class 2
Asia                Unit. Emirates Class 2
Australia Oceanica  Australia      Class 2
Australia Oceanica  New Zealand    Class 2
Australia Oceanica  Vanuatu        Class 2
Europe              Albania        Class 2
Europe              Austria        Class 1
Europe              Belgium        Class 1
Europe              Bosnia and H.  Class 2
Europe              Bulgaria       Class 1
Europe              Croatia        Class 1
Europe              Cyprus         Class 2
Europe              Czech Republic Class 1
Europe              Denmark        Class 1
Europe              Estonia        Class 2
Europe              Finland        Class 1
Europe              France         Class 2
Europe              Germany        Class 1
Europe              Great Britain  Class 1
Europe              Greece         Class 1
Europe              Hungary        Class 1
Europe              Iceland        Class 2
Europe              Ireland        Class 2
Europe              Italy          Class 1
Europe              Latvia         Class 2
Europe              Liechtenstein  Class 2
Europe              Lithuania      Class 2
Europe              Luxembourg     Class 2
Europe              Malta          Class 2
Europe              Moldova        Class 2
Europe              Montenegro     Class 2
Europe              Netherlands    Class 1
Europe              North. Ireland Class 1
Europe              Norway         Class 2
Europe              Poland         Class 1
Europe              Portugal       Class 2
Europe              Romania        Class 1
Europe              Serbia         Class 2
Europe              Slovakia       Class 1
Europe              Slowenia       Class 1
Europe              Spain          Class 2
Europe              Sweden         Class 1
Europe              Switzerland    Class 1
Europe              Turkey         Class 2
North America       Canada         Class 2
North America       United States  Class 2
South America       Argentina      Class 2
South America       Brazil         Class 2
South America       Colombia       Class 2
South America       Falkland Is.   Class 2
=================== ============== =======


Raster Maps
...........

- Switzerland, Digital Glider Map provided by the `Federal Office of Topography
  swisstopo <https://www.swisstopo.admin.ch/en/digital-glider-map>`__
- Switzerland, Digital Aeronautical Chart ICAO provided by the `Federal Office
  of Topography swisstopo
  <https://www.swisstopo.admin.ch/en/digital-aeronautical-chart-icao>`__


Base Maps
.........

Our base maps are generated from `Open Streetmap
<https://www.openstreetmap.org>`_ data. 


Terrain maps
............

Our terrain maps are derived from the `Terrain Tiles Open Dataset on Amazon AWS
<https://registry.opendata.aws/terrain-tiles/>`_. The underlying data sources
are a mix of:

- 3DEP (formerly NED and NED Topobathy) in the United States, 10 meters outside
  of Alaska, 3 meter in select land and territorial water areas
- ArcticDEM strips of 5 meter mosaics across all of the land north of 60°
  latitude, including Alaska, Canada, Greenland, Iceland, Norway, Russia, and
  Sweden
- CDEM (Canadian Digital Elevation Model) in Canada, with variable spatial
  resolution (from 20-400 meters) depending on the latitude.
- data.gov.uk, 2 meters over most of the United Kingdom
- data.gv.at, 10 meters over Austria
- ETOPO1 for ocean bathymetry, 1 arc-minute resolution globally
- EUDEM in most of Europe at 30 meter resolution, including Albania, Austria,
  Belgium, Bosnia and Herzegovina, Bulgaria, Croatia, Cyprus, Czechia, Denmark,
  Estonia, Finland, France, Germany, Greece, Hungary, Iceland, Ireland, Italy,
  Kosovo, Latvia, Liechtenstein, Lithuania, Luxembourg, Macedonia, Malta,
  Montenegro, Netherlands, Norway, Poland, Portugal, Romania, Serbia, Slovakia,
  Slovenia, Spain, Sweden, Switzerland, and United Kingdom
- Geoscience Australia's DEM of Australia, 5 meters around coastal regions in
  South Australia, Victoria, and Northern Territory
- GMTED globally, coarser resolutions at 7.5", 15", and 30" in land areas
- INEGI's continental relief in Mexico
- Kartverket's Digital Terrain Model, 10 meters over Norway
- LINZ, 8 meters over New Zealand
- SRTM globally except high latitudes, 30 meters (90 meters nominal quality) in
  land areas

Attributions
^^^^^^^^^^^^

* ArcticDEM terrain data DEM(s) were created from DigitalGlobe, Inc., imagery
  and funded under National Science Foundation awards 1043681, 1559691, and
  1542736;
* Australia terrain data © Commonwealth of Australia (Geoscience Australia)
  2017;
* Austria terrain data © offene Daten Österreichs – Digitales Geländemodell
  (DGM) Österreich;
* Canada terrain data contains information licensed under the Open Government
  Licence – Canada;
* Europe terrain data produced using Copernicus data and information funded by
  the European Union - EU-DEM layers;
* Global ETOPO1 terrain data U.S. National Oceanic and Atmospheric
  Administration
* Mexico terrain data source: INEGI, Continental relief, 2016;
* New Zealand terrain data Copyright 2011 Crown copyright (c) Land Information
  New Zealand and the New Zealand Government (All rights reserved);
* Norway terrain data © Kartverket;
* United Kingdom terrain data © Environment Agency copyright and/or database
  right 2015. All rights reserved;
* United States 3DEP (formerly NED) and global GMTED2010 and SRTM terrain data
  courtesy of the U.S. Geological Survey.



METAR/TAF
---------

METAR and TAF data is provided by the `Aviation Weather Center
<https://www.aviationweather.gov/>`_, an office of the United States Department
of Commerce.


NOTAMs
------

NOTAMs are provided by the `Federal Aviation Administration
<https://api.faa.gov>`_, an office of the United States Department of
Transportation.
